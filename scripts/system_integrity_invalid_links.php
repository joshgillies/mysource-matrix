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
* $Id: system_integrity_invalid_links.php,v 1.1 2011/07/04 06:06:46 mhaidar Exp $
*
*/

/**
* Finds, and optionally removes links from sq_ast_lnk where one or both sides of the link do not exist.
* For orphan assets or where the majorid side of link is just missing, you can use the system_integrity_orphaned_assets.php script.
* 
* This script can only be run SYSTEM WIDE.
* 
* Note that we're only interested in significant links here.
* NOTICE links can potentially cause issues, but there are good reasons to preserve them.
*
* @author  Nathan Callahan <ncallahan@squiz.net>
* @author  Mohamed Haidar <mhaidar@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix
*/

error_reporting(E_ALL);
if (count($_SERVER['argv']) != 3 || php_sapi_name() != 'cli') {
	echo "This script needs to be run in the following format:\n\n";
	echo "\tphp system_integrity_invalid_links.php [SYSTEM_ROOT] [-fix|-check]\n\n";
	exit(1);
}

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	trigger_error('The directory you specified as the system root does not exist, or is not a directory', E_USER_ERROR);
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';

$ACTION = (isset($_SERVER['argv'][2])) ? $_SERVER['argv'][2] : '';
$ACTION = ltrim($ACTION, '-');
if (empty($ACTION) || ($ACTION != 'fix' && $ACTION != 'check')) {
	trigger_error("No action specified", E_USER_ERROR);
}//end if

// login as root user to avoid problems with safe edit assets
$root_user = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	trigger_error("Failed login in as root user", E_USER_ERROR);
}

$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db');
$sql = "SELECT * FROM sq_ast_lnk a 
		WHERE 
			a.minorid 
		NOT IN 
			( SELECT assetid FROM sq_ast ) 
		AND 
			a.link_type <> :link_type
		UNION
		SELECT * FROM sq_ast_lnk b 
		WHERE 
			b.majorid 
		NOT IN 
			( SELECT assetid FROM sq_ast ) 
		AND 
			b.link_type <> :link_type
		AND
			b.majorid != '0'"; // Need to skip the link for the root folder.

try {
	$query = MatrixDAL::preparePdoQuery($sql);
	MatrixDAL::bindValueToPdo($query, 'link_type', SQ_LINK_NOTICE);
	$links = DAL::executePdoAssoc($query);
	$link_count = count($links);
} catch (Exception $e) {
	throw new Exception('Unable to find invalid links due to database error: '.$e->getMessage());
	exit (1);
}

$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

if ($link_count == 0) {
 	echo "\nNo invalid links found\n";
 	exit(0);
} else {
	echo "\nFound $link_count invalid links\n";
	if ($ACTION == 'fix'){
		echo 'This script is about to be run SYSTEM WIDE in FIX mode. Are you sure you want to continue (y/n): ';
		$confirm = rtrim(fgets(STDIN, 4094));
		if ($confirm != 'y') exit(0);
	} else {
		exit(0);
	}
}

$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db3');

foreach($links as $link) {

	//the upcoming queries have been copied over from Asset_Manager::deleteAssetLinkByLink().
	if (!($link['link_type'] & SQ_SC_LINK_SIGNIFICANT)) continue;
	
	echo "Deleting Link ID: ".$link['linkid']." with Major ID: ".$link['majorid']." and Minor ID: ".$link['minorid']."\n";
	
	// update the parents to tell them that they are going to be one kid less
	$sql = 'UPDATE
				sq_ast_lnk_tree
			SET
				num_kids = num_kids - 1
			WHERE
				treeid IN
				(
					SELECT
						CASE WHEN
							LENGTH(SUBSTR(t.treeid, 1, LENGTH(t.treeid) - '.SQ_CONF_ASSET_TREE_SIZE.')) != 0
						THEN
							SUBSTR(t.treeid, 1, LENGTH(t.treeid) - '.SQ_CONF_ASSET_TREE_SIZE.')
						ELSE
							\'-\'
						END
					FROM
						sq_ast_lnk_tree t
					WHERE
						t.linkid = :linkid
				)';
	$update_tree_parents_query = MatrixDAL::preparePdoQuery($sql);
	try {
		MatrixDAL::bindValueToPdo($update_tree_parents_query, 'linkid', $link['linkid']);
		MatrixDAL::execPdoQuery($update_tree_parents_query);
	} catch (Exception $e) {
		throw new Exception('Unable to update the link tree for linkid: '.$link['linkid'].' due to database error: '.$e->getMessage());
	}

	// we can delete all the links under these nodes because it will be a clean start
	// when we insert into the gap's we create below
	$sql = 'DELETE FROM
				sq_ast_lnk_tree
			WHERE
				treeid in
				(
					SELECT
						ct.treeid
					FROM
						sq_ast_lnk_tree pt, sq_ast_lnk_tree ct
					WHERE
							pt.linkid	= :linkid
						AND	(ct.treeid	LIKE pt.treeid || '.'\''.'%'.'\''.'
						OR ct.treeid	= pt.treeid)
				)';
	$delete_tree_query = MatrixDAL::preparePdoQuery($sql);
	try {
		MatrixDAL::bindValueToPdo($delete_tree_query, 'linkid', $link['linkid']);
		MatrixDAL::execPdoQuery($delete_tree_query);
	} catch (Exception $e) {
		throw new Exception('Unable to delete tree links for linkid: '.$link['linkid'].' due to database error: '.$e->getMessage());
	}

	// Update sort orders of other children of this parent
	$sql = 'UPDATE
				sq_ast_lnk
			SET
				sort_order = sort_order - 1
			WHERE
					majorid		= :majorid
				AND	sort_order	> :sort_order';
	$update_sort_order_query = MatrixDAL::preparePdoQuery($sql);
	try {
		MatrixDAL::bindValueToPdo($update_sort_order_query, 'majorid', $link['majorid']);
		MatrixDAL::bindValueToPdo($update_sort_order_query, 'sort_order', $link['sort_order']);
		MatrixDAL::execPdoQuery($update_sort_order_query);
	} catch (Exception $e) {
		throw new Exception('Unable to update sort orders for majorid: '.$link['majorid'].' due to database error: '.$e->getMessage());
	}

	// Delete from the link table
	try {
		$bind_vars	= Array (
				 'linkid'	=> $link['linkid'],
				 'majorid'	=> $link['majorid'],
				 );
		MatrixDAL::executeQuery('core', 'deleteLink', $bind_vars);
	} catch (Exception $e) {
		throw new Exception('Unable to delete link with linkid: '.$link['linkid'].' due to database error: '.$e->getMessage());
	}

} // end foreach link

$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
