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
* $Id: trigger_action_release_lock.inc,v 1.9 2012/08/30 01:09:13 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';

/**
* Trigger_Action_Release_Lock
*
* A trigger action that releases asset locks
*
*
* @author Rayn Ong <rong@squiz.net>
* @version $Revision: 1.9 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Release_Lock extends Trigger_Action
{


	/**
	* Execute this action
	* Returns an array of data about what it did, or false on error
	*
	* The settings used by this action are in the form:<BR />
	* <PRE>
	* Array(
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
		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset.
			if (empty($state['assetid'])) {
				return FALSE;
			} else {
				$state['asset'] = $GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}

		// release all locks with the same source_lockid prefix
		switch ($settings['release_type']) {
			case 'on_asset':
				if (!$GLOBALS['SQ_SYSTEM']->am->releaseLock($state['asset']->id, 'all')) {
					return FALSE;
				}
			break;

			case 'held':
				// This only works if you're a user
				if ($state['asset'] instanceof User) {
					$state['asset']->releaseUserLocks();
				} else {
					return FALSE;
				}
			break;

		}//end switch

		return Array(
				'assetid'	=> $state['asset']->id,
			   );

	}//end execute()


	/**
	* Get the HTML editing interface for this action. Returns the string instead of printing it
	*
	* @param array		$settings		settings that this condition saves in processInterface()
	* @param string		$prefix		unique prefix
	* @param boolean	$write_access	determines whether the interface is editable
	*
	* @return boolean
	* @access public
	*/
	public static function getInterface($settings, $prefix, $write_access=FALSE)
	{
		$interface = '';
		$lock_type = array_get_index($settings, 'release_type', 'on_asset');

		if (!$write_access) {
			$form_element_extras = 'disabled="disabled"';
		} else {
			$form_element_extras = '';
		}

		ob_start();
			echo '<ul style="margin: 0; list-style: none">';
				echo '<li>';
					echo radio_button($prefix.'[release_type]', 'on_asset', ($lock_type == 'on_asset'), '', $form_element_extras);
					label(translate('trigger_action_release_lock_on_asset'), $prefix.'[release_type]_on_asset');
				echo '</li>';
				echo '<li>';
					echo radio_button($prefix.'[release_type]', 'held', ($lock_type == 'held'), '', $form_element_extras);
					label(translate('trigger_action_release_lock_held'), $prefix.'[release_type]_held');
				echo '</li>';
			echo '</ul>';
		$interface = ob_get_clean();

		return $interface;

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
		$release_type = array_get_index($request_data, 'release_type', FALSE);
		if (!$release_type) return 'Release type has not been specified';

		$release_list = Array(
							'on_asset'	=> 'Release Locks on Asset',
							'held'		=> 'Release Locks Held by Asset',
						);

		// check if the status is valid
		if (isset($release_list[$release_type])) {
			$settings['release_type'] = $release_type;
			return FALSE;
		} else {
			return 'Specified release type is invalid';
		}

	}//end processInterface()


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


}//end class

?>
