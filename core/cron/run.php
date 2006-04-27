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
* $Id: run.php,v 1.14 2006/04/27 03:13:21 lwright Exp $
*
*/

/**
* Index File
*
* The one file through which everything runs
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.14 $
* @package MySource_Matrix
*/

// We need to work out where we have come from, such as whether it's a symbolically
// linked file (PHP's functions give the resolved link). If the path given to
// PHP is absolute, use that, otherwise tack on the working directory (PWD) to it.
// TODO: this doesn't work if there are any '..' in the passed path
// (On Windows, you probably wouldn't have PWD so you can't use this - but then
// you probably don't have symbolic links either!)
if (!empty($_SERVER['PWD'])) {
	$run_dir = $_SERVER['PWD'];
	$script_path = $_SERVER['argv'][0];
	if ($script_path{0} == '/') {
		// absolute path
		define('SQ_SYSTEM_ROOT', dirname(dirname(dirname(make_proper_path($script_path)))));
	} else {
		// relative path - append the run dir to the script path
		define('SQ_SYSTEM_ROOT', dirname(dirname(dirname(make_proper_path($run_dir.'/'.$script_path)))));
	}
}

ini_set('memory_limit', '16M');
ini_set('error_log', SQ_SYSTEM_ROOT.'/cache/error.log');
require_once SQ_SYSTEM_ROOT.'/core/include/init.inc';

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
	if (!$cron_mgr->setAttrValue('running', TRUE)) {
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
