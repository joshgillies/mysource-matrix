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
* $Id: trigger_condition_status.inc,v 1.2 2005/03/01 00:25:31 arailean Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

/**
* Trigger Condition
*
* Checks whether an asset has a certain status set
*
*
* @author   Andrei Railean <arailean@squiz.net>
* @author   Robert Howard <rhoward@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Condition_Status extends Trigger_Condition
{

	/**
	* Evaluate this condition
	*
	* The settings used by this condition are in the form:
	* <PRE>
	* Array(
	*       'status' => int      // the status of the asset to match
	*       );
	* </PRE>
	*
	* @param array(string=>mixed)   $settings   the stored settings for this condition
	* @param array(string=>mixed)   &$state     the state that describes the asset the condition is executing on
	*
	* @return boolean
	* @access public
	*/
	function evaluate($settings, &$state)
	{
		// grab the data we need to check the condition, if it's not already cached
		// note that new state is modified and new data is available to other conditions
		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset
			if (empty($state['assetid'])) {
				return false;
			} else {
				$state['asset'] = &$GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}

		if (empty($settings['status'])) {
			// if no settings, fail
			return false;
		}

		// check the condition
		if ($state['asset']->status == $settings['status']) {
			return true;
		} else {
			return false;
		}

	}//end evaluate()


	/**
	* Get the HTML editing interface for this condition. Returns the string instead of printing it
	*
	* @param Array      $settings  settings that this condition save in processInterface()
	* @param string     $prefix    unique prefix
	* @param boolean    $write_access   write access flag
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

		$status_list = &trigger_condition_status::_getStatusList();

		if (!$write_access) {
			$form_element_extras = 'disabled="disabled"';
		} else {
			$form_element_extras = '';
		}

		// capture anything printed
		ob_start();

		echo "Asset has the current status of:";
		combo_box($prefix.'[status]', $status_list, false, $selected_status, null, $form_element_extras);

		return ob_get_clean();

	}//end getInterface()


	/**
	* Function that handles the conversion of interface to settings
	* together with settings it is expected to populate the hash object
	* Consult the hash object to see the list of setter functions
	*
	* @param Array          $settings       a container for any data the condition might want to save
	* @param object Hash    $hash           hash object that takes data that helps to speed up trigger firing
	* @param array          $request_data   array of data corresponding to this condition as specified by the $prefix in the getInterface.
	*
	* @access public
	* @return boolean
	*/
	function processInterface(&$settings, &$hash, $request_data)
	{
		$status = array_get_index($request_data, 'status', false);
		if (!$status) {
			return false;
		}

		$status_list = &trigger_condition_status::_getStatusList();

		// check if the status is valid
		if (isset($status_list[$status])) {
			$settings['status'] = $status;
			return true;
		} else {
			return false;
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
