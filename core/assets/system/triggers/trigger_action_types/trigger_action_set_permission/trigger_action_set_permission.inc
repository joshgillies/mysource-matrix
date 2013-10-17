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
* $Id: trigger_action_set_permission.inc,v 1.31 2013/10/17 08:18:14 cupreti Exp $
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
* @version $Revision: 1.31 $
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
	public static function execute($settings, &$state)
	{
		// check required settings
		if (!isset($settings['permission'])) return FALSE;

		$parameter_map_value = array_get_index($settings, 'user_map', serialize(Array()));
		$atr_parameter_map = new Asset_Attribute_Parameter_Map(0, $parameter_map_value);
		$userid = $atr_parameter_map->getParameterValue('user');

		if (!empty($userid)) {
			// ensure that the value from the parameter map is a valid asset id
			$user_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($userid, Array('user', 'user_group'), FALSE);
			if (empty($user_info)) return FALSE;
		} else {
			// no user id has been supplied by the parameter map, so use the static userid
			$userid = $settings['userid'];
		}

		// if we still have no userid, bail.
		if (empty($userid)) return FALSE;

		if (empty($state['assetid'])) return FALSE;
		$granted = array_get_index($settings, 'granted', TRUE);
		$dependant_only = array_get_index($settings, 'dependants_only', FALSE);
		$dependant_parents = array_get_index($settings, 'dependant_parents', FALSE);
		$cascade_to_new = array_get_index($settings, 'cascade_to_new', TRUE);

		// If permission change fails, fail silently
		$mute_error = array_get_index($settings, 'mute_error', FALSE);
		
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
					$target_assets[$assetid] = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				}
			}
		} else {
			$target_assets[$state['assetid']] = $GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
		}

		// need to use a hipo job to set status (handles dependant children)
		$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$hipo_assets = Array();
		foreach ($target_assets as $assetid => $asset) {
			$hipo_assets[] = $assetid;
		}

		// If we are deleting a permission, get the current access permission (bug #6605) 
		$previous_access = NULL;
		if ($granted == -1) {
			$current_access = $GLOBALS['SQ_SYSTEM']->am->getAssetPermissionByCascade($state['assetid'], $settings['permission'], $userid);
			if (isset($current_access[0])) {
				$previous_access = array_get_index($current_access[0], 'granted', NULL);
			}
		}

		$vars['permission_changes'] = Array(
											Array(
												'permission'		=> $settings['permission'],
												'granted'			=> $granted,
												'userid'			=> $userid,
												'previous_access'	=> $previous_access,
												'dependants_only'	=> $dependant_only,
												'cascades'			=> $cascade_to_new,
												'assetids'			=> $hipo_assets,
											),
									  );
		$vars['dependants_only'] = $dependant_only;

		$status_errors = $hh->freestyleHipo('hipo_job_edit_permissions', $vars);
		if (!$mute_error && !empty($status_errors)) return FALSE;

		return Array(
				'assetid'		=> $state['assetid'],
				'userid'		=> $userid,
				'permission'	=> $settings['permission'],
				'granted'		=> $granted,
				'cascades'		=> $cascade_to_new,
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
	public static function getInterface($settings, $prefix, $write_access=FALSE)
	{
		$current_schemaid      = array_get_index($settings, 'schemaid', 0);
		$action                = array_get_index($settings, 'action', 1);
		$granted               = array_get_index($settings, 'granted', 1);
		$permission            = array_get_index($settings, 'permission', SQ_PERMISSION_READ);
		$userid                = array_get_index($settings, 'userid', 0);
		$dependants_only       = array_get_index($settings, 'dependants_only', FALSE);
		$dependant_parents     = array_get_index($settings, 'dependant_parents', FALSE);
		$cascade_to_new        = array_get_index($settings, 'cascade_to_new', TRUE);

		ob_start();

		// attribute friendly prefix
		$new_prefix = str_replace(array('[',']'), '_', $prefix);
		hidden_field($prefix.'[new_prefix]', $new_prefix);

		// print the grant/deny selection
		if ($write_access) {
			combo_box($prefix.'[granted]', 
				Array (
					'1' => translate('grant'), 
					'0' => translate('deny'), 
					'-1' => translate('delete'),
				), 
				FALSE, 
				$granted);
		} else {
			echo '<b>';
			switch ($granted) {
			case 1:
				echo translate('grant');
				break;
			case 0:
				echo translate('deny');
				break;
			case -1:
				echo translate('delete');
				break;
			}
			echo '</b> ';
		}

		// print the permission level selection
		$permissions = Trigger_Action_Set_Permission::_getPermissionList();

		if ($write_access) {
			combo_box($prefix.'[permission]', $permissions, FALSE, $permission);
		} else {
			echo '<b>'.$permissions[$permission].'</b>';
		}
		echo ' '.translate('access_for').' ';

		// print the asset finder
		if ($write_access) {
			asset_finder(
				$prefix.'[userid]',
				$userid,
				Array(
					'user'			=> 'D',
					'user_group'	=> 'D',
				)
		 	);
		} else {
			$user = NULL;
			if ($userid > 0) {
				$user = $GLOBALS['SQ_SYSTEM']->am->getAsset($userid);
			}
			if (is_null($user)) {
				echo '<b>['.translate('trigger_no_user_selected').']</b>';
			} else {
				echo '<b>'.$user->name.'</b>';
			}
		}

		$parameter_map_value = array_get_index($settings, 'user_map', serialize(Array()));

		$atr_parameter_map = new Asset_Attribute_Parameter_Map(0, $parameter_map_value);

		$atr_parameter_map->setParameter('user', 'Userid or Groupid');
		echo $atr_parameter_map->paint($new_prefix.'_parameter_map', !$write_access);

		// Only cascade to our dependants (and not all our children)?
		if ($write_access) {
			echo '<br />';
			check_box($prefix.'[dependants_only]', 1, $dependants_only);
			echo translate('trigger_set_permission_dependants_only');
		} else {
			echo '<img src="'.sq_web_path('lib').'/web/images/'.($dependants_only ? 'tick' : 'cross').'.gif" alt="'.($dependants_only ? translate('yes') : translate('no')).'" /> ';
			echo translate('trigger_set_permission_dependants_only');
		}

		// Look for dependant parents (and change them instead)?
		if ($write_access) {
			echo '<br />';
			check_box($prefix.'[dependant_parents]', 1, $dependant_parents);
			echo translate('trigger_set_permission_dependant_parents');
		} else {
			echo '<br />';
			echo '<img src="'.sq_web_path('lib').'/web/images/'.($dependant_parents ? 'tick' : 'cross').'.gif" alt="'.($dependant_parents ? translate('yes') : translate('no')).'" /> ';
			echo translate('trigger_set_permission_dependant_parents');
		}

		// Cascade to new children
		if ($write_access) {
			echo '<br />';
			check_box($prefix.'[cascade_to_new]', 1, $cascade_to_new);
			echo translate('cascade_permission_to_new_children');
		} else {
			echo '<br />';
			echo '<img src="'.sq_web_path('lib').'/web/images/'.($cascade_to_new ? 'tick' : 'cross').'.gif" alt="'.($cascade_to_new ? translate('yes') : translate('no')).'" /> ';
			echo translate('cascade_permission_to_new_children');
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
	public static function processInterface(&$settings, $request_data)
	{
		$new_prefix = str_replace(array('[',']'), '_', array_get_index($request_data, 'new_prefix', ''));

		// check if the permission is valid
		$permission = array_get_index($request_data, 'permission', FALSE);
		if (!$permission) return 'Permission not specified';

		$valid_permissions = Trigger_Action_Set_Permission::_getPermissionList();
		if (isset($valid_permissions[$permission])) {
			$settings['permission'] = $permission;
		} else {
			return 'Specified permission is invalid';
		}

		// access must be either 1 or 0 or -1
		$granted = array_get_index($request_data, 'granted', FALSE);
		if ($granted < -1 || $granted > 1) {
			return 'Access parameter is invalid';
		}
		$settings['granted'] = $granted;

		// The selected userid/groupid must be valid.
		$user_data = array_get_index($request_data, 'userid', FALSE);
		$userid = $user_data['assetid'];

		if (!empty($userid)) {
			$user = $GLOBALS['SQ_SYSTEM']->am->getAsset($userid);
			if (is_null($user)) {
				return 'Specified user is not a valid asset';
			}
		}
		$settings['userid'] = $userid;

		$atr_parameter_map = new Asset_Attribute_Parameter_Map();
		$atr_parameter_map->process($new_prefix.'_parameter_map');
		$settings['user_map'] = $atr_parameter_map->value;

		// Check we have a static id OR some parameter map settings. The param map settings
		// can't really be validated, but checking for something means we ensure they've at least been
		// reminded to configure it.
		$user_map = unserialize($settings['user_map']);
		if (empty($settings['userid']) && (empty($user_map))) {
			return 'No user or group id provided';
		}	

		// dependants_only
		$dependants_only = array_get_index($request_data, 'dependants_only', FALSE);
		$settings['dependants_only'] = ($dependants_only == '1') ? TRUE : FALSE;

		// dependant_parents
		$dependant_parents = array_get_index($request_data, 'dependant_parents', FALSE);
		$settings['dependant_parents'] = ($dependant_parents == '1') ? TRUE : FALSE;

		// dependant_parents
		$cascade_to_new = array_get_index($request_data, 'cascade_to_new', FALSE);
		$settings['cascade_to_new'] = ($cascade_to_new == '1') ? TRUE : FALSE;
		
		return FALSE;

	}//end processInterface()


	/**
	* Returns an array containing all the valid permission levels array(int => string)
	*
	* @return void
	* @access private
	*/
	public static function _getPermissionList()
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
	public static function getLocks($settings, &$state)
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
	public static function addSettingUserid($value, $settings)
	{

		$settings['userid'] = $value;
		return $settings;

	}//end addSettingUserid()


}//end class

?>
