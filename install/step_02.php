<?php
/**
* Install Step 2
*
* Purpose
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Resolve
*/
error_reporting(E_ALL);
$SYSTEM_ROOT = '';
// from cmd line
if ((php_sapi_name() == 'cli')) {
	if (isset($_SERVER['argv'][1])) $SYSTEM_ROOT = $_SERVER['argv'][1];
	$err_msg = "You need to supply the path to the Resolve System as the first argument\n";

} else { 
	if (isset($_GET['SYSTEM_ROOT'])) $SYSTEM_ROOT = $_GET['SYSTEM_ROOT'];
	$err_msg = '
	<div style="background-color: red; color: white; font-weight: bold;">
		You need to supply the path to the Resolve System as a query string variable called SYSTEM_ROOT
	</div>
	';
}

if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	echo $err_msg;
	exit(1);
}

// Let the asset manager know we are installing
$GLOBALS['SQ_INSTALL'] = true;

require_once 'XML/Tree.php';
require_once $SYSTEM_ROOT.'/core/include/init.inc';
$db = &$GLOBALS['SQ_SYSTEM']->db;

$cached_table_columns = Array();

// check if we have already created the database by looking for the
// table column cache file
if (!is_file(SQ_DATA_PATH.'/private/db/table_columns.inc')) {

	// Create database tables
	$input = new XML_Tree(SQ_SYSTEM_ROOT.'/core/db/tables.xml');
	$root  = &$input->getTreeFromFile();
	if (PEAR::isError($root)) {
		trigger_error($root->getMessage()."\n".$root->getUserInfo(), E_USER_ERROR);
		return;
	}

	$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

	for($i = 0; $i < count($root->children); $i++) {
		$table = &$root->children[$i];
		$table_name = $table->attributes['name'];
		$require_rollback = (($table->attributes['require_rollback'] == 1) ? true : false);

		// buffer the columns so we can use them in the rollback table
		// as well, if needed
		ob_start();

		// loop through the table columns
		$table_cols = &$table->children[0]->children;
		for($j = 0; $j < count($table_cols); $j++) {
			$table_column = &$table_cols[$j];
			$column_name = $table_column->attributes['name'];
			$allow_null = (($table_column->attributes['allow_null'] == 1) ? true : false);
			$type = null;
			$default = null;

			$cached_table_columns[$table_name][] = $column_name;

			for($k = 0; $k < count($table_column->children); $k++) {
				$column_var = &$table_column->children[$k];
				
				switch (strtolower($column_var->name)) {
					case 'type' :
						// set the type of the column if it hasnt already been
						// set in a variation (this is the default column type)
						if (is_null($type)) $type = $column_var->content;
						break;
					case 'type_variations' :
						// check for varitions of the column type for his database
						for($l = 0; $l < count($column_var->children); $l++) {
							$variation = &$column_var->children[$l];
							if ($variation->name == $db->phptype) {
								$type = $variation->content;
								break;
							}
						}
						break;
					case 'default' :
						if (trim($column_var->content) != '') $default = $column_var->content;
						break;
					default :
						continue;
						break;
				}

			}// end for

			echo "$column_name $type".((!$allow_null) ? ' NOT NULL' : '').((!is_null($default)) ? " DEFAULT $default" : '').',';

		}// end for

		$table_columns_string = ob_get_contents();
		ob_end_clean();

		// work out the keys
		$primary_key = '';
		$rollback_primary_key = '';
		$other_keys = Array();

		$table_keys = &$table->children[1]->children;
		for($j = 0; $j < count($table_keys); $j++) {
			$table_key = &$table_keys[$j];

			// work out the columns in this key
			$key_columns = Array();
			for($k = 0; $k < count($table_key->children); $k++) {
				$col_name = $table_key->children[$k]->attributes['name'];
				$key_columns[] = $col_name;
			}

			switch (strtolower($table_key->name)) {
				case 'primary_key' :
					// a primary key for the table
					$primary_key = 'PRIMARY KEY('.implode(',',$key_columns).')';
					$rollback_primary_key = 'PRIMARY KEY(effective_from, '.implode(', ',$key_columns).')';
					break;
				case 'unique_key' :
					$other_keys[] = 'UNIQUE('.implode(', ',$key_columns).')';
					break;
				default :
					continue;
					break;
			}

		}// end for


		//// NORMAL TABLE DEFINITION ////
		ob_start();
		echo 'CREATE TABLE '.SQ_TABLE_PREFIX.$table_name.' (';
		echo $table_columns_string;

		echo $primary_key;
		if (!empty($other_keys)) {
			echo ',';
			echo implode(',', $other_keys);
		}
		echo ');';
		$table_sql = ob_get_contents();
		ob_end_clean();

		$result = $db->query($table_sql);
		if (DB::isError($result)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
			exit();
		}


		//// ROLLBACK TABLE DEFINITION ////
		if ($require_rollback) {
			// type variations
			switch ($db->phptype) {
				case 'mysql' :
					$rollback_column_type = 'DATETIME';
				break;
				default :
					$rollback_column_type = 'TIMESTAMP';
				break;
			}

			ob_start();
			echo 'CREATE TABLE '.SQ_TABLE_ROLLBACK_PREFIX.$table_name.' (';
			echo "effective_from $rollback_column_type NOT NULL,";
			echo "effective_to   $rollback_column_type,";
			echo $table_columns_string;

			echo $rollback_primary_key;
			if (!empty($other_keys)) {
				echo ',';
				echo implode(',', $other_keys);
			}
			echo ');';
			$table_sql = ob_get_contents();
			ob_end_clean();

			$result = $db->query($table_sql);
			if (DB::isError($result)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
				exit();
			}
		}


		//// TABLE INDEXES ////

		// check for any indexes that need creating
		$table_indexes = &$table->children[2]->children;
		for($j = 0; $j < count($table_indexes); $j++) {
			$table_index = &$table_indexes[$j];
			
			// work out the columns in this index
			for($k = 0; $k < count($table_index->children); $k++) {
				$index_col_name = $table_index->children[$k]->attributes['name'];
			}

			$index_sql = 'CREATE INDEX '.SQ_TABLE_PREFIX.$table_name.'_'.$index_col_name.' ON '.SQ_TABLE_PREFIX.$table_name.' ('.$index_col_name.');';
			$result = $db->query($index_sql);
			if (DB::isError($result)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
				exit();
			}

			if ($require_rollback) {
				$index_sql = str_replace(SQ_TABLE_PREFIX, SQ_TABLE_ROLLBACK_PREFIX, $index_sql);
				$result = $db->query($index_sql);
				if (DB::isError($result)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
					exit();
				}
			}
		}

	}// end for

	pre_echo("DATABASE TABLE CREATION COMPLETE");

	// Create any necessary sequences
	$db->createSequence(SQ_TABLE_PREFIX.'sequence_asset');
	$db->createSequence(SQ_TABLE_PREFIX.'sequence_asset_link');
	$db->createSequence(SQ_TABLE_PREFIX.'sequence_asset_attribute');
	$db->createSequence(SQ_TABLE_PREFIX.'sequence_asset_url');
	$db->createSequence(SQ_TABLE_PREFIX.'sequence_asset_permission');
	$db->createSequence(SQ_TABLE_PREFIX.'sequence_internal_message');

	pre_echo("DATABASE SEQUENCE CREATION COMPLETE");

}//end if table column cache file does not exist

