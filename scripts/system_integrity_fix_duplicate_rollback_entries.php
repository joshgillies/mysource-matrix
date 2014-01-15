<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: system_integrity_fix_duplicate_rollback_entries.php,v 1.4.2.3 2013/10/17 23:33:05 cupreti Exp $
*
*/


/**
* Reports and attempts to fix the rollback tables with overlapping entries with with same "eff_to" in the overlapping entries set
* The script will remove all duplicate overlapping entries except the one with oldest "eff_from" date
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: 1.4.2.3 $
* @package MySource_Matrix
*/
error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

require_once 'Console/Getopt.php';

$shortopt = 's:';
$longopt = Array('fix-table=', 'show-records');

$args = Console_Getopt::readPHPArgv();
array_shift($args);
$options = Console_Getopt::getopt($args, $shortopt, $longopt);

if ($options instanceof PEAR_Error) {
	usage();
}
if (empty($options[0])) usage();

// Get root folder and include the Matrix init file, first of all
$SYSTEM_ROOT = '';
$fix_overlapping_entries = FALSE;
$show_overlapping_entries = FALSE;
$fix_table = '';
foreach ($options[0] as $index => $option) {
	if ($option[0] == 's' && !empty($option[1]) && empty($SYSTEM_ROOT)) {
		$SYSTEM_ROOT = $option[1];		
	} else if ($option[0] == 's') {
		usage();
	}
	
	if ($option[0] == '--fix-table') {
		 if (empty($option[1])) {
			usage();
		}		
		$fix_overlapping_entries = TRUE;
		$fix_table = strtolower($option[1]);
		$fix_table = preg_replace('|^sq_(rb_)?|', '', $fix_table);
	}
	if ($option[0] == '--show-records') {
		$show_overlapping_entries = TRUE;
	}
}//end foreach

if (empty($SYSTEM_ROOT)) {
	echo "ERROR: You need to supply the path to the System Root as the first argument\n";
	usage();
	exit();
}

