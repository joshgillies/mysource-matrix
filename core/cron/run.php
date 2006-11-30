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
* $Id: run.php,v 1.20 2006/11/30 04:14:36 bcaldwell Exp $
*
*/

/**
* Index File
*
* The one file through which everything runs
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.20 $
* @package MySource_Matrix
*/

if (isset($_SERVER['argv'][1])) {
	define('SQ_SYSTEM_ROOT', $_SERVER['argv'][1]);
} else {
	define('SQ_SYSTEM_ROOT', dirname(dirname(dirname(__FILE__))));
}

require_once SQ_SYSTEM_ROOT.'/core/include/init.inc';
ini_set('memory_limit', SQ_CONF_CRON_MEMORY_LIMIT.'M');

$root_user =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
if (is_null($root_user)) {
	trigger_localised_error('CRON0023', E_USER_ERROR);
}

if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	trigger_localised_error('CRON0022', E_USER_ERROR);
}

$cron_mgr =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
if (is_null($cron_mgr)) {
	trigger_localised_error('CRON0021', E_USER_ERROR);
}

if (!empty($_SERVER['argv'][1]) && $_SERVER['argv'][1] == 'RESET_RUNNING') {
	if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($cron_mgr->id, 'attributes', 0, TRUE)) {
		trigger_localised_error('CRON0016', E_USER_ERROR, $cron_mgr->name);
	}
	if (!$cron_mgr->setAttrValue('running', FALSE)) {
		trigger_localised_error('CRON0010', E_USER_ERROR);
	}
	$GLOBALS['SQ_SYSTEM']->am->releaseLock($cron_mgr->id, 'attributes');
}

$cron_mgr->run();
exit(0);


/**
* Make a proper unix path (ripped from remote content)
*
* Works similar to php's realpath(), but doesn't rely on a file system
* Given a string representing a path, it tries to remove the relative references
* in short: given "/this/dir/../another/file.php" will produce "/this/another/file.php"
**
* NOTE: paths that try to jump outside the root will produce possibly erroneous result
* eg.: "/root/dir/../../../another/file.php" will become "/another/file.php"
* this behaviour is similar to how browsers treat relative paths
*
* @param string	$path	path that needs shortening
*
* @return string
* @access public
*/
function make_proper_path($path='')
{
	if (empty($path))
		return '';

	$root = '';
	$path_components = explode('/',$path);

	if (empty($path_components[0])) {
		$root = '/';
		unset($path_components[0]);
	}

	$stack = Array();

	foreach ($path_components as $component) {
		switch ($component) {
			case '..':
				if (!empty($stack)) array_pop($stack);
			break;

			case '.':
			case '':
				continue;
			break;

			default:
				array_push($stack, $component);
		}
	}

	$new_path = implode('/', $stack);

	return $root.$new_path;

}//end make_proper_path()


?>
