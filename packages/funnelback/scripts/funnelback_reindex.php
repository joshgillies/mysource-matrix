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
* $Id: funnelback_reindex.php,v 1.1 2010/03/01 04:47:37 bpearson Exp $
*
*/

error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

// Get the arguments
$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	trigger_error("You need to supply the path to the System Root as the first argument\n", E_USER_ERROR);
}
$root_collection = (isset($_SERVER['argv'][2])) ? $_SERVER['argv'][2] : '';

define('SQ_SYSTEM_ROOT', $SYSTEM_ROOT);
require_once $SYSTEM_ROOT.'/core/include/init.inc';

$root_user =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user);
$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);

// THE INDEXING STATUS SHOULD BE TURNED ON
$fm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('funnelback_manager');
if (is_null($fm)) {
	trigger_localised_error('FNB0020', E_USER_WARNING);
	exit();
}//end if
if (!$fm->attr('indexing')) {
	echo "\n\nBEFORE RUNNING THE SCRIPT, PLEASE CHECK THAT THE INDEXING STATUS IS TURNED ON\n";
	echo 'Note: You can change this option from the backend "System Management" > "Funnelback Manager" > "Details"'."\n\n";
	exit();
}

// confirm the action
if (empty($root_collection)) {
	echo "DO YOU WANT TO REINDEX THE WHOLE SYSTEM (yes/no)\n";
} else {
	echo "DO YOU WANT TO REINDEX THE COLLECTION ".$root_collection. " (yes/no)\n";
}

// if the answer is different from yes exit
$process = trim(fgets(STDIN, 4094));
if (strcmp(strtolower($process), 'yes') !== 0) {
	echo 'EXIT'."\n";
	exit();
}

echo 'START REINDEXING'."\n";
$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
$vars = Array(
			'collections'=> ((empty($root_collection)) ? Array() : $root_collection),
		);
$errors = $hh->freestyleHipo('hipo_job_funnelback_reindex', $vars, SQ_PACKAGES_PATH.'/funnelback/hipo_jobs');
if (empty($errors)) {
	echo 'FINISHED'."\n";
} else {
	echo 'FAILED'."\n";
}//end if

$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
?>
