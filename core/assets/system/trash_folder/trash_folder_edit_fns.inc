<?php

require_once SQ_CORE_PACKAGE_PATH.'/folder/folder_edit_fns.inc';

/**
* Trash_Folder_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
* @subpackage __core__
*/
class Trash_Folder_Edit_Fns extends Folder_Edit_Fns
{

	/**
	* An array of tab code names and their display names
	* @var Array()
	*/
	var $static_screens = Array('details' => Array('name' => 'Purge Trash',
												   'force_unlock' => true
												   )
								);


	/**
	* Constructor
	*
	*/
	function Trash_Folder_Edit_Fns()
	{
		$this->Folder_Edit_Fns();
	}//end constructor


	/**
	* Prints the interface for purging the trash
	*
	* @param object File				&$asset	the trash asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintPurgeTrash(&$asset, &$o, $prefix)
	{
		if ($asset->purgeAccess()) {
			combo_box($prefix.'_purge_trash', Array(0 => 'No', 1 => 'Yes'), false, 0, 0, "onChange=\"Javascript: if (this.value == 1) { if(!confirm('Are you sure you want to purge all assets currently in the trash?')) { this.value = 0; } } \"");
		} else {
			echo '<b><i>You not not have access to purge the trash</i></b>';
		}
		return true;
	}//end paintPurgeTrash()


	/**
	* Process function for paintPurgeTrash, but doesnt actually do anything
	* All the real work is done in paintBackend for the trash
	*
	* @param object Trash				&$asset	the trash asset
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for form elements
	*
	* @see Trash_Folder::paintBackend()
	* @return boolean
	* @access public
	*/
	function processPurgeTrash(&$asset, &$o, $prefix)
	{
		return false;
	}//end processPurgeTrash()


	/**
	* Delete all assets that are in the trash
	* Starts the progress bar running to do this
	*
	* @param object File				&$asset	the trash asset
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for form elements
	*
	* @return void
	* @access private
	*/
	function _purgeTrash(&$asset, &$o, $prefix)
	{
		if (!$asset->purgeAccess()) return;

		require_once SQ_LIB_PATH.'/progress_bar/progress_bar.inc';
		$pb = new Progress_Bar($o->getCurrentLocation(), $this);
		if (!$pb->loaded) {
			$pb->setFns('_purgeTrashInit');
			$pb->setOption('auto_close', true);
			$pb->setOption('on_complete_url', $o->getCurrentLocation().'&'.$prefix.'_purge_complete=1');

			$pb->addStep('Aquiring Locks', '_purgeTrashStepLock', null, '_purgeTrashStepLockDone');
			$pb->addStep('Purging Trashed Assets', '_purgeTrashStep', null, '_purgeTrashStepDone');
		}

		$pb->run();

		if ($pb->inPopUp()) {
			$o->openRaw();
		} else {
			$o->setHeading('Purge Trash');
			$o->openSection('Confirm Purge Trash');
			$o->openField('WARNING:');
			echo '<b>You are about to delete all assets currently in the the trash.</b><br/>Press the \'Start\' button to begin.<br/>';
		}

		$pb->paint();

		if ($pb->inPopUp()) {
			$o->closeRaw();
		} else {
			$o->closeSection();
		}
	}//end _purgeTrash()


	/**
	* Called by the Progress Bar to initialise the purging process
	* returns true if ready to start
	*
	* @return boolean
	* @access private
	*/
	function _purgeTrashInit(&$pb)
	{
		$vars = $pb->getRunningVars();
		$trash = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('trash_folder');
		
		require_once SQ_INCLUDE_PATH.'/asset_manager_edit_fns.inc';
		$children = Asset_Manager_Edit_Fns::getAllChildLinks($trash->id);
		$children = array_reverse($children);

		// work out the treeid of the trash asset for use later
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$where = 'l.minorid = '.$db->quote($trash->id);
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 't');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
		$sql = 'SELECT t.treeid
				FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_link_tree t INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'asset_link l ON t.linkid = l.linkid
				'.$where.' LIMIT 1';

		$treeid = $db->getOne($sql);
		if (DB::isError($treeid)) {
			trigger_error($treeid->getMessage().'<br/>'.$treeid->getUserInfo(), E_USER_ERROR);
			$pb->abort();
			return;
		}

		$vars['purge_assets'] = $children;   // the assets we are purging
		$vars['num_locked']   = 0;
		$vars['num_purged']   = 0;
		$vars['trash_id']     = $trash->id;  // the asset id of the trash folder
		$vars['trash_treeid'] = $treeid;     // the treeid of the trash folder

