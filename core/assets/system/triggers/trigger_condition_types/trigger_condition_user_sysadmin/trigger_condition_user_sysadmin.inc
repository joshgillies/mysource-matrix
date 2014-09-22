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
* $Id: trigger_condition_user_sysadmin.inc,v 1.4 2012/08/30 01:09:15 ewang Exp $
*
*/


/**
* Trigger Condition
*
* Checks whether current user is a sysadmin
*
*
* @author   Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Condition_User_Sysadmin extends Trigger_Condition
{


	/**
	* Evaluate this condition
	*
	* The settings used by this condition are in the form:
	* <PRE>
	* Array(
	*       'is_sysadmin'			=> BOOLEAN
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
		$res = ($GLOBALS['SQ_SYSTEM']->userSystemAdmin() == $settings['is_sysadmin']);
		return $res;

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
		ob_start();
			$options = Array(0 => translate('is not'), 1 => translate('is'));


			$value = array_get_index($settings, 'is_sysadmin', 1);
			if ($write_access) {
				combo_box($prefix.'[is_sysadmin]', $options, FALSE, $value);
			} else {
				echo '<b>'.$options[$value].'</b>';
			}
			$var = ob_get_contents();
		ob_end_clean();
		return sprintf(translate('Current user %s effectively a system administrator'), $var);


	}//end getInterface()


	/**
	* Function that handles the conversion of interface to settings
	*
	* @param array	&$settings		a container for any data the condition might want to save
	* @param array	$request_data	array of data corresponding to this condition
	*								as specified by the $prefix in the getInterface.
	*
	* @return mixed boolean|string
	* @access public
	*/
	public static function processInterface(&$settings, $request_data)
	{
		if (empty($request_data)) {
			return translate('There was an error with input data');

		}

		$settings['is_sysadmin'] = $request_data['is_sysadmin'];
		return FALSE;

	}//end processInterface()


}//end class
?>