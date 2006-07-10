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
* $Id: trigger_action_set_status.inc,v 1.11.2.1 2006/07/10 06:47:23 rong Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';

/**
* Trigger Action Set Status
*
* A trigger action that sets the status of an asset
*
*
* @author  Andrei Railean <arailean@squiz.net>
* @author  Robert Howard <rhoward@squiz.net>
* @version $Revision: 1.11.2.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Set_Status extends Trigger_Action
{


	/**
	* Execute this action
	*
	* Returns an array of data about what it did, or false on error
	*
	* The settings used by this action are in the form:<BR />
	* <PRE>
	* Array(
	*       'status' => int // the status value to set the asset to
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
		if (empty($settings['status'])) return false;

		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset.
			if (empty($state['assetid'])) {
				return false;
			} else {
				$state['asset'] = &$GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}

		$old_status = $state['asset']->status;
		if ($old_status != $settings['status']) {
			$available_statuses = $state['asset']->getAvailableStatii();

			if (!isset($available_statuses[$settings['status']])) {
				// invalid status, fail
				return false;
			}

			// need to use a hipo job to set status (handles dependant children)
			$hh =& $GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$vars = Array(
						'assetid'		=> $state['assetid'],
						'new_status'	=> $settings['status'],
					);
			$status_errors = $hh->freestyleHipo('hipo_job_edit_status', $vars);

			if (!empty($status_errors)) {
				return FALSE;
			}
		}

		return Array(
				'assetid'		=> $state['asset']->id,
				'old_status'	=> $old_status,
				'new_status'	=> $state['asset']->status,
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
		if (empty($settings['status'])) {
			// set to default
			$selected_status = SQ_STATUS_UNDER_CONSTRUCTION;
		} else {
			$selected_status = $settings['status'];
		}

		$status_list = Trigger_Action_Set_Status::_getStatusList();

		if (!$write_access) {
			$form_element_extras = 'disabled="disabled"';
		} else {
			$form_element_extras = '';
		}

		ob_start();
			combo_box($prefix.'[status]', $status_list, false, $selected_status, null, $form_element_extras);
			$basic_part_1 = ob_get_contents();
		ob_end_clean();


		return translate('trigger_set_status', $basic_part_1);

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
		$status = array_get_index($request_data, 'status', false);
		if (!$status) return 'Status has not been specified';

		$status_list = Trigger_Action_Set_Status::_getStatusList();

		// check if the status is valid
		if (isset($status_list[$status])) {
			$settings['status'] = $status;
			return false;
		} else {
			return 'Specified status is invalid';
		}

	}//end processInterface()


	/**
	* Returns an array containing all the valid statuses
	*
	* @access private
	* @return array(int => string)
	*/
	function _getStatusList()
	{
		$status = Array();
		foreach (get_constant_values('SQ_STATUS_') as $status_value) {
			$status[$status_value] = get_status_description($status_value);
		}
		return $status;

	}//end _getStatusList()


}//end class

?>
