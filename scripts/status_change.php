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
* $Id: status_change.php,v 1.1 2010/03/29 23:27:03 bpearson Exp $
*
*/
ini_set('mem_limit', '-1');
error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

$available_status_codes = Array(1,2,4,8,16,32,64,128,256);

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	trigger_error("You need to supply the path to the System Root as the first argument\n", E_USER_ERROR);
}
$ROOT_NODE = (isset($_SERVER['argv'][2])) ? $_SERVER['argv'][2] : '';
if (empty($ROOT_NODE)) {
	trigger_error("You need to supply a root node as the second argument\n", E_USER_ERROR);
}
$STATUS = (isset($_SERVER['argv'][3])) ? $_SERVER['argv'][3] : '';
if (empty($STATUS) && in_array($STATUS, $available_status_codes)) {
	trigger_error("You need to supply a status code as the third argument\n", E_USER_ERROR);
}
$CHILDREN = (isset($_SERVER['argv'][4]) && $_SERVER['argv'][4] == 'y') ? FALSE : TRUE;
require_once $SYSTEM_ROOT.'/core/include/init.inc';

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
