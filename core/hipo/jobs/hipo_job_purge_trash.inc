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
* $Id: hipo_job_purge_trash.inc,v 1.28.2.2 2005/07/08 12:55:23 gnoel Exp $
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
* @version $Revision: 1.28.2.2 $
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
		$this->uses_trans = false;			// we'll do our own transactions thanks
		$this->HIPO_Job($code_name);

	}//end constructor


	/**
	* Paints backend of this job
	*
	* @param object Backend_Outputter	&$o			reference to the backend outputter
	* @param string						$class		calling class name
	*
	* @return void
	* @access private
	*/
	function paintConfig(&$o, $class)
	{
		$write_access = $this->writeAccess();

		// metadata regeneration threshhold HIPO config entry
		$o->openField(translate('purge_trash_threshold'));

		if ($write_access) {
			text_box($class.'[SQ_HIPO_PURGE_TRASH_THRESHOLD]', SQ_HIPO_PURGE_TRASH_THRESHOLD, 5);
		} else {
			echo SQ_HIPO_PURGE_TRASH_THRESHOLD;
		}
		echo ' '.translate('assets');

		$o->closeField();

	}//end paintConfig()


	/**
	* Sets some info about thresholds that this hipo requires.
	*
	* @return Array
	* @access public
	*/
	function getConfigVars()
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
					'name'			=> translate('hipo_remove_externally_linked_assets'),
					'function_call'	=> Array(
										'process_function'	=> 'removeExternallyLinkedAssetFromTrash',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> true,
					'allow_cancel'	=> true,
					'percent_done'	=> 0,
					'complete'		=> false,
					'message'		=> '',
				),
				Array(
					'name'			=> translate('hipo_purging_trash'),
					'function_call'	=> Array(
										'process_function'	=> 'purgeAssetFromTrash',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> true,
					'allow_cancel'	=> true,
					'percent_done'	=> 0,
					'complete'		=> false,
					'message'		=> '',
				),
			   );

	}//end getInitialStepData()


	/**
	* Returns the current Threshold.
	*
	* Returns a value that represents the total percentage that this hipo would run in freestyle mode in its current state.
	*
	* @return float
	* @access public
	*/
	function getThresholdPercentageRequired()
	{
		if (SQ_HIPO_PURGE_TRASH_THRESHOLD == 0) return 0;
		if (!isset($this->_running_vars['todo_assetids'])) {
			return 0;
		}
		return ((count($this->_running_vars['todo_assetids']) / SQ_HIPO_PURGE_TRASH_THRESHOLD) * 100);

	}//end getThresholdPercentageRequired()


	/**
	* Performs the duties of this hipo, without showing any output on the frontend.
	*
	* @return boolean
	* @access public
	*/
	function freestyle()
	{
		while (!empty($this->_running_vars['todo_links'])) {
			if (!$this->removeExternallyLinkedAssetFromTrash($this->_steps[0], $this->getCodeName())) {
				return false;
			}
		}

		return true;

	}//end freestyle()


	/**
	* Prepares the running vars and gets the children of the asset in question.
	*
	* @return boolean
	* @access public
	*/
	function prepare()
	{
		$trash = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('trash_folder');

		$children = $GLOBALS['SQ_SYSTEM']->am->getAllChildLinks($trash->id);
		$children = array_reverse($children);

		// work out the treeid of the trash asset for use later
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = 'SELECT t.treeid
				FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk_tree t INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l ON t.linkid = l.linkid ';
		$where = 'l.minorid = '.$db->quote($trash->id);
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 't');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
		$sql .= $where.' LIMIT 1';

		$treeid = $db->getOne($sql);
		if (!assert_valid_db_result($treeid)) return false;

		$this->_running_vars['todo_links']     = $children;
		$this->_running_vars['done_links']     = Array();
		$this->_running_vars['trash_treeid']   = $treeid;    // the treeid of the trash folder
		$this->_running_vars['trash_id']       = $trash->id; // the asset id of the trash folder
		$this->_running_vars['to_delete_links'] = Array();

		return parent::prepare();

	}//end prepare()


	/**
	* Checks and removes asset from trash if it has any external links.
	*
	* If asset has external links then it deletes the link between the asset and trash and if
	* no then it pushes the link information into running_vars['to_delete_links']
	*
	* @param array()	&$step_data	a reference to the array of information about the current step
	* @param string		$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function removeExternallyLinkedAssetFromTrash(&$step_data, $prefix)
	{
		$GLOBALS['SQ_PURGING_TRASH'] = true;

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if (!empty($this->_running_vars['todo_links'])) {
			$link_info = array_shift($this->_running_vars['todo_links']);
			if (!$link_info['linkid']) {
				$step_data['message'] = translate('hipo_skipping', $asset->name, translate('hipo_skipping_already_deleted'));
			} else {
				$assetid = $link_info['assetid'];
				$externally_linked_assetids = Array();
				// get asset and check its external links
				$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $link_info['type_code'], true);
				$db = &$GLOBALS['SQ_SYSTEM']->db;
				$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_SC_LINK_SIGNIFICANT, null, false, 'minor');
				$delete_link = false;

				foreach ($links as $link) {
					// checking if the asset has any external links other than trash
					if (($link['majorid']!=$this->_running_vars['trash_id']) && (!is_null($GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($asset->id, $this->_running_vars['trash_id'], SQ_SC_LINK_SIGNIFICANT, null, 'minor')))){
						$delete_link = true;
						break;
					}
				}// end of foreach

				if ($delete_link) {
					// asset has any external link then popping out its children from running_vars['todo_links']
					$children = $GLOBALS['SQ_SYSTEM']->am->getAllChildLinks($asset->id);
					foreach ($children as $child) {
						foreach ($this->_running_vars['todo_links'] as $link_key => $link_details) {
							if ($link_details['linkid'] == $child['linkid']) {
								unset($this->_running_vars['todo_links'][$link_key]);
							}
						}
					}// end foreach checking children

					// deleting the link between the asset and trash as it has an external link
					$result = $GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($link_info['linkid']);
					$step_data['message'] = translate('hipo_deleting_link', $link_info['linkid']);
				} else {
					// if the array has no external link then it is pushed into running_vars['to delete links']
					array_push($this->_running_vars['to_delete_links'], $link_info);
				}// end of cheecking $delete_link

			}// end of else where linkid is not null
			// we have finished checking the current link.. so pushing this link_info into the done_links array
			$this->_running_vars['done_links'][] = $link_info;
		}// end of if where _running_vars['todo_links'] is checked empty or not

		if (empty($this->_running_vars['todo_links'])) {
			$GLOBALS['SQ_PURGING_TRASH'] = false;
			$step_data['percent_done'] = 100;
			$step_data['complete']     = true;
		} else {
			$total = count($this->_running_vars['to_delete_links']) + count($this->_running_vars['done_links']);
			$step_data['percent_done'] =(count($this->_running_vars['done_links']) / $total) * 100;
			$step_data['complete']     = false;
		}
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;
	}//end removeExternallyLinkedAssetFromTrash()


	/**
	* Purge a single asset from the trash
	*
	* @param array()	&$step_data	a reference to the array of information about the current step
	* @param string		$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function purgeAssetFromTrash(&$step_data, $prefix)
	{

		// let everyone know we are purging the trash
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$link_info = array_shift($this->_running_vars['to_delete_links']);

		if (!$link_info['linkid']) {
				$step_data['message'] = translate('hipo_skipping', $link_info['short_name'], translate('hipo_skipping_already_deleted'));
		} else {
			$assetid = $link_info['assetid'];

			$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $link_info['type_code'], true);
			// we have an asset ready to delete, but we need to check with
			// the asset manager that we are allowed to delete it
			if (!$GLOBALS['SQ_SYSTEM']->am->canPurgeAsset($asset)) {
				trigger_localised_error('HIPO0004', E_USER_WARNING, $asset->name, $asset->id);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}

			if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($this->_running_vars['trash_id'], 'all') || !$GLOBALS['SQ_SYSTEM']->am->acquireLock($asset->id, 'all', $this->_running_vars['trash_id'])) {
				trigger_localised_error('HIPO0005', E_USER_WARNING, $asset->name, $asset->id);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}
			// this is the last link for this asset, so lets delete it for good
			if (!$asset->delete(false)) {
				trigger_localised_error('HIPO0004', E_USER_WARNING, $asset->name, $asset->id);
				$GLOBALS['SQ_SYSTEM']->am->releaseLock($asset->id, 'all');
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}

			$GLOBALS['SQ_SYSTEM']->am->releaseLock($this->_running_vars['trash_id'], 'all');

			$step_data['message'] = translate('hipo_deleting', $asset->name, $assetid);
			$this->_running_vars['deleted_assets'][] = $assetid;
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
		}

		if (empty($this->_running_vars['to_delete_links'])) {
			$GLOBALS['SQ_PURGING_TRASH'] = false;
			$step_data['percent_done'] = 100;
			$step_data['complete']     = true;
		} else {
			$total = count($this->_running_vars['to_delete_links']) + count($this->_running_vars['done_links']);
			$step_data['percent_done'] =(count($this->_running_vars['done_links']) / $total) * 100;
			$step_data['complete']     = false;
		}
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}//end purgeAssetFromTrash()


}//end class

?>
