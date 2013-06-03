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
* $Id: system_integrity_invalid_links.php,v 1.11 2013/06/03 04:52:25 akarelia Exp $
*
*/

/**
* Finds, and optionally removes significant links from sq_ast_lnk where one or both sides of the link do not exist.
* Report on the number of orphaned assets left in the system and recommends running system_integrity_orphaned_assets.php.
* 
* This script can only be run system wide.
*
* @author  Nathan Callahan <ncallahan@squiz.net>
* @author  Mohamed Haidar <mhaidar@squiz.net>
* @version $Revision: 1.11 $
* @package MySource_Matrix
*/

error_reporting(E_ALL);
if (count($_SERVER['argv']) < 3 || php_sapi_name() != 'cli') {
	echo "This script needs to be run in the following format:\n\n";
	echo "\tphp system_integrity_invalid_links.php [SYSTEM_ROOT] [-delete|-check] [-remove_notice_links]\n\n";
	exit(1);
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

$ACTION = (isset($_SERVER['argv'][2])) ? $_SERVER['argv'][2] : '';
$ACTION = ltrim($ACTION, '-');
if (empty($ACTION) || ($ACTION != 'delete' && $ACTION != 'check')) {
	echo "ERROR: No action specified";
	exit();
}//end if

$remove_notice_links = FALSE;
$NOTICE_ACTION = (isset($_SERVER['argv'][3])) ? $_SERVER['argv'][3] : '';
$NOTICE_ACTION = ltrim($NOTICE_ACTION, '-');
if ($NOTICE_ACTION == 'remove_notice_links') {
	if ($ACTION != 'delete') trigger_error("Cannot remove notice links if the action is not '-delete'", E_USER_ERROR);
	$remove_notice_links = TRUE;
} else if ($NOTICE_ACTION != '') {
	if ($ACTION == 'delete') echo "\nThird argument was mentioned but was not '-remove_notice_links'. Notice Links will not be removed\n\n";
}

// login as root user to avoid problems with safe edit assets
$root_user = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	echo "ERROR: Failed login in as root user";
	exit();
}

$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db');
$sql  = "SELECT * FROM sq_ast_lnk a WHERE NOT EXISTS (SELECT assetid FROM sq_ast WHERE assetid = a.minorid)";
if (!$remove_notice_links && $ACTION != 'check') $sql .= " AND a.link_type <> :link_type";

$sql .= " UNION SELECT * FROM sq_ast_lnk b WHERE NOT EXISTS (SELECT assetid FROM sq_ast WHERE assetid = b.majorid) AND b.majorid <> '0'";
if (!$remove_notice_links && $ACTION != 'check') $sql .= " AND b.link_type <> :link_type";

try {
	$query = MatrixDAL::preparePdoQuery($sql);
	if (!$remove_notice_links && $ACTION != 'check') MatrixDAL::bindValueToPdo($query, 'link_type', SQ_LINK_NOTICE);
	$links = DAL::executePdoAssoc($query);
	$link_count = count($links);
} catch (Exception $e) {
	trigger_error('Unable to find invalid links due to database error: '.$e->getMessage(), E_USER_ERROR);
}

$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

if ($link_count == 0) {
 	echo "\nNo invalid links found\n";
 	exit(0);
} else {
	echo "\nFound $link_count invalid links\n";
	if ($ACTION == 'delete'){
		echo 'This script is about to be run SYSTEM WIDE in DELETE mode. Are you sure you want to continue (y/n): ';
		$confirm = rtrim(fgets(STDIN, 4094));
		if ($confirm != 'y') exit(0);
	} else {
		exit(0);
	}
}

$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');

foreach($links as $link) {

	//the upcoming queries have been copied over from Asset_Manager::deleteAssetLinkByLink().
	if (!($link['link_type'] & SQ_SC_LINK_SIGNIFICANT) && !$remove_notice_links) continue;

	$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
	
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
		trigger_error('Unable to update the link tree for linkid: '.$link['linkid'].' due to database error: '.$e->getMessage(), E_USER_ERROR);
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
		trigger_error('Unable to delete tree links for linkid: '.$link['linkid'].' due to database error: '.$e->getMessage(), E_USER_ERROR);
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
		trigger_error('Unable to update sort orders for majorid: '.$link['majorid'].' due to database error: '.$e->getMessage(), E_USER_ERROR);
	}

	// Delete from the link table
	try {
		$bind_vars	= Array (
				 'linkid'	=> $link['linkid'],
				 'majorid'	=> $link['majorid'],
				 );
		MatrixDAL::executeQuery('core', 'deleteLink', $bind_vars);
	} catch (Exception $e) {
		trigger_error('Unable to delete link with linkid: '.$link['linkid'].' due to database error: '.$e->getMessage(), E_USER_ERROR);
	}

	$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
	echo "Deleted Link ID: ".$link['linkid']." with Major ID: ".$link['majorid']." and Minor ID: ".$link['minorid']."\n";
	
} // end foreach link

$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db');

$sql = 'SELECT count(*)
		FROM sq_ast a
		WHERE NOT EXISTS (SELECT linkid FROM sq_ast_lnk WHERE minorid = a.assetid OR majorid = a.assetid);';
try {
	$query = MatrixDAL::preparePdoQuery($sql);
	$orphans = DAL::executePdoOne($query);
} catch (Exception $e) {
	trigger_error('Unable to count orphaned assets due to database error: '.$e->getMessage(), E_USER_ERROR);
}

if (!empty($orphans)) echo "There are $orphans orphan assets found. You must run system_integrity_orphaned_assets.php on the root folder to rescue these assets.\n";

$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
