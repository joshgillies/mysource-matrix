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
* $Id: hipo_job_purge_trash.inc,v 1.15 2004/09/16 11:01:00 brobertson Exp $
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
				);


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
		while (!empty($this->_running_vars['todo_links'])) {
			if (!$this->processTrash($this->_steps[0], $this->getCodeName())) return false;
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

		$this->_running_vars['todo_links']     = $children;
		$this->_running_vars['done_links']     = Array();
		$this->_running_vars['deleted_assets'] = Array();
		$this->_running_vars['trash_treeid']   = $treeid;    // the treeid of the trash folder
		$this->_running_vars['trash_id']       = $trash->id; // the asset id of the trash folder

		return parent::prepare();
	
	}//end prepare()


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

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if (!empty($this->_running_vars['todo_links'])) {

			$link_info = array_shift($this->_running_vars['todo_links']);

			if (!$link_info['linkid']) {
				$step_data['message'] = 'Skipping "'.$asset->name.'" as it has already been deleted';
			} else {
				$assetid = $link_info['assetid'];
				
				if (in_array($assetid, $this->_running_vars['deleted_assets'])) {
					// asset was already deleted
					$step_data['message'] = 'Skipping link #'.$link_info['linkid'].' as it has already been deleted';
				} else {

					$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $link_info['type_code'], true);

					$db = &$GLOBALS['SQ_SYSTEM']->db;
					
					$sql = 'SELECT t2.treeid, l1.linkid, l2.minorid, l2.link_type, l2.dependant
							FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_link l1 INNER JOIN 
								 '.SQ_TABLE_RUNNING_PREFIX.'asset_link_tree t1 ON l1.linkid = t1.linkid,
								 '.SQ_TABLE_RUNNING_PREFIX.'asset_link l2 INNER JOIN
								 '.SQ_TABLE_RUNNING_PREFIX.'asset_link_tree t2 ON l2.linkid = t2.linkid,
								 '.SQ_TABLE_RUNNING_PREFIX.'asset a 
							WHERE l1.minorid = '.$db->quote($asset->id).'
							  AND SUBSTRING(t1.treeid FROM 1 FOR CHAR_LENGTH(t2.treeid)) = t2.treeid
							  AND a.assetid=l2.minorid
							  AND t2.treeid <> \'\'
							ORDER BY t2.treeid DESC';

					$other_links = $db->getAll($sql);
					if (DB::isError($other_links)) {
						trigger_error($other_links->getMessage().'<br/>'.$other_links->getUserInfo(), E_USER_WARNING);
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						return false;
					}
					
					$outside_links = Array();
					$found_ourself = false;
					$inside_links = Array();
					$last_linkid = 0;

					foreach ($other_links as $outside_link_info) {
						$found_ourself_this_run = false;
						if ($outside_link_info['minorid'] == $asset->id) $found_ourself_this_run = true;

						// check if we are looking at links that are in the trash
						if (substr($outside_link_info['treeid'], 0, strlen($this->_running_vars['trash_treeid'])) == $this->_running_vars['trash_treeid']) {
							if ($outside_link_info['minorid'] == $asset->id) $inside_links[] = $outside_link_info['linkid'];
							continue;
						}
						
						if ($found_ourself_this_run) {
							$found_ourself = true;
							// if we still have a linkid in our buffer, add it into the list
							// of outside links so we dont lose it
							if ($last_linkid) $outside_links[] = $last_linkid;
							
							// at this point, we know this link is not in the trash because we would have
							// continued in the above IF statement, so add it to the buffer
							$last_linkid = $outside_link_info['linkid'];
						}

						if (!$found_ourself) continue;
						
						// we dont care about dependant links because we want to
						// go right to the top of the dependant tree
						if ($outside_link_info['dependant']) continue;
						if ($outside_link_info['link_type'] & SQ_SC_LINK_SIGNIFICANT) {
							if ($last_linkid) $outside_links[] = $last_linkid;
						}
						$last_linkid = 0;
						$found_ourself = false;
					}//end foreach
					$inside_links = count(array_unique($inside_links));

					// if there are no links inside the trash, chances are we already deleted them, so skip this asset
					if (!$inside_links) {
						$step_data['message'] = 'Skipping asset #'.$asset->id.' as it no longer has any links inside the trash';
					} else {
					
						// if the link we are deleting is also one of the outside
						// links, then we leave this link alone
						if (in_array($link_info['linkid'], $outside_links)) {
							$step_data['message'] = 'Skipping link #'.$link_info['linkid'].' as it also exists outside of the trash';
						} else {

							// we need to check how many internal links
							// there are before deciding if we should delete the asset or not
							$num_other_links = $GLOBALS['SQ_SYSTEM']->am->countLinks($asset->id, 'minor', SQ_SC_LINK_BACKEND_NAV, '', true, $link_info['linkid']);

							if (count($outside_links) >= 1 || $num_other_links || $inside_links > 1) {

								// this asset has more than one link, so just delete the link
								$link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($link_info['linkid'], $asset->id, 'minor');
								
								if (empty($link['majorid'])) {
									// if the link does not exists it was probably deleted
									// earlier, so we just skip this entry
									$step_data['message'] = 'Skipping link #'.$link_info['linkid'].' as it has already been deleted';
								} else {
									$parent = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['majorid'], $link['major_type_code']);
									if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($asset->id, 'links') || !$GLOBALS['SQ_SYSTEM']->am->acquireLock($parent->id, 'links')) {
										trigger_error('Purge aborted, unable to lock the links of either "'.$asset->name.'" or its parent "'.$parent->name.'"', E_USER_WARNING);
										$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
										return false;
									}
									
									if (!$parent->deleteLink($link_info['linkid'])) {
										trigger_error('Purge aborted, unable to delete link '.$link_info['linkid'].' between "'.$parent->name.'" and "'.$asset->name.'"', E_USER_WARNING);

										$GLOBALS['SQ_SYSTEM']->am->releaseLock($asset->id, 'links');
										$GLOBALS['SQ_SYSTEM']->am->releaseLock($parent->id, 'links');

										$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
										return false;
									}
									$GLOBALS['SQ_SYSTEM']->am->releaseLock($asset->id, 'links');
									$GLOBALS['SQ_SYSTEM']->am->releaseLock($parent->id, 'links');

									$step_data['message'] = 'Deleting link #'.$link_info['linkid'];
								}
	
							} else {

								// we have an asset ready to delete, but we need to check with
								// the asset manager that we are allowed to delete it
								if (!$GLOBALS['SQ_SYSTEM']->am->canPurgeAsset($asset)) {
									trigger_error('Purge aborted, unable to delete "'.$asset->name.'" ['.$asset->id.']', E_USER_WARNING);
									$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
									return false;
								}
	
								if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($this->_running_vars['trash_id'], 'all') || !$GLOBALS['SQ_SYSTEM']->am->acquireLock($asset->id, 'all', $this->_running_vars['trash_id'])) {
									trigger_error('Purge aborted, unable to lock the asset "'.$asset->name.'"', E_USER_WARNING);
									$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
									return false;
								}

								// this is the last link for this asset, so lets delete it for good
								if (!$asset->delete(false)) {
									trigger_error('Purge aborted, unable to delete "'.$asset->name.'" ['.$asset->id.']', E_USER_WARNING);
									$GLOBALS['SQ_SYSTEM']->am->releaseLock($asset->id, 'all');
									$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
									return false;
								}

								$GLOBALS['SQ_SYSTEM']->am->releaseLock($this->_running_vars['trash_id'], 'all');

								$step_data['message'] = 'Deleting "'.$asset->name.'" [#'.$assetid.']';
								$this->_running_vars['deleted_assets'][] = $assetid;
								$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
							}
						}

					}//end if no inside links
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

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}//end processTrash()


}//end class

?>
