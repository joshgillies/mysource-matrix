<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: step_02.php,v 1.51 2004/09/28 23:04:38 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/

/**
* Install Step 2
*
* Purpose
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
* @subpackage install
*/
ini_set('memory_limit', -1);
error_reporting(E_ALL);
$SYSTEM_ROOT = '';
// from cmd line
if ((php_sapi_name() == 'cli')) {
	if (isset($_SERVER['argv'][1])) $SYSTEM_ROOT = $_SERVER['argv'][1];
	$err_msg = "You need to supply the path to the System Root as the first argument\n";

} else { 
	if (isset($_GET['SYSTEM_ROOT'])) $SYSTEM_ROOT = $_GET['SYSTEM_ROOT'];
	$err_msg = '
	<div style="background-color: red; color: white; font-weight: bold;">
		You need to supply the path to the System Root as a query string variable called SYSTEM_ROOT
	</div>
	';
}

if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	trigger_error($err_msg, E_USER_ERROR);
}

// Let the asset manager know we are installing
$GLOBALS['SQ_INSTALL'] = true;

require_once $SYSTEM_ROOT.'/core/include/init.inc';
require_once 'XML/Tree.php';
$db = &$GLOBALS['SQ_SYSTEM']->db;

// Re-generate the Config to make sure that we get any new defines that may have been issued
require_once SQ_INCLUDE_PATH.'/system_config.inc';
$cfg = new System_Config();
$cfg->save(Array(), false);


$cached_table_columns = Array();

$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

