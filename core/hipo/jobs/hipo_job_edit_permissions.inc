<?php
require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';

/**
* Highly Intensive Processing Object (HIPO) to edit asset permissions
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Resolve
* @subpackage lib
*/
class HIPO_Job_Edit_Permissions extends HIPO_Job
{

	/**
	* The name of this HIPO job (should not be modified by code)
	* @var string
	*/
	var $_hipo_name = 'Edit Permissions';

	/**
	* Array of steps to perform in this HIPO
	* @var Array
	*/
	var $_steps = Array(1 => Array( 'name'             => 'Updating Permissions',
									'paint_function'   => 'paintPermissions',
									'process_function' => 'processPermissions',
									'running_mode'     => 'web',
									'auto_step'        => true,
									'percent_done'     => 0,
									'complete'         => false,
									'message'          => ''),
						);


	/**
	* Constructor
	*
	* @param string	$codename		a unique codename the HIPO
	* @param string	$display_mode	the mode this HIPO is being displayed in
	*								(web = browser, server = HIPO server)
	*/
	function HIPO_Job_Edit_Permissions($codename='', $display_mode='web')
	{
		$this->HIPO_Job($codename, $display_mode);

	}//end constructor


	/**
	* Returns a unique codename the HIPO can use to ensure it is not being run twice
	*
	* The Edit Permission HIPO appends the ID of the asset that the permissions are being set at
	* (the top most parent) so that any other permission jobs on this asset do not start while it
	* is running. Due to locking of the interface, nobody should be allowed to run this HIPO at
	* the same time on the same parent asset.
	*
	* @return string
	* @access public
	*/
	function getCodeName()
	{
		return parent::getCodeName().$this->_running_vars['assetid'];

	}//end getCodeName()


	/**
	* Set up vars and database information that the HIPO will need to run
	*
	* The Edit Permissions HIPO also checks that there are permissions to edit and that
	* there is an assetid to start this action on (the top most parent). It also saves a
	* list of assets to perform this action on in the running vars.
	*
	* @return boolean
	* @access public
	*/
	function initialise()
	{
		// to start editing permission, we need to have an asset id to start editing permission at
		if (!isset($this->_running_vars['assetid'])) {
			trigger_error('Cannot initialise Edit Permissions HIPO, no start asset ID specified', E_USER_WARNING);
			return false;
		}

		// so we have an assetid to start at, but make sure it is a valid assetid
		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['assetid']);
		if (is_null($asset)) {
			trigger_error('Cannot initialise Edit Permissions HIPO, invalid start asset ID specified ('.$this->_running_vars['assetid'].')', E_USER_WARNING);
			return false;
		}

		$this->_running_vars['done_assetids'] = Array();
		if (empty($this->_running_vars['permission_changes'])) {
			// no permission changes, so do nothing
			$this->_running_vars['todo_assetids'] = Array();
		} else {
			// we have a valid asset, so get all the assets below this one in the tree(s) so we know what assets
			// we will be affecting out list of permission changes on
			$child_assets = Array();
			$child_assets = array_merge(Array($asset->id), $asset->getChildren());
			$this->_running_vars['todo_assetids'] = $child_assets;
		}

		unset($asset);

		return parent::initialise();

	}//end initialise()


	/**
	* Paint a progress bar showing where we are up to
	*
	* @param array()	&$step_data	a reference to the array of information about the current step
	* @param string		$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function paintPermissions(&$step_data, $prefix)
	{
		$this->paintProgressBar($step_data['percent_done'], '#412F49', $step_data['message']);
		if ($this->_hipo_vars['running_mode'] == 'server') sleep(2);

	}//end paintPermissions()


	/**
	* Edit permissions for a single asset in the to-do list
	*
	* @param array()	&$step_data	a reference to the array of information about the current step
	* @param string		$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function processPermissions(&$step_data, $prefix)
	{
		if (!empty($this->_running_vars['todo_assetids'])) {
			$assetid = array_shift($this->_running_vars['todo_assetids']);
			$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);

			if (!is_null($asset)) {
				foreach ($this->_running_vars['permission_changes'] as $perm => $perm_data) {
					require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
					$perm_name = permission_type_name($perm);

					foreach ($perm_data as $set_data) {
						if ($set_data['userid']) {
							$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($set_data['userid']);
							if (is_null($user)) $user_name = 'Unknown User';
							else                $user_name = $user->name;
						} else {
							$user_name = 'General Public';
						}

						if ($set_data['previous_access'] === null) {
							// to edit this permission, there must not currently be a permission set
							// at all - either grant OR deny - for this user
							$current = $asset->getPermission($perm, !(bool)$set_data['access'], false, false, true);
							if (isset($current[$set_data['userid']])) {
								$new_access     = ($set_data['access']) ? 'grant' : 'revoke';
								$current_access = ($current[$set_data['userid']]) ? 'granted' : 'revoked';
								$this->_addError('Could not '.$new_access.' '.$perm_name.' access for "'.$user_name.'" on "'.$asset->name.'" because this asset already has '.$current_access.' '.$perm_name.' access to "'.$user_name.'". The current permission for "'.$asset->name.'" has been retained.', true);
								continue;
							}
						} else {
							// we need to have a permission set and the access level be the same
							$current = $asset->getPermission($perm, (bool)$set_data['previous_access'], false);
							if (empty($current)) {
								$new_access     = ($set_data['previous_access']) ? 'grant'   : 'revoke';
								$current_access = ($set_data['previous_access']) ? 'revoked' : 'granted';
								$this->_addError('Could not '.$new_access.' '.$perm_name.' access for "'.$user_name.'" on "'.$asset->name.'" because this asset already has '.$current_access.' '.$perm_name.' access to "'.$user_name.'". The current permission for "'.$asset->name.'" has been retained.', true);
								continue;
							}
						}

						switch ($set_data['access']) {
							case -1 :
								// delete a permission
								$asset->deletePermission($set_data['userid'], $perm);
							break;

							case 0  :
							case 1  :
								// deny a permission
								// grant a permission
								$asset->setPermission($set_data['userid'], $perm, $set_data['access']);
							break;
						}
					}
				}

				$step_data['message'] = 'Editing Permissions for "'.$asset->name.'"';
				$GLOBALS['SQ_SYSTEM']->am->unregisterAsset($asset);
				unset($asset);
			} else {
				$step_data['message'] = 'Skipping asset '.$assetid;
				$this->_addError('Could not edit permissions for asset #'.$assetid.', asset does not exist');
			}

			// add this assetid to the done array so we dont do it again
			$this->_running_vars['done_assetids'][] = $assetid;
		}

		if (empty($this->_running_vars['todo_assetids'])) {
			$step_data['percent_done'] = 100;
			$step_data['complete']     = true;
		} else {
			$total = count($this->_running_vars['todo_assetids']) + count($this->_running_vars['done_assetids']);
			$step_data['percent_done'] =(count($this->_running_vars['done_assetids']) / $total) * 100;
			$step_data['complete']     = false;
		}

		return true;

	}//end processPermissions()

}//end class

?>