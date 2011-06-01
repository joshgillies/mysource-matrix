<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: oracle_reset_sequences.php,v 1.2 2008/02/18 05:28:41 lwright Exp $
*
*/

/**
* Rebuilds Oracle Sequences from highest primary key value in the database
*
* @author  Avi Miller <avi.miller@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix
* @subpackage scripts
*/

ini_set('memory_limit', -1);
error_reporting(E_ALL);

if (php_sapi_name() != 'cli') trigger_error("You can only run this script from the command line\n", E_USER_ERROR);

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	echo "ERROR: You need to supply the path to the source System Root as the first argument\n";
	exit();
}

define('SQ_SYSTEM_ROOT', $SYSTEM_ROOT);
define('SQ_LOG_PATH', SQ_SYSTEM_ROOT.'/data/private/logs');

require_once 'DB.php';
require_once $SYSTEM_ROOT.'/data/private/conf/main.inc';
require_once 'XML/Tree.php';
require_once $SYSTEM_ROOT.'/fudge/dev/dev.inc';
require_once $SYSTEM_ROOT.'/core/include/general.inc';

$info = parse_tables_xml($SYSTEM_ROOT.'/core/assets/tables.xml');

$dal_conf = require_once($SYSTEM_ROOT.'/data/private/conf/db.inc');
$oci_db = MatrixDAL::dbConnect($dal_conf['db2'], 'db2');
MatrixDAL::changeDb('db2');

// Drop any Oracle sequences that exist
bam('Dropping Oracle sequences');

	$oci_seqs = $oci_db->getAll('SELECT sequence_name FROM user_sequences');
	foreach($oci_seqs as $key => $value) {
		$del_seqs[] = strtolower(substr($value[0], 0, strlen($value[0]) - 4));
	}

	foreach ($info['sequences'] as $sequence) {
		if ($sequence == 'trig_id') {
			$table = 'trig';
		} else {
			$table = $sequence;
		}

		$key = $info['tables'][$table]['primary_key'][0];
		$sequence_values[$sequence] = MatrixDAL::executeSqlOne('SELECT '.$key.' FROM sq_'.$table.' WHERE rownum = 1 ORDER BY '.$key.' DESC');
	}

	if(isset($del_seqs)) {
		foreach ($del_seqs as $sequence) {
			printName('Dropping: '.strtolower($sequence).'_seq');
			MatrixDAL::executeSql('DROP SEQUENCE '.$sequence.'_seq');
			printUpdateStatus('OK');
		}
	}


// Recreate the sequences in Oracle
bam('Rebuilding Sequences');

foreach ($info['sequences'] as $sequence) {

	$new_seq_start = $sequence_values[$sequence] + 1;

	printName('Creating sq_'.$sequence.'_seq (New start: '.$new_seq_start.')');
	MatrixDAL::executeSql('CREATE SEQUENCE sq_'.$sequence.'_seq START WITH '.$new_seq_start);
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
