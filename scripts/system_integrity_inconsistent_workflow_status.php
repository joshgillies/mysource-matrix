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
 * $Id: system_integrity_inconsistent_workflow_status.php,v 1.1.6.1 2013/07/04 05:24:16 cupreti Exp $
 */

/**
 *    This script finds out assets in status which is inconsistent with its workflow state(when FIXME error occurs)
 *    Specifically, it detects following cases:
 *    Asset is in status of Live, Safe Editting, Approved To Go Live, Under Construction but it has a running workflow.
 *    Asset is in status of Pending Approval,  but it doesn't have a running workflow.
 *    Status like Safe Editting Pending Approval is OK with or without a running workflow, so it's not concern of this script.
 * 
 *    Usage: php ./system_integrity_inconsistent_workflow_status.php --system=<SYSTEM_ROOT> [--email=<EMAIL_ADDRESS>] [--fix]
 *    <SYSTEM_ROOT> : The root directory of Matrix system.
 *    <EMAIL_ADDRESS> : If specified, an email with report summary will be sent to the provided email address instead of printing to screen.
 *    [--fix]    : If specified, fix the inconsistence instead of outputing the report. 
 *    Assets in status that shouldn't have running workflow will get running worflow removed. Assets in status that should have running wokflow will be set to under construction.
 *
 *
 * @author  Edison Wang <ewang@squiz.com.au>
 * @version $Revision: 1.1.6.1 $
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
require_once $SYSTEM_ROOT.'/core/include/init.inc';
if (ini_get('memory_limit') != '-1') ini_set('memory_limit', '-1');

$toFix = getCLIArg('fix');
$toEmail = getCLIArg('email');

// asset is Live, Safe Editting, Approved To Go Live, Under Construction but it has running workflow by mistake.
$sql = 'SELECT a.assetid, a.status FROM sq_ast_wflow w, sq_ast a WHERE a.assetid = w.assetid AND w.wflow is not null and a.status in (2, 8, 16, 64)';
try {
    $query = MatrixDAL::preparePdoQuery($sql);
    $assets_should_not_have_workflow = MatrixDAL::executePdoAssoc($query);
} catch (Exception $e) {
    echo "ERROR: ".$e->getMessage()."\n";
    exit(1);
}  
    
    
 // asset is Pending Approval but it does't have running workflow by mistake.
$sql = 'SELECT a.assetid, a.status FROM sq_ast a LEFT JOIN sq_ast_wflow w ON a.assetid = w.assetid WHERE w.wflow is null and a.status in (4)';
try {
    $query = MatrixDAL::preparePdoQuery($sql);
    $assets_should_have_workflow = MatrixDAL::executePdoAssoc($query);
} catch (Exception $e) {
    echo "ERROR: ".$e->getMessage()."\n";
    exit(1);
}  

// nothing to do
 if(empty($assets_should_have_workflow) && empty($assets_should_not_have_workflow)) {
     exit();
 }

$workflows_to_update = Array();
$assets_to_update = Array();
    
ob_start();
if(!empty($assets_should_not_have_workflow)) {
    echo "\nFollowing assets are found to be in running workflow state but not in workflow status\n";
    foreach ($assets_should_not_have_workflow as $data) {
		echo "#".$data['assetid']." has status ".$data['status']."\n";
		$workflows_to_update[$data['assetid']] = MatrixDAL::quote($data['assetid']);
    }
}

if(!empty($assets_should_have_workflow)) {
    echo "\nFollowing assets are found to be in workflow status but not in running workflow state\n";
    foreach ($assets_should_have_workflow as $data) {
		echo "#".$data['assetid']." has status ".$data['status']."\n";
		$assets_to_update[$data['assetid']] = MatrixDAL::quote($data['assetid']);
    }
}
$content = ob_get_contents();
ob_end_clean();
    
    
if($toFix) {
    $GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
	$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

	if (!empty($workflows_to_update)) {
	    $sql ='UPDATE sq_ast_wflow SET wflow = null WHERE assetid IN ('.implode(',', $workflows_to_update).')';
    	$query = MatrixDAL::preparePdoQuery($sql);
	    $result = MatrixDAL::execPdoQuery($query);	
    	if($result) {
			echo "\n".$result." asset(s) fixed by removing running workflow state.\n";
	    }
	}

	if (!empty($assets_to_update)) {
	    $sql ='UPDATE sq_ast SET status = 2 WHERE assetid IN ('.implode(',', $assets_to_update).')';
    	$query = MatrixDAL::preparePdoQuery($sql);
	    $result = MatrixDAL::execPdoQuery($query);	
    	if($result) {
			echo "\n".$result." asset(s) fixed by setting status to Under Construction.\n";
    	}
	}

	$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
    $GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

	// Also flush the Deja Vu cache for the updated assets
	$deja_vu = $GLOBALS['SQ_SYSTEM']->getDejaVu();
	if ($deja_vu->enabled()) {
		$assets_to_update = array_keys($assets_to_update);
		foreach($assets_to_update as $assetid) {
			$deja_vu->forget('asset', $assetid);
		}//end foreach
	}//end if

}
else {
    if($toEmail) {
	mail($toEmail, 'Inconsistent Workflow Assets Found', $content, 'From: ' . SQ_CONF_DEFAULT_EMAIL . "\r\n");
    }
    else {
	echo $content;
    }
}
    
/**
 * Get CLI Argument
 * Check to see if the argument is set, if it has a value, return the value
 * otherwise return true if set, or false if not
 *
 * @params $arg string argument
 *
 * @return string/boolean
 * @author Matthew Spurrier
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
    echo "\nThis script finds out assets in status which is inconsistent with its workflow state(when FIXME error occurs).";
    echo "\nSpecifically, it detects following inconsistent cases:";
    echo "\nAsset is in status of Live, Safe Editting, Approved To Go Live, Under Construction but it has a running workflow";
    echo "\nAsset is in status of Pending Approval,  but it doesn't have a running workflow";

    echo "Usage: php ".basename(__FILE__)." --system=<SYSTEM_ROOT> [--email=<EMAIL_ADDRESS>] [--fix]\n\n";
    echo "\t<SYSTEM_ROOT> : The root directory of Matrix system.\n";
    echo "\t<EMAIL_ADDRESS> : If specified, an email with report summary will be sent to the provided email address instead of printing to screen.\n";
    echo "\t<--fix>    : If specified, fix the inconsistence instead of printing or emailing the report .\n\tAssets in status that shouldn't have running workflow will get running worflow removed. Assets in status that should have running wokflow will be set to under construction.\n";

}//end print_usage()



?>

