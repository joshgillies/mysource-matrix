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
* $Id: trigger_condition_request_method.inc,v 1.2 2012/08/30 01:09:15 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_condition/trigger_condition.inc';

/**
* Trigger_Condition_Request_Method
*
* Trigger Condition on The HTTP Request Method
*
* @author  Shelley Liu <sliu@squiz.co.uk>
* @version $Revision: 1.2 $
* @package Squiz_UK_Clients
* @subpackage __core__
*/
class Trigger_Condition_Request_Method extends Trigger_Condition
{


	/**
	* Evaluate this condition
	*
	* The settings used by this condition are in the form:
	* <PRE>
	* Array(
	*       'request_method' => int      // the status of the asset to match
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
		$type_list = Array(0 => 'GET', 1 => 'POST', 2 => 'PUT', 3 => 'DELETE');

		return ($_SERVER['REQUEST_METHOD'] == $type_list[$settings['request_method']]);

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
		$options = Array(0 => 'GET', 1 => 'POST', 2 => 'PUT', 3 => 'DELETE');

		$value = array_get_index($settings, 'request_method', 'GET');

		ob_start();
			if ($write_access) {
				combo_box($prefix.'[request_method]', $options, FALSE, $value);
			} else {
				echo '<b>'.$options[$value].'</b>';
			}
			$var = ob_get_contents();
		ob_end_clean();

		return sprintf(translate('HTTP Request Method: %s'), $var);


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
		if (empty($request_data)) {
			return translate('There was an error with input data');

		}

		$settings['request_method'] = $request_data['request_method'];
		return FALSE;

	}//end processInterface()


}//end class

?>