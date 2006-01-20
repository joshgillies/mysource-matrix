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
* $Id: run.php,v 1.12 2006/01/20 06:07:22 rong Exp $
*
*/

/**
* Index File
*
* The one file through which everything runs
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.12 $
* @package MySource_Matrix
*/
ini_set('memory_limit', '16M');
ini_set('error_log', dirname(dirname(dirname(__FILE__))).'/cache/error.log');
require_once dirname(dirname(__FILE__)).'/include/init.inc';

$root_user = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
if (is_null($root_user)) {
	trigger_localised_error('CRON0023', E_USER_ERROR);
}

if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	trigger_localised_error('CRON0022', E_USER_ERROR);
}

$cron_mgr = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
if (is_null($cron_mgr)) {
	trigger_localised_error('CRON0021', E_USER_ERROR);
}

if (!empty($_SERVER['argv'][1]) && $_SERVER['argv'][1] == 'RESET_RUNNING') {
	if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($cron_mgr->id, 'attributes', 0, true)) {
		trigger_localised_error('CRON0016', E_USER_ERROR, $cron_mgr->name);
	}
	if (!$cron_mgr->setAttrValue('running', false)) {
		trigger_localised_error('CRON0010', E_USER_ERROR);
	}
	$GLOBALS['SQ_SYSTEM']->am->releaseLock($cron_mgr->id, 'attributes');
}

$cron_mgr->run();

?>
