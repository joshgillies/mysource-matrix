<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: run.php,v 1.5 2003/10/02 05:46:21 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

/**
* Index File
*
* The one file through which everything runs
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/
ini_set('memory_limit', '8M');
ini_set('error_log', dirname(dirname(dirname(__FILE__))).'/cache/error.log');
require_once dirname(dirname(__FILE__)).'/include/init.inc';

$root_user = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
if (is_null($root_user)) {
	trigger_error('Unable to get Root User', E_USER_ERROR);
}

if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	trigger_error('Unable to set root user as the current user', E_USER_ERROR);
}

$cron_mgr = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
if (is_null($cron_mgr)) {
	trigger_error('Unable to get the Cron Manager', E_USER_ERROR);
}

if (!empty($_SERVER['argv'][1]) && $_SERVER['argv'][1] == 'RESET_RUNNING') {
	if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($cron_mgr->id, '', 0, true)) {
		trigger_error('Unable to acquire lock of "'.$cron_mgr->name.'", aborting run', E_USER_ERROR);
	}
	if (!$cron_mgr->setAttrValue('running', false)) {
		trigger_error('SET RUNNING FAILED', E_USER_ERROR);
	}
	$GLOBALS['SQ_SYSTEM']->am->releaseLock($cron_mgr->id);
}

$cron_mgr->run();

?>
