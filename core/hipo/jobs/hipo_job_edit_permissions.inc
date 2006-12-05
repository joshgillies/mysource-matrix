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
* $Id: hipo_job_edit_permissions.inc,v 1.58 2006/12/05 05:39:09 bcaldwell Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';

/**
* Highly Intensive Processing Object (HIPO) to edit the permissions of multiple assets
*
* <pre>
* Initial Running Vars :
*    array  assets              -  the IDs of the assets we are modifying permissions for in the form :
*                                  $assetids = Array([assetid] => Array(
																		'type_code' => [type_code],
																		),
*    array  permission_changes  -  the values that we are trying to set for our children, in the form :
*                                  $changes = Array([perm] => Array (
*                                                                    Array(
*                                                                          'granted'         => $granted,
*                                                                          'userid'          => $userid,
*                                                                          'previous_access' => null,
*                                                                          ),
*                                                   ),
* </pre>
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.58 $
* @package MySource_Matrix
* @subpackage HIPO
*/
class HIPO_Job_Edit_Permissions extends HIPO_Job
{


	/**
	* Constructor
	*
	* @param string	$code_name		a unique codename the HIPO
	*/
	function HIPO_Job_Edit_Permissions($code_name='')
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
		$o->openField(translate('permissions_threshold'));

		if ($write_access) {
			text_box($class.'[SQ_HIPO_PERMISSIONS_THRESHOLD]', SQ_HIPO_PERMISSIONS_THRESHOLD, 5);
			echo ' '.translate('assets');
			$o->note(translate('hipo_value_divided_by_permission'));
		} else {
			echo SQ_HIPO_PERMISSIONS_THRESHOLD;
			echo ' '.translate('assets');
		}

