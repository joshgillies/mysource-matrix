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
* $Id: permission_change.php,v 1.4 2011/08/08 04:42:30 akarelia Exp $
*
*/
if (ini_get('memory_limit') != '-1') ini_set('memory_limit', '-1');
error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	trigger_error("You need to supply the path to the System Root as the first argument\n", E_USER_ERROR);
}
define('SQ_SYSTEM_ROOT', $SYSTEM_ROOT);
require_once $SYSTEM_ROOT.'/core/include/init.inc';

// Other options
$ROOT_NODE = (isset($_SERVER['argv'][2])) ? $_SERVER['argv'][2] : '';
if (empty($ROOT_NODE)) {
	trigger_error("You need to supply a root node as the second argument\n", E_USER_ERROR);
}
$USER = (isset($_SERVER['argv'][3])) ? $_SERVER['argv'][3] : '';
if (empty($USER)) {
	trigger_error("You need to supply a user as the third argument\n", E_USER_ERROR);
}
$available_permissions = Array('read'=>SQ_PERMISSION_READ, 'write'=>SQ_PERMISSION_WRITE, 'admin'=>SQ_PERMISSION_ADMIN);
$PERMISSION = (isset($_SERVER['argv'][4])) ? $_SERVER['argv'][4] : '';
if (empty($PERMISSION) || !array_key_exists($PERMISSION, $available_permissions)) {
	trigger_error("You need to supply a permission as the fourth argument\n", E_USER_ERROR);
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
