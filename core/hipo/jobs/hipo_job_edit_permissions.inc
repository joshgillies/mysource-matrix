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
* $Id: hipo_job_edit_permissions.inc,v 1.75 2012/08/30 01:09:18 ewang Exp $
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
* @version $Revision: 1.75 $
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
	public static function paintConfig(&$o, $class, $write_access)
	{
		// metadata regeneration threshhold HIPO config entry
		$o->openField(translate('Permissions Threshold'));


		echo '<span class="sq-inline-fields-wrapper">';
		if ($write_access) {
			text_box($class.'[SQ_HIPO_PERMISSIONS_THRESHOLD]', SQ_HIPO_PERMISSIONS_THRESHOLD, 1);
		} else {
			echo SQ_HIPO_PERMISSIONS_THRESHOLD;
		}
		echo ' '.translate('assets');

		$o->note(translate('Value will be divided by number of affected permissions.'));

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
				'SQ_HIPO_PERMISSIONS_THRESHOLD'	=> Array('editable'	=> 1, 'default'	=> 20),
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
		return parent::getCodeName().'-'.md5(serialize($this->_running_vars['permission_changes']));

	}//end getCodeName()


	/**
	* Returns the (localised) HIPO name
	*
	* @return string
	* @access public
	*/
	function getHipoName()
	{
		return translate('Edit Permissions');


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
					'name'			=> translate('Updating Permissions'),

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
		return (($this->_running_vars['total_changes']) / (SQ_HIPO_PERMISSIONS_THRESHOLD / count($this->_running_vars['permission_changes'])) * 100);

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
			if (!$this->processPermissions($this->_steps[0], get_class($this))) {
				return FALSE;
			}
		}
		return TRUE;

	}


	/**
	* Prepares the running vars and gets the children of the assets in question.
	*
	* @return boolean
	* @access public
	*/
	function prepare()
	{
		if (empty($this->_running_vars['permission_changes'])) {
			trigger_localised_error('HIPO0014', translate('Unable to initialise Edit Permissions HIPO, no permission changes specified'), E_USER_WARNING);
			return '';
		}

		$this->_running_vars['done_changes']  = 0;
		$this->_running_vars['total_changes'] = 0;

		if (empty($this->_running_vars['permission_changes'])) {
			// no schema changes, so do nothing
			$this->_running_vars['todo_assetids'] = Array();
		} else if (empty($this->_running_vars['permission_screen_assetid'])) {
			$this->_running_vars['todo_assetids'] = Array();
			foreach ($this->_running_vars['permission_changes'] as $perm_change) {
				$assetids = $perm_change['assetids'];
				foreach ($assetids as $assetid) {
					$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($assetid));

					if (empty($asset_info)) {
						trigger_localised_error('HIPO0013', sprintf(translate('Unable to initialise Edit Permissions HIPO, invalid asset ID specified (%s)'), $assetid), E_USER_WARNING);
						return '';
					}

					if ((!array_get_index($perm_change, 'cascades', TRUE)) || array_get_index($perm_change, 'dependants_only')) {
						$child_assets = $GLOBALS['SQ_SYSTEM']->am->getDependantChildren($assetid);
					} else {
						$child_assets = $GLOBALS['SQ_SYSTEM']->am->getChildren($assetid);
					}

					// add the parent asset to the list too
					$child_assets[$assetid]	= Array (
												0	=> Array (
														'type_code'	=> $asset_info[$assetid]['type_code'],
													   ),
											  );

					$child_assets = array_reverse($child_assets, TRUE);

					// Now assign permission changes to each asset
					foreach ($child_assets as $child_assetid => $child_asset) {
						$type_code = $child_asset[0]['type_code'];

						$this_todo =& $this->_running_vars['todo_assetids'][$child_assetid];
						$this_todo['type_code'] = $type_code;
						$this_todo['permission_changes'][] = $perm_change;
						$this->_running_vars['total_changes']++;
					}
				}
			}
		} else {
			// Job from static permission screen
			$need_all = false;
			foreach ($this->_running_vars['permission_changes'] as $perm_change) {
				if (!((!array_get_index($perm_change, 'cascades', TRUE)) || array_get_index($perm_change, 'dependants_only'))) {
					$need_all = true;
				}
			}//end foreach

			$assetid = $this->_running_vars['permission_screen_assetid'];
			$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($assetid));
			if (empty($asset_info)) {
				trigger_localised_error('HIPO0013', sprintf(translate('Unable to initialise Edit Permissions HIPO, invalid asset ID specified (%s)'), $assetid), E_USER_WARNING);
				return '';
			}

			if ($need_all) {
				$children = $GLOBALS['SQ_SYSTEM']->am->getChildren($assetid);
			} else {
				$children = $GLOBALS['SQ_SYSTEM']->am->getDependantChildren($assetid);
			}

			// Add the parent asset to the list too
			$todo_assetids = Array(
								$assetid => Array(
												'type_code' => $asset_info[$assetid]['type_code'],
											),
								);
			foreach($children as $todo_assetid => $info) {
				if (!isset($todo_assetids[$todo_assetid])) {
					$todo_assetids[$todo_assetid]['type_code'] = $info[0]['type_code'];
				}
			}//end foreach
			unset($dep_children);
			unset($children);

			$this->_running_vars['todo_assetids'] = $todo_assetids;
			$this->_running_vars['total_changes'] = count($todo_assetids);
		}

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
			reset($this->_running_vars['todo_assetids']);
			$assetid = key($this->_running_vars['todo_assetids']);
			$asset_type = $this->_running_vars['todo_assetids'][$assetid]['type_code'];
			$perm_changes = empty($this->_running_vars['permission_screen_assetid']) ? $this->_running_vars['todo_assetids'][$assetid]['permission_changes'] : $this->_running_vars['permission_changes'];
			unset($this->_running_vars['todo_assetids'][$assetid]);
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $asset_type);

			if (!is_null($asset) ) {
				// check we can acquire lock and change permissions, otherwise silently pass by
				 if ($GLOBALS['SQ_SYSTEM']->am->acquireLock($assetid, 'permissions')) {
					if ($asset->adminAccess('permissions')) {
						require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
						$this->_running_vars['done_changes']++;

						foreach ($perm_changes as $set_data) {
							$perm = $set_data['permission'];
							$perm_name = permission_type_name($perm);

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
									$new_access     = ($set_data['granted']) ? translate('Grant') : translate('Revoke');


									$current_access = ($current[$set_data['userid']]) ? translate('Granted') : translate('Revoked');


									$this->_addError(sprintf(translate('Could not %1$s %2$s access for "%3$s" on "%4$s" because this asset already has %5$s %2$s access to "%3$s". The current permission for "%4$s" has been retained.'), $new_access, $perm_name, $user_name, $asset->name, $current_access), true);

									continue;
								}
							} else {
								// we need to have a permission set and the access level be the same
								$current = $GLOBALS['SQ_SYSTEM']->am->getPermission($asset->id, $perm, null, false, false, true);
								if (!isset($current[$set_data['userid']])) {
									continue;
								} else if ($current[$set_data['userid']] != $set_data['previous_access']) {
									$new_access     = ($set_data['previous_access']) ? translate('Grant') : translate('Revoke');


									$current_access = ($set_data['previous_access']) ? translate('Revoked') : translate('Granted');


									$this->_addError(sprintf(translate('Could not %1$s %2$s access for "%3$s" on "%4$s" because this asset already has %5$s %2$s access to "%3$s". The current permission for "%4$s" has been retained.'), $new_access, $perm_name, $user_name, $asset->name, $current_access), true);

									continue;
								}

							}

							switch ($set_data['granted']) {
								case -1 :
									// delete a permission
									$step_data['message'] = htmlentities('Removing '.$perm_name.' permission from "'.$asset->name.'" (#'.$asset->id.')', ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET);
									$GLOBALS['SQ_SYSTEM']->am->deletePermission($asset->id, $set_data['userid'], $perm);
								break;

								case 0  :
								case 1  :
									// deny a permission
									// grant a permission
									$step_data['message'] = htmlentities(($set_data['granted'] ? 'Apply' : 'Deny').'ing '.$perm_name.' permission to "'.$asset->name.'" (#'.$asset->id.')', ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET);
									$GLOBALS['SQ_SYSTEM']->am->setPermission($asset->id, $set_data['userid'], $perm, $set_data['granted'], $set_data['cascades']);
								break;
							}
						}

						// If there are multiple schema changes to make, say we are
						// (otherwise use more specific message set above)
						if (count($perm_changes) > 1) {
							$step_data['message'] = htmlentities('Making '.count($perm_changes).' permission changes to '.$asset->name.' (Id: #'.$asset->id.')', ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET);
						} else if (count($perm_changes) == 0) {
							$step_data['message'] = sprintf(translate('Skipping asset #%s'), $assetid);

						}
						$GLOBALS['SQ_SYSTEM']->am->releaseLock($assetid, 'permissions');
						$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
						unset($asset);

					} else {
						// we do not have admin access
						trigger_localised_error('SYS0111', sprintf(translate('Unable to set permission for "%s", permission denied'), $asset->name), E_USER_WARNING);
					}
				} else {
					// we do not acquire lock
					trigger_localised_error('SYS0100', sprintf(translate('Unable to acquire "%1$s" lock on "%2$s", %3$s', 'permissions'), $asset->name, ' skipping asset.'), E_USER_WARNING);
				}
			} else {
				$step_data['message'] = sprintf(translate('Skipping asset #%s'), $assetid);

				$this->_addError(sprintf(translate('Could not edit permissions for asset #%s, asset does not exist'), $assetid));

			}

		}//end if

		if (empty($this->_running_vars['todo_assetids'])) {
			$step_data['percent_done'] = 100;
			$step_data['complete']     = TRUE;
		} else {
			$step_data['percent_done'] = ($this->_running_vars['done_changes'] / $this->_running_vars['total_changes']) * 100;
			$step_data['complete']     = FALSE;
		}

		return true;

	}//end processPermissions()



}//end class

?>
