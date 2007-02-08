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
* $Id: trigger_action_set_permission.inc,v 1.14 2007/02/08 23:08:29 emcdonald Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';
require_once SQ_SYSTEM_ROOT.'/core/attributes/parameter_map/parameter_map.inc';

/**
* Trigger_Action_Set_Permission
*
* A trigger action that sets a permission for an asset
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.14 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Set_Permission extends Trigger_Action
{


	/**
	* Execute this action
	* Returns an array of data about what it did, or false on error
	*
	* The settings used by this action are in the form:<BR />
	* <PRE>
	* Array(
	*       'permission' => int    // the permission level (read, write or admin)
	*       'granted'    => int    // is access granted ('1') or denied ('0')
	*       'userid'     => string // the assetid of the user the permission is being set for
	*       );
	* </PRE>
	*
	* @param array	$settings	the stored settings for this action
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return mixed array|boolean
	* @access public
	*/
	function execute($settings, &$state)
	{
		// check required settings
		if (!isset($settings['permission'])) return FALSE;

		// must provide a static id, even if it is only a default for the param map
		if (empty($settings['userid'])) return FALSE;

		$parameter_map_value = array_get_index($settings, 'user_map', serialize(Array()));
		$atr_parameter_map =& new Asset_Attribute_Parameter_Map(0, $parameter_map_value);
		$userid = $atr_parameter_map->getParameterValue('user');

		if (!empty($userid)) {
			// ensure that the value from the parameter map is a valid asset id
			$user_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($userid, Array('user', 'user_group'), FALSE);
			if (empty($user_info)) return FALSE;
		} else {
			// no user id has been supplied by the parameter map, so use the static userid
			$userid = $settings['userid'];
		}

		if (empty($state['assetid'])) return FALSE;
		$granted = array_get_index($settings, 'granted', TRUE);
		$dependant_only = array_get_index($settings, 'dependant_only', FALSE);
		$dependant_parents = array_get_index($settings, 'dependant_parents', FALSE);

		// if dependant_parents is turned on, look for all our dependant parents
		// and change their statuses, instead of our own
		$target_assets = Array();
		if ($dependant_parents) {
			// get only the top level
			$target_assets = $GLOBALS['SQ_SYSTEM']->am->getDependantParents($state['assetid'], '', TRUE, FALSE);
			if (!empty($target_assets)) {
				// returns Array(0 => Assetid), so flip the array
				$target_assets = array_flip($target_assets);
				foreach ($target_assets as $assetid => $null) {
					$target_assets[$assetid] =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				}
			}
		} else {
			$target_assets[$state['assetid']] =& $GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
		}

		// need to use a hipo job to set status (handles dependant children)
		$hh =& $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		foreach ($target_assets as $assetid => $asset) {
			$vars['assets'][$assetid] = Array(
											'type_code'	=> $asset->type(),
										);
		}
		$vars['permission_changes'] = Array(
										$settings['permission']	=> Array(
																	Array(
																		'granted'			=> $granted,
																		'userid'			=> $userid,
																		'previous_access'	=> NULL,
																	),
																   ),
									  );
		$vars['dependants_only'] = $dependant_only;

		$status_errors = $hh->freestyleHipo('hipo_job_edit_permissions', $vars);

		if (!empty($status_errors)) return FALSE;

		return Array(
				'assetid'		=> $state['assetid'],
				'userid'		=> $userid,
				'permission'	=> $settings['permission'],
				'granted'		=> $granted,
			   );

	}//end execute()


	/**
	* Get the HTML editing interface for this action. Returns the string instead of printing it
	*
	* @param array		$settings		settings that this condition saves in processInterface()
	* @param string		$prefix			unique prefix
	* @param boolean	$write_access	determines whether the interface is editable
	*
	* @return boolean
	* @access public
	*/
	function getInterface($settings, $prefix, $write_access=FALSE)
	{

	// due to "access for" being the only stable text in this interface
		// we are not translating like the rest but just the text as it appears
		ob_start();

		// attribute friendly prefix
		$new_prefix = ereg_replace('\[', '_', ereg_replace('\]', '_', $prefix));
		hidden_field($prefix.'[new_prefix]', $new_prefix);

		// print the grant/deny selection
		if (!isset($settings['granted'])) {
			$selected_granted = '1';
		} else {
			$selected_granted = $settings['granted'];
		}
		if ($write_access) {
			combo_box($prefix.'[granted]', Array(
											'1'	=> translate('grant'),
											'0'	=> translate('deny'),
										   ), FALSE, $selected_granted);
		} else {
			echo '<b>';
			echo ($selected_granted) ? translate('grant') : translate('deny');
			echo '</b> ';
		}

		// print the permission level selection
		if (empty($settings['permission'])) {
			$selected_permission = SQ_PERMISSION_READ;
		} else {
			$selected_permission = $settings['permission'];
		}
		$permissions = Trigger_Action_Set_Permission::_getPermissionList();
		if ($write_access) {
			combo_box($prefix.'[permission]', $permissions, FALSE, $selected_permission);
		} else {
			echo '<b>'.$permissions[$selected_permission].'</b>';
		}
		echo ' '.translate('access_for').' ';



		// print the asset finder
		if (empty($settings['userid'])) {
			$selected_userid = 0;
		} else {
			$selected_userid = $settings['userid'];
		}
		if ($write_access) {
			asset_finder(
				$prefix.'[userid]',
				$selected_userid,
				Array(
					'user'			=> 'D',
					'user_group'	=> 'D',
				)
			);
		} else {
			$user = NULL;
			if ($selected_userid > 0) {
				$user =& $GLOBALS['SQ_SYSTEM']->am->getAsset($selected_userid);
			}
			if (is_null($user)) {
				echo '<b>['.translate('trigger_no_user_selected').']</b>';
			} else {
				echo '<b>'.$user->name.'</b>';
			}
		}

		$parameter_map_value = array_get_index($settings, 'user_map', serialize(Array()));

		$atr_parameter_map =& new Asset_Attribute_Parameter_Map(0, $parameter_map_value);

		$atr_parameter_map->setParameter('user', 'Userid or Groupid');
		echo $atr_parameter_map->paint($new_prefix.'_parameter_map', !$write_access);

		// dependants_only
		$dependants_only = array_get_index($settings, 'dependants_only', FALSE);
		if ($write_access) {
			echo '<br />';
			check_box($prefix.'[dependants_only]', 1, array_get_index($settings, 'dependants_only', FALSE));
			echo translate('trigger_set_permission_dependants_only');
		} else {
			echo '<br />Dependant Children Only : <b>'.($dependants_only ? 'True' : 'False').'</b>';
		}

		// dependant_parents
		$dependant_parents = array_get_index($settings, 'dependant_parents', FALSE);
		if ($write_access) {
			echo '<br />';
			check_box($prefix.'[dependant_parents]', 1, array_get_index($settings, 'dependant_parents', FALSE));
			echo translate('trigger_set_permission_dependant_parents');
		} else {
			echo '<br />Dependant Parent : <b>'.($dependant_parents ? 'True' : 'False').'</b>';
		}

		return ob_get_clean();

	}//end getInterface()


	/**
	* Function that handles the conversion of interface to settings
	* together with settings it is expected to populate the hash object
	*
	* @param array	&$settings		a container for any data the action might want to save
	* @param string	$request_data	array of data corresponding to this action as
	*								specified by the $prefix in getInterface.
	*
	* @return boolean
	* @access public
	*/
	function processInterface(&$settings, $request_data)
	{
		$new_prefix = ereg_replace('\[', '_', ereg_replace('\]', '_', array_get_index($request_data, 'new_prefix', '')));

		// check if the permission is valid
		$permission = array_get_index($request_data, 'permission', FALSE);
		if (!$permission) return 'Permission not specified';

		$valid_permissions = Trigger_Action_Set_Permission::_getPermissionList();
		if (isset($valid_permissions[$permission])) {
			$settings['permission'] = $permission;
		} else {
			return 'Specified permission is invalid';
		}

		// access must be either 1 or 0
		$granted = array_get_index($request_data, 'granted', FALSE);
		if ($granted != '1' && $granted != '0') {
			return 'Access parameter is invalid';
		}
		$settings['granted'] = $granted;

		// a userid must have been selected
		$user_data = array_get_index($request_data, 'userid', FALSE);
		$userid = $user_data['assetid'];

		if (empty($userid)) return 'User id is not specified';


		$atr_parameter_map =& new Asset_Attribute_Parameter_Map();
		$atr_parameter_map->process($new_prefix.'_parameter_map');


		$user =& $GLOBALS['SQ_SYSTEM']->am->getAsset($userid);
		if (is_null($user)) {
			return 'Specified user is not a valid asset';
		}
		$settings['userid'] = $userid;


		$settings['user_map'] = $atr_parameter_map->value;

		// dependants_only
		$dependants_only = array_get_index($request_data, 'dependants_only', FALSE);
		$settings['dependants_only'] = ($dependants_only == '1') ? TRUE : FALSE;

		// dependant_parents
		$dependant_parents = array_get_index($request_data, 'dependant_parents', FALSE);
		$settings['dependant_parents'] = ($dependant_parents == '1') ? TRUE : FALSE;

		return FALSE;

	}//end processInterface()


	/**
	* Returns an array containing all the valid permission levels array(int => string)
	*
	* @return void
	* @access private
	*/
	function _getPermissionList()
	{
		return Array(
				SQ_PERMISSION_READ	=> 'read',
				SQ_PERMISSION_WRITE	=> 'write',
				SQ_PERMISSION_ADMIN	=> 'admin',
			   );

	}//end _getPermissionList()


	/**
	* Get the list of locks that the action needs acquire before executing
	*
	* @param array	$settings	a container for any data the action might want to save
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return array
	* @access public
	*/
	function getLocks($settings, &$state)
	{
		return Array($state['assetid'] => Array('permissions'));

	}//end getLocks()


	/**
	* Add the userid value to a settings array.
	*
	* @param string	$value		The userid value to be added to the settings array
	* @param array	$settings	a container for any data the action might want to save
	*
	* @return array
	* @access public
	*/
	function addSettingUserid($value, $settings)
	{

		$settings['userid'] = $value;
		return $settings;

	}//end addSettingUserid()


}//end class

?>