// check if we have already created the database by looking for the
// table column cache file
if (!is_file(SQ_DATA_PATH.'/private/db/table_columns.inc')) {

	// create database tables
	$input = new XML_Tree(SQ_SYSTEM_ROOT.'/core/db/tables.xml');
	$root  = &$input->getTreeFromFile();
	if (PEAR::isError($root)) {
		trigger_error($root->getMessage()."\n".$root->getUserInfo(), E_USER_ERROR);
		return;
	}

	for ($i = 0; $i < count($root->children); $i++) {
		$table = &$root->children[$i];
		$table_name = $table->attributes['name'];
		$require_rollback = (($table->attributes['require_rollback'] == 1) ? true : false);

		// buffer the columns so we can use them in the rollback table as well, if needed
		ob_start();
			// loop through the table columns
			$table_cols = &$table->children[0]->children;
			for ($j = 0; $j < count($table_cols); $j++) {
				$table_column = &$table_cols[$j];
				$column_name = $table_column->attributes['name'];
				$allow_null = (($table_column->attributes['allow_null'] == 1) ? true : false);
				$type = null;
				$default = null;
	
				$cached_table_columns[$table_name]['columns'][] = $column_name;
	
				for ($k = 0; $k < count($table_column->children); $k++) {
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
					}//end switch
	
				}//end for
	
				echo "$column_name $type".((!$allow_null) ? ' NOT NULL' : '').((!is_null($default)) ? " DEFAULT $default" : '').',';
	
			}//end for
	
			$table_columns_string = ob_get_contents();
		ob_end_clean();

		// work out the keys
		$primary_key = '';
		$rollback_primary_key = '';
		$other_keys = Array();
		$other_rollback_keys = Array();

		$table_keys = &$table->children[1]->children;
		for ($j = 0; $j < count($table_keys); $j++) {
			$table_key = &$table_keys[$j];

			// work out the columns in this key
			$key_columns = Array();
			for ($k = 0; $k < count($table_key->children); $k++) {
				$col_name = $table_key->children[$k]->attributes['name'];
				$key_columns[] = $col_name;
				
				// cache the primary key columns for this table
				if ($table_key->name == 'primary_key') {
					$cached_table_columns[$table_name]['primary_key'][] = $col_name;
				}
			}

			switch (strtolower($table_key->name)) {
				case 'primary_key' :
					// a primary key for the table
					$primary_key = 'PRIMARY KEY('.implode(',',$key_columns).')';
					$rollback_primary_key = 'PRIMARY KEY('.SQ_TABLE_PREFIX.'effective_from, '.implode(', ',$key_columns).')';
				break;
				case 'unique_key' :
					$other_keys[] = 'UNIQUE('.implode(', ',$key_columns).')';
					$other_rollback_keys[] = 'UNIQUE('.SQ_TABLE_PREFIX.'effective_from, '.implode(', ',$key_columns).')';
				break;
				default :
					continue;
				break;
			}//end switch

		}//end for


//--        NORMAL TABLE DEFINITION        --//


		ob_start();
			echo 'CREATE TABLE '.SQ_TABLE_PREFIX.$table_name.' (';
			echo $table_columns_string;
	
			echo $primary_key;
			if (!empty($other_keys)) {
				echo ',';
				echo implode(',', $other_keys);
			}
			echo ')';
			$table_sql = ob_get_contents();
		ob_end_clean();

		$result = $db->query($table_sql);
		assert_valid_db_result($result);


//--        ROLLBACK TABLE DEFINITION        --//


		if ($require_rollback) {
			ob_start();
				echo 'CREATE TABLE '.SQ_TABLE_ROLLBACK_PREFIX.$table_name.' (';
				echo SQ_TABLE_PREFIX.'effective_from TIMESTAMP NOT NULL,';
				echo SQ_TABLE_PREFIX.'effective_to   TIMESTAMP,';
				echo $table_columns_string;
	
				echo $rollback_primary_key;
				if (!empty($other_rollback_keys)) {
					echo ',';
					echo implode(',', $other_rollback_keys);
				}
				echo ')';
				$table_sql = ob_get_contents();
			ob_end_clean();

			$result = $db->query($table_sql);
			assert_valid_db_result($result);
		}


//--        TABLE INDEXES        --//


		// check for any indexes that need creating
		$table_indexes = &$table->children[2]->children;
		for ($j = 0; $j < count($table_indexes); $j++) {
			$table_index = &$table_indexes[$j];
			
			// work out the columns in this index
			for ($k = 0; $k < count($table_index->children); $k++) {
				$index_col_name = $table_index->children[$k]->attributes['name'];
			}

			$index_sql = 'CREATE INDEX '.SQ_TABLE_PREFIX.$table_name.'_'.$index_col_name.' ON '.SQ_TABLE_PREFIX.$table_name.' ('.$index_col_name.')';
			$result = $db->query($index_sql);
			assert_valid_db_result($result);

			if ($require_rollback) {
				$index_sql = str_replace(SQ_TABLE_PREFIX, SQ_TABLE_ROLLBACK_PREFIX, $index_sql);
				$result = $db->query($index_sql);
				assert_valid_db_result($result);
			}
		}

	}//end for

	pre_echo('DATABASE TABLE CREATION COMPLETE');

	// create any necessary sequences
	$db->createSequence(SQ_TABLE_PREFIX.'sequence_asset');
	$db->createSequence(SQ_TABLE_PREFIX.'sequence_asset_link');
	$db->createSequence(SQ_TABLE_PREFIX.'sequence_asset_attribute');
	$db->createSequence(SQ_TABLE_PREFIX.'sequence_asset_url');
	$db->createSequence(SQ_TABLE_PREFIX.'sequence_internal_message');

	pre_echo('DATABASE SEQUENCE CREATION COMPLETE');


