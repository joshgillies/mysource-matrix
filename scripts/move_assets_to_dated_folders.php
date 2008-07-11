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
* $Id: move_assets_to_dated_folders.php,v 1.1.2.3 2008/07/11 01:45:47 mbrydon Exp $
*
*/

/**
* Move assets directly residing under a parent asset into a folder structure
* based on the "created" or "published" date of each asset. The resulting folder
* structure is stored under the supplied parent asset.
*
* If a suitable folder structure exists under the parent asset, it will be re-used.
*
* Credit to Richard Hulse for this concept which is now available to the Matrix Community!
*
* @author  Mark Brydon <mbrydon@squiz.net>
* @version $Revision: 1.1.2.3 $
* @package MySource_Matrix
*/


/**
* Prints out some basic help info detailing how to use this script
*
* @return void
* @access public
*/
function printUsage()
{
        echo "Move assets into dated folders\n\n";
        echo "Usage: move_assets_to_dated_folders [system root] [root node asset ID] [asset type code] [asset date field] [time period] (folder link type)\n";
        echo "system root         : The Matrix System Root directory\n";
		echo "root node asset ID  : The asset under which child assets are to be moved\n";
		echo "asset type code     : The type of assets to be moved\n";
		echo "asset date field    : The date field determining the destination for each asset - either 'created' or 'published'\n";
        echo "time period         : A specification of the folder structure to be created - either 'year', 'month' or 'day'\n";
		echo "                      eg; when 'asset date field' is 'created', 'day' will create or re-use a structure of 2008 > 03 > 28 for assets created on 28th March 2008\n";
		echo "folder link type    : The link type for newly-created folders (either 1 or 2). This is optional and will default to Type 2\n\n";

}//end printUsage()


/**
* Searches the Matrix System for an existing year/month/day folder under the specified parent, based on the supplied timestamp.
* If the expected folders do not exist under the parent, they will be created.
* The asset ID of the matching year/month/day folder is returned.
*
* @param int	$parent_id			The asset ID from which to search for matching a year/month folder combination
* @param int	$create_timestamp	The folder specification to search (eg; where period = "month", create_timestamp = 0 = 1 Jan 1970 = search for folder structure 1970 > 01)
* @param string	$period				The period to search or create - one of "year", "month", or "day"
* @param int	$folder_link_type	The asset link type to use when creating new dated folders
*
* @return int
* @access public
*/
function searchExistingDatedFolder($parent_id, $create_timestamp, $period, $folder_link_type=SQ_LINK_TYPE_2)
{
	$am =& $GLOBALS['SQ_SYSTEM']->am;

	// Year and month folder names. Month and day are zero-padded numbers
	$year = date('Y', $create_timestamp);
	$month = str_pad(date('m', $create_timestamp), 2, '0', STR_PAD_LEFT);
	$day = str_pad(date('d', $create_timestamp), 2, '0', STR_PAD_LEFT);

	// Variable housekeeping
	$matching_year_folder_id = 0;
	$matching_month_folder_id = 0;
	$matching_day_folder_id = 0;
	$matching_folder_id = 0;

	$matching_year_folders = searchExistingAsset($parent_id, $year, 'folder');

	$num_found_assets = count($matching_year_folders);

	if ($num_found_assets > 1) {
		echo "\n- FAILED - found ".$num_found_assets.' year folders for '.$year."\n";
		exit();
	}

	if ($num_found_assets == 1) {
		$keys = array_keys($matching_year_folders);
		$matching_year_folder_id = $keys[0];
	}

	if ($matching_year_folder_id == 0) {
		echo "\n- Creating Year Folder ".$year.'... ';
		$matching_year_folder_id = createAsset('folder', $year, $parent_id, $folder_link_type);
		echo 'asset #'.$matching_year_folder_id."\n";
	}

	$matching_folder_id = $matching_year_folder_id;

	// If we're looking for a month or day, dig deeper
	if (($matching_year_folder_id > 0) && ($period != 'year')) {
		$matching_month_folders = searchExistingAsset($matching_year_folder_id, $month, 'folder');

		$num_found_assets = count($matching_month_folders);

		if ($num_found_assets > 1) {
			echo "\n- FAILED - found ".$num_found_assets.' month folders for year/month '.$year.'/'.$month."\n";
			exit();
		}

		if ($num_found_assets == 1) {
			$keys = array_keys($matching_month_folders);
			$matching_month_folder_id = $keys[0];
		}

		if ($matching_month_folder_id == 0) {
			echo "\n- Creating Month Folder ".$month.' under Year '.$year.'... ';
			$matching_month_folder_id = createAsset('folder', $month, $matching_year_folder_id, $folder_link_type);
			echo 'asset #'.$matching_month_folder_id."\n";
		}

		$matching_folder_id = $matching_month_folder_id;
	}

	// Searching for a day - the last possible level
	if (($matching_month_folder_id > 0) && ($period == 'day')) {
		$matching_day_folders = searchExistingAsset($matching_month_folder_id, $day, 'folder');

		$num_found_assets = count($matching_day_folders);

		if ($num_found_assets > 1) {
			echo "\n- FAILED - found ".$num_found_assets.' day folders for year/month/day '.$year.'/'.$month.'/'.$day."\n";
			exit();
		}

		if ($num_found_assets == 1) {
			$keys = array_keys($matching_day_folders);
			$matching_day_folder_id = $keys[0];
		}

		if ($matching_day_folder_id == 0) {
			echo "\n- Creating Day Folder ".$day.' under Year/Month '.$year.'/'.$month.'... ';
			$matching_day_folder_id = createAsset('folder', $day, $matching_month_folder_id, $folder_link_type);
			echo 'asset #'.$matching_day_folder_id."\n";
		}

		$matching_folder_id = $matching_day_folder_id;
	}

	return $matching_folder_id;

}//end searchExistingDatedFolder()


