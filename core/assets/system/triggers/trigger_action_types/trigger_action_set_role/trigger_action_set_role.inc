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
* $Id: trigger_action_set_role.inc,v 1.1 2007/01/31 03:36:24 rhoward Exp $
*
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

/**
* Trigger Action Create Clone
*
* A trigger action that creates a clone of the firing asset
*
*
* @author   Huan Nguyen <hnguyen@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Set_Role extends Trigger_Action
{


	/**
	* Execute this action
	* Returns an array of data about what it did, or false on error
	*
	* The settings used by this action are in the form:
	* <PRE>
	* Array(
	*       'roleid'				=> int,		// the assetid of the role
	* 		'role_action'			=> string	// whether to add or delete
	*		'userids'
	*		);
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
		// if no settings, fail
		if (empty($settings['roleid'])) return FALSE;

		if (!$GLOBALS['SQ_SYSTEM']->am->assetExists($settings['roleid'])) {
			return FALSE;
		}

		$roleid		        	= array_get_index($settings, 'roleid', 0);
		$role_action			= array_get_index($settings, 'role_action', 'add');
		$userids				= array_get_index($settings, 'userid', 0);
		// check the optional settings, assign defaults

		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset.
			if (empty($state['assetid'])) {
				return FALSE;
			} else {
				$state['asset'] = &$GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}

		$am =& $GLOBALS['SQ_SYSTEM']->am;

		$major =& $am->getAsset($settings['roleid']);
		$minor =& $state['asset'];



		switch ($role_action) {
			case 'add':
				if (is_array($userids)) {
					foreach ($userids as $user_id) {
						$GLOBALS['SQ_SYSTEM']->am->setRole($state['assetid'], $roleid, $user_id);
					}
				} else {
					$GLOBALS['SQ_SYSTEM']->am->setRole($state['assetid'], $roleid, $userids);
				}

			break;
			case 'delete':
				$GLOBALS['SQ_SYSTEM']->am->deleteRole($state['assetid'], $roleid, $userids);
			break;
		}

		return Array(
				'assetid'	=> $state['asset']->id,
				'roleid'	=> $roleid,
				'userids'	=> $userids,
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
		// check settings, set defaults if necessary
		$selected_roleid		= array_get_index($settings, 'roleid', 0);
		$role_action			= array_get_index($settings, 'role_action', 'add');
		$selected_userids		= array_get_index($settings, 'userid', Array());

		// check if the selected asset is valid
		if ($selected_roleid && !$GLOBALS['SQ_SYSTEM']->am->assetExists($selected_roleid)) {
			$selected_roleid = 0;
		}

		ob_start();
			if ($write_access) {
				asset_finder($prefix.'[role]', $selected_roleid, Array('role' => 'I'));
			} else {
				if ($selected_roleid) {
					$selected_role_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($selected_roleid, Array(), TRUE, 'name');
					echo '<b>'.$selected_role_info[$selected_roleid].' (#'.$selected_roleid.')</b>';
				} else {
					echo '<b>['.translate('trigger_set_role_no_role_selected').']</b>';
				}
			}
			$role_select_role_part = ob_get_contents();
		ob_end_clean();

		ob_start();
			if ($write_access) {
				multiple_asset_finder($prefix.'[user_assetid]', $selected_userids, Array('user' => 'D', 'user_group' => 'D'));
			} else {
				if ($selected_userids) {
					$selected_assetname = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($selected_userids, Array(), TRUE, 'name');
					echo '<ul>';
					foreach ($selected_assetname as $assetid => $asset_name) {
						echo '<li><b>'.$asset_name.' (#'.$assetid.')</b></li>';
					}
					echo '</ul>';
				} else {
					echo '<b>['.translate('trigger_set_role_no_users_selected').']</b>';
				}
			}
			$role_select_user_part = ob_get_contents();
		ob_end_clean();

		$contents = ''.translate('select_a_role').'<br />'.
					$role_select_role_part.'<br />'.
					'<br />'.translate('select_users_for_role').'<br />'.
					$role_select_user_part.'';

		return $contents;

	}//end getInterface()


	/**
	* Handle the conversion of interface to settings
	* together with settings it is expected to populate the hash object
	*
	* @param array	&$settings		a container for any data the action might want to save
	* @param string	$request_data	array of data corresponding to this action as specified
	*								by the $prefix in getInterface.
	*
	* @return boolean
	* @access public
	*/
	function processInterface(&$settings, $request_data)
	{
		$link_type_list = get_link_type_names();

		// make sure the assetid isn't blank
		if (!empty($request_data['role']['assetid'])) {
			$selected_roleid = $request_data['role']['assetid'];
		} else {
			return translate('no_roles_set');
		}

		if (empty($request_data['user_assetid'])) {
			return translate('select_users_for_role');
		}

		$raw_userids = $request_data['user_assetid'];
		$userids     = Array();
		foreach ($raw_userids as $user) {
			if (!empty($user['assetid'])) {
				$userids[] = $user['assetid'];
			}
		}

		// optional fields
		$settings['role_action'] = array_get_index($request_data, 'role_action', 'add');
		$settings['roleid']      = $selected_roleid;
		$settings['userid']      = $userids;

		return FALSE;

	}//end processInterface()


	/**
	* Get the list of locks that the action needs to acquire before executing
	*
	* @param array	$settings	a container for any data the action might want to save
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return array
	* @access public
	*/
	function getLocks($settings, &$state)
	{
		return Array(
				$state['assetid']	=> Array('all'),
			   );

	}//end getLocks()


}//end class
?>
