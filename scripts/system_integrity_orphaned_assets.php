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
* $Id: system_integrity_orphaned_assets.php,v 1.20 2013/01/29 05:44:57 ewang Exp $
*
*/

/**
* Finds and links orphaned assets (ie. ones with no links to them, ie. ones without links where they are
* the minor) underneath a specified asset id, preferably a folder
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.20 $
* @package MySource_Matrix
*/
error_reporting(E_ALL);
ini_set('memory_limit', '-1');
if ((php_sapi_name() != 'cli')) {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT)) {
	echo "ERROR: You need to supply the path to the System Root as the first argument\n";
	exit();
}

if (!is_dir($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT.'/core/include/init.inc')) {
	echo "ERROR: Path provided doesn't point to a Matrix installation's System Root. Please provide correct path and try again.\n";
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
	echo "ERROR: Failed login in as root user\n";
	exit();
}

$db =& $GLOBALS['SQ_SYSTEM']->db;

$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);

// go entire asset DB, lock it, validate it, unlock it
$sql = 'SELECT assetid, type_code FROM sq_ast WHERE NOT EXISTS (
			SELECT * FROM sq_ast_lnk WHERE link_type IN ('.SQ_LINK_TYPE_1.','.SQ_LINK_TYPE_2.','.SQ_LINK_TYPE_3.') AND minorid = assetid
		)';
$query = MatrixDAL::preparePdoQuery($sql);
$assets = MatrixDAL::executePdoAssoc($query);
	
foreach ($assets as $data) {
	$assetid = $data['assetid'];
	$type_code = $data['type_code'];

	printAssetName($assetid);

	$sql = 'SELECT
				linkid,
				majorid,
				link_type,
				sort_order
			FROM
				sq_ast_lnk
			WHERE
				minorid = :minorid
				AND
				link_type <> :link_type';

	$query = MatrixDAL::preparePdoQuery($sql);
	MatrixDAL::bindValueToPdo($query, 'minorid', $assetid);
	MatrixDAL::bindValueToPdo($query, 'link_type', SQ_LINK_NOTICE);
	$links = MatrixDAL::executePdoAssoc($query);

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
								t.linkid = :linkid';

				$sql = 'UPDATE
							sq_ast_lnk_tree
						SET
							num_kids = num_kids - 1
						WHERE
							treeid in ('.$sub_sql.')';

				$query = MatrixDAL::preparePdoQuery($sql);
				MatrixDAL::bindValueToPdo($query, 'linkid', $link['linkid']);
				MatrixDAL::execPdoQuery($query);


				// we can delete all the links under these nodes because it will be a clean start
				// when we insert into the gap's we create below
				$sub_sql   = 'SELECT
									ct.treeid
							FROM
								sq_ast_lnk_tree pt,
								sq_ast_lnk_tree ct
							WHERE
									pt.linkid = :linkid
								AND	ct.treeid LIKE pt.treeid || '.MatrixDAL::quote('%').'
								AND	ct.treeid > pt.treeid';

				$sql = 'DELETE FROM
							sq_ast_lnk_tree
						WHERE
							treeid in ('.$sub_sql.')';

				$query = MatrixDAL::preparePdoQuery($sql);
				MatrixDAL::bindValueToPdo($query, 'linkid', $link['linkid']);
				MatrixDAL::execPdoQuery($query);

				// we are going to set the treeid nodes that this link is associated
				// with to zero so that we can find it as a gap when we createLink() later on

				$sql = 'UPDATE
							sq_ast_lnk_tree
						SET
							linkid = :linkid,
							num_kids = :num_kids

						WHERE
							linkid = :old_linkid';

				$query = MatrixDAL::preparePdoQuery($sql);
				MatrixDAL::bindValueToPdo($query, 'linkid',     '0');
				MatrixDAL::bindValueToPdo($query, 'num_kids',   '0');
				MatrixDAL::bindValueToPdo($query, 'old_linkid', $link['linkid']);
				MatrixDAL::execPdoQuery($query);

			}//end if significant link

			// move 'em up, higher
			$sql = 'UPDATE
						sq_ast_lnk
					SET
						sort_order = sort_order - 1
					WHERE
							majorid = :majorid
						AND
							sort_order > :sort_order';

			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'majorid',    $link['majorid']);
			MatrixDAL::bindValueToPdo($query, 'sort_order', $link['sort_order']);
			MatrixDAL::execPdoQuery($query);

			$sql = 'DELETE FROM
						sq_ast_lnk
					WHERE
						linkid  = :linkid';

			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'linkid', $link['linkid']);
			MatrixDAL::execPdoQuery($query);

			// tell the asset it has updated
			$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $type_code);
			$asset->linksUpdated();

			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			unset($links[$linkid]);
		}//end if
	}//end foreach

	if ($errors) {			// no links
		printUpdateStatus('FAILED');
		continue;
	}

	if (empty($links)) {	
		// no links
		$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $type_code);
		if (empty($asset) || !$GLOBALS['SQ_SYSTEM']->am->createAssetLink($map_asset, $asset, SQ_LINK_TYPE_2, 'Orphaned Asset')) {
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
