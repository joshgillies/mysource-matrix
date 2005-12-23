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
* $Id: pgsql_to_oracle_conversion.php,v 1.1.2.1 2005/12/23 00:24:35 amiller Exp $
*
*/

/**
* PostgreSQL to Oracle Conversion Script
*
* Migrates an existing PostgreSQL-based Matrix database into an Oracle database
*
* @author  Avi Miller <avi.miller@squiz.net>
* @version $Revision: 1.1.2.1 $
* @package MySource_Matrix
* @subpackage scripts
*/

ini_set('memory_limit', -1);
error_reporting(E_ALL);

// Let's setup some datasources
define('PGSQL_DSN', 'pgsql://username:password@unix()/database');
define('ORACLE_DSN', 'oci8://username:password@(
            DESCRIPTION=(
                          ADDRESS_LIST=(
                                        ADDRESS=(PROTOCOL=TCP)
                                        (HOST=host.domain.com)
                                        (PORT=1521))
                        )
                        (CONNECT_DATA=(SID=ORCL)
                        (SERVER=DEDICATED))
            )');

if (php_sapi_name() != 'cli') trigger_error("You can only run this script from the command line\n", E_USER_ERROR);

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	echo "ERROR: You need to supply the path to the source System Root as the first argument\n";
	exit();
}

require_once 'XML/Tree.php';
require_once 'DB.php';
require_once $SYSTEM_ROOT.'/fudge/dev/dev.inc';
require_once $SYSTEM_ROOT.'/core/include/general.inc';

$pg_db  = & DB::connect(PGSQL_DSN);
$oci_db = & DB::connect(ORACLE_DSN);

if (PEAR::isError($pg_db)) dbDie($pg_db); 
if (PEAR::isError($oci_db)) dbDie($oci_db); 

$pg_db->setFetchMode(DB_FETCHMODE_ASSOC);
$oci_db->autoCommit(false);

$info = parse_tables_xml($SYSTEM_ROOT.'/core/assets/tables.xml');



// Drop any Oracle sequences that exist
bam('Dropping Oracle sequences');

	$oci_seqs = $oci_db->getAll('SELECT sequence_name FROM user_sequences');
	foreach($oci_seqs as $key => $value) {
		$del_seqs[] = strtolower(substr($value[0], 0, strlen($value[0]) - 4));
	}

	foreach ($info['sequences'] as $sequence) {
		$sequence_values[$sequence] = $pg_db->nextId('sq_'.$sequence);
	}
	
	if(isset($del_seqs)) {
		foreach ($del_seqs as $sequence) {
			printName('Dropping: '.strtolower($sequence));
			$ok = $oci_db->dropSequence($sequence);
			if (PEAR::isError($ok)) dbDie($ok); 
			printUpdateStatus('OK');
		}
	}

// Drop non primary-key indexes (to improve import performance)
bam('Dropping Oracle Indexes');

	$oci_idx = $oci_db->getAll('SELECT index_name FROM user_indexes');
	foreach($oci_idx as $key => $value) {
		$del_idx[] = strtolower($value[0]);
	}
	
	if (isset($del_idx)) {
		foreach($del_idx as $index) {
			if (substr($index, 0, 3) == 'sq_') {
				printName('Dropping: '.$index);
				$ok = $oci_db->query('DROP INDEX '.$index);
				if (PEAR::isError($ok)) dbDie($ok); 
				printUpdateStatus('OK');
			}//end if
		}//end foreach
	}//end if


// Empty out the Oracle tables
bam('Truncating Oracle Tables');

	foreach ($info['tables'] as $tablename => $table_info) {
	
		printName('Truncating: sq_'.$tablename);
		$ok = $oci_db->query('TRUNCATE TABLE sq_'.$tablename);
		if (PEAR::isError($ok)) dbDie($ok);
		printUpdateStatus('OK');
			
	}//end foreach




