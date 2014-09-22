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
* $Id: trigger_condition_status_change_to.inc,v 1.2 2012/08/30 01:09:15 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

/**
* Trigger Condition
*
* Checks whether an asset status set to change to is what we are looking for
* This can nly be used in lines with
* Trigger Event Status Change and Trigger Event Before Status Change
*
* @author  Ash Karelia <akarelia@squiz.com.au>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Condition_Status_Change_To extends Trigger_Condition
{


	/**
	* Evaluate this condition
	*
	* The settings used by this condition are in the form:
	* <PRE>
	* Array(
	*       'statii' => Array(int, int)      // the statii of the asset to match
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

		if (empty($settings['statii'])) {
			// if no settings, fail
			return FALSE;
		}

		// check the condition
		if (in_array($state['event']['data']['new_status'], $settings['statii'])) return TRUE;

		return FALSE;

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
		if (empty($settings['statii'])) {
			// set to default
			$selected_status = Array();
		} else {
			$selected_status = $settings['statii'];
		}

		$status_list = self::_getStatusList();

		if (!$write_access) {
			$form_element_extras = 'disabled="disabled"';
		} else {
			$form_element_extras = '';
		}

		// begin buffering basic options
		ob_start();
			if ($write_access) {
				combo_box($prefix.'[statii]', $status_list, TRUE, $selected_status, NULL, $form_element_extras);
			} else {
				echo '<table style="margin: 5px 0 0 10px;">';
				foreach($selected_status as $print_status) {
					echo '<tr><td style="padding: 0 0 2px 0;">'.get_asset_status_icon($print_status).'<strong>'. get_status_description($print_status).'</strong></td></tr>';
				}
				echo '</table>';
			}
			$basic_part_1 = ob_get_contents();
		ob_end_clean();

		return sprintf(translate('Status of the <i>(firing)</i> Asset is changed to: %s'), $basic_part_1);


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
		$statii = array_get_index($request_data, 'statii', Array());
		if (!$statii) return 'No status selected';

		$status_list = self::_getStatusList();

		// check if the status is valid
		foreach($statii as $status) {
			if (isset($status_list[$status])) {
				$settings['statii'][] = $status;
			} else {
				return translate('Selected status is invalid');

			}
		}

		return FALSE;

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
