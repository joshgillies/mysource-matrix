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
* $Id: hipo_job_purge_trash.inc,v 1.7 2004/02/11 18:47:49 brobertson Exp $
* $Name: not supported by cvs2svn $
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
* @version $Version$ - 1.0
* @package MySource_Matrix
* @subpackage HIPO
*/
class HIPO_Job_Purge_Trash extends HIPO_Job
{

	/**
	* The name of this HIPO job (should not be modified by code)
	* @var string
	*/
	var $hipo_name = 'Purge Trash';

	/**
	* Array of steps to perform in this HIPO
	* @var Array
	*/
	var $_steps = Array(
					Array(
						'name'			=> 'Acquiring Locks',
						'hipo_job'		=> Array(
												'init_details_function' => 'getLockAssetDetails',
												),
						'running_mode'	=> 'server',
						'auto_step'		=> true,
						'percent_done'	=> 0,
						'complete'		=> false,
						'message'		=> '',
					),
					Array(
						'name'             => 'Purging Trash',
						'function_call'    => Array(
												'process_function' => 'processTrash',
												),
						'running_mode'     => 'server',
						'auto_step'        => true,
						'percent_done'     => 0,
						'complete'         => false,
						'message'          => '',
					),
					Array(
						'name'			=> 'Releasing Locks',
						'function_call'    => Array(
												'process_function' => 'releaseLocks',
												),
						'running_mode'	=> 'server',
						'auto_step'		=> true,
						'percent_done'	=> 0,
						'complete'		=> false,
						'message'		=> '',
					),
				);


