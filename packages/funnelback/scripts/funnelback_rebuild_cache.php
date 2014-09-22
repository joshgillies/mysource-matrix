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
* $Id: funnelback_rebuild_cache.php,v 1.8 2012/08/30 00:58:44 ewang Exp $
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

$root_user =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user);
$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);

$ROOT_NODE_ID = (isset($_SERVER['argv'][2])) ? $_SERVER['argv'][2] : '1';

// if the asset does not exists
if (($ROOT_NODE_ID > 1) && !$GLOBALS['SQ_SYSTEM']->am->assetExists($ROOT_NODE_ID)) {
	trigger_error("The asset #".$ROOT_NODE_ID." is not VALID\n", E_USER_ERROR);
}

// THE INDEXING STATUS SHOULD BE TURNED ON
$fm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('funnelback_manager');
if (is_null($fm)) {
	trigger_localised_error('FNB0020', translate('Unable to load Funnelback Manager, Funnelback Manager is not installed'), E_USER_WARNING)
	exit();
}//end if
if (!$fm->attr('indexing')) {
	echo "\n\nBEFORE RUNNING THE SCRIPT, PLEASE CHECK THAT THE INDEXING STATUS IS TURNED ON\n";
	echo 'Note: You can change this option from the backend "System Management" > "Funnelback Manager" > "Details"'."\n\n";
	exit();
}

// Check for a lock file
if (file_exists(SQ_TEMP_PATH.'/funnelback.rebuilder')) {
	trigger_localised_error('FNB0019', translate('Lockfile found. Scripts cannot be run concurrently.'), E_USER_WARNING)
	exit();
}//end if

// Create a lock file
touch(SQ_TEMP_PATH.'/funnelback.rebuilder');

// Start rebuilding
$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
$vars = Array('root_assetid'=> $ROOT_NODE_ID);
$errors = $hh->freestyleHipo('hipo_job_funnelback_rebuild_cache', $vars, SQ_PACKAGES_PATH.'/funnelback/hipo_jobs');
if (!empty($errors)) {
	echo 'Funnelback Cache Rebuild FAILED'."\n";
	foreach ($errors as $error) {
		$line = array_get_index($error, 'message', '');
		if (!empty($line)) {
			echo $line."\n";
		}//end if
	}//end foreach
}//end if

// Remove if finished
if (file_exists(SQ_TEMP_PATH.'/funnelback.rebuilder')) {
	unlink(SQ_TEMP_PATH.'/funnelback.rebuilder');
}//end if

$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
?>
