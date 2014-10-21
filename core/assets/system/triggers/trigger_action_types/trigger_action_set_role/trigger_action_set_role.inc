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
* $Id: trigger_action_set_role.inc,v 1.15 2012/08/30 01:09:14 ewang Exp $
*
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';

/**
* Trigger Action Set Role
*
* A trigger action that creates a clone of the firing asset
*
*
* @author   Huan Nguyen <hnguyen@squiz.net>
* @version $Revision: 1.15 $
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
	public static function execute($settings, &$state)
	{
		$result = TRUE;
		// if no settings, fail
		if (empty($settings['roleid'])) return FALSE;

		if (!$GLOBALS['SQ_SYSTEM']->am->assetExists($settings['roleid'])) {
			return FALSE;
		}

		// check the optional settings, assign defaults
		$roleid						= array_get_index($settings, 'roleid', 0);
		$role_action				= array_get_index($settings, 'role_action', 'add');
		$current_user				= array_get_index($settings, 'use_current_user', FALSE);
		$userids					= array_get_index($settings, 'userid', Array());
		$cascade					= array_get_index($settings, 'cascade', FALSE);
		$cascade_to_dependents		= array_get_index($settings, 'cascade_to_dependents', FALSE);
		$delete_all_before_adding	= array_get_index($settings, 'delete_all_before_adding', FALSE);

		if (!is_array($userids)) $userids = Array ($userids);

		if($current_user) {
			$userids = array_merge($userids, Array($GLOBALS['SQ_SYSTEM']->user->id));
		}

		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset.
			if (empty($state['assetid'])) {
				return FALSE;
			} else {
				$state['asset'] = $GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}

		$am =& $GLOBALS['SQ_SYSTEM']->am;

		$major = $am->getAsset($settings['roleid']);
		$minor =& $state['asset'];

		$role_vars = Array();
		switch ($role_action) {
			case 'add':
				$role_vars['add'][$roleid] = $userids;
			break;

			case 'delete':
				$role_vars['delete'][$roleid] = 0;
			break;
		}

		if($delete_all_before_adding) {
			$current_roles = $GLOBALS['SQ_SYSTEM']->am->getRole($state['assetid']);
			if(!empty($current_roles)) {
				$GLOBALS['SQ_SYSTEM']->am->deleteRole($state['assetid'], $roleid);
			}
		}

		if (!empty($role_vars)) {
			if ($cascade || $cascade_to_dependents) {
				$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
				$vars = Array(
							'assets'		=> Array(
												$state['asset']->id	=> Array(
																		'type_code'	=> $state['asset']->type(),
																	   ),
											   ),
							'role_changes'	=> $role_vars,
						);

				if (!$cascade) $vars['dependants_only'] = 1;
				$errors = $hh->freestyleHIPO('hipo_job_edit_roles', $vars);
				$result = empty($errors);
			} else {
				if (!empty($role_vars['add'])) {
					foreach ($role_vars['add'] as $roleid => $userids) {
						foreach ($userids as $userid) {
							if (!$GLOBALS['SQ_SYSTEM']->am->setRole($state['assetid'], $roleid, $userid)) {
								$result = FALSE;
								break;
							}
						}
					}
				}

				if ($result && !empty($role_vars['delete'])) {
					foreach ($role_vars as $roleid => $null) {
						if (!$GLOBALS['SQ_SYSTEM']->am->deleteRole($state['assetid'], $roleid)) {
							$result = FALSE;
							break;
						}
					}
				}

			}//end if ($cascade || $cascade_to_dependents)
		}//end if (!empty($role_vars))

		if (!$result) {
			return FALSE;
		} else {
			return Array(
					'assetid'				=> $state['asset']->id,
					'roleid'				=> $roleid,
					'userids'				=> $userids,
					'cascade'				=> $cascade,
					'cascade_to_dependents'	=> $cascade_to_dependents,
				   );
		}

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
		// check settings, set defaults if necessary
		$selected_roleid			= array_get_index($settings, 'roleid', 0);
		$role_action				= array_get_index($settings, 'role_action', 'add');
		$selected_userids			= array_get_index($settings, 'userid', Array());
		$current_user				= array_get_index($settings, 'use_current_user', FALSE);
		$cascade					= array_get_index($settings, 'cascade', FALSE);
		$cascade_to_dependents		= array_get_index($settings, 'cascade_to_dependents', FALSE);
		$delete_all_before_adding	= array_get_index($settings, 'delete_all_before_adding', FALSE);

		// check if the selected asset is valid
		if ($selected_roleid && !$GLOBALS['SQ_SYSTEM']->am->assetExists($selected_roleid)) {
			$selected_roleid = 0;
		}

		ob_start();
			if ($write_access) {
				asset_finder($prefix.'[role]', $selected_roleid, Array('role' => 'I'));
			} else {
				if ($selected_roleid) {
					echo get_asset_tag_line($selected_roleid);
				} else {
					echo '<em>['.translate('No Role Selected').']</em>';

				}
			}
			$role_select_role_part = ob_get_contents();
		ob_end_clean();

		ob_start();
			if ($write_access) {
				multiple_asset_finder($prefix.'[user_assetid]', $selected_userids, Array('user' => 'D', 'user_group' => 'D'));
				echo '<em>AND / OR</em><br/>';
				echo check_box($prefix.'[use_current_user]', 1, $current_user);
				label(translate('Use Current User (one who fires the trigger)'), $prefix.'[use_current_user]');

				echo '<br/><br/>';
			} else {
				if ($selected_userids || $current_user) {
					if($selected_userids) {
						$selected_assetname = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($selected_userids, Array(), TRUE, 'name');
						echo '<ul>';
						foreach ($selected_assetname as $assetid => $asset_name) {
							echo '<li><b>'.$asset_name.' (#'.$assetid.')</b></li>';
						}
						echo '</ul>';
					}
					echo '<br/>';
					echo '<img src="'.sq_web_path('lib').'/web/images/'.($current_user ? 'tick' : 'cross').'.gif" alt="'.($current_user ? translate('Yes') : translate('No')).'" /> ';


					echo translate('Use Current User (one who fires the trigger)');

					echo '<br/>';

				} else {
					echo '<b>['.translate('No Users Selected').']</b>';

				}
			}
			$role_select_user_part = ob_get_contents();
		ob_end_clean();

		ob_start();
			if ($write_access) {
				check_box($prefix.'[cascade]', '1', $cascade);
				label(translate('Yes, cascade role assignment changes'), $prefix.'[cascade]');

				echo '<br />';
				check_box($prefix.'[cascade_to_dependents]', '1', $cascade_to_dependents);
				label(translate('Cascade roles to dependents'), $prefix.'[cascade_to_dependents]');

			} else {
				echo '<img src="'.sq_web_path('lib').'/web/images/'.($cascade ? 'tick' : 'cross').'.gif" alt="'.($cascade ? translate('Yes') : translate('No')).'" /> ';


				echo translate('Yes, cascade role assignment changes');


				echo '<br />';
				echo '<img src="'.sq_web_path('lib').'/web/images/'.($cascade_to_dependents ? 'tick' : 'cross').'.gif" alt="'.($cascade_to_dependents ? translate('Yes') : translate('No')).'" /> ';


				echo translate('Cascade roles to dependents');

			}
			$role_cascade_part = ob_get_contents();
		ob_end_clean();

		ob_start();
			if ($write_access) {
				check_box($prefix.'[delete_all_before_adding]', '1', $delete_all_before_adding);
				label(translate('Delete all the Users from the Role before assigning new one'), $prefix.'[delete_all_before_adding]');

				echo '<br />';
			} else {
				echo '<img src="'.sq_web_path('lib').'/web/images/'.($delete_all_before_adding ? 'tick' : 'cross').'.gif" alt="'.($delete_all_before_adding ? translate('Yes') : translate('No')).'" /> ';


				echo translate('Delete all the Users from the Role before assigning new one');

			}
			$role_delete_all_part = ob_get_contents();
		ob_end_clean();

		$contents = '<div class="sq-break-line-inputs">'.translate('Select a role').': '.

					$role_select_role_part.'</div>'.
					'<div class="sq-break-line-inputs">'.translate('Select the users who will perform the role on this asset').': '.

					$role_select_user_part.'</div>'.
					'<div class="sq-break-line-inputs">'.$role_cascade_part.'<br />'.
					$role_delete_all_part.'</div>';

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
	public static function processInterface(&$settings, $request_data)
	{
		$link_type_list = get_link_type_names();

		// make sure the assetid isn't blank
		if (!empty($request_data['role']['assetid'])) {
			$selected_roleid = $request_data['role']['assetid'];
		} else {
			return translate('No roles have been assigned');

		}

		$raw_userids = $request_data['user_assetid'];
		$userids     = Array();
		foreach ($raw_userids as $user) {
			if (!empty($user['assetid'])) {
				$userids[] = $user['assetid'];
			}
		}
		$settings['use_current_user']   = array_get_index($request_data, 'use_current_user', FALSE);

		if (empty($userids) && !$settings['use_current_user']) {
			return translate('Select the users who will perform the role on this asset');

		}

		// optional fields
		$settings['role_action']				= array_get_index($request_data, 'role_action', 'add');
		$settings['roleid']						= $selected_roleid;
		$settings['userid']						= $userids;
		$settings['cascade']					= array_get_index($request_data, 'cascade', FALSE);
		$settings['delete_all_before_adding']	= array_get_index($request_data, 'delete_all_before_adding', FALSE);
		if ($settings['cascade']) {
			$settings['cascade_to_dependents'] = TRUE;
		} else {
			$settings['cascade_to_dependents'] = array_get_index($request_data, 'cascade_to_dependents', FALSE);
		}

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
	public static function getLocks($settings, &$state)
	{
		return Array(
				$state['assetid']	=> Array('all'),
			   );

	}//end getLocks()


}//end class
?>
