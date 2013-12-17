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
* $Id: trigger_condition_rest_response.inc,v 1.3 2013/01/29 00:24:55 cupreti Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_condition/trigger_condition.inc';

/**
* Trigger Condition REST Response
*
* Checks whether an asset has a certain status set
*
*
* @author   Maki Maruyama <mmaruyama@squiz.co.uk>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/
class Trigger_Condition_Rest_Response extends Trigger_Condition
{


	/**
	* Evaluate this condition
	*
	* @param array	$settings	the stored settings for this condition
	* @param array	&$state		the state that describes the asset the condition is executing on
	*
	* @return boolean
	* @access public
	*/
	public static function evaluate($settings, &$state)
	{
		// the responses are passed in $state
		// we need something in response to test the condition
		if (!isset($state['event']['data']) || (is_array($state['event']['data']) && empty($state['event']['data']))) return FALSE;
		
		// if any of the options are missing, we can't compare values properly
		if (!isset($settings['match_target']) || !isset($settings['match_condition_type']) || 
			!isset($settings['match_type'])   || !isset($settings['match_value'])) {
			return FALSE;
		}//end if

		// looks like we have all the values we need to evaluate the condition
		// let's find out the values we should looked at
		$target_values = Array();
		switch ($settings['match_target']) {
			case 'rest_response_match_target_req_urls':
				$target_values = $state['event']['data']['request']['urls'];
				break;
			case 'rest_response_match_target_http_code':
			case 'rest_response_match_target_body':
				foreach ($state['event']['data']['responses'] as $response) {
					if ($settings['match_target'] == 'rest_response_match_target_http_code') {
						$target_values[] = $response['info']['http_code'];
					} else if ($settings['match_target'] == 'rest_response_match_target_body') {
						$target_values[] = $response['body'];
					}
				}
				break;
			default:
				// if unsupported option, don't pass this condition test
				return FALSE;
		}//end switch ($settings['match_target'])

		// loop through the target values we found and see if they match other condition options
		foreach ($target_values as $target_value) {
			$match_result = FALSE;
			switch ($settings['match_type']) {
				case 'rest_response_match_type_match':
					$match_result = strval($target_value) === $settings['match_value'] ? TRUE : FALSE;
					break;
				case 'rest_response_match_type_contain':
					$match_result = strpos($target_value, $settings['match_value']) === FALSE ? FALSE : TRUE;
					break;
				case 'rest_response_match_type_gt':
					if (!is_numeric($target_value) || !is_numeric($settings['match_value'])) return FALSE;
					$match_result = intval($target_value) > intval($settings['match_value']) ? TRUE : FALSE;
					break;
				case 'rest_response_match_type_lt':
					if (!is_numeric($target_value) || !is_numeric($settings['match_value'])) return FALSE;
					$match_result = intval($target_value) < intval($settings['match_value']) ? TRUE : FALSE;
					break;
				case 'rest_response_match_type_regex_match':
					$match_result = preg_match('/'.$settings['match_value'].'/', $target_value) ? TRUE : FALSE;
					break;
				default:
					$match_result = FALSE;
			}//end switch ($settings['match_type'])
			
			// if 'all match' option is selected but found the target value that does not match the specified value($target_value),
			// we have found a value does not satisfy the trigger condition
			if ($settings['match_condition_type'] == 'rest_response_match_condition_all' && !$match_result) return FALSE;
			
			// if 'at least one match' option is selected and if we found a target value that matches the specified value ($target_value),
			// it satisfies the trigger condition
			if ($settings['match_condition_type'] == 'rest_response_match_condition_one' && $match_result) return TRUE;
		}//end foreach ($target_values as $target_value)

		// if 'all match' option is selected and has reached this far, all values matched and satisfies the trigger condition
		if ($settings['match_condition_type'] == 'rest_response_match_condition_all') return TRUE;
		
		// if 'at least one match' option is selected and has reached this far, there was no value that has matched
		// and does not satisfy the trigger condition
		if ($settings['match_condition_type'] == 'rest_response_match_condition_one') return FALSE;

		// nothing should reach this far but if it did, it shouldn't cause any harm by letting this move on to trigger actions
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
		$conditions = array_get_index($settings, 'conditions', Array());

		$match_condition_type = Array(
									'rest_response_match_condition_all'    => translate('rest_response_match_condition_all'),
									'rest_response_match_condition_one'    => translate('rest_response_match_condition_one'),
								);
		$match_target         = Array(
									'rest_response_match_target_req_urls'  => translate('rest_response_match_target_req_urls'),
									'rest_response_match_target_http_code' => translate('rest_response_match_target_http_code'),
									'rest_response_match_target_body'      => translate('rest_response_match_target_body'),
								);
		$match_type           = Array(
									'rest_response_match_type_match'       => translate('rest_response_match_type_match'),
									'rest_response_match_type_contain'     => translate('rest_response_match_type_contain'),
									'rest_response_match_type_gt'          => translate('rest_response_match_type_gt'),
									'rest_response_match_type_lt'          => translate('rest_response_match_type_lt'),
									'rest_response_match_type_regex_match' => translate('rest_response_match_type_regex_match'),
								);

		// user default values if the setting has not been set
		if (!array_key_exists('match_condition_type', $settings)) {
			$settings['match_condition_type'] = 'rest_response_match_condition_all';
		}
		
		if (!array_key_exists('match_target', $settings)) {
			$settings['match_target'] = 'rest_response_match_target_http_code';
		}
		
		if (!array_key_exists('match_type', $settings)) {
			$settings['match_type'] = 'rest_response_match_type_match';
		}

		if (!array_key_exists('match_value', $settings)) {
			$settings['match_value'] = '';
		}

		// construct html elements
		ob_start();
			if ($write_access) {
				combo_box($prefix.'[match_condition_type]', $match_condition_type, FALSE, $settings['match_condition_type']);
				echo ' ';
				combo_box($prefix.'[match_target]', $match_target, FALSE, $settings['match_target']);
				echo ' ';
				combo_box($prefix.'[match_type]', $match_type, FALSE, $settings['match_type']);
				echo ' ';
				text_box($prefix.'[match_value]', array_get_index($settings, 'match_value'), 80, '', false, 'style="margin-top: 5px;"');
			} else {
				echo '<strong>'.$match_condition_type[$settings['match_condition_type']].'</strong> ';
				echo '<strong>'.$match_target[$settings['match_target']].'</strong> ';
				echo '<strong>'.$match_type[$settings['match_type']].'</strong> ';
				echo '<strong>'.$settings['match_value'].'</strong> ';
			}//end if

			$conditions = ob_get_contents();
		ob_end_clean();

		return translate('trigger_rest_response', $conditions);

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
		// check numeric only values
		if (($request_data['match_type'] == 'rest_response_match_type_gt' || 
				$request_data['match_type'] =='rest_response_match_type_lt') && 
				!is_numeric($request_data['match_value'])) {
			return translate('trigger_numeric_only');
		}//end if
		
		// save the selected options
		$settings['match_condition_type'] = array_get_index($request_data, 'match_condition_type', 'rest_response_match_condition_all');;
		$settings['match_target'] = array_get_index($request_data, 'match_target','rest_response_match_target_http_code');
		$settings['match_type'] = array_get_index($request_data, 'match_type','rest_response_match_type_match');
		$settings['match_value'] = array_get_index($request_data, 'match_value','');
		
		return FALSE;	// the return value is expected to indicate whether there has been an error or not

	}//end processInterface()


	/**
	* Determines whether a condition is allowed to be used multiple times in a trigger
	*
	* @return boolean
	* @access public
	*/
	public static function allowMultiple()
	{
		return TRUE;

	}//end allowMultiple()
	
}//end class

?>
