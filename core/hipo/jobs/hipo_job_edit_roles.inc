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
* $Id: hipo_job_edit_roles.inc,v 1.14 2012/08/30 01:09:18 ewang Exp $
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
* @version $Revision: 1.14 $
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
		$o->openField(translate('Edit Role Threshold'));


		echo '<span class="sq-inline-fields-wrapper">';
		if ($write_access) {
			text_box($class.'[SQ_HIPO_EDIT_ROLE_THRESHOLD]', SQ_HIPO_EDIT_ROLE_THRESHOLD, 1);
		} else {
			echo SQ_HIPO_EDIT_ROLE_THRESHOLD;
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
				'SQ_HIPO_EDIT_ROLE_THRESHOLD'	=> Array('editable'	=> 1, 'default'	=> 20),
			   );

	}//end getConfigVars()


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
		return translate('Edit Roles');


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
					'name'			=> translate('Updating Roles'),

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
		if (SQ_HIPO_EDIT_ROLE_THRESHOLD == 0) return 0;
		if (!isset($this->_running_vars['todo_assetids']) || !isset($this->_running_vars['role_changes'])) {
			return 0;
		}

		//total userids in 'add' and 'delete' role changes - include duplicated userids because we are counting the number of calls to setRole and deleteRole methods
		$total_userids = 0;
		foreach ($this->_running_vars['role_changes'] as $action_data) {
			foreach ($action_data as $userids) {
				$total_userids += count($userids);
			}
		}

		//we have to multiply with the number of users because we want to run runQueuedJobs() instead of freestyle() in hipo job
		//when we want to set role for 1 asset but 1000 users. It will take time in this case
		return ((count($this->_running_vars['todo_assetids']) * $total_userids / SQ_HIPO_EDIT_ROLE_THRESHOLD) * 100);

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
			if (!$this->processRoles($this->_steps[0], get_class($this))) {
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

			$child_assets[$assetid]	= Array (
										0	=> Array (
												'type_code'	=> $asset_info[$assetid]['type_code'],
											   ),
									  );
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
			reset($this->_running_vars['todo_assetids']);
			$assetid = key($this->_running_vars['todo_assetids']);
			$asset_type = $this->_running_vars['todo_assetids'][$assetid][0]['type_code'];
			unset($this->_running_vars['todo_assetids'][$assetid]);
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $asset_type);

			if (!is_null($asset)) {
				// check we can acquire lock and change roles, otherwise silently pass by
				if ($GLOBALS['SQ_SYSTEM']->am->acquireLock($assetid, 'roles')) {
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
						$step_data['message'] = sprintf(translate('Editing Roles for "%s"'), htmlentities($asset->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET));

						$GLOBALS['SQ_SYSTEM']->am->releaseLock($assetid, 'roles');
						$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
						unset($asset);
					} else {
						// we do not have admin access
						trigger_localised_error('SYS0313', E_USER_WARNING, $asset->name);
					}
				} else {
					// we do not acquire lock
					trigger_localised_error('SYS0100', E_USER_WARNING, 'roles', $asset->name, ' skipping asset.');
				}
			} else {
				$step_data['message'] = sprintf(translate('Skipping asset #%s'), $assetid);

				$this->_addError(sprintf(translate('Could not edit roles for asset #%s, asset does not exist'), $assetid));

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

		foreach ($children as $dep_child_id => $dep_child_data) {
			$GLOBALS['SQ_SYSTEM']->am->releaseLock($dep_child_id, 'roles');
		}

		$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($assetid));

		$step_data['message'] = sprintf(translate('Releasing locks on %s'), $asset_info[$assetid]['name']).translate('and its dependent assets');



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