// Grab the PostgreSQL data for each table and insert it into Oracle
foreach ($info['tables'] as $tablename => $table_info) {

	bam('Starting table: sq_'.$tablename);
	
		$columns = array_keys($table_info['columns']);
		asort($columns);
		$sql = generateSQL($oci_db, $tablename, $columns);


	printName('Preparing SQL INSERT Query');
	
		$prepared_sql = $oci_db->prepare($sql);
	
	printUpdateStatus('OK');
	
	printName('Grabbing source data');

		$source_data = $pg_db->getAll('select * from sq_'.$tablename);
		
	printUpdateStatus('OK');
	
	printName('Inserting Data ('.count($source_data).' rows)');
	
		foreach($source_data as $key => $data) {
		
			ksort($data);
			$ok = $oci_db->execute($prepared_sql, $data);
			if (PEAR::isError($ok)) {
				bam($data);
				dbDie($ok);
			}
		}

	$oci_db->commit();	
	printUpdateStatus('OK');
	
}//end foreach

// Recreate the indexies in Oracle
bam('Rebuilding Indexes');

foreach($info['tables'] as $tablename => $table_info) {

	if (!empty($table_info['indexes'])) {
		foreach ($table_info['indexes'] as $index_col => $index_info) {
			if (is_null($index_info['db_type']) || $index_info['db_type'] == 'oci8') {

				printName('Creating index sq_'.$tablename.'_'.$index_info['name']);
				$sql = create_index_sql($tablename, $index_col, $index_info['name'], $index_info['type']);
				$ok = $oci_db->query($sql);
				if (PEAR::isError($ok)) dbDie($ok);
				printUpdateStatus('OK');

				if ($table_info['rollback']) {
						printName('Creating index sq_rb_'.$tablename.'_'.$index_info['name']);
						$sql = create_index_sql('rb_'.$tablename, $index_col, $index_info['name'], $index_info['type']);
						$ok = $oci_db->query($sql);
						if (PEAR::isError($ok)) dbDie($ok);
						printUpdateStatus('OK');
				}

			}
		}// end foreach
	}//end if

}

// Recreate the sequences in Oracle
bam('Rebuilding Sequences');

foreach ($info['sequences'] as $sequence) {

	$new_seq_start = $sequence_values[$sequence];
	
	printName('Creating sq_'.$sequence.'_seq ('.$new_seq_start.')');
	$ok = $oci_db->query('CREATE SEQUENCE sq_'.$sequence.'_seq START WITH '.$new_seq_start);
	if (PEAR::isError($ok)) dbDie($ok);
	printUpdateStatus('OK');
}

