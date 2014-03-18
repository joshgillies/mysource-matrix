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
* $Id: move_assets_to_dated_folders.php,v 1.9 2012/10/05 07:20:38 akarelia Exp $
*
*/

/**
* Move assets directly residing under a parent asset into a folder structure
* based on the "created" or "published" date of each asset. The resulting folder
* structure is stored under the supplied parent asset.
*
* If a suitable folder structure exists under the parent asset, it will be re-used.
*
* Credit to Richard Hulse (Radio NZ) for this concept which is now available to the Matrix Community!
*
* @author  Mark Brydon <mbrydon@squiz.net>
* @version $Revision: 1.9 $
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
	echo "Usage: move_assets_to_dated_folders [system root] --root=[root node asset ID] --type=[asset type code]\n";
	echo "                                    --field=[asset date field] --period=[time period] (--folder-link-type=[folder link type])\n";
	echo "                                    (--move-asset-status=[move asset status]) (--make-folders-live)\n\n";
	echo "REQUIRED PARAMETERS ==========\n";
        echo "system root         : The Matrix System Root directory\n";
	echo "root node asset ID  : The asset under which child assets are to be moved\n";
	echo "asset type code     : The type of assets to be moved\n";
	echo "asset date field    : The date field determining the destination for each asset - either 'created' or 'published'\n";
        echo "time period         : A specification of the folder structure to be created - either 'year', 'month' or 'day'\n";
	echo "                      eg; when 'asset date field' is 'created', 'day' will create or re-use a structure of 2008 > 03 > 28 for assets created on 28th March 2008\n\n";
	echo "OPTIONAL PARAMETERS ==========\n";
	echo "folder link type    : The link type for newly-created folders (either 1 or 2). Type 2 links are used by default\n";
	echo "move asset status   : The status code which must be matched for each asset before moving (eg; set to '16' to move only 'Live' assets)\n";
	echo "-make-folders-live  : Set the new date folders to 'Live' upon creation. Otherwise they will be 'Under Construction'\n\n";

}//end printUsage()


/**
* Searches the Matrix System for an existing year/month/day folder under the specified parent, based on the supplied timestamp.
* If the expected folders do not exist under the parent, they will be created.
* The asset ID of the matching year/month/day folder is returned.
*
* @param int		$parent_id			The asset ID from which to search for matching a year/month folder combination
* @param int		$create_timestamp	The folder specification to search (eg; where period = "month", create_timestamp = 0 = 1 Jan 1970 = search for folder structure 1970 > 01)
* @param string		$period				The period to search or create - one of "year", "month", or "day"
* @param int		$folder_link_type	The asset link type to use when creating new dated folders
* @param boolean	$make_folders_live      Whether to make the newly-created dated folders live (default = Under Construction)
*
* @return int
* @access public
*/
function searchExistingDatedFolder($parent_id, $create_timestamp, $period, $folder_link_type, $make_folders_live=FALSE)
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
		return 0;
	}

	if ($num_found_assets == 1) {
		$matching_year_folder_id = $matching_year_folders[0];
	}

	if ($matching_year_folder_id == 0) {
		echo "\n- Creating Year Folder ".$year.'... ';
		$matching_year_folder_id = createAsset('folder', $year, $parent_id, $folder_link_type);
		echo 'asset #'.$matching_year_folder_id."\n";
		if ($make_folders_live) {
			setAssetStatus($matching_year_folder_id, SQ_STATUS_LIVE);
		}
	}

	$matching_folder_id = $matching_year_folder_id;

	// If we're looking for a month or day, dig deeper
	if (($matching_year_folder_id > 0) && ($period != 'year')) {
		$matching_month_folders = searchExistingAsset($matching_year_folder_id, $month, 'folder');

		$num_found_assets = count($matching_month_folders);

		if ($num_found_assets > 1) {
			echo "\n- FAILED - found ".$num_found_assets.' month folders for year/month '.$year.'/'.$month."\n";
			return 0;
		}

		if ($num_found_assets == 1) {
			$matching_month_folder_id = $matching_month_folders[0];
		}

		if ($matching_month_folder_id == 0) {
			echo "\n- Creating Month Folder ".$month.' under Year '.$year.'... ';
			$matching_month_folder_id = createAsset('folder', $month, $matching_year_folder_id, $folder_link_type);
			echo 'asset #'.$matching_month_folder_id."\n";
			if ($make_folders_live) {
				setAssetStatus($matching_month_folder_id, SQ_STATUS_LIVE);
			}
		}

		$matching_folder_id = $matching_month_folder_id;
	}

	// Searching for a day - the last possible level
	if (($matching_month_folder_id > 0) && ($period == 'day')) {
		$matching_day_folders = searchExistingAsset($matching_month_folder_id, $day, 'folder');

		$num_found_assets = count($matching_day_folders);

		if ($num_found_assets > 1) {
			echo "\n- FAILED - found ".$num_found_assets.' day folders for year/month/day '.$year.'/'.$month.'/'.$day."\n";
			return 0;
		}

		if ($num_found_assets == 1) {
			$matching_day_folder_id = $matching_day_folders[0];
		}

		if ($matching_day_folder_id == 0) {
			echo "\n- Creating Day Folder ".$day.' under Year/Month '.$year.'/'.$month.'... ';
			$matching_day_folder_id = createAsset('folder', $day, $matching_month_folder_id, $folder_link_type);
			echo 'asset #'.$matching_day_folder_id."\n";
			if ($make_folders_live) {
				setAssetStatus($matching_day_folder_id, SQ_STATUS_LIVE);
			}
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
	$db = MatrixDAL::getDb();
	$sql = 'SELECT l.minorid, a.name '.
		'FROM sq_ast_lnk l, sq_ast a '.
			'WHERE l.majorid = :majorid ';

	if (!empty($asset_type_code)) {
		$sql .= 'AND a.type_code = :type_code ';
	}

	$sql .= 'AND a.assetid = l.minorid '.
		'AND a.name = :asset_name';

	try {
		$query = MatrixDAL::preparePdoQuery($sql);
		MatrixDAL::bindValueToPdo($query, 'majorid', $parent_id);
		MatrixDAL::bindValueToPdo($query, 'asset_name', $asset_name);
		if (!empty($asset_type_code)) {
			MatrixDAL::bindValueToPdo($query, 'type_code', $asset_type_code);
		}
		$matching_assets = MatrixDAL::executePdoAssoc($query, 0);
	} catch (Exception $e) {
		throw new Exception('Unable to search for an existing '.$asset_name.' asset: '.$e->getMessage());
	}

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
    $new_asset = new $asset_type();

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
* @param int		$asset_id			The asset to move
* @param int		$parent_id			The destination for the folder structure
* @param string		$asset_date_field	One of 'created' or 'published' - the asset timestamp value to be used to associate the asset with a folder
* @param string		$time_period		One of 'year', 'month' or 'day' - the folder structure to create (ie; three-folder structure created/used for day - 2008 > 03 > 28)
* @param int		$folder_link_type	The asset link type to use when creating new dated folders
* @param boolean	$make_folders_live	Whether to make the newly-created dated folders live (default = Under Construction)
*
* @return boolean
* @access public
*/
function moveAssetToDatedFolder($asset_id, $parent_id, $asset_date_field, $time_period, $folder_link_type, $make_folders_live=FALSE)
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
			$destination_folder_id = searchExistingDatedFolder($parent_id, $asset_timestamp, $time_period, $folder_link_type, $make_folders_live);

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


/**
* Immediately sets the status of a given asset
*
* @param int    $asset_id       The asset ID of the asset to set the status
* @param int    $status         The status to set
*
* @return void
* @access public
*/
function setAssetStatus($asset_id, $status)
{
	$am =& $GLOBALS['SQ_SYSTEM']->am;
	$vars = Array(
			'new_status'	=> $status,
			'assetid'	=> $asset_id,
		);
	$hh =& $GLOBALS['SQ_SYSTEM']->getHipoHerder();
	$errors = $hh->freestyleHipo('hipo_job_edit_status', $vars);
	return (count($errors) == 0);

}//end setAssetStatus()


/************************** MAIN PROGRAM ****************************/

if (defined('E_STRICT') && (E_ALL & E_STRICT)) {
	error_reporting(E_ALL ^ E_DEPRECATED ^ E_STRICT);
} else {
	if (defined('E_DEPRECATED')) {
		error_reporting(E_ALL ^ E_DEPRECATED);
	} else {
		error_reporting(E_ALL);
	}
}

if (ini_get('memory_limit') != '-1') ini_set('memory_limit', '-1');

if ((php_sapi_name() != 'cli')) {
	printUsage();
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

require_once 'Console/Getopt.php';

$shortopt = '';
$longopt = Array('root=', 'type=', 'field=', 'period=', 'folder-link-type=', 'make-folders-live', 'move-asset-status=');

$con = new Console_Getopt();
$args = $con->readPHPArgv();
array_shift($args);
$options = $con->getopt($args, $shortopt, $longopt);
if (empty($options[0])) {
	printUsage();
	exit();
}

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT)) {
	echo "ERROR: You need to supply the path to the System Root as the first argument\n";
	printUsage();
	exit();
}

if (!is_dir($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT.'/core/include/init.inc')) {
	echo "ERROR: Path provided doesn't point to a Matrix installation's System Root. Please provide correct path and try again.\n";
	printUsage();
	exit();
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';
$am =& $GLOBALS['SQ_SYSTEM']->am;

$SYSTEM_ROOT = '';
$root_node = 0;
$asset_type_code = '';
$asset_date_field = '';
$time_period = '';
$folder_link_type = SQ_LINK_TYPE_2;
$make_folders_live = FALSE;
$move_asset_status = -1;

$mandatory_options = Array('--root', '--type', '--field', '--period');
$found_options = Array();

// Initial check for mandatory parameters
foreach ($options[0] as $option) {
	$found_options[] = $option[0];
}
reset($options[0]);

$num_found_options = 0;
foreach ($found_options as $found_option) {
	if (in_array($found_option, $mandatory_options)) {
		$num_found_options++;
	}
}

if ($num_found_options <> count($mandatory_options)) {
	printUsage();
	exit();
}

// Process dynamic command-line parameters
foreach ($options[0] as $option) {
	switch ($option[0]) {
		case '--root':
			$root_node = (isset($option[1])) ? (int)$option[1] : 0;
			if ($root_node <= 0) {
				echo "Please specify a root node asset ID (--root)\n";
				printUsage();
				exit();
			}

			$parent_asset =& $am->getAsset($root_node);
			if (!$parent_asset->id) {
				echo "The specified root node asset was not found\n";
				printUsage();
				exit();
			}
			$am->forgetAsset($parent_asset);
		break;

		case '--type':
			$asset_type_code = (isset($option[1])) ? $option[1] : '';
			if (empty($asset_type_code)) {
				echo "Please specify an asset type (--type)\n";
				printUsage();
				exit();
			} else {
				// Verify that the supplied asset code is correct
				$am->includeAsset($asset_type_code);
			}
		break;

		case '--field':
			$asset_date_field = (isset($option[1])) ? strtolower($option[1]) : '';
			if (empty($asset_date_field)) {
				echo "Please specify an asset date field (--field)\n";
				printUsage();
				exit();
			} else if (($asset_date_field != 'created') && ($asset_date_field != 'published')) {
				echo "Please specify either 'created' or 'published' for the asset date field\n";
				printUsage();
				exit();
			}
		break;

		case '--period':
			$time_period = (isset($option[1])) ? strtolower($option[1]) : '';
			if (empty($time_period)) {
				echo "Please specify a time period (--period)\n";
				printUsage();
				exit();
			} else if (($time_period != 'year') && ($time_period != 'month') && ($time_period != 'day')) {
				echo "Please specify either 'year', 'month' or 'day' for the time period\n";
				printUsage();
				exit();
			}
		break;

		case '--folder-link-type':
			$folder_link_type = (isset($option[1])) ? (int)$option[1] : SQ_LINK_TYPE_2;
			if (($folder_link_type != SQ_LINK_TYPE_1) && ($folder_link_type != SQ_LINK_TYPE_2)) {
				echo "Please specify either link type 1 or 2 for the folder link type (--folder-link-type)\n";
				printUsage();
				exit();
			}
		break;

		case '--make-folders-live':
			$make_folders_live = TRUE;
		break;

		case '--move-asset-status':
			$move_asset_status = (isset($option[1])) ? (int)$option[1] : -1;
		break;
	}//end switch
}//end foreach

// Check that the correct root password was entered
$root_user =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');

// Log in as root
if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
    echo "ERROR: Failed login as root user\n";
	exit();
}

// All validated - ready to Folderise(tm)

// Get the immediate children of the parent asset that match the supplied type code
$assets_to_move = $am->getChildren($root_node, $asset_type_code, TRUE, NULL, NULL, NULL, TRUE, 1, 1);
$failed_asset_move = Array();

if (count($assets_to_move) > 0) {
	// Move the child assets
	foreach (array_keys($assets_to_move) as $asset_id_to_move) {
		// Only process assets in our "move list" that match the "move state"
		$asset =& $am->getAsset($asset_id_to_move);
		if (($move_asset_status == -1) || ($asset->status == $move_asset_status)) {
			echo '- Moving asset #'.$asset_id_to_move.'... ';
			$result = moveAssetToDatedFolder($asset_id_to_move, $root_node, $asset_date_field, $time_period, $folder_link_type, $make_folders_live);
			if ($result) {
				echo "done\n";
			} else {
				echo "FAILED!\n";
				$failed_asset_move[] = $asset_id_to_move;
			}
		}
		$am->forgetAsset($asset);
	}

	if (!empty($failed_asset_move) > 0) {
		echo "\n*** The following assets could not be moved due to errors detailed above:\n";
		foreach ($failed_asset_move as $failed_asset_move_id) {
			echo '- Asset # '.$failed_asset_move_id."\n";
		}
		echo "\n";
	}
} else {
	echo "- No matching assets were found to move. Script aborted\n";
}

$GLOBALS['SQ_SYSTEM']->restoreCurrentUser();
echo "- All done!\n";

?>
