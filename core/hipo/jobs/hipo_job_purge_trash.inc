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
* $Id: hipo_job_purge_trash.inc,v 1.60 2013/03/25 03:31:41 csmith Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';

/**
* Highly Intensive Processing Object (HIPO) to acquire locks
*
* <pre>
* Initial Running Vars :
*    There are no running vars for this HIPO
* </pre>
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.60 $
* @package MySource_Matrix
* @subpackage HIPO
*/
class HIPO_Job_Purge_Trash extends HIPO_Job
{


	/**
	* Constructor
	*
	* @param string	$code_name	a unique codename the HIPO
	*/
	function HIPO_Job_Purge_Trash($code_name='')
	{
		// we'll do our own transactions thanks
		$this->uses_trans = FALSE;

		$this->HIPO_Job($code_name);

	}//end constructor


	/**
	* Paints backend of this job
	*
	* @param object	&$o		reference to the backend outputter
	* @param string	$class	calling class name
	*
	* @return void
	* @access private
	*/
	public static function paintConfig(&$o, $class, $write_access)
	{
		// metadata regeneration threshhold HIPO config entry
		$o->openField(translate('purge_trash_threshold'));

		echo '<span class="sq-inline-fields-wrapper">';
		if ($write_access) {
			text_box($class.'[SQ_HIPO_PURGE_TRASH_THRESHOLD]', SQ_HIPO_PURGE_TRASH_THRESHOLD, 1);
		} else {
			echo SQ_HIPO_PURGE_TRASH_THRESHOLD;
		}
		echo ' '.translate('assets');
		echo '</span>';

		$o->closeField();

	}//end paintConfig()


	/**
	* Sets some info about thresholds that this hipo requires.
	*
	* @return array
	* @access public
	*/
	public static function getConfigVars()
	{
		return Array(
				'SQ_HIPO_PURGE_TRASH_THRESHOLD'	=> Array('editable'	=> 1, 'default'	=> 1),
			   );

	}//end getConfigVars()


	/**
	* Returns the (localised) HIPO name
	*
	* @return string
	* @access public
	*/
	function getHipoName()
	{
		return translate('hipo_name_purge_trash');

	}//end getHipoName()


