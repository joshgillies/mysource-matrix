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
* $Id: run.php,v 1.8 2003/11/26 00:51:08 gsherwood Exp $
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
	if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($cron_mgr->id, 'attributes', 0, true)) {
		trigger_error('Unable to acquire lock of "'.$cron_mgr->name.'", aborting run', E_USER_ERROR);
	}
	if (!$cron_mgr->setAttrValue('running', false)) {
		trigger_error('SET RUNNING FAILED', E_USER_ERROR);
	}
	$GLOBALS['SQ_SYSTEM']->am->releaseLock($cron_mgr->id, 'attributes');
}

$cron_mgr->run();

?>