/**
* Searches the Matrix System for an existing direct child asset which matches the specified name (and optionally, asset type)
* The asset IDs of matching assets are returned.
*
* @param int	$parent_id			The asset ID under which to search for direct matching child assets
* @param string	$asset_name			The asset name to match
* @param string	$asset_type_code	The asset type code to match (optional)
*
* @return array
* @access public
*/
function searchExistingAsset($parent_id, $asset_name, $asset_type_code='')
{
    $db =& $GLOBALS['SQ_SYSTEM']->db;

    $sql = 'SELECT l.minorid, a.name '.
            'FROM sq_ast_lnk l, sq_ast a '.
            'WHERE l.majorid = '.$db->quote($parent_id).' ';

    if (!empty($asset_type_code)) {
        $sql .= 'AND a.type_code = '.$db->quote($asset_type_code).' ';
    }

    $sql .= 'AND a.assetid = l.minorid '.
            'AND a.name = '.$db->quote($asset_name);


    $matching_assets = $db->getAssoc($sql);
    assert_valid_db_result($matching_assets);

    return $matching_assets;

}//end searchExistingAsset()


/**
* Creates a simple asset based on the supplied parameters. Returns the asset ID of the new asset
*
* @param string	$asset_type			The type of page to create
* @param string	$asset_name			The name for the new asset
* @param int	$parent_asset_id	The parent asset ID
* @param int	$link_type			The link type of the new asset
*
* @return int
* @access public
*/
function createAsset($asset_type, $asset_name, $parent_asset_id, $link_type=SQ_LINK_TYPE_1)
{
    $am =& $GLOBALS['SQ_SYSTEM']->am;
	$am->includeAsset($asset_type);

	$new_asset_id = 0;
    $new_asset =& new $asset_type();

    $parent_asset =& $am->getAsset($parent_asset_id);
    if ($parent_asset->id) {
    	$new_asset->setAttrValue('name', $asset_name);

	    $link = Array(
					'asset'			=> &$parent_asset,
					'link_type'		=> $link_type,
					'value'			=> '',
					'sort_order'	=> NULL,
					'is_dependant'	=> 0,
					'is_exclusive'	=> 0,
				);

	    if ($new_asset->create($link)) {
			$new_asset_id = $new_asset->id;
		}
    }
	$am->forgetAsset($parent_asset);

    return $new_asset_id;

}//end createAsset()


/**
* Moves an asset to a dated Folder structure under the specified parent asset
*
* @param int	$asset_id			The asset to move
* @param int	$parent_id			The destination for the folder structure
* @param string	$asset_date_field	One of 'created' or 'published' - the asset timestamp value to be used to associate the asset with a folder
* @param string	$time_period		One of 'year', 'month' or 'day' - the folder structure to create (ie; three-folder structure created/used for day - 2008 > 03 > 28)
* @param int	$folder_link_type	The asset link type to use when creating new dated folders
*
* @return boolean
* @access public
*/
function moveAssetToDatedFolder($asset_id, $parent_id, $asset_date_field, $time_period, $folder_link_type=SQ_LINK_TYPE_2)
{
	$result = FALSE;
	$am =& $GLOBALS['SQ_SYSTEM']->am;

	$asset =& $am->getAsset($asset_id);
	if ($asset->id) {
		$asset_timestamp = $asset->created;
		if ($asset_date_field == 'published') {
			$asset_timestamp = $asset->published;
		}

		if ($asset_timestamp != NULL) {
			// Find a destination folder for our asset
			$destination_folder_id = searchExistingDatedFolder($parent_id, $asset_timestamp, $time_period, $folder_link_type);

			if ($destination_folder_id > 0) {
				if ($asset_id == $destination_folder_id) {
					echo 'asset is our destination dated folder. SKIPPING this asset. ';

					// Continue anyway
					$result = TRUE;
				} else {
					$result = moveAsset($asset->id, $parent_id, $destination_folder_id);
				}
			}
		} else {
				echo 'asset '.$asset_date_field.' date does not exist. SKIPPING this asset. ';

				// Continue anyway
				$result = TRUE;
		}
	}

	$am->forgetAsset($asset);

	return $result;

}//end moveAssetToDatedFolder()


