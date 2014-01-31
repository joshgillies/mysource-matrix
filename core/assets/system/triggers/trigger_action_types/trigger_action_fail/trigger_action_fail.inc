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
* $Id: trigger_action_fail.inc,v 1.8 2012/08/30 01:09:13 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';

/**
* Trigger_Action_Fail
*
* Only those who dare to fail greatly can ever achieve greatly.
*	-- Robert F. Kennedy:
*
* This action just fails.
*
* @author  Elden McDonald <emcdonald@squiz.net>
* @version $Revision: 1.8 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Fail extends Trigger_Action
{


	/**
	* Execute this action
	*
	* ie fail.
	*
	* @param array	$settings	the stored settings for this action
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return boolean
	* @access public
	*/
	public static function execute($settings, &$state)
	{
		$error_level = $settings['error_level'];
		if (!empty($error_level)) {
			trigger_error($settings['error_message'], $error_level);
		}
		return FALSE;

	}//end execute()


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

		// check settings, set defaults if necessary
		$error_level		= array_get_index($settings, 'error_level', 0);
		$error_message        = array_get_index($settings, 'error_message', '');

			ob_start();
				$error_level_list = Array(
										0				=> translate('no_error'),
										E_USER_NOTICE	=> translate('notice'),
										E_USER_WARNING	=> translate('warning'),
										E_USER_ERROR	=> translate('error'),
									);
				if ($write_access) {
					combo_box($prefix.'[error_level]', $error_level_list, FALSE, $error_level);
				} else {
					echo '<b>'.$error_level_list[$error_level].'</b>';
				}

				$part_1 = ob_get_contents();
			ob_end_clean();

			ob_start();

				if ($write_access) {
					text_area($prefix.'[error_message]', $error_message, 40, 5);
				} else {
					echo '<b>'.$error_message.'</b>';
				}

				$part_2 = ob_get_contents();
			ob_end_clean();


			// now patch together the output
			$contents =	translate('trigger_fail_level_label').$part_1.
						'<br /><br />'.
						'<span style="vertical-align:top;">'.translate('trigger_fail_message_label').'</span>'.$part_2.
						'<br /><br />'.
						'<span><b>'.translate('trigger_always_fails_warning').'</b></span>';

		return $contents;

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


		$settings['error_level'] = $request_data['error_level'];

		$settings['error_message']        = array_get_index($request_data, 'error_message', '');

		return FALSE;

	}//end processInterface()


	/**
	* Get the list of locks that the action needs acquire before executing
	*
	* @param array	$settings	the stored settings for this action
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return array
	* @access public
	*/
	public static function getLocks($settings, &$state)
	{
		return Array();

	}//end getLocks()


}//end class

?>