	/**
	* Constructor
	*
	* @param string	$code_name	a unique codename the HIPO
	*/
	function HIPO_Job_Purge_Trash($code_name='')
	{
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
		$o->openField('Purge Trash Threshold');

		if($write_access) {
			text_box($class.'[SQ_HIPO_PURGE_TRASH_THRESHOLD]', SQ_HIPO_PURGE_TRASH_THRESHOLD, 5);
		} else {
			echo SQ_HIPO_PURGE_TRASH_THRESHOLD;
		}

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
			'SQ_HIPO_PURGE_TRASH_THRESHOLD' => Array('editable' => 1, 'default' => 1),
		);

	}//end getConfigVars()


	/**
	* Returns the current Threshold.
	*
	* Returns a value that represents the total percentage that this hipo would run in freestyle mode in its current state.
	* 
	* @return float
	* @access public
	*/
	function getThreshold()
	{
		if (SQ_HIPO_PURGE_TRASH_THRESHOLD == 0) return 0;
		if (!isset($this->_running_vars['todo_assetids'])) return 0;
		return ((count($this->_running_vars['todo_assetids']) / SQ_HIPO_PURGE_TRASH_THRESHOLD) * 100);
	
	}//end getThreshold()


	/**
	* Performs the duties of this hipo, without showing any output on the frontend.
	* 
	* @return boolean
	* @access public
	*/
	function freestyle()
	{
		while (!empty($this->_running_vars['todo_assetids'])) {
			if (!$this->processTrash($this->_steps[0], get_class($this))) return false;
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
		
		require_once SQ_INCLUDE_PATH.'/asset_manager_edit_fns.inc';
		$children = Asset_Manager_Edit_Fns::getAllChildLinks($trash->id);
		$children = array_reverse($children);

		// work out the treeid of the trash asset for use later
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = 'SELECT t.treeid
				FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_link_tree t INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'asset_link l ON t.linkid = l.linkid ';
		$where = 'l.minorid = '.$db->quote($trash->id);
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 't');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
		$sql .= $where.' LIMIT 1';

		$treeid = $db->getOne($sql);
		if (DB::isError($treeid)) {
			trigger_error($treeid->getMessage().'<br/>'.$treeid->getUserInfo(), E_USER_ERROR);
			return false;
		}

		$this->_running_vars['todo_links']    = $children;
		$this->_running_vars['done_links']    = Array();
		$this->_running_vars['trash_treeid']  = $treeid;    // the treeid of the trash folder
		$this->_running_vars['trash_id']      = $trash->id; // the asset id of the trash folder

		return parent::prepare();
	
	}//end prepare()


	/**
	* Set up vars and database information that the HIPO will need to run
	*
	* @param string	$source_code_name	the code name of another job that wants to use this job
	*
	* @return string	returns the code_name for the new job
	* @access public
	*/
	function initialise($source_code_name=null)
	{
		if (!$this->is_prepared) {
			$this->prepare();
		}
		return parent::initialise($source_code_name);

	}//end initialise()


	/**
	* Purge a single asset from the trash
	*
	* @param array()	&$step_data	a reference to the array of information about the current step
	* @param string		$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function processTrash(&$step_data, $prefix)
	{
		// let everyone know we are purging the trash
		$GLOBALS['SQ_PURGING_TRASH'] = true;

		if (!empty($this->_running_vars['todo_links'])) {

			$link_info = array_shift($this->_running_vars['todo_links']);

			if (!$link_info['linkid']) {
				$step_data['message'] = 'Skipping "'.$asset->name.'" as it has already been deleted';
			} else {
				$assetid = $link_info['assetid'];
				$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $link_info['type_code'], true);
				if (is_null($asset)) {
					// if the asset doesnt exist it was probably deleted earlier on
					// thats why we mute errors above
					$step_data['message'] = 'Skipping link #'.$link_info['linkid'].' as it has already been deleted';
				} else {

					$db = &$GLOBALS['SQ_SYSTEM']->db;
					
					$num_other_links = 0;
					$outside_links = Array();

					// we are going to try and find out if this asset has any links
					// outside the trash folder so we dont delete it if it is being used
					// somewhere else outside the trash
					$sql = 'SELECT l.linkid 
							FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_link_tree t INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'asset_link l ON t.linkid = l.linkid ';
					$where = 'l.minorid='.$db->quote($link_info['assetid']).' 
							  AND t.treeid NOT LIKE '.$db->quote($this->_running_vars['trash_treeid'].'%').' 
							  AND (l.link_type & '.$db->quote(SQ_SC_LINK_SIGNIFICANT).') > 0';
					$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 't');
					$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');

					$outside_links = $db->getCol($sql.$where);
					if (DB::isError($outside_links)) {
						trigger_error($outside_links->getMessage().'<br/>'.$outside_links->getUserInfo(), E_USER_WARNING);
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						return false;
					}

					// if the link we are deleting is also one of the outside
					// links, then we leave this link alone
					if (in_array($link_info['linkid'], $outside_links)) {
						$step_data['message'] = 'Skipping link #'.$link_info['linkid'].' as it also exists outside of the trash';
					} else {

						// if there are outside links, we are not going to delete the asset
						// but if there are not, then we need to check how many internal links
						// there are before deciding if we should delete the asset or not
						if (!count($outside_links)) {
							$num_other_links = $GLOBALS['SQ_SYSTEM']->am->countLinks($asset->id, 'minor', SQ_SC_LINK_SIGNIFICANT, '', true, $link_info['linkid']);
						}

						if ($num_other_links || count($outside_links)) {
							// this asset has more than one link,
							// or has another link outside of the trash
							// so just delete the link
							$link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($link_info['linkid'], $asset->id, 'minor');
							
							if (empty($link['majorid'])) {
								// if the link does not exists it was probably deleted
								// earlier, so we just skip this entry
								$step_data['message'] = 'Skipping link #'.$link_info['linkid'].' as it has already been deleted';
							} else {
								$parent = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['majorid'], $link['major_type_code']);
								if (!$parent->deleteLink($link_info['linkid'])) {
									trigger_error('Purge aborted, unable to delete link '.$link_info['linkid'].' between "'.$parent->name.'" and "'.$asset->name.'"', E_USER_WARNING);
									return false;
								}
								$step_data['message'] = 'Deleting link #'.$link_info['linkid'];
							}

						} else {
							// we have an asset ready to delete, but we need to check with
							// the asset manager that we are allowed to delete it
							if (!$GLOBALS['SQ_SYSTEM']->am->canPurgeAsset($asset)) {
								trigger_error('Purge aborted, unable to delete "'.$asset->name.'" ['.$asset->id.']', E_USER_WARNING);
								return false;
							}

							// this is the last link for this asset, so lets delete it for good
							if (!$asset->delete(false)) {
								trigger_error('Purge aborted, unable to delete "'.$asset->name.'" ['.$asset->id.']', E_USER_WARNING);
								return false;
							}
							$step_data['message'] = 'Deleting "'.$asset->name.'" [#'.$assetid.']';
						}
					}
				}
			}
		}

		$this->_running_vars['done_links'][] = $link_info;

		$GLOBALS['SQ_PURGING_TRASH'] = false;

		if (empty($this->_running_vars['todo_links'])) {
			$step_data['percent_done'] = 100;
			$step_data['complete']     = true;
		} else {
			$total = count($this->_running_vars['todo_links']) + count($this->_running_vars['done_links']);
			$step_data['percent_done'] =(count($this->_running_vars['done_links']) / $total) * 100;
			$step_data['complete']     = false;
		}

		return true;

	}//end processTrash()


	/**
	* This function is used to set-up the details for the locking the assets by the nested HIPO_Job
	*
	* @param string		&$job_type
	* @param Array()	&$running_vars
	* @param Array()	&$options
	*
	* @return void
	* @access public
	*/
	function getLockAssetDetails(&$job_type, &$running_vars, &$options)
	{
		$job_type = 'hipo_job_acquire_lock';

		$options['auto_complete'] = true;

		$running_vars['assetid']         = $this->_running_vars['trash_id'];
		$running_vars['lock_type']       = 'all';
		$running_vars['dependants_only'] = false;

	}//end getLockAssetDetails()


	/**
	* Release the locks we acquired while setting the permissions
	*
	* @param array()	&$step_data	a reference to the array of information about the current step
	* @param string		$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function releaseLocks(&$step_data, $prefix)
	{
		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['trash_id']);
		if (!is_null($asset)) {
			$GLOBALS['SQ_SYSTEM']->am->releaseLock($asset->id, 'all');
			$step_data['message'] = 'Releasing Locks';
		}
		$step_data['percent_done'] = 100;
		$step_data['complete']     = true;
		return true;

	}//end releaseLocks()

}//end class

?>