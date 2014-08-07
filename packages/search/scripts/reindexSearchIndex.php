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
* $Id: reindexSearchIndex.php,v 1.6 2012/09/20 02:38:59 cupreti Exp $
*
*/

/**
 * Purpose:
 * Matrix Search: Reindex the system
 *
 * @author  Chiranjivi Upreti <cupreti@squiz.com.au>
 * @version $Revision: 1.6 $
 * @package MySource_Matrix
 */


error_reporting(E_ALL);
if (ini_get('memory_limit') != '-1') ini_set('memory_limit', '-1');

if ((php_sapi_name() != 'cli')) {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';

// Check for valid system root
$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT)) {
    echo "ERROR: You need to supply the path to the System Root as the first argument\n";
    printUsage();
    exit(1);
}

if (!is_dir($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT.'/core/include/init.inc')) {
    echo "ERROR: Path provided doesn't point to a Matrix installation's System Root. Please provide correct path and try again.\n";
    printUsage();
    exit(1);
}

// File to communicate between the child and parent process
define('SYNC_FILE', $SYSTEM_ROOT.'/data/temp/reindex_search.assetids');

$ROOT_NODES = (isset($_SERVER['argv'][2])) ? $_SERVER['argv'][2] : 1;
$root_node_ids = array_unique(explode(',', trim($ROOT_NODES, ' ,')));
if (in_array(1, $root_node_ids)) {
	echo "Do you want to reindex the whole system (yes/no) ";
	$process = trim(fgets(STDIN, 4094));
	if (trim(strtolower($process)) != 'yes') {
		echo "Aborting script.\n";
		exit();	
	}
	
	// Since main root node is selected, we just need to reindex one root node
	$root_node_ids = array(1);
}

// Use the batch size of 100 assets by default
$BATCH_SIZE = (isset($_SERVER['argv'][3])) ? $_SERVER['argv'][3] : 100;
if ($BATCH_SIZE <= 0) {
	$BATCH_SIZE = 100;
}

$pid = fork();
if (!$pid) {

	// NOTE: This seemingly ridiculousness allows us to workaround Oracle, forking and CLOBs
	// if a query is executed that returns more than 1 LOB before a fork occurs,
	// the Oracle DB connection will be lost inside the fork.

	require_once $SYSTEM_ROOT.'/core/include/init.inc';

	$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
	$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user);
	
	// The index should be turned on in the system
	$sm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('search_manager');
	if (!$sm->attr('indexing')) {
		echo "Search indexing is not turned on.\n\n";
		file_put_contents(SYNC_FILE, serialize(Array('assetids' => Array(), 'contextids' => Array())));		
		exit();
	}
	
	$all_assetids = Array();	
	foreach ($root_node_ids as $root_node_id) {
		
		// if the asset does not exist
		if (!$GLOBALS['SQ_SYSTEM']->am->getAssetInfo($root_node_id)) {
			echo "\nWARNING: The asset #".$root_node_id." is not valid assetid\n";
			continue;
		}		
		// Get the assets under the root node
		$all_assetids = array_merge($all_assetids, array_keys($GLOBALS['SQ_SYSTEM']->am->getChildren($root_node_id)));
		$all_assetids[] = $root_node_id;
		
	}//end foreach
	
	$all_assetids = array_unique($all_assetids);
	$asset_count = count($all_assetids);
	
	// Chunk the assets into the given batch size
	$start_index = 0;
	$batched_assetids = Array();
	while($start_index < $asset_count) {
		$batched_assetids[] = array_slice($all_assetids, $start_index, $BATCH_SIZE);
		$start_index += $BATCH_SIZE;
	}//end while	
	$GLOBALS['SQ_SYSTEM']->restoreCurrentUser();
	
	// Also get the context ids
	$contextids = array_keys($GLOBALS['SQ_SYSTEM']->getAllContexts());
	
	// Write the data to the sync file for the parent process to read from
	file_put_contents(SYNC_FILE, serialize(Array('assetids' => $batched_assetids, 'contextids' => $contextids)));
	
	exit;
	
}//end child process

// Get the assetid data from the child process
$data = unserialize(file_get_contents(SYNC_FILE));
$batched_assetids = $data['assetids'];
$all_contextids = $data['contextids'];

echo "Batch size: ".$BATCH_SIZE."\n";
echo "Total assets to reindex: ".(count($batched_assetids, COUNT_RECURSIVE) - count($batched_assetids))."\n\n";

if (!empty($batched_assetids)) {
	echo "Reindexing ...";
	// Reindex each batch in the seperate process
	foreach($batched_assetids as $assetids) {

		$pid = fork();		
		if (!$pid) {
				
			require_once $SYSTEM_ROOT.'/core/include/init.inc';
			
			$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
			$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user);
				
			$sm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('search_manager');	
				
			// Child process
			foreach($all_contextids as $contextid) {
				$GLOBALS['SQ_SYSTEM']->changeContext($contextid);
					
				foreach($assetids as $assetid) {
					$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
					if (is_null($asset)) {
						continue;
					}
					$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
					$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
						
					$sm->reindexAsset($asset, Array('all'));
					$sm->reindexAttributes($asset, Array('all'), TRUE);
					$sm->reindexContents($asset, Array('all'), TRUE);
					$sm->reindexMetadata($asset->id, Array('all'));
						 
					$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
						
					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
					 
				}//end foreach
					
				$GLOBALS['SQ_SYSTEM']->restoreContext();
			}//end foreach
				
			$GLOBALS['SQ_SYSTEM']->restoreCurrentUser();
				
			exit;
		}//end child process			
		echo ".";
	}//end foreach
	echo " Done.\n";
}//end if


if (file_exists(SYNC_FILE)) {
	unlink(SYNC_FILE);
}

exit();

// END OF THE MAIN PROGRAM ////////////////////////////////////////////


/*
* Fork child process. The parent process will sleep until the child
* exits
*
* @return string
* @access public
*/
function fork()
{
	$child_pid = pcntl_fork();
	
	switch ($child_pid) {
		case -1:
			trigger_error("Forking failed!");
			return null;
		break;
		case 0: // child process
			return $child_pid;
		break;
		default : // parent process
			$status = null;
			pcntl_waitpid(-1, $status);
			return $child_pid;
		break;
	}
}//end fork()


/**
 * Print the usage of this script
 *
 */
function printUsage() 
{	
    echo "Usage: php ".basename(__FILE__)." <SYSTEM_ROOT> [ROOT_NODES] [BATCH_SIZE]\n\n";
    echo "\t<SYSTEM_ROOT>:\t The root directory of the Matrix system.\n";
    echo "\t[ROOT_NODES]:\t Comma seperated root node assetids to reindex. If ommited, whole system will be reindexed.\n";
    echo "\t[BATCH_SIZE]:\t Number of assets to include in a batch. Default size is 100\n\n";

}//end print_usage()

?>