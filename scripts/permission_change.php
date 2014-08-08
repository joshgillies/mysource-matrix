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
* $Id: permission_change.php,v 1.7 2012/08/30 01:04:53 ewang Exp $
*
*/

echo 'This script is deprecated because it has memory issue when used on large amount of assets. Use system_apply_permission.php instead.'."\n";
// ask for the root password for the system
echo 'Are you sure you want to continue? (y/N): ';
$force_denied = rtrim(fgets(STDIN, 4094));
if (strtoupper($force_denied) != 'Y') {
             exit;
}

if (ini_get('memory_limit') != '-1') ini_set('memory_limit', '-1');
error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT)) {
	echo "ERROR: You need to supply the path to the System Root as the first argument\n";
	exit();
}

if (!is_dir($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT.'/core/include/init.inc')) {
	echo "ERROR: Path provided doesn't point to a Matrix installation's System Root. Please provide correct path and try again.\n";
	exit();
}

define('SQ_SYSTEM_ROOT', $SYSTEM_ROOT);
require_once $SYSTEM_ROOT.'/core/include/init.inc';

// Other options
$ROOT_NODE = (isset($_SERVER['argv'][2])) ? $_SERVER['argv'][2] : '';
if (empty($ROOT_NODE)) {
	echo "ERROR: You need to supply a root node as the second argument\n";
	exit();
}
$USER = (isset($_SERVER['argv'][3])) ? $_SERVER['argv'][3] : '';
if (empty($USER)) {
	echo "ERROR: You need to supply a user as the third argument\n";
	exit();
}
$available_permissions = Array('read'=>SQ_PERMISSION_READ, 'write'=>SQ_PERMISSION_WRITE, 'admin'=>SQ_PERMISSION_ADMIN);
$PERMISSION = (isset($_SERVER['argv'][4])) ? $_SERVER['argv'][4] : '';
if (empty($PERMISSION) || !array_key_exists($PERMISSION, $available_permissions)) {
	echo "ERROR: You need to supply a permission as the fourth argument\n";
	exit();
}
$GRANTED  = (isset($_SERVER['argv'][5]) && $_SERVER['argv'][5] == 'y') ? '1' : '0';
$CHILDREN = (isset($_SERVER['argv'][6]) && $_SERVER['argv'][6] == 'y') ? TRUE : FALSE;

// Start the process
$root_user =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user);
$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);

echo 'START PERMISSION CHANGE'."\n";
$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
$vars = Array(
			'permission_changes' => Array(
										Array(
											'permission' 		=> $available_permissions[$PERMISSION],
											'granted'	 		=> $GRANTED,
											'assetids'			=> Array($ROOT_NODE),
											'userid'	 		=> $USER,
											'previous_access'	=> NULL,
											'cascades'			=> $CHILDREN,
										),
									),
		);
$errors = $hh->freestyleHipo('hipo_job_edit_permissions', $vars);
echo 'FINISHED';
if (!empty($errors)) {
	echo "... But with errors!\n";
	foreach ($errors as $error) {
		echo "\t".
		$line = array_get_index($error, 'message', '');
		if (!empty($line)) {
			echo "\t".$line."\n";
		}//end if
	}//end foreach
} else {
	echo "\n";
}//end if

$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
?>