//--        PGSQL GRANT_ACCESS()        --//


	// if this is PostgreSQL we need to do a couple of other things for the secondary user
	if ($db->phptype == 'pgsql') {

		$function_sql = "
		CREATE OR REPLACE FUNCTION ".SQ_TABLE_PREFIX."grant_access(character varying) RETURNS TEXT
		AS '
		DECLARE
			user_name ALIAS FOR $1;
			table RECORD;
			tablename TEXT;
		BEGIN
			FOR table IN SELECT c.relname AS name FROM pg_catalog.pg_class c LEFT JOIN pg_catalog.pg_namespace n ON n.oid = c.relnamespace WHERE c.relkind IN (''r'',''v'',''S'','''') AND n.nspname NOT IN (''pg_catalog'', ''pg_toast'') AND pg_catalog.pg_table_is_visible(c.oid) LOOP
				tablename=table.name;
				RAISE NOTICE ''tablename is %'', tablename;
				EXECUTE ''GRANT ALL ON '' || quote_ident(tablename) || '' TO '' || quote_ident(user_name::text);
			END LOOP;
			RETURN ''access granted.'';
		END;
		'
		LANGUAGE plpgsql;
		";
		$result = $db->query($function_sql);
		if (DB::isError($result)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
		}

		$primary_dsn   = DB::parseDSN(SQ_CONF_DB_DSN);
		$secondary_dsn = DB::parseDSN(SQ_CONF_DB2_DSN);

		if ($primary_dsn['username'] != $secondary_dsn['username']) {
			$grant_sql = 'SELECT '.SQ_TABLE_PREFIX.'grant_access('.$db->quote($secondary_dsn['username']).')';
			$result = $db->query($grant_sql);
			if (DB::isError($result)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
			}
		}// end if
		pre_echo("PGSQL SECONDARY USER PERMISSIONS FIXED");

	}// end if

	require_once SQ_FUDGE_PATH.'/file_versioning/file_versioning.inc';
	if (!File_Versioning::initRepository(SQ_DATA_PATH.'/file_repository', $db)) {
		trigger_error('Unable to initialise File Versioning Repository', E_USER_ERROR);
	}

}//end if table column cache file does not exist


//--        CACHE TABLES        --//


// if we didnt just create the database tables, loop through and work out the
// columns in each of our database tables
if (empty($cached_table_columns)) {
	$input = new XML_Tree(SQ_SYSTEM_ROOT.'/core/db/tables.xml');
	$root  = &$input->getTreeFromFile();
	if (PEAR::isError($root)) {
		$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		trigger_error($root->getMessage()."\n".$root->getUserInfo(), E_USER_ERROR);
	}

	for ($i = 0; $i < count($root->children); $i++) {
		$table = &$root->children[$i];
		$table_name = $table->attributes['name'];
		$table_cols = &$table->children[0]->children;
		for ($j = 0; $j < count($table->children[0]->children); $j++) {
			$cached_table_columns[$table_name]['columns'][] = $table_cols[$j]->attributes['name'];
		}
		$table_keys = &$table->children[1]->children;
		for ($j = 0; $j < count($table_keys); $j++) {
			$table_key = &$table_keys[$j];
			if ($table_key->name == 'primary_key') {
				for ($k = 0; $k < count($table_key->children); $k++) {
					$col_name = $table_key->children[$k]->attributes['name'];
					$cached_table_columns[$table_name]['primary_key'][] = $col_name;
				}
			} else if ($table_key->name == 'unique_key') {
				for ($k = 0; $k < count($table_key->children); $k++) {
					$col_name = $table_key->children[$k]->attributes['name'];
					$cached_table_columns[$table_name]['unique_key'][] = $col_name;
				}
			}
		}
	}
}

// write a new cache file with all the table names and their columns
require_once SQ_FUDGE_PATH.'/general/file_system.inc';
$cached_table_columns_string = '<'.'?php $tables = '.var_export($cached_table_columns, true).'; ?'.'>';
if (!string_to_file($cached_table_columns_string, SQ_DATA_PATH.'/private/db/table_columns.inc')) {
	$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
	trigger_error('Failed writing database table column cache file', E_USER_ERROR);
}
pre_echo('DATABASE TABLE COLUMN CACHING COMPLETE');

// its all good
$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
$GLOBALS['SQ_INSTALL'] = false;

?>