		$pb->setRunningVars($vars);
		return true;
	}//end _purgeTrashInit()


	/**
	* Called repeatedly by the Progress Bar over multiple page reloads until it set's the percent to 100
	* Deletes the assets or the links after a bit of checking
	*
	* @param object Progress_Bar	&$pb	the progress bar
	*
	* @return void
	* @access private
	*/
	function _purgeTrashStep(&$pb)
	{
		// let everyone know we are purging the trash
		$GLOBALS['SQ_PURGING_TRASH'] = true;

		$vars = $pb->getRunningVars();
		
		$deleted = $vars['deleted_assets'] = Array();
		$num_to_purge = count($vars['purge_assets']);
		$count = -1;

		if (!$num_to_purge) {
			// nothing to purge in the trash
			$pb->setPercent(100);
			$pb->setRunningVars($vars);
			return;
		}

		foreach ($vars['purge_assets'] as $link_info) {
			$count++;
			if ($vars['num_purged'] && $count < $vars['num_purged']) continue;
			if (!$link_info['linkid']) continue;

			$assetid = $link_info['assetid'];
			$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $link_info['type_code'], true);
			if (is_null($asset)) {
				// if the asset doesnt exist it was probably deleted earlier on
				// thats why we mute errors above
				$vars['num_purged']++;
				continue;
			}

			$db = &$GLOBALS['SQ_SYSTEM']->db;
			
			$num_other_links = 0;
			$outside_links = Array();

			// we are going to try and find out if this asset has any links
			// outside the trash folder so we dont delete it if it is being used
			// somewhere else outside the trash
			$where = 'l.minorid='.$db->quote($link_info['assetid']).' 
					  AND t.treeid NOT LIKE '.$db->quote($vars['trash_treeid'].'%').' 
					  AND (l.link_type & '.$db->quote(SQ_SC_LINK_SIGNIFICANT).') > 0';
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 't');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
			$sql = 'SELECT l.linkid 
					FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_link_tree t INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'asset_link l ON t.linkid = l.linkid 
					'.$where;

			$outside_links = $db->getCol($sql);
			if (DB::isError($outside_links)) {
				trigger_error($outside_links->getMessage().'<br/>'.$outside_links->getUserInfo(), E_USER_WARNING);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$pb->abort();
				return;
			}

			// if the link we are deleting is also one of the outside
			// links, then we leave this link alone
			if (in_array($link_info['linkid'], $outside_links)) {
				$vars['num_purged']++;
				continue;
			}

			// if there are outside links, we are not going to delete the asset
			// but if there are not, then we need to check how many internal links
			// there are before deciding if we should delete the asset or not
			if (!count($outside_links)) {
				$num_other_links = $asset->countLinks('minor', SQ_SC_LINK_SIGNIFICANT, '', true, $link_info['linkid']);
			}

			if ($num_other_links || count($outside_links)) {
				// this asset has more than one link,
				// or has another link outside of the trash
				// so just delete the link
				$link = $asset->getLinkById($link_info['linkid'], 'minor');
				
				if (empty($link['majorid'])) {
					// if the link does not exists it was probably deleted
					// earlier, so we just skip this entry
					$vars['num_purged']++;
					continue;
				}

				$parent = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['majorid'], $link['type_code']);

				if (!$parent->deleteLink($link_info['linkid'])) {
					trigger_error('Purge aborted, unable to delete link '.$link_info['linkid'].' between "'.$parent->name.'" and "'.$asset->name.'"', E_USER_WARNING);
					$pb->abort();
					return;
				}

			} else {
				// we have an asset ready to delete, but we need to check with
				// the asset manager that we are allowed to delete it
				if (!$GLOBALS['SQ_SYSTEM']->am->canPurgeAsset($asset)) {
					trigger_error('Purge aborted, unable to delete "'.$asset->name.'" ['.$asset->id.']', E_USER_WARNING);
					$pb->abort();
					return;
				}

				// this is the last link for this asset, so lets delete it for good
				if (!$asset->delete(false)) {
					trigger_error('Purge aborted, unable to delete "'.$asset->name.'" ['.$asset->id.']', E_USER_WARNING);
					$pb->abort();
					return;
				}
			}

			// only delete one asset/link at a time
			break;
		}

		$vars['num_purged']++;
		$pb->setPercent($vars['num_purged'] / $num_to_purge);
		$pb->setRunningVars($vars);

		$GLOBALS['SQ_PURGING_TRASH'] = false;

	}//end _purgeTrashStep()


	/**
	* Called after _purgeTrashStep() above has set the percent to 100
	* Deletes the records created in the lock table and clears the status message
	*
	* @param object Progress_Bar	&$pb	the progress bar
	*
	* @return void
	* @access private
	*/
	function _purgeTrashStepDone(&$pb)
	{
		// now we need to release all locks we acquired
		$vars = $pb->getRunningVars();
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$where_cond = 'source_asset = '.$db->quote($vars['trash_id']);
		if (!$GLOBALS['SQ_SYSTEM']->rollbackDelete('asset_lock', $where_cond)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$pb->abort();
			return false;
		}

		$pb->setStatusMsg('');
	}// end _purgeTrashStepDone()


	/**
	* Called repeatedly by the Progress Bar over multiple page reloads until it set's the percent to 100
	* Performs the locking of the assets we are going to purge
	*
	* @param object Progress_Bar	&$pb	the progress bar
	*
	* @return void
	* @access private
	*/
	function _purgeTrashStepLock(&$pb)
	{
		$vars = $pb->getRunningVars();

		$done_a_lock = false;
		$num_to_lock = count($vars['purge_assets']);
		$count = -1;
		foreach ($vars['purge_assets'] as $link_info) {
			$count++;
			if ($vars['num_locked'] && $count < $vars['num_locked']) continue;

			// if this asset is dependant, it will be locked when
			// we call acquire_lock of the asset it is dependant on, so
			// we can skip these and just lock non-dependant ones
			if (!$link_info['dependant']) {
				if ($done_a_lock) break;
				$assetid = $link_info['assetid'];
				$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				if (!$asset->acquireLock($vars['trash_id'])) {
					trigger_error('Purge aborted, unable to acquire lock for "'.$asset->name.'" for purging trash', E_USER_WARNING);
					$pb->abort();
					return;
				} else {
					$done_a_lock = true;
				}
			}
			$vars['num_locked']++;
		}

		$pb->setPercent($vars['num_locked'] / $num_to_lock);
		$pb->setRunningVars($vars);
	}//end _purgeTrashStepLock()


	/**
	* Called after _purgeTrashStepLock() above has set the percent to 100
	* Just clears the status msg
	*
	* @param object Progress_Bar	&$pb	the progress bar
	*
	* @return void
	* @access private
	*/
	function _purgeTrashStepLockDone(&$pb)
	{
		$pb->setStatusMsg('');
	}// end _purgeTrashStepLockDone()


}//end class
?>
