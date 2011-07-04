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
* $Id: clear_cron_deadlock.php,v 1.1 2011/07/04 08:09:34 cupreti Exp $
*
*/

/**
* Clear a cron deadlock on a matrix instance via command line
*
* @author  Matthew Spurrier <mspurrier@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix
*/


error_reporting(E_ALL);

/**
* Returns script usage information
*
* @return void
*/
function printHelp() 
{
	print "Usage: clear_cron_deadlock.php SYSTEM_ROOT [--fix]\r\n\r\n";
}


if ((php_sapi_name() != 'cli')) trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	printHelp();
	exit();
}

$ACTION = (isset($_SERVER['argv'][2])) ? $_SERVER['argv'][2] : '';
require_once $SYSTEM_ROOT.'/core/include/init.inc';
$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	trigger_error("Failed login in as root user\n", E_USER_ERROR);
}

$cronManager = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
if ($cronManager->attr('running') && (int) $cronManager->attr('run_check') >= (int) $cronManager->attr('warn_after_num_run_checks')) {
	if ($ACTION == "--fix") {
		$GLOBALS['SQ_SYSTEM']->am->acquireLock($cronManager->id,'attributes');
		$cronManager->setAttrValue('running', FALSE);
		$cronManager->setAttrValue('run_check', 0);
		$cronManager->saveAttributes();
		$GLOBALS['SQ_SYSTEM']->am->releaseLock($cronManager->id,'attributes');
		echo "Deadlock Cleared\r\n";
	} else {
		echo "Deadlock Detected\r\n";
	}
} else {
	echo "No Deadlock Detected\r\n";
}

?>
