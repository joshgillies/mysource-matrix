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
* $Id: move_assets.php,v 1.9 2012/08/30 01:04:53 ewang Exp $
*
*/

/**
* Move all assets of a single type from a specified parent asset to another asset
* args: system-root, from-parent id, to-parent id, asset-type, [link-type]
*
* @author  Matt Keehan <mkeehan@squiz.co.uk>
* @author  Anh Ta <ata@squiz.co.uk>
* @version $Version$ - 2.0
* @package MySource_Matrix
*/

error_reporting(E_ALL);
ini_set('memory_limit', '-1');
if ((php_sapi_name() != 'cli')) trigger_error("You can only run this script from the command line\n", E_USER_ERROR);

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT)) {
	printUsage("ERROR: You need to supply the path to the System Root as the first argument");
}

if (!is_dir($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT.'/core/include/init.inc')) {
	printUsage("ERROR: Path provided doesn't point to a Matrix installation's System Root. Please provide correct path and try again.");
}

//FROM PARENT ID
$FROM_PARENT_ID = (isset($_SERVER['argv'][2])) ? $_SERVER['argv'][2] : '';
if (empty($FROM_PARENT_ID)) {
	printUsage('ERROR: You need to provide the from-parent-id as the second argument');
}

//TO PARENT ID
$TO_PARENT_ID = (isset($_SERVER['argv'][3])) ? $_SERVER['argv'][3] : '';
if (empty($TO_PARENT_ID)) {
	printUsage('ERROR: You need to provide the to-parent-id as the third argument');
}

//ASSET_TYPE
$ASSET_TYPE = (isset($_SERVER['argv'][4])) ? $_SERVER['argv'][4] : '';
if (empty($ASSET_TYPE)) {
	printUsage('ERROR: You need to enter the asset_type as the fourth argument');
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';

//LINK_TYPE
$LINK_TYPE = (isset($_SERVER['argv'][5])) ? $_SERVER['argv'][5] : SQ_SC_LINK_SIGNIFICANT;

// ask for the root password for the system
echo 'Enter the root password for "'.SQ_CONF_SYSTEM_NAME.'": ';
$root_password = rtrim(fgets(STDIN, 4094));

// check that the correct root password was entered
$root_user = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
if (!$root_user->comparePassword($root_password)) {
	echo "ERROR: The root password entered was incorrect\n";
	exit();
}

// log in as root
if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	echo "ERROR: Failed login in as root user\n";
	exit();
}

moveAssets($FROM_PARENT_ID, $TO_PARENT_ID, $ASSET_TYPE, $LINK_TYPE);

$GLOBALS['SQ_SYSTEM']->restoreCurrentUser();

echo "Finish moving assets!\n";


/**
 * Move all assets under a parent asset to a new parent asset 
 * 
 * @param $from_parent_assetid	The old parent asset ID
 * @param $to_parent_assetid	The new parent asset ID
 * @param $type_code			The type code of the child assets that need to be moved
 * @param $link_type			The link type of the links between the old parent asset and the child assets
 * @param $chunk_size			The size of the chunk of assets that are moved each time
 * @return void
 */
function moveAssets($from_parent_assetid, $to_parent_assetid, $type_code = '', $link_type = NULL, $max_size = 1000, $chunk_size = 100)
{
	//get the assetids to move
	$child_assetids = $GLOBALS['SQ_SYSTEM']->am->getChildren($from_parent_assetid, $type_code, TRUE, NULL, NULL, NULL, TRUE, 1, 1);
	
	//convert it to one dimension array
	$child_assetids = array_keys($child_assetids);
	
	//get no more than $max_size assets
	$child_assetids = array_slice($child_assetids, 0, $max_size);

	//split assetid array to smaller chunks
	$child_assetids = array_chunk($child_assetids, $chunk_size);
	
	//move each chunk of assets to the new parent
	$total_moved_asset = 0;
	foreach ($child_assetids as $assetid_chunk) {
		//we are working, please wait
		echo "Moving assets ...\n";
		
		//move asset chunk
		moveAssetChunk($assetid_chunk, $from_parent_assetid, $to_parent_assetid, $link_type);
		
		//output the progress
		$total_moved_asset += count($assetid_chunk);
		echo "Total of $total_moved_asset assets have been moved from #$from_parent_assetid to #$to_parent_assetid\n";
		
	}
	
}//end moveAssets()


/**
 * Move a chunk of assets under a parent asset to another asset
 * 
 * @param $assetids				The list of asset IDs that need to be moved
 * @param $from_parent_assetid	The old parent asset ID
 * @param $to_parent_assetid	The new parent asset ID
 * @param $link_type			The link type of the links between the old parent asset and the child assets
 * @return void
 */
function moveAssetChunk($assetids, $from_parent_assetid, $to_parent_assetid, $link_type = NULL)
{
	$am = $GLOBALS['SQ_SYSTEM']->am;
	
	$assets = Array();
	foreach ($assetids as $assetid) {
		//get all links between the moving assetids and the parent parent id
		$links = $am->getLinkByAsset($assetid, $from_parent_assetid, $link_type, NULL, 'minor', TRUE);
		//put all the link information to the assets array
		foreach ($links as $link) {
			$assets[$assetid][] = Array(
										'linkid'	=> $link['linkid'],
										'link_type'	=> $link['link_type'],
										'parentid'	=> $from_parent_assetid,
									   );
		}
	}
	
	//get HIPO Herder
	$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
	
	//create running vars
	$vars = Array(
					'link_action'		=> 'move',
					'assets'			=> $assets,
					'to_parent_assetid'	=> $to_parent_assetid,
					'to_parent_pos'		=> getNextSortOrder($to_parent_assetid),
				 );
				 
	$hh->freestyleHipo('hipo_job_create_links', $vars);
	
}//end moveAssetChunk()


/**
 * Get the next sort order for the next asset under a parent asset
 * 
 * @param $parent_assetid	The parent asset to get its children' next sort order
 * @return int
 */
function getNextSortOrder($parent_assetid)
{
	$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db');
	
	$sql = 'SELECT
				COUNT(*) as count, MAX(sort_order) as max
			FROM
				sq_ast_lnk
			WHERE
				majorid = :majorid';

	try {
		$query = MatrixDAL::preparePdoQuery($sql);
		MatrixDAL::bindValueToPdo($query, 'majorid', $parent_assetid);
		$result = MatrixDAL::executePdoAll($query);
		$row = $result[0];
		unset($result);
	} catch (Exception $e) {
		throw new Exception("Unable to get the last sort order of the parent asset #$parent_assetid , due to database error: ".$e->getMessage());
	}

	$next_sort_order = ($row['count'] > 0)? max($row['count'], $row['max']+1) : 0;
	
	$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
	
	return $next_sort_order;
	
}//end getNextSortOrder()


/**
 * Print the usage of this script and exit
 * 
 * @param $error	The error message to print before printing the usage
 * @return void
 */
function printUsage($error)
{
	echo "$error\n\n";
	echo "This script move assets of a certain type from a parent asset ID to another parent asset ID. The default maximum of 1000 assets can be moved to protect againts the problem of purging too many assets in trash.\n";
	echo "Usage: move_assets.php SYSTEM_ROOT FROM_ASSET_ID TO_ASSET_ID ASSET_TYPE [LINK_TYPE]\n\n";
	
	exit();
	
}//end printUsage()


?>
