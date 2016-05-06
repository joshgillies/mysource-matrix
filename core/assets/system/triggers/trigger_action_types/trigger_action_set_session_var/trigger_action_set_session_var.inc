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
* $Id: trigger_action_set_session_var.inc,v 1.9 2012/10/02 06:19:40 akarelia Exp $
*
*/


require_once SQ_SYSTEM_ROOT.'/core/attributes/parameter_map/parameter_map.inc';

/**
* Trigger_Action_Set_Session_Var
*
* Purpose
* 	Trigger Action to set a session variable
*
* The settings used by this action are in the form:<BR />
* <PRE>
* Array(
*       'name'		=> string // the name of variable stored in session
*       'value'		=> string // the default value
*       'value_map'	=> string // parameter map setting
*       );
* </PRE>
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.9 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Set_Session_Var extends Trigger_Action
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Trigger_Action_Set_Session_Var($assetid=0)
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

		$settings['name']	= $request_data['name'];
		$settings['value']	= $request_data['value'];
		$settings['value_map'] = '';

		$new_prefix = str_replace(array('[',']'), '_', array_get_index($request_data, 'new_prefix', ''));
		if ($new_prefix) {
			$atr_parameter_map = new Asset_Attribute_Parameter_Map();
			$atr_parameter_map->process($new_prefix.'_parameter_map');
			$settings['value_map'] = $atr_parameter_map->value;
		}

		return FALSE;

	}//end processInterface()


	/**
	* Execute this action
	* Returns an array of data about what it did, or false on error
	*
	* The settings used by this action are in the form:<BR />
	* <PRE>
	* Array(
	*       'name'		=> string // the attribute to modify
	*       'value'     => string // the value to set
	*       'value_map' => string // parameter map setting
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
		$var_name = @$settings['name'];
		if (empty($var_name)) return FALSE;

		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset.
			if (empty($state['assetid'])) {
				return FALSE;
			} else {
				$state['asset'] = $GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}

		$old_current_asset = $GLOBALS['SQ_SYSTEM']->getGlobalDefine('CURRENT_ASSET', NULL);
		$GLOBALS['SQ_SYSTEM']->setGlobalDefine('CURRENT_ASSET', $state['asset']);

		$parameter_map_value = array_get_index($settings, 'value_map', serialize(Array()));
		$atr_parameter_map = new Asset_Attribute_Parameter_Map(0, $parameter_map_value);
		$param_map_value = $atr_parameter_map->getParameterValue('value');

		if (is_null($old_current_asset)) {
			$GLOBALS['SQ_SYSTEM']->unsetGlobalDefine('CURRENT_ASSET');
		} else {
			$GLOBALS['SQ_SYSTEM']->setGlobalDefine('CURRENT_ASSET', $old_current_asset);
		}

		// settings contains the 'fallback' static value.
		if (!empty($param_map_value)) {
			$value = $param_map_value;
		} else {
			$value = $settings['value'];
		}

		$_SESSION[SQ_SESSION_SANDBOX_INDEX][$var_name] = $value;

		return Array(
				'name'		=> $var_name,
				'value'		=> $value,
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
		$disabled  = ($write_access) ? '' : ' disabled="disabled"';
		$read_only = ($write_access) ? FALSE : TRUE;
		ob_start();
		$name = array_get_index($settings, 'name');
		text_box($prefix.'[name]', $name, 30, '', FALSE, $disabled);
		$name_input = ob_get_clean();

		ob_start();
		$value = array_get_index($settings, 'value');
		text_box($prefix.'[value]', $value, 40, '', FALSE, $disabled);
		$value_input = ob_get_clean();

		ob_start();
			echo '<div>';
			$new_prefix = str_replace(array('[',']'), '_', $prefix);
			hidden_field($prefix.'[new_prefix]', $new_prefix);

			$parameter_map_value = array_get_index($settings, 'value_map', serialize(Array()));
			$atr_parameter_map = new Asset_Attribute_Parameter_Map(0, $parameter_map_value);

			$atr_parameter_map->setParameter('value', 'Value');
			echo $atr_parameter_map->paint($new_prefix.'_parameter_map', $read_only);
			echo '</div>';

		$parameter_map_input = ob_get_clean();

		$output = sprintf(translate('<div class="sq-break-line-inputs">Set session variable name %1$s to value %2$s</div> %3$s'), $name_input, $value_input, $parameter_map_input);


		return $output;

	}//end getInterface()


}//end class

?>