// if we didnt just create the database tables, loop through and work out the
// columns in each of our database tables
if (empty($cached_table_columns)) {
	$input = new XML_Tree(SQ_SYSTEM_ROOT.'/core/db/tables.xml');
	$root  = &$input->getTreeFromFile();
	if (PEAR::isError($root)) {
		trigger_error($root->getMessage()."\n".$root->getUserInfo(), E_USER_ERROR);
		$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		exit();
	}

	for($i = 0; $i < count($root->children); $i++) {
		$table = &$root->children[$i];
		$table_name = $table->attributes['name'];
		$table_cols = &$table->children[0]->children;
		for($j = 0; $j < count($table->children[0]->children); $j++) {
			$cached_table_columns[$table_name][] = $table_cols[$j]->attributes['name'];
		}
	}
}


$cached_table_columns_string = '';
ob_start();

echo "<?php\n";
echo '$tables = Array('."\n";
foreach ($cached_table_columns as $table_name => $columns) {
	echo "\t'$table_name' => Array(\n";
	foreach ($columns as $column_name) {
		echo "\t\t'$column_name',\n";
	}
	echo "\t),\n";
}
echo ");\n";
echo '?>';

$cached_table_columns_string = ob_get_contents();
ob_end_clean();


// write a new cache file with all the table names and their columns
require_once SQ_FUDGE_PATH.'/general/file_system.inc';
if (!string_to_file($cached_table_columns_string, SQ_DATA_PATH.'/private/db/table_columns.inc')) {
	$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
	trigger_error('Failed writing database table column cache file', E_USER_ERROR);
	exit();
}
pre_echo("DATABASE TABLE COLUMN CACHING COMPLETE");

// its all good
$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
$GLOBALS['SQ_INSTALL'] = false;

?>