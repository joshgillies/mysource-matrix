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
* $Id: status_change.php,v 1.6 2012/08/30 01:04:53 ewang Exp $
*
*/
if (ini_get('memory_limit') != '-1') ini_set('memory_limit', '-1');
error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

$available_status_codes = Array(1,2,4,8,16,32,64,128,256);

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

$ROOT_NODE = (isset($_SERVER['argv'][2])) ? $_SERVER['argv'][2] : '';
if (empty($ROOT_NODE)) {
	echo "You need to supply a root node as the second argument\n";
	exit();
}
$STATUS = (isset($_SERVER['argv'][3])) ? $_SERVER['argv'][3] : '';
if (empty($STATUS) || !in_array($STATUS, $available_status_codes)) {
	echo "You need to supply a status code as the third argument\n";
	exit();
}
$CHILDREN = (isset($_SERVER['argv'][4]) && $_SERVER['argv'][4] == 'y') ? FALSE : TRUE;

$root_user =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user);
$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);

echo 'START STATUS CHANGE'."\n";
$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
$vars = Array('assetid' => $ROOT_NODE, 'new_status' => $STATUS, 'dependants_only' => $CHILDREN);
$errors = $hh->freestyleHipo('hipo_job_edit_status', $vars);
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
