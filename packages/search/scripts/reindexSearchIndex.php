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
* $Id: reindexSearchIndex.php,v 1.5 2012/08/30 00:59:15 ewang Exp $
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

// THE INDEXING STATUS SHOULD BE TURNED ON
$sm =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('search_manager');
if (!$sm->attr('indexing')) {
	echo "\nBEFORE RUNNING THE SCRIPT, PLEASE CHECK THAT THE INDEXING STATUS IS TURNED ON\n";
	echo 'Note: You can change this option from the backend "System Management" > "Search Manager" > "Details"'."\n\n";
	exit();
}

$root_user =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user);
$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);

// ask for the ids to reindex
echo 'Enter the #IDs of the root nodes to reindex (space separated) or press ENTER to reindex the whole system: ';
$root_node_ids = trim(fgets(STDIN, 4094));

// if the user chooses to reindex the whole system
if (empty($root_node_ids)) {
	$root_node_ids = array(1);
} else {
	$root_node_ids = explode(' ', $root_node_ids);
}

/**
 * if the user typed 1 in the list anywhere,
 * we only need to do one reindex
 */
if (in_array(1, $root_node_ids)) {
	$root_node_ids = array(1);
}

foreach ($root_node_ids as $root_node_id) {
	if (!is_numeric($root_node_id)) {
		trigger_error("Asset ID " . $root_node_id . " is not valid. Supply an integer", E_USER_WARNING);
		continue;
	}

	// if the asset does not exist
	if (($root_node_id > 1) && !$GLOBALS['SQ_SYSTEM']->am->assetExists($root_node_id)) {
		trigger_error("The asset #".$root_node_id." is not valid", E_USER_WARNING);
		continue;
	}

	// confirm the action
	if ($root_node_id == 1) {
		echo "Do you want to reindex the whole system (yes/no) ";
	} else {
		echo "Do you want to reindex the root node #".$root_node_id. " (yes/no) ";
	}

	// if the answer is different from yes, skip this asset.
	$process = trim(fgets(STDIN, 4094));
	if (strcmp(strtolower($process), 'yes') !== 0) {
		echo "Skipping .. \n";
		continue;
	}

	echo 'Start Reindexing'."\n";
	$all_contextids = array_keys($GLOBALS['SQ_SYSTEM']->getAllContexts());
	$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();

	$vars = Array(
				'root_assetid'       => $root_node_id,
			);

	foreach ($all_contextids as $contextid) {
		$vars['contextid'] = $contextid;
		$hh->freestyleHipo('hipo_job_reindex', $vars, SQ_PACKAGES_PATH.'/search/hipo_jobs');
	}
	echo 'Finished'."\n";
}

$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
?>