/**
* Moves an asset to underneath a new parent
*
* @param int	$source_asset_id		The asset to move
* @param int	$source_parent_id		The parent of the existing asset. Links will only be moved that are associated directly with this parent asset
* @param int	$destination_parent_id	The eventual new home for this asset
*
* @return boolean
* @access public
*/
function moveAsset($source_asset_id, $source_parent_id, $destination_parent_id)
{
	$result = FALSE;

	$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($source_asset_id, SQ_SC_LINK_SIGNIFICANT, '', TRUE, 'minor');
	foreach ($links as $link) {
		if ($link['majorid'] == $source_parent_id) {
			$link_info = $GLOBALS['SQ_SYSTEM']->am->getLinkById($link['linkid'], $source_asset_id, 'minor');

			$assets[$source_asset_id][] = Array(
											'linkid'	=> $link_info['linkid'],
											'link_type'	=> $link_info['link_type'],
											'parentid'	=> $link_info['majorid'],
										  );
		}
	}

	$hh =& $GLOBALS['SQ_SYSTEM']->getHipoHerder();
	$vars = Array(
				'link_action'		=> 'move',
				'assets'			=> $assets,
				'to_parent_assetid'	=> $destination_parent_id,
				'to_parent_pos'		=> 1,
			);

	$errors = $hh->freestyleHipo('hipo_job_create_links', $vars);
	$result = (count($errors) == 0);
	if (!$result) print_r($errors);

	return $result;

}//end moveAsset()


/************************** MAIN PROGRAM ****************************/

error_reporting(E_ALL);
ini_set('memory_limit', '-1');

if ((php_sapi_name() != 'cli')) {
	printUsage();
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	printUsage();
	echo "Please specify the path to the System Root as the first argument\n";
	exit();
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';

$root_node = (isset($_SERVER['argv'][2])) ? (int)$_SERVER['argv'][2] : 0;
if ($root_node <= 0) {
	printUsage();
	echo "Please specify a root node asset ID as the second argument\n";
	exit();
}

$am =& $GLOBALS['SQ_SYSTEM']->am;
$parent_asset =& $am->getAsset($root_node);
if (!$parent_asset->id) {
	printUsage();
	echo "The specified root node asset was not found\n";
	exit();
}
$am->forgetAsset($parent_asset);

$asset_type_code = (isset($_SERVER['argv'][3])) ? $_SERVER['argv'][3] : '';
if (empty($asset_type_code)) {
	printUsage();
	echo "Please specify an asset type code as the third argument\n";
	exit();
}

// Verify that the supplied asset code is correct
$am->includeAsset($asset_type_code);

$asset_date_field = (isset($_SERVER['argv'][4])) ? strtolower($_SERVER['argv'][4]) : '';
if (empty($asset_date_field)) {
	printUsage();
	echo "Please specify an asset date field as the fourth argument\n";
	exit();
} else if (($asset_date_field != 'created') && ($asset_date_field != 'published')) {
	printUsage();
	echo "Please specify either 'created' or 'published' for the asset date field\n";
	exit();
}

$time_period = (isset($_SERVER['argv'][5])) ? strtolower($_SERVER['argv'][5]) : '';
if (empty($time_period)) {
	printUsage();
	echo "Please specify a time period as the fifth argument\n";
	exit();
} else if (($time_period != 'year') && ($time_period != 'month') && ($time_period != 'day')) {
	printUsage();
	echo "Please specify either 'year', 'month' or 'day' for the time period\n";
	exit();
}

$folder_link_type = (isset($_SERVER['argv'][6])) ? (int)$_SERVER['argv'][6] : SQ_LINK_TYPE_2;
if (($folder_link_type != SQ_LINK_TYPE_1) && ($folder_link_type != SQ_LINK_TYPE_2)) {
	printUsage();
	echo "Please specify either link type 1 or 2 for the folder link type\n";
	exit();
}

// Ask for the root password for the system
echo 'Enter the root password for "'.SQ_CONF_SYSTEM_NAME.'": ';
$root_password = rtrim(fgets(STDIN, 4094));

// Check that the correct root password was entered
$root_user =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
if (!$root_user->comparePassword($root_password)) {
	echo "ERROR: The root password entered was incorrect\n";
	exit();
}

// Log in as root
if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
    trigger_error("Failed login as root user\n", E_USER_ERROR);
}

// All validated - ready to Folderise(tm)

// Get the immediate children of the parent asset that match the supplied type code
$assets_to_move = $am->getChildren($root_node, $asset_type_code, TRUE, NULL, NULL, NULL, TRUE, 1, 1);

if (count($assets_to_move) > 0) {
	// Move the child assets
	foreach (array_keys($assets_to_move) as $asset_id_to_move) {
		echo '- Moving asset #'.$asset_id_to_move.'... ';
		$result = moveAssetToDatedFolder($asset_id_to_move, $root_node, $asset_date_field, $time_period, $folder_link_type);
		if ($result) {
			echo "done\n";
		} else {
			echo "FAILED!\n";
			exit();
		}
	}
} else {
	echo "- No matching assets were found to move. Script aborted\n";
}

$GLOBALS['SQ_SYSTEM']->restoreCurrentUser();
echo "- All done!\n";

?>