		$o->closeField();

	}//end paintConfig()


	/**
	* Sets some info about thresholds that this hipo requires.
	*
	* @return array
	* @access public
	*/
	function getConfigVars()
	{
		return Array(
				'SQ_HIPO_PERMISSIONS_THRESHOLD'	=> Array('editable'	=> 1, 'default'	=> 1),
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
		return translate('hipo_name_edit_permissions');

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
					'name'			=> translate('hipo_acquiring_permission_locks'),
					'hipo_job'		=> Array(
										'init_details_function'	=> 'getLockAssetDetails',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> true,
					'allow_cancel'	=> true,
					'percent_done'	=> 0,
					'complete'		=> false,
					'message'		=> '',
				),
				Array(
					'name'			=> translate('hipo_updating_permissions'),
					'function_call'	=> Array(
										'process_function'	=> 'processPermissions',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> true,
					'allow_cancel'	=> true,
					'percent_done'	=> 0,
					'complete'		=> false,
					'message'		=> '',
				),
				Array(
					'name'			=> translate('hipo_releasing_permission_locks'),
					'function_call'	=> Array(
										'process_function'	=> 'releaseLocks',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> true,
					'allow_cancel'	=> false,
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
		if (SQ_HIPO_PERMISSIONS_THRESHOLD == 0) return 0;
		if (!isset($this->_running_vars['todo_assetids'])) {
			return 0;
		}
		return ((count($this->_running_vars['todo_assetids']) / (SQ_HIPO_PERMISSIONS_THRESHOLD / count($this->_running_vars['permission_changes']))) * 100);

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
			$result = $GLOBALS['SQ_SYSTEM']->am->acquireLock($id,'permissions');
			if ($result) {
				if (!$this->processPermissions($this->_steps[1], get_class($this))) {
					return false;
				}
				if ($result == 1) {
					$result = $GLOBALS['SQ_SYSTEM']->am->releaseLock($id, 'permissions');
				}
			} else {
				return false;
			}
			//$count++;
		}
		return true;

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
			trigger_localised_error('HIPO0015', E_USER_WARNING);
			return '';
		}

		if (empty($this->_running_vars['permission_changes'])) {
			trigger_localised_error('HIPO0014', E_USER_WARNING);
			return '';
		}

		$assets = $this->_running_vars['assets'];
		$child_assets = Array();
		for (reset($assets); null !== ($assetid = key($assets)); next($assets)) {
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

		$child_assets = array_reverse($child_assets,true);
		$this->_running_vars['todo_assetids'] = $child_assets;
		$this->_running_vars['done_assetids'] = Array();
		return parent::prepare();

	}//end prepare()


	/**
	* Edit permissions for a single asset in the to-do list
	*
	* @param array	&$step_data	a reference to the array of information about the current step
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function processPermissions(&$step_data, $prefix)
	{
		if (!empty($this->_running_vars['todo_assetids'])) {
			$assetid = array_shift(array_keys($this->_running_vars['todo_assetids']));
			$asset_type = $this->_running_vars['todo_assetids'][$assetid];
			unset($this->_running_vars['todo_assetids'][$assetid]);
			$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $asset_type);

			if (!is_null($asset)) {
				// check we can change permissions, otherwise silently pass by
				if ($asset->adminAccess('permissions')) {
					foreach ($this->_running_vars['permission_changes'] as $perm => $perm_data) {
						require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
						$perm_name = permission_type_name($perm);

						foreach ($perm_data as $set_data) {
							if ($set_data['userid']) {
								// use getAssetInfo() because deleting permissions
								// held by deleted users (eg.LDAP) will cause an assertion
								$user_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($set_data['userid']), 'user', false);

								// check that the passed userid is a user or user_group
								if (empty($user_info)) {
									$user_name = 'Unknown User';
								} else {
									$user_name = $user_info[$set_data['userid']]['name'];
								}
							} else {
								$user_name = 'General Public';
							}
							if ($set_data['previous_access'] === null) {
								// to edit this permission, there must not currently be a permission set
								// at all - either grant OR deny - for this user
								$current = $GLOBALS['SQ_SYSTEM']->am->getPermission($asset->id, $perm, !(bool)$set_data['granted'], FALSE, FALSE, TRUE, TRUE);
								if (isset($current[$set_data['userid']])) {
									$new_access     = ($set_data['granted']) ? translate('grant') : translate('revoke');
									$current_access = ($current[$set_data['userid']]) ? translate('granted') : translate('revoked');
									$this->_addError(translate('hipo_cannot_modify_permission', $new_access, $perm_name, $user_name, $asset->name, $current_access), true);
									continue;
								}
							} else {
								// we need to have a permission set and the access level be the same
								$current = $GLOBALS['SQ_SYSTEM']->am->getPermission($asset->id, $perm, null, false, false, true);
								if (!isset($current[$set_data['userid']])) {
									continue;
								} else if ($current[$set_data['userid']] != $set_data['previous_access']) {
									$new_access     = ($set_data['previous_access']) ? translate('grant') : translate('revoke');
									$current_access = ($set_data['previous_access']) ? translate('revoked') : translate('granted');
									$this->_addError(translate('hipo_cannot_modify_permission', $new_access, $perm_name, $user_name, $asset->name, $current_access), true);
									continue;
								}

							}

							switch ($set_data['granted']) {
								case -1 :
									// delete a permission
									$GLOBALS['SQ_SYSTEM']->am->deletePermission($asset->id, $set_data['userid'], $perm);
								break;

								case 0  :
								case 1  :
									// deny a permission
									// grant a permission
									$GLOBALS['SQ_SYSTEM']->am->setPermission($asset->id, $set_data['userid'], $perm, $set_data['granted']);
								break;
							}
						}
					}

					$step_data['message'] = translate('hipo_edit_permission', $asset->name);
					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
					unset($asset);
				} else {
						// we do not have admin access
						trigger_localised_error('SYS0111', E_USER_WARNING, $asset->name);
				}
			} else {
				$step_data['message'] = translate('hipo_skipping_asset', $assetid);
				$this->_addError(translate('hipo_skipping_permission', $assetid));
			}

			// add this assetid to the done array so we dont do it again
			$this->_running_vars['done_assetids'][] = $assetid;

		}//end if

		if (empty($this->_running_vars['todo_assetids'])) {
			$step_data['percent_done'] = 100;
			$step_data['complete']     = true;
			unset($this->_running_vars['done_assetids']);
		} else {
			$total = count($this->_running_vars['todo_assetids']) + count($this->_running_vars['done_assetids']);
			$step_data['percent_done'] =(count($this->_running_vars['done_assetids']) / $total) * 100;
			$step_data['complete']     = false;
		}

		return true;

	}//end processPermissions()


	/**
	* This function is used to set-up the details for the locking the assets by the nested HIPO_Job
	*
	* @param string	&$job_type
	* @param array	&$running_vars
	* @param array	&$options
	*
	* @return void
	* @access public
	*/
	function getLockAssetDetails(&$job_type, &$running_vars, &$options)
	{
		$job_type = 'hipo_job_acquire_locks';
		$options['auto_complete'] = true;

		$running_vars = Array(
							'assetids'			=> array_keys($this->_running_vars['assets']),
							'lock_type'			=> Array('permissions'),
							'forceably_acquire'	=> false,
							'dependants_only'	=> isset($this->_running_vars['dependants_only']),
						);

	}//end getLockAssetDetails()


	/**
	* Release the locks we acquired while setting the permissions
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
			$GLOBALS['SQ_SYSTEM']->am->releaseLock($dep_child_id, 'permissions');
		}

		$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($assetid));

		$step_data['message'] = translate('hipo_release_locks', $asset_info[$assetid]['name']).translate('hipo_release_locks_dependents');

		$GLOBALS['SQ_SYSTEM']->am->releaseLock($assetid, 'all');

		$this->_running_vars['done_assetids'][] = $assetid;

		if (empty($this->_running_vars['re_lock_assets'])) {
			unset($this->_running_vars['done_assetids']);
			$step_data['percent_done'] = 100;
			$step_data['complete']     = true;
		} else {
			$step_data['percent_done'] = (count($this->_running_vars['done_assetids']) / count($this->_running_vars['assets'])) * 100;
			$step_data['complete']     = false;
		}

		return true;
	}//end releaseLocks()


}//end class

?>