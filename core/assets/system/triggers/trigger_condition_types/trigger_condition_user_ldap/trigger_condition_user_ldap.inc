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
* $Id: trigger_condition_user_ldap.inc,v 1.1 2013/06/21 04:38:07 akarelia Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_condition/trigger_condition.inc';

/**
* Trigger Condition
*
* Checks whether current user is a LDAP user
*
*
* @author  Ash Karelia <akarelia@squiz.com.au>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Condition_User_LDAP extends Trigger_Condition
{


	/**
	* Evaluate this condition
	*
	* The settings used by this condition are in the form:
	* <PRE>
	* Array(
	*       'is_ldap_user' => boolean,
	*      );
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
		$current_user   = $GLOBALS['SQ_SYSTEM']->am->getAsset($GLOBALS['SQ_SYSTEM']->currentUserId());
		$is_a_ldap_user = ($current_user instanceof Ldap_User || $current_user instanceof Ldap_Simple_Edit_User || $current_user instanceof Ldap_Backend_User);

		return ($is_a_ldap_user == $settings['is_ldap_user']);

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


			$value = array_get_index($settings, 'is_ldap_user', 1);
			if ($write_access) {
				combo_box($prefix.'[is_ldap_user]', $options, FALSE, $value);
			} else {
				echo '<b>'.$options[$value].'</b>';
			}
		$is_ldap_user_str = ob_get_clean();

		return sprintf(translate('Current user %s a LDAP user'), $is_ldap_user_str);


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

		$settings['is_ldap_user'] = $request_data['is_ldap_user'];

		return FALSE;

	}//end processInterface()


}//end class
?>
