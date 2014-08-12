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
* $Id: clear_cron_deadlock.php,v 1.4 2012/08/30 01:04:53 ewang Exp $
*
*/

/**
* Clear a cron deadlock on a matrix instance via command line
*
* @author  Matthew Spurrier <mspurrier@squiz.net>
* @version $Revision: 1.4 $
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
	print "Usage: clearCronDeadlock.php SYSTEM_ROOT [--reset] [--force]\r\n\r\n";
};

if ((php_sapi_name() != 'cli')) trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT)) {
	echo "ERROR: You need to supply the path to the System Root as the first argument\n";
	printHelp();
	exit();
}

if (!is_dir($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT.'/core/include/init.inc')) {
	echo "ERROR: Path provided doesn't point to a Matrix installation's System Root. Please provide correct path and try again.\n";
	printHelp();
	exit();
}

$RESET = (isset($_SERVER['argv'][2]) && $_SERVER['argv'][2] == '--reset') ? true : false;
$FORCE = (isset($_SERVER['argv'][3]) && $_SERVER['argv'][3] == '--force') ? true : false;

require_once $SYSTEM_ROOT.'/core/include/init.inc';

$root_user=&$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	echo "ERROR: Failed login in as root user\n";
	exit();
}

$cronManager=&$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
$cronRunning=$cronManager->attr('running');
$cronRunCheck=(int)$cronManager->attr('run_check');
$cronWarnLimit=(int)$cronManager->attr('warn_after_num_run_checks');
$cr=($cronRunning) ? 'Yes' : 'No';

print 'Cron Running: ' . $cr . ', Run Checks: ' . $cronRunCheck . '/' . $cronWarnLimit . "\r\n";
if ($RESET) {
        if (($cronRunning && $cronRunCheck >= $cronWarnLimit) || $FORCE) {
                $GLOBALS['SQ_SYSTEM']->am->acquireLock($cronManager->id,'attributes');
                $cronManager->setAttrValue('running', FALSE);
                $cronManager->setAttrValue('run_check', 0);
                $cronManager->saveAttributes();
                $GLOBALS['SQ_SYSTEM']->am->releaseLock($cronManager->id,'attributes');
                if ($FORCE) {
                        echo "Manual Reset Complete\r\n";
                } else {
                        echo "Deadlock Cleared\r\n";
                };
        } else {
                echo "No Deadlock Detected\r\n";
        };
} else {
        if ($cronRunning && $cronRunCheck >= $cronWarnLimit) {
                echo "Deadlock Detected\r\n";
        } else {
                echo "No Deadlock Detected, To force manual reset use --reset --force\r\n";
        };
};
?>
