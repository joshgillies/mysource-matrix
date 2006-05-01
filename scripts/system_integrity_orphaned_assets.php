<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: system_integrity_orphaned_assets.php,v 1.11 2006/05/01 01:03:34 emcdonald Exp $
*
*/

/**
* Finds and links orphaned assets (ie. ones with no links to them, ie. ones without links where they are
* the minor) underneath a specified asset id, preferably a folder
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.11 $
* @package MySource_Matrix
*/
error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	echo "ERROR: You need to supply the path to the System Root as the first argument\n";
	exit();
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';

$MAP_ASSETID = (isset($_SERVER['argv'][2])) ? $_SERVER['argv'][2] : '0';

$map_asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($MAP_ASSETID),'asset',FALSE);

if (empty($MAP_ASSETID) || empty($map_asset_info)) {
	echo "ERROR: You need to supply the assetid of a valid asset that orphaned assets will be mapped to as the second argument\n";
	exit();
} else {
	$map_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($MAP_ASSETID);
}

$ROOT_ASSETID = (isset($_SERVER['argv'][3])) ? $_SERVER['argv'][3] : '1';
if ($ROOT_ASSETID == 1) {
	echo "\nWARNING: You are running this integrity checker on the whole system.\nThis is fine, but it may take a long time\n\n";
}

// ask for the root password for the system
echo 'Enter the root password for "'.SQ_CONF_SYSTEM_NAME.'": ';
$root_password = rtrim(fgets(STDIN, 4094));

// check that the correct root password was entered
$root_user =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
if (!$root_user->comparePassword($root_password)) {
	echo "ERROR: The root password entered was incorrect\n";
	exit();
}

// log in as root
if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	trigger_error("Failed login in as root user\n", E_USER_ERROR);
}

$db =& $GLOBALS['SQ_SYSTEM']->db;

$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);

// go through each child of the specified asset, lock it, validate it, unlock it
$assets = $GLOBALS['SQ_SYSTEM']->am->getChildren($ROOT_ASSETID, 'asset', FALSE);
foreach ($assets as $assetid => $type_code) {


	printAssetName($assetid);

	$sql = 'SELECT
				linkid,
				majorid,
				link_type,
				sort_order
			FROM
				sq_ast_lnk
			WHERE
					minorid		= '.$db->quote($assetid).'
				AND	link_type	<> '.$db->quote(SQ_LINK_NOTICE);

	$links = $db->getAll($sql);
	assert_valid_db_result($links);

	$updated = FALSE;
	$errors = FALSE;

	foreach (array_keys($links) as $linkid) {
		$link =& $links[$linkid];
		if ($link['linkid'] == 1) continue;

		$major_asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($link['majorid']),'asset',FALSE);

		if (empty($major_asset_info)) {

			// ARGH, we have a link but our major isn't there anymore... this could be a problem
			// if it was an exclusive link...!
			// basically we need to do the whole rubbish regarding deleting asset links in here
			// because deleteAssetLink() checks to see if the asset is alive... which it isn't >_>;;;

			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
			$db =& $GLOBALS['SQ_SYSTEM']->db;

			// open the transaction
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

			// if this is a significant link
			if ($link['link_type'] & SQ_SC_LINK_SIGNIFICANT) {

				//// UPDATE THE TREE ////

				// update the parents to tell them that they are going to be one kid less
				$sub_sql = 'SELECT
								SUBSTR(t.treeid, 1, (LENGTH(t.treeid) - '.SQ_CONF_ASSET_TREE_SIZE.'))
							FROM
								sq_ast_lnk_tree t
							WHERE
								t.linkid = '.$db->quote($link['linkid']);

				$sql = 'UPDATE
							sq_ast_lnk_tree
						SET
							num_kids = num_kids - 1
						WHERE
							treeid in ('.$sub_sql.')';

				$result = $db->query($sql);
				assert_valid_db_result($result);


				// we can delete all the links under these nodes because it will be a clean start
				// when we insert into the gap's we create below
				$sub_sql   = 'SELECT
									ct.treeid
							FROM
								sq_ast_lnk_tree pt,
								sq_ast_lnk_tree ct
							WHERE
									pt.linkid = '.$db->quote($link['linkid']).'
								AND	ct.treeid LIKE pt.treeid || '.$db->quote('%').'
								AND	ct.treeid > pt.treeid';

				$sql = 'DELETE FROM
							sq_ast_lnk_tree
						WHERE
							treeid in ('.$sub_sql.')';

				$result = $db->query($sql);
				assert_valid_db_result($result);

				// we are going to set the treeid nodes that this link is associated
				// with to zero so that we can find it as a gap when we createLink() later on

				$sql = 'UPDATE
							sq_ast_lnk_tree
						SET
							linkid = '.$db->quoteSmart('0').',
							num_kids = '.$db->quoteSmart('0').'

						WHERE
							linkid = '.$db->quote($link['linkid']);

				$result = $db->query($sql);
				assert_valid_db_result($result);

			}//end if significant link

			// move 'em up, higher
			$sql = 'UPDATE
						sq_ast_lnk
					SET
						sort_order = sort_order - 1
					WHERE
							majorid		= '.$db->quote($link['majorid']).'
						AND	sort_order 	> '.$db->quote($link['sort_order']);

			$result = $db->query($sql);
			assert_valid_db_result($result);

			$where_cond = ' linkid  = '.$db->quote($link['linkid']);
			$sql = 'DELETE FROM
						sq_ast_lnk
					WHERE
						linkid  = '.$db->quote($link['linkid']);

			$result = $db->query($sql);
			assert_valid_db_result($result);

			// tell, the asset it has updated
			$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $type_code);
			if (!$asset->linksUpdated()) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				$errors = TRUE; break 2;
			}

			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			unset($links[$linkid]);
		}//end if
	}//end foreach

	if ($errors) {			// no links
		printUpdateStatus('FAILED');
		continue;
	}

	if (empty($links)) {			// no links
		$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $type_code);
		if (!$GLOBALS['SQ_SYSTEM']->am->createAssetLink($map_asset, $asset, SQ_LINK_TYPE_2, 'Orphaned Asset')) {
			printUpdateStatus('FAILED');
			continue;
		}
		$updated = TRUE;
	}

	if ($updated) {
		printUpdateStatus('OK');
	} else {
		printUpdateStatus('--');
	}
	$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);

}//end foreach

$GLOBALS['SQ_SYSTEM']->restoreRunLevel();


/**
* Prints the name of the Asset as a padded string
*
* Pads name to 40 columns
*
* @param string	$assetid	the id of the asset of which we want to print the name
*
* @return void
* @access public
*/
function printAssetName($assetid)
{
	$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($assetid);
	$str = '[ #'.$assetid.' ]'.$asset_info[$assetid]['name'];
	if (strlen($str) > 66) {
		$str = substr($str, 0, 66).'...';
	}
	printf ('%s%'.(70 - strlen($str)).'s', $str,'');

}//end printAssetName()


/**
* Prints the status of the container integrity check
*
* @param string	$status	the status of the check
*
* @return void
* @access public
*/
function printUpdateStatus($status)
{
	echo "[ $status ]\n";

}//end printUpdateStatus()


?>