	/**
	* Returns the steps in this hipo, possibly with localised step names
	* and messages
	*
	* @return string
	* @access public
	*/
	function getInitialStepData()
	{
		return Array(
				Array(
					'name'			=> translate('hipo_purging_trash_determining'),
					'function_call'	=> Array(
										'process_function'	=> 'buildListToDelete',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> TRUE,
					'allow_cancel'	=> TRUE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
				),
				Array(
					'name'			=> translate('hipo_purging_trash_deleting'),
					'function_call'	=> Array(
										'process_function'	=> 'purgeAsset',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> TRUE,
					'allow_cancel'	=> TRUE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
				),
				Array(
					'name'			=> translate('hipo_purging_trash_cleaning'),
					'function_call'	=> Array(
										'process_function'	=> 'deleteRemainingTrashLinks',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> TRUE,
					'allow_cancel'	=> TRUE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
				),
			   );

	}//end getInitialStepData()


	/**
	* Returns the current threshold
	*
	* Returns a value that represents the total percentage that this hipo
	* would run in freestyle mode in its current state
	*
	* @return float
	* @access public
	*/
	function getThresholdPercentageRequired()
	{
		if (SQ_HIPO_PURGE_TRASH_THRESHOLD == 0) return 0;
		if (!isset($this->_running_vars['to_check'])) {
			return 0;
		}
		return ((count($this->_running_vars['to_check']) / SQ_HIPO_PURGE_TRASH_THRESHOLD) * 100);

	}//end getThresholdPercentageRequired()


	/**
	* Prepares the running vars and gets the children of the trash folder
	*
	* @return boolean
	* @access public
	*/
	function prepare()
	{
		$trash = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('trash_folder');
		$this->_running_vars['to_delete'] = Array();
		$this->_running_vars['trash_id'] = $trash->id;
		$asset_tree_ids = $GLOBALS['SQ_SYSTEM']->am->getAssetTreeids($trash->id);
		$this->_running_vars['trash_treeid'] = reset($asset_tree_ids);

		if (!empty($this->_running_vars['purge_root_linkid'])) {

			$purge_root_link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($this->_running_vars['purge_root_linkid']);
			$this->_running_vars['purge_root_id'] = $purge_root_link['minorid'];
			$purge_root = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['purge_root_id']);

			// is the root in the trash?
			if (!$GLOBALS['SQ_SYSTEM']->am->assetInTrash($purge_root->id)) {
				trigger_localised_error('HIPO0083', E_USER_WARNING);
				return FALSE;
			}
			$purge_root_children = $GLOBALS['SQ_SYSTEM']->am->getChildren($purge_root->id);
			// also delete the purge root
			$purge_root_children[$purge_root->id][0]['type_code'] = $purge_root->type();
			$link_tree_ids = $GLOBALS['SQ_SYSTEM']->am->getLinkTreeId($this->_running_vars['purge_root_linkid']);
			$root_tree_id = reset($link_tree_ids);
			$this->_running_vars['purge_root_treeid'] = $root_tree_id[0][0];
		} else {
			$purge_root = $trash;
			$purge_root_children = $GLOBALS['SQ_SYSTEM']->am->getChildren($purge_root->id);
			$this->_running_vars['purge_root_treeid'] = $this->_running_vars['trash_treeid'];
		}

		$db = MatrixDAL::getDb();
		try {
			$bind_vars['treeid'] = $this->_running_vars['purge_root_treeid'].'%';
			$this->_running_vars['delete_order'] = MatrixDAL::executeAssoc('core', 'getPurgeTrashHipoDeleteOrder', 0, $bind_vars);
		} catch (Exception $e) {
			throw new Exception('Failed to get delete order for purge trash HIPO: '.$e->getMessage());
		}

		// Sort the 'to_check' assetid list by asset tree level. The parent assets will come on top of the array.
		$ordered_to_check_list = Array();
		foreach ($this->_running_vars['delete_order'] as $assetid) {
			if (isset($purge_root_children[$assetid])) {
				$ordered_to_check_list[$assetid] = $purge_root_children[$assetid];
			}
		}
		$this->_running_vars['to_check'] = $ordered_to_check_list;
		$this->_running_vars['total_assets'] = count($this->_running_vars['to_check']);

		return parent::prepare();

	}//end prepare()


	/**
	* Performs the duties of this hipo, without showing any output
	*
	* @return boolean
	* @access public
	*/
	function freestyle()
	{
		while (!empty($this->_running_vars['to_check'])) {
			if (!$this->buildListToDelete($this->_steps[0], $this->getCodeName())) {
				return FALSE;
			}
		}

		while (!empty($this->_running_vars['to_delete'])) {
			if (!$this->purgeAsset($this->_steps[1], $this->getCodeName())) {
				return FALSE;
			}
		}

		if (!$this->deleteRemainingTrashLinks($this->_steps[2], $this->getCodeName())) {
			return FALSE;
		}

		return TRUE;

	}//end freestyle()


	/**
	* Puts assets in the trash into a list of assets to delete iff they are linked only in the part of the trash being purged
	*
	* @param array	&$step_data	a reference to the array of information about the current step
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function buildListToDelete(&$step_data, $prefix)
	{
		$GLOBALS['SQ_PURGING_TRASH'] = TRUE;
		if (!empty($this->_running_vars['to_check'])) {
			reset($this->_running_vars['to_check']);
			list($assetid, $type_code) = each($this->_running_vars['to_check']);
			unset($this->_running_vars['to_check'][$assetid]);

			if (strpos($assetid, ':') === FALSE) {
				$db = MatrixDAL::getDb();
				try {
					$bind_vars = Array(
									'treeid'	=> $this->_running_vars['purge_root_treeid'].'%',
									'assetid'	=> $assetid,
								 );
					$num_external_links = MatrixDAL::executeOne('core', 'getPurgeTrashHipoLinkCount', $bind_vars);
				} catch (Exception $e) {
					throw new Exception('Failed to get asset external link count for purge trash HIPO: '.$e->getMessage());
					$num_external_links = -1;
				}

				if ($num_external_links == 0) {
					$this->_running_vars['to_delete'][$assetid] = $type_code;
				} else if ($num_external_links > 0) {
					// This asset has links outside the Trash, therefore all of its children have external links as well.
					// We remove its children's assetids from the 'to_check' list so that we do not have to check them later.
					$children = $GLOBALS['SQ_SYSTEM']->am->getChildren($assetid);
					foreach ($children as $child_assetid => $type_code) {
						if (isset($this->_running_vars['to_check'][$child_assetid])) {
							unset($this->_running_vars['to_check'][$child_assetid]);
						}
					}
				}
			}
		}

		if (empty($this->_running_vars['to_check'])) {
			$GLOBALS['SQ_PURGING_TRASH'] = FALSE;
			$step_data['percent_done'] = 100;
			$step_data['complete']     = TRUE;
			$this->_running_vars['total_to_delete'] = count($this->_running_vars['to_delete']);

			// Now we've got everything sorted, we don't need the delete order.
			// Everything is sorted for us in the to_delete list.
			unset($this->_running_vars['delete_order']);
		} else {
			$step_data['percent_done'] = (int)((($this->_running_vars['total_assets'] - count($this->_running_vars['to_check'])) / $this->_running_vars['total_assets']) * 100);
			$step_data['complete']     = FALSE;
		}

		return TRUE;

	}//end buildListToDelete()


	/**
	* Purge a single asset in the to-delete list from the trash
	*
	* @param array	&$step_data	a reference to the array of information about the current step
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function purgeAsset(&$step_data, $prefix)
	{
		$GLOBALS['SQ_PURGING_TRASH'] = TRUE;
		if (!empty($this->_running_vars['to_delete'])) {
			$assetid    = NULL;
			$asset_info = end($this->_running_vars['to_delete']);
			if ($asset_info !== FALSE) {
				$assetid = key($this->_running_vars['to_delete']);
				unset($this->_running_vars['to_delete'][$assetid]);
			}

			if (!is_null($assetid)) {
				$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $asset_info[0]['type_code']);

				$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
				$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

				// we have an asset ready to delete, but we need to check with
				// the asset manager that we are allowed to delete it
				if (!$GLOBALS['SQ_SYSTEM']->am->canPurgeAsset($asset)) {
					trigger_localised_error('HIPO0004', E_USER_WARNING, $asset->name, $asset->id);
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return FALSE;
				}

				if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($this->_running_vars['trash_id'], 'all') || !$GLOBALS['SQ_SYSTEM']->am->acquireLock($asset->id, 'all', $this->_running_vars['trash_id'])) {
					trigger_localised_error('HIPO0005', E_USER_WARNING, $asset->name, $asset->id);
					$GLOBALS['SQ_SYSTEM']->am->releaseLock($this->_running_vars['trash_id'], 'all');
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return FALSE;
				}

				// Get all the shadow asset links in which this asset is the major party and delete it
				$shadow_links = $GLOBALS['SQ_SYSTEM']->am->getShadowLinkByAsset($asset->id, NULL, NULL, NULL, 'major', TRUE);
				foreach($shadow_links as $shadow_link) {
					$GLOBALS['SQ_SYSTEM']->am->deleteShadowAssetLink($shadow_link['linkid']);
				}

				// because we delete the deepest assets first, it's safe to assume
				// that anything linked under this asset is also linked outside the
				// trash (or it would already have been deleted itself), so we will
				// now delete all links in which this asset is the major party
				$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_SC_LINK_ALL, '', TRUE);
				foreach ($links as $link) {
					$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($link['linkid'], FALSE);
				}

				// the delete function will remove all links in which the asset is the minor party
				if (!$asset->delete(FALSE, FALSE)) {
					trigger_localised_error('HIPO0004', E_USER_WARNING, $asset->name, $asset->id);
					$GLOBALS['SQ_SYSTEM']->am->releaseLock($asset->id, 'all');
					$GLOBALS['SQ_SYSTEM']->am->releaseLock($this->_running_vars['trash_id'], 'all');
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return FALSE;
				}

				$GLOBALS['SQ_SYSTEM']->am->releaseLock($this->_running_vars['trash_id'], 'all');

				$step_data['message'] = translate('hipo_deleting', htmlentities($asset->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET), $assetid);
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);

				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			}//end if
		}//end if

		if (empty($this->_running_vars['to_delete'])) {
			$step_data['percent_done'] = 100;
			$step_data['complete']     = TRUE;
		} else {
			$step_data['percent_done'] = (int)((($this->_running_vars['total_to_delete'] - count($this->_running_vars['to_delete'])) / $this->_running_vars['total_to_delete']) * 100);
			$step_data['complete']     = FALSE;
		}

		$GLOBALS['SQ_PURGING_TRASH'] = FALSE;
		return TRUE;

	}//end purgeAsset()


	/**
	* Delete all remaining links to the trash
	*
	* Links that remain at this point are to assets that are linked elsewhere.
	*
	* @param array	&$step_data	a reference to the array of information about the current step
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function deleteRemainingTrashLinks(&$step_data, $prefix)
	{
		$GLOBALS['SQ_PURGING_TRASH'] = TRUE;
		$links = Array();
		if (empty($this->_running_vars['purge_root_id'])) {
			$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->_running_vars['trash_id'], SQ_SC_LINK_ALL);
		} else {
			// if the root node has not been deleted, the selected purge root link should be.
			$links[] = $GLOBALS['SQ_SYSTEM']->am->getLinkById($this->_running_vars['purge_root_linkid'], $this->_running_vars['purge_root_id'], 'minor');
		}

		// We reverse the array to start deleting the assets with the highest sort-order.
		// This is *much* faster because we don't have to update the sort order  
		// of all the other assets under the trash each time
		foreach (array_reverse($links) as $link) {
			// Extra check to make sure the link is under trash exclusively before deleting the link itself
			$link_info = $GLOBALS['SQ_SYSTEM']->am->getLinkById($link['linkid']);
			if (!is_null($link_info['minorid'])) {
				$under_trash = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($this->_running_vars['trash_id'], $link_info['minorid']);
				if ($GLOBALS['SQ_SYSTEM']->am->assetInTrash($link_info['minorid'], TRUE) || !empty($under_trash)) {
					// we know it has other links so no new trash link will be created
					$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($link['linkid'], FALSE);
				}//end if
			} else {
				$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($link['linkid'], FALSE);
			}//end if
		}//end foreach
		$step_data['percent_done'] = 100;
		$step_data['complete']     = TRUE;
		$GLOBALS['SQ_PURGING_TRASH'] = FALSE;
		return TRUE;

	}//end deleteRemainingTrashLinks()


}//end class
?>
