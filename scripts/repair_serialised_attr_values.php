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
*/

/**
* Script to fix the broken serialsed data in the asset attribute table
* If the script cannot fixed the broken data, it will leave the data as it is and will
* report the entry as "CANNOT FIX".
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: $
* @package MySource_Matrix
*/

error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) {
    trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

$SYSTEM_ROOT = getCLIArg('system');
if (!$SYSTEM_ROOT) {
	echo "ERROR: You need to supply the path to the System Root\n";
	print_usage();
	exit(1);
}
if (!is_dir($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT.'/core/include/init.inc')) {
	echo "ERROR: Path provided doesn't point to a Matrix installation's System Root. Please provide correct path and try again.\n";
	print_usage();
	exit(1);
}

$FIX_DB = getCLIArg('fix-db');
if ($FIX_DB) {
    echo "\nIMPORTANT: You running the script in 'fix db' mode. Are you sure you want to proceed (Y/N)? \n";
    $yes_no = rtrim(fgets(STDIN, 4094));
    if (strtolower($yes_no) != 'y') {
        echo "\nScript aborted. \n";
        exit;
    }
}

$INCLUDE_ROLLBACK = getCLIArg('include-rollback');

if (ini_get('memory_limit') != '-1') ini_set('memory_limit', '-1');
require_once $SYSTEM_ROOT.'/core/include/init.inc';

 // Matrix attribute types with serialised value
 $serialsed_attrs = Array(
						'option_list',
						'email_format',
						'parameter_map',
						'serialise',
						'http_request',
						'oauth',
					);

$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
if ($FIX_DB) {
	$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
}

$sql = "SELECT attrid FROM sq_ast_attr WHERE type IN ('".implode("','", $serialsed_attrs)."')";
$serialise_attrids = array_keys(MatrixDAL::executeSqlGrouped($sql));

// Select the entries in the asset attribute value tables with serialsed data
$invalid_count = 0;
$fixed_count = 0;
foreach(Array('sq_', 'sq_rb_') as $table_prefix) {
	echo "\nLooking into '".$table_prefix."' tables .";
	foreach(array_chunk($serialise_attrids, 50) as $attrids_chunk) {
		$sql = "SELECT assetid, attrid, contextid, custom_val FROM ".$table_prefix."ast_attr_val WHERE attrid IN ('".implode("','", $attrids_chunk)."')";
		$entries = MatrixDAL::executeSqlGrouped($sql);
		foreach($entries as $assetid => $attr_values) {
			echo '.';
			foreach($attr_values as $data) {
				$attrid = isset($data[0]) ? $data[0] : '';
				$contextid = isset($data[1]) ? $data[1] : '';
				$value = isset($data[2]) ? $data[2] : '';
				if (empty($value) || empty($attrid)) {
					continue;
				}//end if

				if (!valid_serialised_value($value)) {
					echo "\nInvalid serialised value assetid #".$assetid." attrid #".$attrid." contextid #".$contextid;
					$invalid_count++;

					// Try fix the serialsed value
					$value = fix_bad_serialsed_value($value);
					if (!$value) {
						echo "\n CANNOT BE FIXED!";
					} else {
						$fixed_count++;
					}

					if ($FIX_DB && $value) {
						// Update the db with the fixed serialsed data
						try {
							$sql = 'UPDATE sq_ast_attr_val SET custom_val=:value WHERE assetid=:assetid AND attrid=:attrid AND contextid=:contextid';
							$update_sql = MatrixDAL::preparePdoQuery($sql);
							MatrixDAL::bindValueToPdo($update_sql, 'value', $value);
							MatrixDAL::bindValueToPdo($update_sql, 'assetid', $assetid);
							MatrixDAL::bindValueToPdo($update_sql, 'attrid', $attrid);
							MatrixDAL::bindValueToPdo($update_sql, 'contextid', $contextid);
							$execute = MatrixDAL::executePdoAssoc($update_sql);
						} catch (Exception $e) {
							echo "Unexpected error occured while updating database: ".$e->getMessage();
							echo "\nNo database changes were made";
							$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
							exit(1);
						}
					}
				}//end if invalid serialsed data

			}//end foreach asset attr values
		}//end foreach result entries
	}//end foreach attrs chuck

	if (!$INCLUDE_ROLLBACK) {
		// Next interation looks into rollback table, which we dont need here
		break;
	}
}//end foreach table types

if ($FIX_DB) {
	$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
}
$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

echo "\n";
echo "\nTotal number of bad serialsed value entries: ".$invalid_count;
echo "\nTotal number of entries ".($FIX_DB ? "fixed: " : "that can be fixed: ").$fixed_count;

if (!$FIX_DB) {
    echo "\nNOTE: The script ran in \"report only\" mode. No db changes were made.\n";
}
echo "\n\n";


/**
* Try fix the broken serialsed value
* If successful return the fixed serialsed value otherwise return FALSE
*
* @return string|boolean
*/
function fix_bad_serialsed_value($value)
{
	$value = preg_replace_callback('!s:(\d+):([\\\\]?"[\\\\]?"|[\\\\]?"((.*?)[^\\\\])[\\\\]?");!sm', "serialize_fix_callback", $value);

	return valid_serialised_value($value) ? $value : FALSE;

}//end fix_bad_serialsed_value()


/*
* Call backfunction for fix_bad_serialsed_value()
*
* @return string
*/
function serialize_fix_callback($match)
{
	return isset($match[3]) ? 's:'.strlen($match[3]).':"'.$match[3].'";' : $match[0];

}//end serialize_fix_callback()


function valid_serialised_value($value)
{
	return ($value==serialize(FALSE) || @unserialize($value) !== FALSE);

}//end valid_serialised_value()


/**
* Get CLI Argument
*
* @params $arg string argument
* @return string/boolean
*/
function getCLIArg($arg)
{
	return (count($match = array_values(preg_grep("/--" . $arg . "(\=(.*)|)/i",$_SERVER['argv']))) > 0 === TRUE) ? ((preg_match('/--(.*)=(.*)/',$match[0],$reg)) ? $reg[2] : true) : false;

}//end getCLIArg()


/**
 * Print the usage of this script
 *
 * @return void
 */
function print_usage()
{
	echo "\nThis script attempts to fix the broken serialsed data in the asset attribute value table.";
	echo "\nIf script is not able to fix the broken serialsed data, the script will report is as 'CANNOT FIX'.\n\n";

    echo "Usage: php ".basename(__FILE__)." --system=<SYSTEM_ROOT> [--fix-db] [--include-rollback]\n\n";
    echo "\t<SYSTEM_ROOT>       : The root directory of Matrix system.\n";
    echo "\t[--fix-db]          : If ommitted the script will run in report mode without fixing the db.\n";
    echo "\t[--include-rollback] : Whether include rollback asset attribute value table.\n";
	echo "\n";

}//end print_usage()

?>
