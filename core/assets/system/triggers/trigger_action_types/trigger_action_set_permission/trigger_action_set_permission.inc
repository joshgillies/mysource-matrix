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
* $Id: trigger_action_set_permission.inc,v 1.8 2006/02/07 02:27:12 sdanis Exp $
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
* @version $Revision: 1.8 $
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
		if (empty($settings['permission'])) return FALSE;
		if (empty($settings['userid'])) return FALSE;
		if (empty($state['assetid'])) return FALSE;
		$granted = array_get_index($settings, 'granted', TRUE);

		// set the permission on the asset
		// note that the asset manager already checks if the permission has been
		// and returns true if it has
		if (!$GLOBALS['SQ_SYSTEM']->am->setPermission($state['assetid'], $settings['userid'], $settings['permission'], $granted)) {
			return FALSE;
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
	* @return boolean
	* @access public
	*/
	function getInterface($settings, $prefix, $write_access=FALSE)
	{
		// due to "access for" being the only stable text in this interface
		// we are not translating like the rest but just the text as it appears
		ob_start();

		// print the grant/deny selection
		if (empty($settings['granted'])) {
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

		$user =& $GLOBALS['SQ_SYSTEM']->am->getAsset($userid);
		if (is_null($user)) {
			return 'Specified user is not a valid asset';
		}
		$settings['userid'] = $userid;

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


}//end class

?>
