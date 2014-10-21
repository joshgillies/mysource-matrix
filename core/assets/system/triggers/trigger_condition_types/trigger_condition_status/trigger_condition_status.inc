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
* $Id: trigger_condition_status.inc,v 1.16 2012/08/30 01:09:15 ewang Exp $
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
* @version $Revision: 1.16 $
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
	* @param array	$settings	the stored settings for this condition
	* @param array	&$state		the state that describes the asset the condition is executing on
	*
	* @return boolean
	* @access public
	*/
	public static function evaluate($settings, &$state)
	{
		// grab the data we need to check the condition, if it's not already cached
		// note that new state is modified and new data is available to other conditions
		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset
			if (empty($state['assetid'])) {
				return FALSE;
			} else {
				$state['asset'] = $GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}

		if (empty($settings['status'])) {
			// if no settings, fail
			return FALSE;
		}

		// check the condition
		if ($state['asset']->status == $settings['status']) {
			return TRUE;
		} else {
			return FALSE;
		}

	}//end evaluate()


	/**
	* Get the HTML editing interface for this condition. Returns the string instead of printing it
	*
	* @param array		$settings		settings that this condition save in processInterface()
	* @param string		$prefix			unique prefix
	* @param boolean	$write_access	write access flag
	*
	* @return boolean
	* @access public
	*/
	public static function getInterface($settings, $prefix, $write_access=FALSE)
	{
		if (empty($settings['status'])) {
			// set to default
			$selected_status = SQ_STATUS_UNDER_CONSTRUCTION;
		} else {
			$selected_status = $settings['status'];
		}

		$status_list = trigger_condition_status::_getStatusList();

		if (!$write_access) {
			$form_element_extras = 'disabled="disabled"';
		} else {
			$form_element_extras = '';
		}

		// begin buffering basic options
		ob_start();
			if ($write_access) {
				combo_box($prefix.'[status]', $status_list, FALSE, $selected_status, NULL, $form_element_extras);
			} else {
				echo get_asset_status_icon($selected_status).'<strong>'. get_status_description($selected_status).'</strong>';
			}
			$basic_part_1 = ob_get_contents();
		ob_end_clean();

		return sprintf(translate('Asset has the current status of: %s'), $basic_part_1);


	}//end getInterface()


	/**
	* Function that handles the conversion of interface to settings
	*
	* @param array	&$settings		a container for any data the condition might want to save
	* @param array	$request_data	array of data corresponding to this condition as specified by the $prefix in the getInterface.
	*
	* @return mixed boolean|string
	* @access public
	*/
	public static function processInterface(&$settings, $request_data)
	{
		$status = array_get_index($request_data, 'status', FALSE);
		if (!$status) return 'No status selected';

		$status_list = trigger_condition_status::_getStatusList();

		// check if the status is valid
		if (isset($status_list[$status])) {
			$settings['status'] = $status;
			return FALSE;
		} else {
			return translate('Selected status is invalid');

		}

	}//end processInterface()


	/**
	* Returns an array containing all the valid statuses
	*
	* @return array
	* @access private
	*/
	protected static function _getStatusList()
	{
		$status = Array();
		foreach (get_constant_values('SQ_STATUS_') as $status_value) {
			$status[$status_value] = get_status_description($status_value);
		}
		return $status;

	}//end _getStatusList()


}//end class

?>