/**
* Parses the tables xml and returns an array of information specific to Oracle
*
* @param string $xml_file the tables xml file to parse
*
* @return Array()
* @access public
*/
function parse_tables_xml($xml_file)
{
	$input =& new XML_Tree($xml_file);
	$root = &$input->getTreeFromFile();

	if (PEAR::isError($root)) {
		trigger_error($root->getMessage()."\n".$root->getUserInfo(), E_USER_WARNING);
		return false;
	}
	if ($root->name != 'schema' || $root->children[0]->name != 'tables' || $root->children[1]->name != 'sequences') {
		trigger_error($root->getMessage()."\n".$root->getUserInfo(), E_USER_WARNING);
		return false;
	}

	$info = Array();
	$info['tables'] = Array();

	//--        TABLES        --//

	for ($i = 0; $i < count($root->children[0]->children); $i++) {
		$table      = &$root->children[0]->children[$i];
		$table_name = $table->attributes['name'];

		$info['tables'][$table_name] = Array();
		$info['tables'][$table_name]['rollback'] = (($table->attributes['require_rollback'] == 1) ? true : false);
		$table_cols = &$table->children[0]->children;

		//--        TABLE COLUMNS        --//

		$info['tables'][$table_name]['columns'] = Array();

		for ($j = 0; $j < count($table_cols); $j++) {
			$table_column = &$table_cols[$j];
			$column_name  = $table_column->attributes['name'];

			$info['tables'][$table_name]['columns'][$column_name] = Array();
			$info['tables'][$table_name]['columns'][$column_name]['allow_null'] = (($table_column->attributes['allow_null'] == 1) ? true : false);

			//--        TABLE COLUMN VARS        --//

			$type    = null;
			$default = null;

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
						for ($l = 0; $l < count($column_var->children); $l++) {
							$variation = &$column_var->children[$l];
							if ($variation->name == 'oci8') {
								$type = $variation->content;
								break;
							}
						}
					break;
					case 'default' :
						if (trim($column_var->content) != '') {
							$default = $column_var->content;
						}
					break;
					default :
						continue;
					break;
				}
			}
			$info['tables'][$table_name]['columns'][$column_name]['type'] = $type;
			$info['tables'][$table_name]['columns'][$column_name]['default'] = $default;

			//--        KEYS        --//

			$info['tables'][$table_name]['primary_key'] = Array();
			$info['tables'][$table_name]['unique_key'] = Array();

			$table_keys = &$table->children[1]->children;
			for ($jj = 0; $jj < count($table_keys); $jj++) {
				$table_key = &$table_keys[$jj];

				// work out the columns in this key
				$key_columns = Array();
				for ($k = 0; $k < count($table_key->children); $k++) {
					$col_name = $table_key->children[$k]->attributes['name'];
					$key_columns[] = $col_name;

					// cache the primary key columns for this table
					if ($table_key->name == 'primary_key') {
						$info['tables'][$table_name]['primary_key'][] = $col_name;
					}
					if ($table_key->name == 'unique_key') {
						$info['tables'][$table_name]['unique_key'][] = $col_name;
					}
				}
			}
			
			//--        INDEXES        --//

			// check for any indexes that need creating
			$table_indexes = &$table->children[2]->children;
			for ($kk = 0; $kk < count($table_indexes); $kk++) {
				$table_index = &$table_indexes[$kk];

				// work out the columns in this index
				for ($k = 0; $k < count($table_index->children); $k++) {
					$index_col_name = $table_index->children[$k]->attributes['name'];
				}

				// work out the name of the index
				$index_name    = array_get_index($table_index->attributes, 'name', $index_col_name);
				$index_type    = array_get_index($table_index->attributes, 'type', null);
				$index_db_type = array_get_index($table_index->attributes, 'db', null);

				$index_info = Array(
								'name'		=> $index_name,
								'type'		=> $index_type,
								'db_type'	=> $index_db_type,
							 );
				$info['tables'][$table_name]['indexes'][$index_col_name] = $index_info;
			}//end for
			
			
		}
		
	}
	
	for ($i = 0; $i < count($root->children[1]->children); $i++) {
		$sequence = &$root->children[1]->children[$i];
		$sequence_name = $sequence->attributes['name'];
		$info['sequences'][] = $sequence_name;
	}
	
	return $info;

}//end parse_tables_xml()


function generateSQL(&$oci_db, $tablename, $columns)
{
	
	$sql = 'INSERT INTO sq_'.$tablename.' (';
		
	foreach ($columns as $columnname) {
		$sql .= strtoupper($columnname).', ';
	}//end foreach
	
	$sql = substr($sql, 0, strlen($sql) - 2);
	
	$sql .= ') values (';

	for ($i = 0; $i < count($columns); $i++) {
			$sql .= '?, ';
	}//end for
	
	$sql = substr($sql, 0, strlen($sql) - 2);

	$sql .= ')';

	return $sql;

}//end function

function create_index_sql($tablename, $column, $index_name=null, $index_type=null)
{

	if (is_null($index_name)) $index_name = $column;

	$sql = 'CREATE INDEX sq_'.$tablename.'_'.$index_name.' ON sq_'.$tablename;
	if (!empty($index_type)) {
			$sql .= '('.$column.') indextype is '.$index_type;
	} else {
		$sql .= ' ('.$column.')';
	}

	return $sql;
	
}//end create_index_sql()


  ////////////////////////
 //  HELPER FUNCTIONS  //
////////////////////////
function printName($name)
{
	printf ('%s%'.(60 - strlen($name)).'s', $name, '');

}//end printName()


function printUpdateStatus($status)
{
	echo "[ $status ]\n";

}//end printUpdateStatus()

function dbDie(&$db) 
{
	echo 'Standard Message: ' . $db->getMessage() . "\n";
    echo 'Standard Code: ' . $db->getMessage() . "\n";
    echo 'DBMS/User Message: ' . $db->getUserInfo() . "\n";
    echo 'DBMS/Debug Message: ' . $db->getDebugInfo() . "\n";
    exit;

}

?>
