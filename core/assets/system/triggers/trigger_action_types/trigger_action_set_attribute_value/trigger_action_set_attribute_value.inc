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
* $Id: trigger_action_set_attribute_value.inc,v 1.24 2012/08/30 01:09:13 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';
require_once SQ_SYSTEM_ROOT.'/core/attributes/parameter_map/parameter_map.inc';

/**
* Trigger_Action_Set_Attribute_Value
*
* A trigger action that sets a the value of an asset attribute
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.24 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Set_Attribute_Value extends Trigger_Action
{


	/**
	* Execute this action
	* Returns an array of data about what it did, or false on error
	*
	* The settings used by this action are in the form:<BR />
	* <PRE>
	* Array(
	*       'attribute' => string // the attribute to modify
	*       'value'     => string // the value to set
	*       'value_map' => string // parameter map setting
	*       'replace_keywords_in_set_value' => boolean (0|1), // the flag to indicate if this trigger action will replace the keywords in the set value with the broadcasting asset acting as the current asset in the replacement context
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
		if (empty($settings['attribute'])) return FALSE;

		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset.
			if (empty($state['assetid'])) {
				return FALSE;
			} else {
				$state['asset'] = $GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}

		if (is_null($state['asset'])) return FALSE;


		$parameter_map_value = array_get_index($settings, 'value_map', serialize(Array()));
		$atr_parameter_map = new Asset_Attribute_Parameter_Map(0, $parameter_map_value);
		$param_map_value = $atr_parameter_map->getParameterValue('attribute_value');


		// settings contains the 'fallback' static value.
		if (!empty($param_map_value)) {
			$value = $param_map_value;
		} else {
			$value = $settings['value'];
		}


		// handle serialised attributes
		if (is_scalar($value)) {
			$attr = $state['asset']->getAttribute($settings['attribute']);
			if (is_null($attr)) return FALSE;

			switch (get_class($attr)) {
				case 'Asset_Attribute_HTTP_Request' :
				case 'Asset_Attribute_Serialise' :
				case 'Asset_Attribute_Parameter_Map' :
				case 'Asset_Attribute_Email_Format' :
					if (Trigger_Action_Set_Attribute_Value::_safeToEval($value)) {
						// set $value to be the contents of the array

						eval('$value = '.$value.';');
					} else {
						trigger_localised_error('TRIG0003', E_USER_WARNING);
						return FALSE;
					}
				break;
			}

			//if the replace keywords checkbox is checked, do keyword replacement here before setting the value to attribute
			if (array_get_index($settings, 'replace_keywords_in_set_value', 0)) {
				// replace global keywords
				$old_current_asset = $GLOBALS['SQ_SYSTEM']->getGlobalDefine('CURRENT_ASSET', NULL);
				$GLOBALS['SQ_SYSTEM']->setGlobalDefine('CURRENT_ASSET', $state['asset']);

				replace_global_keywords($value);

				if (is_null($old_current_asset)) {
					$GLOBALS['SQ_SYSTEM']->unsetGlobalDefine('CURRENT_ASSET');
				} else {
					$GLOBALS['SQ_SYSTEM']->setGlobalDefine('CURRENT_ASSET', $old_current_asset);
				}

				// replace broadcasting asset keywords
				$keywords = retrieve_keywords_replacements($value);
				$replacements = Array();
				foreach ($keywords as $keyword) {
					$replacements[$keyword] = $state['asset']->getKeywordReplacement($keyword);
				}
				replace_keywords($value, $replacements);
			}

		}

		// dont change anything if the attribute already has the required value
		$current_value = $state['asset']->attr($settings['attribute']);
		if ($current_value != $value) {
			if (!$state['asset']->setAttrValue($settings['attribute'], $value)) {

				return FALSE;
			}
			if (!$state['asset']->saveAttributes()) return FALSE;
		}

		return Array(
				'assetid'	=> $state['asset']->id,
				'attribute'	=> $settings['attribute'],
				'old_value'	=> $current_value,
				'new_value'	=> $state['asset']->attr($settings['attribute']),
			   );

	}//end execute()


	/**
	* Returns true if the passed string is safe to run through eval
	*
	* A string is safe if it contains only digits, quoted strings, the 'array' keyword and punctuation
	*
	* @param string	$str	The string to evaluate
	*
	* @return boolean
	* @access private
	*/
	private static function _safeToEval($str)
	{
		// first check the value contains only allowable components
		$allowed_expressions = Array('array', '(', ')', '=>', ',', ';');
		$reduced_value = str_replace($allowed_expressions, '', strtolower($str));

		// digits are ok
		$reduced_value = preg_replace('/\d/', '', $reduced_value);
		// check for TRUE/FALSE/NULL (dependent on no white space, so trim() the string first)
		$reduced_value = trim($reduced_value);
		$reduced_value = preg_replace('/(^|[^[:alnum:]])(TRUE|FALSE|NULL)([^[:alnum:]]|$)/i', '$1$3', $reduced_value);
		// escaped double-quoted strings are ok
		$reduced_value = preg_replace('/\\\\"/', '', $reduced_value);
		// escaped single-quoted strings are ok
		$reduced_value = preg_replace("/\\\\'/", '', $reduced_value);
		// double-quoted strings are ok
		$reduced_value = preg_replace('/"([^"]*)"/', '', $reduced_value);
		// single-quoted strings are ok
		$reduced_value = preg_replace("/'([^']*)'/", '', $reduced_value);
		$reduced_value = trim($reduced_value);

		return empty($reduced_value);

	}//end _safeToEval()


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

		$selected_type = array_get_index($settings, 'asset_type', '');
		ob_start();
			if ($write_access) {
				asset_type_chooser($prefix.'[asset_type]', FALSE, Array($selected_type), TRUE);
			} else {
				echo '<b>'.$selected_type.'</b>';
			}

			$basic_part_1 = ob_get_contents();
		ob_end_clean();

		ob_start();
			if ($selected_type == '') {
				echo '<b>['.translate('asset_type_not_selected').']</b';
			} else {
				$attribute = array_get_index($settings, 'attribute', '');
				$attrs = $GLOBALS['SQ_SYSTEM']->am->getAssetTypeAttributes($selected_type, Array('name'));
				if (empty($attrs)) {
					echo '<b>['.translate('asset_type_no_attributes_found').']</b>';
				} else {
					if ($write_access) {
						$attr_options = Array('' => '');
						foreach ($attrs as $attr_name) {
							$attr_options[$attr_name] = $attr_name;
						}
						combo_box($prefix.'[attribute]', $attr_options, FALSE, $attribute);

					} else {
						echo '<b>'.$attribute.'</b>';
					}
				}
			}

			$basic_part_2 = ob_get_contents();
		ob_end_clean();

		ob_start();
			$value = array_get_index($settings, 'value', '');
			if ($write_access) {
				text_area($prefix.'[value]', $value, 80, 5, '', 'style="margin-top: 10px;"');
			} else {
				echo '<b>'.$value.'</b>';
			}

			echo '<div style="padding-top: 10px;">';
			$new_prefix = str_replace(array('[',']'), '_', $prefix);
			hidden_field($prefix.'[new_prefix]', $new_prefix);

			$parameter_map_value = array_get_index($settings, 'value_map', serialize(Array()));
			$atr_parameter_map = new Asset_Attribute_Parameter_Map(0, $parameter_map_value);

			$atr_parameter_map->setParameter('attribute_value', 'Attribute Value');
			echo $atr_parameter_map->paint($new_prefix.'_parameter_map');
			echo '</div>';

			$basic_part_3 = ob_get_contents();
		ob_end_clean();

		$output = sprintf(translate('trigger_set_attribute_value_basic'), $basic_part_1, $basic_part_2, $basic_part_3);

		if (!empty($selected_type) && isset($attrs[$attribute])) {
			if (in_array($attrs[$attribute], Array('serialise', 'parameter_map'))) {
				$output .= '<br /><i>'.translate('trigger_set_attribute_value_serialise_note').'</i>';
			}
		}

		// Replace keywords checkbox
		ob_start();
			check_box($prefix.'[replace_keywords_in_set_value]', '1', array_get_index($settings, 'replace_keywords_in_set_value', 0), NULL, ($write_access? '' : 'disabled="disabled"'));
			label('Replace keywords in the set value', $prefix.'[replace_keywords_in_set_value]');
		$basic_part_4 = '<br />'.ob_get_clean();

		$output .= $basic_part_4;

		return $output;

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
		$type_code = array_get_index($request_data, 'asset_type', FALSE);
		if (!$type_code) {
			return translate('asset_type_not_specified');
		}
		$settings['asset_type'] = $type_code;

		$settings['attribute'] = array_get_index($request_data, 'attribute', '');
		$settings['value'] = array_get_index($request_data, 'value', '');

		if (!empty($settings['attribute']) && !empty($settings['asset_type'])) {
			$GLOBALS['SQ_SYSTEM']->am->includeAsset($settings['asset_type']);
			$dummy_asset = new $settings['asset_type'];
			$attr = $dummy_asset->getAttribute($settings['attribute']);
			if (is_null($attr)) {
				// selected asset type does not have an attribute with the selected name, so reset
				$settings['attribute'] = '';
			}

			$new_prefix = str_replace(array('[',']'), '_', array_get_index($request_data, 'new_prefix', ''));
			if ($new_prefix) {
				$atr_parameter_map = new Asset_Attribute_Parameter_Map();
				$atr_parameter_map->process($new_prefix.'_parameter_map');
				$settings['value_map'] = $atr_parameter_map->value;
			}


			switch (get_class($attr)) {
				case 'Asset_Attribute_Serialise' :
				case 'Asset_Attribute_Parameter_Map' :
				case 'Asset_Attribute_Email_Format' :
					if (!Trigger_Action_Set_Attribute_Value::_safeToEval($settings['value'])) {
						trigger_localised_error('TRIG0002', E_USER_NOTICE);
						$settings['value'] = '';
					}
				break;
			}
		}

		$settings['replace_keywords_in_set_value']	= array_get_index($request_data, 'replace_keywords_in_set_value', 0);

		return FALSE;

	}//end processInterface()


	/**
	* Get the list of locks that the action needs acquire before executing
	*
	* @param array	$settings	a container for any data the action might want to save
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return array
	* @access public
	*/
	public static function getLocks($settings, &$state)
	{
		return Array($state['assetid'] => Array('attributes'));

	}//end getLocks()


}//end class

?>
