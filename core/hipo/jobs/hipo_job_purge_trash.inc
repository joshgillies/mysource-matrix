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
* $Id: hipo_job_purge_trash.inc,v 1.38 2006/01/26 22:34:08 lwright Exp $
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
* @version $Revision: 1.38 $
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
		$this->uses_trans = false;

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
					'name'			=> translate('hipo_purging_trash_determining'),
					'function_call'	=> Array(
										'process_function'	=> 'buildListToDelete',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> true,
					'allow_cancel'	=> true,
					'percent_done'	=> 0,
					'complete'		=> false,
					'message'		=> '',
				),
				Array(
					'name'			=> translate('hipo_purging_trash_deleting'),
					'function_call'	=> Array(
										'process_function'	=> 'purgeAsset',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> true,
					'allow_cancel'	=> true,
					'percent_done'	=> 0,
					'complete'		=> false,
					'message'		=> '',
				),
				Array(
					'name'			=> translate('hipo_purging_trash_cleaning'),
					'function_call'	=> Array(
										'process_function'	=> 'deleteRemainingTrashLinks',
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
		$trash = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('trash_folder');

		$this->_running_vars['to_check'] = $GLOBALS['SQ_SYSTEM']->am->getChildren($trash->id);
		$this->_running_vars['total_assets'] = count($this->_running_vars['to_check']);
		$this->_running_vars['to_delete'] = Array();
		$this->_running_vars['trash_id'] = $trash->id;
		$this->_running_vars['trash_treeid'] = reset($GLOBALS['SQ_SYSTEM']->am->getAssetTreeids($trash->id));

		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$sql = 'SELECT a.assetid, length(t.treeid)
				FROM sq_ast_lnk_tree t
					INNER JOIN sq_ast_lnk l ON t.linkid = l.linkid
					JOIN sq_ast a ON l.minorid = a.assetid
				WHERE t.treeid like '.$db->quote($this->_running_vars['trash_treeid'].'%').'
				ORDER by length(t.treeid)';
		$this->_running_vars['delete_order'] = $db->getCol($sql);
		assert_valid_db_result($this->_running_vars['delete_order']);

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
				return false;
			}
		}

		while (!empty($this->_running_vars['to_delete'])) {
			if (!$this->purgeAsset($this->_steps[1], $this->getCodeName())) {
				return false;
			}
		}

		if (!$this->deleteRemainingTrashLinks($this->_steps[2], $this->getCodeName())) {
			return false;
		}

		return true;

	}//end freestyle()


	/**
	* Puts assets in the trash into a list of assets to delete iff they are linked only in the trash
	*
	* @param array	&$step_data	a reference to the array of information about the current step
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function buildListToDelete(&$step_data, $prefix)
	{
		$GLOBALS['SQ_PURGING_TRASH'] = true;

		if (!empty($this->_running_vars['to_check'])) {
			reset($this->_running_vars['to_check']);
			list($assetid, $type_code) = each($this->_running_vars['to_check']);
			unset($this->_running_vars['to_check'][$assetid]);

			$db =& $GLOBALS['SQ_SYSTEM']->db;
			$sql = 'SELECT COUNT(l.linkid)
					FROM sq_ast_lnk_tree t INNER JOIN sq_ast_lnk l ON t.linkid = l.linkid
					WHERE l.minorid = '.$db->quote($assetid).'
					AND NOT treeid LIKE '.$db->quote($this->_running_vars['trash_treeid'].'%');
			$num_external_links = $db->getOne($sql);
			assert_valid_db_result($num_external_links);
			if ($num_external_links == 0) {
				$this->_running_vars['to_delete'][$assetid] = $type_code;
			}
		}

		if (empty($this->_running_vars['to_check'])) {
			$GLOBALS['SQ_PURGING_TRASH'] = false;
			$step_data['percent_done'] = 100;
			$step_data['complete']     = true;
			$this->_running_vars['total_to_delete'] = count($this->_running_vars['to_delete']);
		} else {
			$step_data['percent_done'] = (int)((($this->_running_vars['total_assets'] - count($this->_running_vars['to_check'])) / $this->_running_vars['total_assets']) * 100);
			$step_data['complete']     = false;
		}

		return true;

	}//end buildListToDelete()


	/**
	* Purge a single asset in the to-delete list from the trash
	*
	* @param array()	&$step_data	a reference to the array of information about the current step
	* @param string		$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function purgeAsset(&$step_data, $prefix)
	{
		$GLOBALS['SQ_PURGING_TRASH'] = true;
		if (!empty($this->_running_vars['to_delete'])) {

			$assetid = null;
			while (!isset($this->_running_vars['to_delete'][$assetid])) {
				$assetid = array_pop($this->_running_vars['delete_order']);
				if (is_null($assetid)) break;
			}
			if (!is_null($assetid)) {
				$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $this->_running_vars['to_delete'][$assetid]);
				unset($this->_running_vars['to_delete'][$assetid]);

				$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
				$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

				// we have an asset ready to delete, but we need to check with
				// the asset manager that we are allowed to delete it
				if (!$GLOBALS['SQ_SYSTEM']->am->canPurgeAsset($asset)) {
					trigger_localised_error('HIPO0004', E_USER_WARNING, $asset->name, $asset->id);
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return false;
				}

				if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($this->_running_vars['trash_id'], 'all') || !$GLOBALS['SQ_SYSTEM']->am->acquireLock($asset->id, 'all', $this->_running_vars['trash_id'])) {
					trigger_localised_error('HIPO0005', E_USER_WARNING, $asset->name, $asset->id);
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return false;
				}

				// because we delete the deepest assets first, it's safe to assume
				// that anything linked under this asset is also linked outside the
				// trash (or it would already have been deleted itself), so we will
				// now delete all links in which this asset is the major party
				$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_SC_LINK_ALL, '', true);
				foreach ($links as $link) {
					$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($link['linkid']);
				}

				// the delete function will remove all links in which the asset is the minor party
				if (!$asset->delete(false)) {
					trigger_localised_error('HIPO0004', E_USER_WARNING, $asset->name, $asset->id);
					$GLOBALS['SQ_SYSTEM']->am->releaseLock($asset->id, 'all');
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return false;
				}

				$GLOBALS['SQ_SYSTEM']->am->releaseLock($this->_running_vars['trash_id'], 'all');

				$step_data['message'] = translate('hipo_deleting', $asset->name, $assetid);
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);

				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			}
		}

		if (empty($this->_running_vars['to_delete'])) {
			$step_data['percent_done'] = 100;
			$step_data['complete']     = true;
		} else {
			$step_data['percent_done'] = (int)((($this->_running_vars['total_to_delete'] - count($this->_running_vars['to_delete'])) / $this->_running_vars['total_to_delete']) * 100);
			$step_data['complete']     = false;
		}

		$GLOBALS['SQ_PURGING_TRASH'] = false;
		return true;

	}//end purgeAsset()


	/**
	* Delete all remaining links to the trash
	*
	* Links that remain at this point are to assets that are linked elsewhere.
	*
	* @param array()	&$step_data	a reference to the array of information about the current step
	* @param string		$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function deleteRemainingTrashLinks(&$step_data, $prefix)
	{
		$GLOBALS['SQ_PURGING_TRASH'] = true;
		$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->_running_vars['trash_id'], SQ_SC_LINK_ALL);

		foreach ($links as $link) {
			$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($link['linkid']);  // we know it has other links so no new trash link will be created
		}
		$step_data['percent_done'] = 100;
		$step_data['complete']     = true;
		$GLOBALS['SQ_PURGING_TRASH'] = false;
		return true;

	}//end deleteRemainingTrashLinks()


}//end class
?>
