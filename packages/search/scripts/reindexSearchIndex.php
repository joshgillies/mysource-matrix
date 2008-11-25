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
* $Id: reindexSearchIndex.php,v 1.2 2008/11/25 05:03:13 mbrydon Exp $
*
*/

error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) {
    trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	trigger_error("You need to supply the path to the System Root as the first argument\n", E_USER_ERROR);
}
require_once $SYSTEM_ROOT.'/core/include/init.inc';

$root_user =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user);
$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);


// ask for the id of the root node to reindex
echo 'Enter the #ID of the root node to reindex or press ENTER to reindex the whole system: ';
$root_node_id = (int)trim(fgets(STDIN, 4094));

// if the user chooses to reindex the whole system 
if (empty($root_node_id)) {
	$root_node_id = 1;
}

// if the id entered is not an int (should not complain normally)
if (!is_int($root_node_id)) {
	trigger_error("You need to supply an integer\n", E_USER_ERROR);
}

// if the asset does not exists
if (($root_node_id > 1) && !$GLOBALS['SQ_SYSTEM']->am->assetExists($root_node_id)) {
	trigger_error("The asset #".$root_node_id." is not VALID\n", E_USER_ERROR);
}

// THE INDEXING STATUS SHOULD BE TURNED ON
$sm =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('search_manager');
if (!$sm->attr('indexing')) {	
	echo "\n\nBEFORE RUNNING THE SCRIPT, PLEASE CHECK THAT THE INDEXING STATUS IS TURNED ON\n";
	echo 'Note: You can change this option from the backend "System Management" > "Search Manager" > "Details"'."\n\n";
	exit();
}

// confirm the action
if ($root_node_id == 1) {
	echo "DO YOU WANT TO REINDEX THE WHOLE SYSTEM (yes/no)\n";
} else {
	echo "DO YOU WANT TO REINDEX THE ROOT NODE #".$root_node_id. " (yes/no)\n";
}

// if the answer is different from yes exit
$process = trim(fgets(STDIN, 4094));
if (strcmp(strtolower($process), 'yes') !== 0) {
	echo 'EXIT'."\n";
	exit();
}

echo 'START REINDEXING'."\n";
$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
$vars = Array (
				'root_assetid'       => $root_node_id,
			);
$hh->freestyleHipo('hipo_job_reindex', $vars, SQ_PACKAGES_PATH.'/search/hipo_jobs');
echo 'FINISHED'."\n";

$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
?>
