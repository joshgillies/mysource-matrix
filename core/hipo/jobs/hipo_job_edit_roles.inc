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
* $Id: hipo_job_edit_roles.inc,v 1.4 2007/02/18 23:34:51 rhoward Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';

/**
* Highly Intensive Processing Object (HIPO) to edit the roles of multiple assets
*
* <pre>
* Initial Running Vars :
*    array  assets              -  the IDs of the assets we are modifying roles for in the form :
*                                  $assetids = Array([assetid] => Array(
																	'type_code' => [type_code],
																  ),
*    array  role_changes        -  the values that we are trying to set for our children, in the form :
*                                  $changse = Array(
* 												'add'		=> Array(
																[role_assetid] => Array([userids])
															   ),
* 												'delete'	=> Array(
*																[role_assetid] => Array([userids])
*															   ),
*											  );
*    bool  dependants_only      -  whether or not to cascade the role changes down the tree
* </pre>
*
* @author  Nathan de Vries <ndvries@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix
* @subpackage HIPO
*/
class HIPO_Job_Edit_Roles extends HIPO_Job
{


	/**
	* Constructor
	*
	* @param string	$code_name		a unique codename the HIPO
	*/
	function HIPO_Job_Edit_Roles($code_name='')
	{
		$this->HIPO_Job($code_name);

	}//end constructor


	/**
	* Returns a unique codename the HIPO can use to ensure it is not being run twice
	*
	* @return string
	* @access public
	*/
	function getCodeName()
	{
		return parent::getCodeName().'-'.md5(implode('-',array_keys($this->_running_vars['assets'])));

	}//end getCodeName()


