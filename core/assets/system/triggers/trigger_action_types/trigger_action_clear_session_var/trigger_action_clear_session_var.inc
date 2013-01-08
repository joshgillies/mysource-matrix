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
* $Id: trigger_action_clear_session_var.inc,v 1.1 2013/01/08 00:36:05 cupreti Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';
require_once SQ_SYSTEM_ROOT.'/core/attributes/parameter_map/parameter_map.inc';

/**
* Trigger_Action_Clear_Session_Var
*
* Purpose
* 	Trigger Action to clear one or more session variables
*
* The settings used by this action are in the form:<BR />
* <PRE>
* Array(
*       'name'		=> string // the name of session variable to clear
*       'is_prefix'	=> string // whether the name the var prefix or the full var name
*       'clear_all'	=> string // whether to clear all the session variables
*       );
* </PRE>
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Clear_Session_Var extends Trigger_Action
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


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
		$settings['name'] = array_get_index($request_data, 'name', '');
		$settings['is_prefix'] = array_get_index($request_data, 'is_prefix', 0);
		$settings['clear_all'] = array_get_index($request_data, 'clear_all', 0);

		return FALSE;

	}//end processInterface()


	/**
	* Execute this action
	* Returns an array of data about what it did, or false on error
	*
	* The settings used by this action are in the form:<BR />
	* <PRE>
	* Array(
	*       'name'		=> string // the name of session variable to clear
	*       'is_prefix'	=> string // whether the name the var prefix or the full var name
	*       'clear_all'	=> string // whether to clear all the session variables
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
		// check required settings
		$clear_all = array_get_index($settings, 'clear_all', 0);
		$var_name = trim(array_get_index($settings, 'name', ''));
		$is_prefix = array_get_index($settings, 'is_prefix', 0);

		if (!isset($_SESSION[SQ_SESSION_SANDBOX_INDEX]) || (!$clear_all && !$var_name)) {
			return FALSE;
		}

		$removed_vars = Array();
		if ($clear_all) {
			// Remove all the session vars
			$removed_vars[] = array_keys($_SESSION[SQ_SESSION_SANDBOX_INDEX]);
			$_SESSION[SQ_SESSION_SANDBOX_INDEX] = Array();
		} else if ($is_prefix) {
			// Remove the session vars starting with the given prefix
			foreach($_SESSION[SQ_SESSION_SANDBOX_INDEX] as $key => $val) {
				if (strpos($key, $var_name) === 0) {
					unset($_SESSION[SQ_SESSION_SANDBOX_INDEX]);
					$removed_vars[] = $key;
				}
			}//end foreach
		} else {
			// Just remove the specified var
			if (isset($_SESSION[SQ_SESSION_SANDBOX_INDEX][$var_name])) {
				unset($_SESSION[SQ_SESSION_SANDBOX_INDEX][$var_name]);
				$removed_vars[] = $var_name;
			}
		}

		return Array(
				'cleared_vars'	=> $removed_vars,
			   );

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
		// begin buffering basic options
		$prefix_hash = md5($prefix);
		ob_start();
		$clear_all = array_get_index($settings, 'clear_all');
		check_box($prefix.'[clear_all]', 1, $clear_all, '', !$write_access ? 'disabled' : '');
		$clear_all_input = ob_get_clean();

		ob_start();
		$name = array_get_index($settings, 'name', '');
		if ($write_access) {
			text_box($prefix.'[name]', $name);
		} else {
			echo '<b>'.$name.'</b>';
		}
		$name_input = ob_get_clean();

		ob_start();
		$is_prefix = array_get_index($settings, 'is_prefix', 0);
		combo_box($prefix.'[is_prefix]', Array('name', 'prefix'), FALSE, $is_prefix, 0, !$write_access ? 'disabled' : '');
		$is_prefix_input = ob_get_clean();

		$output = translate('trigger_clear_session_var_interface', $name_input, $is_prefix_input, $clear_all_input);

		return $output;

	}//end getInterface()


}//end class

?>
