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
* $Id: system_integrity_orphaned_assets.php,v 1.1 2004/06/17 04:00:31 lwright Exp $
* $Name: not supported by cvs2svn $
*/

/**
* Finds and links orphaned assets (ie. ones with no links to them, ie. ones without links where they are
* the minor) underneath a specified asset id, preferably a folder
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/
error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) trigger_error("You can only run this script from the command line\n", E_USER_ERROR);

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	echo "ERROR: You need to supply the path to the System Root as the first argument\n";
	exit();
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';

$MAP_ASSETID = (isset($_SERVER['argv'][2])) ? $_SERVER['argv'][2] : '0';

$map_asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($MAP_ASSETID),'asset',false);

if (empty($MAP_ASSETID) || empty($map_asset_info)) {
	echo "ERROR: You need to supply the assetid of a valid asset that orphaned assets will be mapped to as the second argument\n";
	exit();
} else {
	$map_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($MAP_ASSETID);
}

$ROOT_ASSETID = (isset($_SERVER['argv'][3])) ? $_SERVER['argv'][3] : '1';
if ($ROOT_ASSETID == 1) {
	echo "\nWARNING: You are running this integrity checker on the whole system.\nThis is fine but:\n\tit may take a long time; and\n\tit will acquire locks on many of your assets (meaning you wont be able to edit content for a while)\n\n";
}

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
	trigger_error("Failed login in as root user\n", E_USER_ERROR);
}

$db =& $GLOBALS['SQ_SYSTEM']->db;

// go trough each wysiwyg in the system, lock it, validate it, unlock it
$assets = $GLOBALS['SQ_SYSTEM']->am->getChildren($ROOT_ASSETID, 'asset', false);
foreach ($assets as $assetid => $type_code) {
	
	$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $type_code);
	printAssetName($asset);
	
	// try to lock the asset
	if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($asset->id, 'links')) {
		printUpdateStatus('LOCK');
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
		continue;
	}
	

	$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_SC_LINK_ALL, 'asset', false, 'minor');

	$select = 'SELECT  linkid, majorid, link_type, sort_order';
	$from   = 'FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_link';
	$where = 'minorid = '.$db->quote($asset->id);

	$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
	$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');

	$links = $db->getAll($select.' '.$from.' '.$where);


	$updated = false;
	$errors = false;

	foreach(array_keys($links) as $linkid) {
		$link =& $links[$linkid];	
		if ($link['linkid'] == 1) continue;

		$major_asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($link['majorid']),'asset',false);

		if (empty($major_asset_info)) {

			// ARGH, we have a link but our major isn't there anymore... this could be a problem
			// if it was an exclusive link...!
			// basically we need to do the whole rubbish regarding deleting asset links in here
			// because deleteAssetLink() checks to see if the asset is alive... which it isn't >_>;;;

			require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';

			$db = &$GLOBALS['SQ_SYSTEM']->db;

			// open the transaction
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

			// if this is a significant link
			if ($link['link_type'] & SQ_SC_LINK_SIGNIFICANT) {

				//// UPDATE THE TREE ////

				// Update the parents to tell them that they are going to be one kid less
				$where_cond_string = 'treeid in (~SQ0~)';
				$sub_where = 't.linkid = '.$db->quote($link['linkid']);
				$sub_where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($sub_where, 't');
				$subs = Array(' SELECT SUBSTRING(t.treeid FROM 1 FOR (CHARACTER_LENGTH(t.treeid) - '.SQ_CONF_ASSET_TREE_SIZE.'))
							FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_link_tree t '.$sub_where);

				$where_cond = db_extras_subquery($db, $where_cond_string, $subs);
				if (DB::isError($where_cond)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					trigger_error($where_cond->getMessage().'<br/>'.$where_cond->getUserInfo(), E_USER_WARNING);
					$errors = true; break 2;
				}

				// add a rollback entry for the tree
				$values = Array('num_immediate_kids' => 'num_immediate_kids - 1');

				if (!$GLOBALS['SQ_SYSTEM']->rollbackUpdate('asset_link_tree', $values, $where_cond)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$errors = true; break 2;
				}

				// We can delete all the links under these nodes because it will be a clean start
				// when we insert into the gap's we create below
				$where_cond_string = 'treeid in (~SQ0~)';
				$concat = ($db->phptype == 'mysql') ? 'CONCAT(pt.treeid, '.$db->quote('%').')' : 'pt.treeid || '.$db->quote('%');
				$sub_where = 'pt.linkid = '.$db->quote($link['linkid']).'
							  AND ct.treeid LIKE '.$concat.'
							  AND ct.treeid > pt.treeid';
				$sub_where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($sub_where, 'pt');
				$sub_where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($sub_where, 'ct');
				$subs = Array('SELECT ct.treeid
								FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_link_tree pt, '.SQ_TABLE_RUNNING_PREFIX.'asset_link_tree ct
								'.$sub_where);

				$where_cond = db_extras_subquery($db, $where_cond_string, $subs);
				if (DB::isError($where_cond)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					trigger_error($where_cond->getMessage().'<br/>'.$where_cond->getUserInfo(), E_USER_WARNING);
					$errors = true; break 2;
				}

				if (!$GLOBALS['SQ_SYSTEM']->rollbackDelete('asset_link_tree', $where_cond)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$errors = true; break 2;
				}

				// We are going to set the treeid nodes that this link is associated
				// with to zero so that we can find it as a gap when we createLink() later on
				$where = 'linkid = '.$db->quote($link['linkid']);
				$values = Array('linkid' => $db->quote('0'),
								'num_immediate_kids' => $db->quote('0')
								);
				if (!$GLOBALS['SQ_SYSTEM']->rollbackUpdate('asset_link_tree', $values, $where)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$errors = true; break 2;
				}

			}// end if significant link

			// move 'em up, higher
			$where_cond = ' majorid        = '.$db->quote($link['majorid']).'
							AND sort_order > '.$db->quote($link['sort_order']);
			$values = Array('sort_order' => 'sort_order - 1');
			if (!$GLOBALS['SQ_SYSTEM']->rollbackUpdate('asset_link', $values, $where_cond)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$errors = true; break 2;
			}

			$where_cond = ' linkid  = '.$db->quote($link['linkid']);
			if (!$GLOBALS['SQ_SYSTEM']->rollbackDelete('asset_link', $where_cond)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$errors = true; break 2;
			}

			// tell, the asset it has updated
			if (!$asset->linksUpdated()) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$errors = true; break 2;
			}

			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			unset($links[$linkid]);
		}
	}

	if($errors) {			// no links
		printUpdateStatus('FAILED');
		continue;
	}

	if(empty($links)) {			// no links
		if (!$GLOBALS['SQ_SYSTEM']->am->createAssetLink($map_asset, $asset, SQ_LINK_TYPE_2, 'Orphaned Asset')) {
			printUpdateStatus('FAILED');
			continue;
		}
		$updated = true;
	}

	// try to unlock the asset
	if (!$GLOBALS['SQ_SYSTEM']->am->releaseLock($asset->id, 'links')) {
		printUpdateStatus('!!');
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
		continue;
	}

	if ($updated) {
		printUpdateStatus('OK');
	} else {
		printUpdateStatus('--');
	}
	$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
	
}//end foreach


/**
* Prints the name of the Asset as a padded string
*
* Pads name to 40 columns
*
* @param string	$name	the name of the container
*
* @return void
* @access public
*/
function printAssetName(&$asset)
{
	$str = $asset->name . ' [ # '. $asset->id. ' ]';
	printf ('%s%'.(40 - strlen($str)).'s', $str,'');
	
}//end printAssetName()


/**
* Prints the status of the container integrity check
*
* @param string	status	the status of the check
*
* @return void
* @access public
*/
function printUpdateStatus($status)
{
	echo "[ $status ]\n";
	
}//end printUpdateStatus()


?>