	/**
	* Returns the (localised) HIPO name
	*
	* @return string
	* @access public
	*/
	function getHipoName()
	{
		return translate('hipo_name_edit_roles');

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
					'name'			=> translate('hipo_acquiring_role_locks'),
					'hipo_job'		=> Array(
										'init_details_function'	=> 'getLockAssetDetails',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> TRUE,
					'allow_cancel'	=> TRUE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
				),
				Array(
					'name'			=> translate('hipo_updating_roles'),
					'function_call'	=> Array(
										'process_function'	=> 'processRoles',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> TRUE,
					'allow_cancel'	=> TRUE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> FALSE,
				),
				Array(
					'name'			=> translate('hipo_releasing_role_locks'),
					'function_call'	=> Array(
										'process_function'	=> 'releaseLocks',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> TRUE,
					'allow_cancel'	=> FALSE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
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
		if (!isset($this->_running_vars['todo_assetids'])) {
			return 0;
		}
		return ((count($this->_running_vars['todo_assetids']) / count($this->_running_vars['role_changes'])) * 100);

	}//end getThresholdPercentageRequired()


	/**
	* Performs the duties of this hipo, without showing any output on the frontend.
	*
	* @return boolean
	* @access public
	*/
	function freestyle()
	{
		while (!empty($this->_running_vars['todo_assetids'])) {
			$id = key($this->_running_vars['todo_assetids']);
			$result = $GLOBALS['SQ_SYSTEM']->am->acquireLock($id, 'roles');
			if ($result) {
				if (!$this->processRoles($this->_steps[1], get_class($this))) {
					return FALSE;
				}
				if ($result == 1) {
					$result = $GLOBALS['SQ_SYSTEM']->am->releaseLock($id, 'roles');
				}
			} else {
				return FALSE;
			}
		}
		return TRUE;

	}//end freestyle()


	/**
	* Prepares the running vars and gets the children of the assets in question.
	*
	* @return boolean
	* @access public
	*/
	function prepare()
	{
		if (empty($this->_running_vars['assets'])) {
			trigger_localised_error('HIPO0075', E_USER_WARNING);
			return '';
		}

		if (empty($this->_running_vars['role_changes'])) {
			$this->_running_vars['role_changes'] = Array();
		}

		$assets = $this->_running_vars['assets'];
		$child_assets = Array();
		for (reset($assets); NULL !== ($assetid = key($assets)); next($assets)) {
			$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($assetid));
			if (empty($asset_info)) {
				trigger_localised_error('HIPO0013', E_USER_WARNING, $assetid);
				return '';
			}

			if (array_get_index($this->_running_vars, 'dependants_only')) {
				$child_assets += $GLOBALS['SQ_SYSTEM']->am->getDependantChildren($assetid);
			} else {
				$child_assets += $GLOBALS['SQ_SYSTEM']->am->getChildren($assetid);
			}

			$child_assets[$assetid] = $asset_info[$assetid]['type_code'];
		}

		$child_assets = array_reverse($child_assets,TRUE);
		$this->_running_vars['todo_assetids'] = $child_assets;
		$this->_running_vars['done_assetids'] = Array();
		return parent::prepare();

	}//end prepare()


	/**
	* Edit roles for a single asset in the to-do list
	*
	* @param array	&$step_data	a reference to the array of information about the current step
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function processRoles(&$step_data, $prefix)
	{
		if (!empty($this->_running_vars['todo_assetids'])) {
			$assetid = array_shift(array_keys($this->_running_vars['todo_assetids']));
			$asset_type = $this->_running_vars['todo_assetids'][$assetid];
			unset($this->_running_vars['todo_assetids'][$assetid]);
			$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $asset_type);

			if (!is_null($asset)) {
				// check we can change roles, otherwise silently pass by
				if ($asset->adminAccess('roles')) {
					foreach ($this->_running_vars['role_changes'] as $action => $action_data) {
						foreach ($action_data as $roleid => $userids) {
							foreach ($userids as $userid) {
								switch ($action) {
									case 'add':
										$GLOBALS['SQ_SYSTEM']->am->setRole($asset->id, $roleid, $userid);
									break;
									case 'delete':
										$GLOBALS['SQ_SYSTEM']->am->deleteRole($asset->id, $roleid, $userid);
									break;
								}
							}
						}
					}

					$step_data['message'] = translate('hipo_edit_role', $asset->name);
					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
					unset($asset);
				} else {
					// we do not have admin access
					trigger_localised_error('SYS0313', E_USER_WARNING, $asset->name);
				}
			} else {
				$step_data['message'] = translate('hipo_skipping_asset', $assetid);
				$this->_addError(translate('hipo_skipping_role', $assetid));
			}

			// add this assetid to the done array so we dont do it again
			$this->_running_vars['done_assetids'][] = $assetid;

		}//end if

		if (empty($this->_running_vars['todo_assetids'])) {
			$step_data['percent_done'] = 100;
			$step_data['complete']     = TRUE;
			unset($this->_running_vars['done_assetids']);
		} else {
			$total = count($this->_running_vars['todo_assetids']) + count($this->_running_vars['done_assetids']);
			$step_data['percent_done'] =(count($this->_running_vars['done_assetids']) / $total) * 100;
			$step_data['complete']     = FALSE;
		}

		return TRUE;

	}//end processRoles()


	/**
	* This function is used to set-up the details for the locking the assets by the nested HIPO_Job
	*
	* @param string	&$job_type		the hipo job type
	* @param array	&$running_vars	array contains the information for the hipo to run
	* @param array	&$options		options
	*
	* @return void
	* @access public
	*/
	function getLockAssetDetails(&$job_type, &$running_vars, &$options)
	{
		$job_type = 'hipo_job_acquire_locks';
		$options['auto_complete'] = TRUE;

		$running_vars = Array(
							'assetids'			=> array_keys($this->_running_vars['assets']),
							'lock_type'			=> Array('roles'),
							'forceably_acquire'	=> FALSE,
							'dependants_only'	=> isset($this->_running_vars['dependants_only']),
						);

	}//end getLockAssetDetails()


	/**
	* Release the locks we acquired while setting the roles
	*
	* @param array	&$step_data	a reference to the array of information about the current step
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function releaseLocks(&$step_data, $prefix)
	{
		if (!isset($this->_running_vars['re_lock_assets'])) {
			$this->_running_vars['re_lock_assets'] = $this->_running_vars['assets'];
		}

		$assetid = key($this->_running_vars['re_lock_assets']);
		unset($this->_running_vars['re_lock_assets'][$assetid]);

		$children = $GLOBALS['SQ_SYSTEM']->am->getDependantChildren($assetid);

		foreach ($children as $dep_child_id => $dep_child_type_code) {
			$GLOBALS['SQ_SYSTEM']->am->releaseLock($dep_child_id, 'roles');
		}

		$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($assetid));

		$step_data['message'] = translate('hipo_release_locks', $asset_info[$assetid]['name']).translate('hipo_release_locks_dependents');

		$GLOBALS['SQ_SYSTEM']->am->releaseLock($assetid, 'all');

		$this->_running_vars['done_assetids'][] = $assetid;

		if (empty($this->_running_vars['re_lock_assets'])) {
			unset($this->_running_vars['done_assetids']);
			$step_data['percent_done'] = 100;
			$step_data['complete']     = TRUE;
		} else {
			$step_data['percent_done'] = (count($this->_running_vars['done_assetids']) / count($this->_running_vars['assets'])) * 100;
			$step_data['complete']     = FALSE;
		}

		return TRUE;

	}//end releaseLocks()


}//end class

?>
