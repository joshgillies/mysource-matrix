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
* $Id: trigger_action_set_permission.inc,v 1.4 2005/04/20 04:20:36 gsherwood Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';

/**
* Trigger_Action_Set_Permission
*
* A trigger action that sets a permission for an asset
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Set_Permission extends Trigger_Action
{


	/**
	* Execute this action
	*
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
	* @return array | boolean
	* @access public
	*/
	function execute($settings, &$state)
	{
		// check required settings
		if (empty($settings['permission'])) return false;
		if (empty($settings['userid'])) return false;
		if (empty($state['assetid'])) return false;
		$granted = array_get_index($settings, 'granted', true);

		// set the permission on the asset
		// note that the asset manager already checks if the permission has been
		// and returns true if it has
		if (!$GLOBALS['SQ_SYSTEM']->am->setPermission($state['assetid'], $settings['userid'], $settings['permission'], $granted)) {
			return false;
		}

		return Array(
				'assetid'		=> $state['assetid'],
				'userid'		=> $settings['userid'],
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
	* @access public
	* @return boolean
	*/
	function getInterface($settings, $prefix, $write_access=false)
	{
		ob_start();

		// print the grant/deny selection
		if (empty($settings['granted'])) {
			$selected_granted = '1';
		} else {
			$selected_granted = $settings['granted'];
		}
		if ($write_access) {
			combo_box($prefix.'[granted]', Array('1' => 'Grant', '0' => 'Deny'), false, $selected_granted);
		} else {
			echo '<b>';
			echo ($selected_granted) ? 'Grant' : 'Deny';
			echo '</b> ';
		}

		// print the permission level selection
		if (empty($settings['permission'])) {
			$selected_permission = SQ_PERMISSION_READ;
		} else {
			$selected_permission = $settings['permission'];
		}
		$permissions = &Trigger_Action_Set_Permission::_getPermissionList();
		if ($write_access) {
			combo_box($prefix.'[permission]', $permissions, false, $selected_permission);
		} else {
			echo '<b>'.$permissions[$selected_permission].'</b>';
		}
		echo ' access for ';

		// print the asset finder
		if (empty($settings['userid'])) {
			$selected_userid = 0;
		} else {
			$selected_userid = $settings['userid'];
		}
		if ($write_access) {
			asset_finder($prefix.'[userid]', $selected_userid, Array('user' => 'D', 'user_group' => 'D'));
		} else {
			$user = null;
			if ($selected_userid > 0) {
				$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($selected_userid);
			}
			if (is_null($user)) {
				echo '<b>[No User Selected]</b>';
			} else {
				echo '<b>'.$user->name.'</b>';
			}
		}

		return ob_get_clean();

	}//end getInterface()


	/**
	* Function that handles the conversion of interface to settings
	* together with settings it is expected to populate the hash object
	*
	* @param array	$settings		a container for any data the action might want to save
	* @param string	$request_data	array of data corresponding to this action as
	* 								specified by the $prefix in getInterface.
	*
	* @access public
	* @return boolean
	*/
	function processInterface(&$settings, $request_data)
	{
		// check if the permission is valid
		$permission = array_get_index($request_data, 'permission', false);
		if (!$permission) return 'Permission not specified';

		$valid_permissions = &Trigger_Action_Set_Permission::_getPermissionList();
		if (isset($valid_permissions[$permission])) {
			$settings['permission'] = $permission;
		} else {
			return 'Specified permission is invalid';
		}

		// access must be either 1 or 0
		$granted = array_get_index($request_data, 'granted', false);
		if ($granted != '1' && $granted != '0') {
			return 'Access parameter is invalid';
		}
		$settings['granted'] = $granted;

		// a userid must have been selected
		$user_data = array_get_index($request_data, 'userid', false);
		$userid = $user_data['assetid'];

		if (empty($userid)) return 'User id is not specified';

		$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($userid);
		if (is_null($user)) {
			return 'Specified user is not a valid asset';
		}
		$settings['userid'] = $userid;

		return false;

	}//end processInterface()


	/**
	* Returns an array containing all the valid permission levels
	*
	* @access private
	* @return array(int => string)
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
	* @access public
	* @return array
	*/
	function getLocks($settings, &$state)
	{
		return Array($state['assetid'] => Array('permissions'));

	}//end getLocks()


}//end class

?>
