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
* $Id: give_funnelback_permission.php,v 1.2 2012/08/30 00:58:44 ewang Exp $
*
*/
ini_set('mem_limit', '-1');
error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) {
	trigger_error("You can only run this script from the command line", E_USER_ERROR);
}

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	trigger_error("You need to supply the path to the System Root as the first argument", E_USER_ERROR);
	exit();
}
define('SQ_SYSTEM_ROOT', $SYSTEM_ROOT);
require_once $SYSTEM_ROOT.'/core/include/init.inc';

// Other options
$ROOT_NODE = (isset($_SERVER['argv'][2])) ? $_SERVER['argv'][2] : '';
if (empty($ROOT_NODE)) {
	trigger_error("You need to supply a root node as the second argument", E_USER_ERROR);
	exit();
}

$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');

$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user);
$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);

// Load the funnelback manager
$fm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('funnelback_manager');
$user = NULL;
$user_id = $fm->attr('user');
if (!empty($user_id)) {
	$user = $GLOBALS['SQ_SYSTEM']->am->getAsset($user_id, '', TRUE);
}//end if
if (is_null($user)) {
	trigger_error("No user asset specified in Matrix", E_USER_ERROR);
	exit();
}//end if

// Start the process
echo 'APPLYING PERMISSIONS FOR '.$user->name."\n";
$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
$vars = Array(
			'permission_changes' => Array(
										Array(
											'permission' 		=> SQ_PERMISSION_WRITE,
											'granted'	 		=> '1',
											'assetids'			=> Array($ROOT_NODE),
											'userid'	 		=> $user->id,
											'previous_access'	=> NULL,
											'cascades'			=> TRUE,
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

$GLOBALS['SQ_SYSTEM']->restoreCurrentUser();
$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
?>