if (!is_dir($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT.'/core/include/init.inc')) {
	echo "ERROR: Path provided doesn't point to a Matrix installation's System Root. Please provide correct path and try again.\n";
	usage();
	exit();
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';

if ($fix_overlapping_entries) {
	if ($fix_table == 'all') {
		echo "\nIMPORTANT: You have selected the option to fix all the duplicate rollback entries from the Rollback table.";	
	} else {
		echo "\nIMPORTANT: You have selected the option to fix the duplicate rollback entries from the Rollback table \"sq_rb_$fix_table\"";	
	}
	echo "\nThis will remove all duplicate entries in the table(s) except the one with oldest \"eff_from\" date in the duplicate entries set.";
	echo "\nBacking up db is recommended before running this script.";
    echo "\nAre you sure you want to proceed (Y/n)?\n";

    $yes_no = rtrim(fgets(STDIN, 4094));
    if ($yes_no != 'Y') {
        echo "\nScript aborted. \n";
        exit;
    }
}

require_once SQ_INCLUDE_PATH.'/rollback_management.inc';
require_once SQ_LIB_PATH.'/db_install/db_install.inc';

// Get all the rollback tables
$rollback_tables = rollback_table_info();
foreach($rollback_tables as $table => $table_info) {
	
	if (!isset($table_info['primary_key'])) {
		continue;
	}
	$keys = implode(',', $table_info['primary_key']);	
	if (isset($table_info['unique_key'])) {
		$keys = $keys.','.implode(',', $table_info['unique_key']);
	}
	if (empty($keys)) {
		continue;
	}
	
	echo "Checking table sq_rb_".$table.' ... ';
	$sql = 'SELECT '.$keys.', sq_eff_to, count(*) as occ FROM sq_rb_'.$table.' GROUP BY '.$keys.', sq_eff_to HAVING count(*) > 1';
	$results = MatrixDAL::executeSqlAssoc($sql);
	if (empty($results)) {
		echo '[ OK ]';
	} else {
		echo count($results).' set(s) of overlapping entries found. ';
		if ($show_overlapping_entries) {
			pre_echo($results);
		}		
		echo '[ NOT OK ]';
		if ($fix_overlapping_entries && ($fix_table == $table || $fix_table == 'all')) {
			echo "\nFixing the rollback table sq_rb_".$table;
			$table_fixed = fix_rollback_table($table, $table_info, $results);
			echo ' [ '.($table_fixed ? 'FIXED' : 'FAILED').' ]';
			echo "\n";
		}		
	}
	echo "\n";
	
}//end foreach

// END OF MAIN PROGRAM


/**
* Fix rollback table
*
* @param $table_name
* @param $table_info
* @param $entries
*
* @return boolean
* @access public
*/
function fix_rollback_table($table_name, $table_info, $entries=Array())
{
	$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
	$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

	$success = TRUE;	
	switch($table_name) {

		// TODO: If individual tables are required to be handled specifically then it should be done here

		default:
			// For all rollback tables, delete all the duplicates expect one with min(eff_from) i.e. oldest one
			$keys = $table_info['primary_key'];
			if (isset($table_info['unique_key'])) {
				$keys = array_unique(array_merge($keys, $table_info['unique_key']));
			}
			if (empty($keys)) {
				echo "\nPrimary key fields not found the rollback table ".$table_name;
				$success = FALSE;
				break;
			}

			$where_sql = '';
			foreach($keys as $key) {
				$where_sql .= $key.' = :'.$key.' AND ';
			}
			$where_sql .= 'sq_eff_to = :sq_eff_to' ;
			// Get the oldest 'eff_from'
			$sub_sql = 'SELECT min(sq_eff_from) FROM sq_rb_'.$table_name.' WHERE '.$where_sql;
			$sql = 'DELETE FROM sq_rb_'.$table_name.' WHERE '.$where_sql.' AND sq_eff_from > ('.$sub_sql.')';

			foreach($entries as $entry) {
				try {
					$update_sql = MatrixDAL::preparePdoQuery($sql);
					foreach($entry as $row_name => $row_val) {
						if ($row_name == 'occ') continue;
						MatrixDAL::bindValueToPdo($update_sql, $row_name , $row_val);
					}
					$execute = MatrixDAL::executePdoAssoc($update_sql);
				} catch (Exception $e) {
					$success = FALSE;
					break;
				}
			}
		
		break;		
		
	}//end switch

	$GLOBALS['SQ_SYSTEM']->doTransaction($success ? 'COMMIT' : 'ROLLBACK');
	$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

	if (!$success) {
		echo "\nUnexpected error occured while updating database: ".$e->getMessage();
	}
	
	return $success;

}//end fix_rollback_table()


/**
* Returns the table info of the rollback database tables
*
* @return array
* @access public
*/
function rollback_table_info()
{
	$table_info = Array();

	$packages_installed = $GLOBALS['SQ_SYSTEM']->getInstalledPackages();

	if (empty($packages_installed)) return Array();

	foreach ($packages_installed as $package_array) {
		if ($package_array['code_name'] == '__core__') {
			$table_file = SQ_CORE_PACKAGE_PATH.'/tables.xml';
		} else {
			$table_file = SQ_PACKAGES_PATH.'/'.$package_array['code_name'].'/tables.xml';
		}

		if (!file_exists($table_file)) continue;

		try {
			$root = simplexml_load_string(file_get_contents($table_file), 'SimpleXMLElement', LIBXML_NOCDATA);
		} catch (Exception $e) {
			throw new Exception('Unable to parse table file : '.$table_file.' due to the following error: '.$e->getMessage());
		}//end try catch

		foreach ($root->children() as $child) {
			$first_child_name = $child->getName();
			break;
		}//end foreach
		if ($root->getName() != 'schema' || $first_child_name != 'tables') {
			trigger_error('Invalid table schema for file "'.$table_file.'"', E_USER_ERROR);
		}

		$table_root = $child;

		foreach ($table_root->children() as $table_child) {
			if ((string) $table_child->attributes()->require_rollback) {
				$table_name = (string) $table_child->attributes()->name;
				if (isset($table_child->keys) && (count($table_child->keys->children()) > 0)) {
						foreach ($table_child->keys->children() as $table_key) {
							$index_db_type = $table_key->attributes()->db;
							if (!is_null($index_db_type) && ((string)$index_db_type != MatrixDAL::getDbType())) {
								continue;
							}

							// work out the columns in this key
							$key_columns = Array();
							foreach ($table_key->column as $table_key_column) {
								$col_name = (string)$table_key_column->attributes()->name;
								$key_columns[] = $col_name;

								// cache the primary key columns for this table
								if ($table_key->getName() == 'primary_key') {
									$table_info[$table_name]['primary_key'][] = $col_name;
								}
								if ($table_key->getName() == 'unique_key') {
									$table_info[$table_name]['unique_key'][] = $col_name;
								}
							}//end foreach
						}//end foreach
					}//end if				
			}//end if
		}//end foreach
	}

	return $table_info;

}//end rollback_table_info()


/**
* Prints the usage for this script and exits
*
* @return void
* @access public
*/
function usage()
{
	echo "\nUSAGE: system_integrity_fix_duplicate_rollback_entries.php -s <system_root> [--show-records] [--fix-table=<table_name>]\n".
		"--show-records  	Show overlapping rollback db records\n".
		"--fix-table 		Rollback table to fix for the overlapping entries. Enter \"all\" for all rollback tables\n";
	exit();

}//end usage()

?>
