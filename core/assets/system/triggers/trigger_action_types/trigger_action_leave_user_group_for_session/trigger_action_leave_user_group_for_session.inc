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
*
*/


require_once SQ_SYSTEM_ROOT.'/core/attributes/parameter_map/parameter_map.inc';

/**
* Trigger_Action_Leave_User_Group_For_Session
*
* Purpose
* 	Trigger Action to add the current user to a user group while the current session lasts
*
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Leave_User_Group_For_Session extends Trigger_Action
{
	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);
	}//end constructor


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
		// we only allow user group type
		$type_codes = Array(
								'user_group' => 'D',
							);

		// buffer the user group asset chooser
		ob_start();
		$user_group_to_leave = isset($settings['user_group_to_leave']) ? $settings['user_group_to_leave'] : Array();
		if ($write_access) {
			multiple_asset_finder($prefix.'[user_group_to_leave]', $user_group_to_leave, $type_codes);
		} else if (!empty($user_group_to_leave)) {
			echo '<ul>';
			foreach($user_group_to_leave as $group) {
				    echo '<li>';
				    echo get_asset_tag_line($group);
				    echo '</li>';
			}
			echo '</ul>';
		}
		$user_group_input = ob_get_clean();

		// display both the asset chooser and the text box
		$output = sprintf(translate('<p>Temporarily remove current user from specified user groups for current session.</p>Select user groups %1$s'), $user_group_input);


		return $output;

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
		$settings['user_group_to_leave'] = Array();
		if (isset($request_data['user_group_to_leave']) && !empty($request_data['user_group_to_leave'])) {
			foreach ($request_data['user_group_to_leave'] as $array) {
				// check for fields with no value
				if ($array['assetid'] != 0) {
					$settings['user_group_to_leave'][] = $array['assetid'];
				}
			}
		}
		return FALSE;

	}//end processInterface()


	/**
	* Execute this action
	* Returns an array of data about what it did, or false on error
	*
	* @param array	$settings	the stored settings for this action
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return mixed array|boolean
	* @access public
	*/
	public static function execute($settings, &$state)
	{


		// get the current user id and double check it's a User asset
		$current_user = $GLOBALS['SQ_SYSTEM']->am->getAsset($GLOBALS['SQ_SYSTEM']->user->id);
		if (!$current_user || !$current_user instanceof User) {
			return FALSE;
		}

		// get user groups the current user is in
		$current_groups = $current_user->getUserGroups();

		// groups to remove from
		$groups = isset($settings['user_group_to_leave']) ? $settings['user_group_to_leave'] : Array();

		 if(!empty($groups)) {
		     // exclude  groups from current session
		     $_SESSION['sq_effective_user_groups'][$GLOBALS['SQ_SYSTEM']->user->id] = array_diff($_SESSION['sq_effective_user_groups'][$GLOBALS['SQ_SYSTEM']->user->id], $groups);
		 }

		return TRUE;

	}//end execute()


}//end class

?>
