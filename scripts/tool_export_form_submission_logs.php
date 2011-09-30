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
*/

/**
* Export the submission log of a Form in a specific period.
* Run this script without argument to see its usage.
*
* @author  Anh Ta <ata@squiz.co.uk>
* @version $Revision: 1.1.4.2 $
* @package MySource_Matrix
*/


error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

// Get the list of arguments
$args = $_SERVER['argv'];

// Remove the first argument which is this file name 
array_shift($args);

// Get the Matrix System Root Directory
$MATRIX_ROOT_DIR = array_shift($args);
if (empty($MATRIX_ROOT_DIR) || !is_dir($MATRIX_ROOT_DIR)) {
	print_usage("ERROR: You need to enter the Matrix System Root directory as the first argument.");
}

require $MATRIX_ROOT_DIR.'/core/include/init.inc';

// Set root user as current user
$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user);

// Get the Form asset ID
$form_assetid = array_shift($args);
if (empty($form_assetid)) {
	print_usage("ERROR: You need to enter the Form asset ID as the second argument.");
}

// Get Form asset
$form = $GLOBALS['SQ_SYSTEM']->am->getAsset($form_assetid, 'form_email', TRUE);
if (empty($form)) {
	print_usage("ERROR: Invalid Form asset ID.");
}

// Get from time
$from_time = array_shift($args);
if (empty($from_time)) {
	print_usage("ERROR: You need to enter the From Time as the third argument.");
}

$from_time = strtotime($from_time);
if ($from_time === FALSE) {
	print_usage("ERROR: Invalid From Time.");
}

// Get to time
$to_time = array_shift($args);
if (empty($to_time)) {
	print_usage("ERROR: You need to enter the To Time as the forth argument.");
}

$to_time = strtotime($to_time);
if ($to_time === FALSE) {
	print_usage("ERROR: Invalid To Time.");
}

// Get export file name
$export_file = array_shift($args);

// Create CSV log file
$form_edit_fns = $form->getEditFns();
$csv = $form_edit_fns->createCSVSubmissionLogs($form, $from_time, $to_time);

// If there is export file name specified, set it
if (!is_null($export_file)) {
	$csv->setFilepath($export_file);
}

// Print to screen or save to file
$csv->export(TRUE);

// Restore current user
$GLOBALS['SQ_SYSTEM']->restoreCurrentUser();


/*-------------------------------FUNCTIONS--------------------------------*/


/**
 * Print the usage of this script
 * 
 * @param string $err_msg	The error message to print before the usage
 */
function print_usage($err_msg = '')
{
	echo "$err_msg\n\n";
	echo "Usage:\n";
	echo "\tphp export_form_submission_logs.php MATRIX_ROOT_DIRECTORY FORM_ASSET_ID FROM_TIME T0_TIME [EXPORT_FILE]\n\n";
	echo "\t\tMATRIX_ROOT_DIRECTORY   The Matrix System Root directory.\n";
	echo "\t\tFORM_ASSET_ID           The asset ID of the Form asset. Note that this is not the Custom Form asset but the Form Contents asset under it.\n";
	echo "\t\tFROM_TIME               The start time of a specific period to get Form Submissions from. It can be a specific time string like 'Y-m-d H:i:s' or a relative one like 'yesterday H:i:s'. For more information on the accepted formats, refer at http://uk3.php.net/manual/en/datetime.formats.php\n";
	echo "\t\tT0_TIME                 The end time of a specific period to get Form Submissions from. It has the same format as FROM_TIME.\n";
	echo "\t\tEXPORT_FILE             The file name or file path to write the export file to. If no file name is specified, the CSV export will be printed to the screen.\n";
	
	exit;
	
}//end print_usage()


?>
