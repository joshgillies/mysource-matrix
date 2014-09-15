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
* $Id: trigger_condition_attribute_value.inc,v 1.11 2012/08/30 01:09:15 ewang Exp $
*
*/

require_once dirname(dirname(__FILE__)).'/trigger_condition_asset_type/trigger_condition_asset_type.inc';

/**
* Trigger Condition
*
* Checks whether an asset is of a particular type
*
*
* @author   Andrei Railean <arailean@squiz.net>
* @author   Robert Howard <rhoward@squiz.net>
* @version $Revision: 1.11 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Condition_Attribute_Value extends Trigger_Condition_Asset_Type
{

	/**
	* Get the options for the comparison operator field
	*
	* @return array
	* @access public
	*/
	protected static function getOperatorOptions()
	{
		return Array(
				'='		=> strtolower(translate('equals')),
				'!='	=> strtolower(translate('does_not_equal')),
			   );

	}//end getOperatorOptions()


	/**
	* Evaluate this condition
	*
	* The settings used by this condition are in the form:
	* <PRE>
	* Array(
	*       'asset_type'        => string   // the asset type_code of the asset to match
	*       'descendants'       => boolean  // determines whether to match just the asset_type, or the asset_type and all the descendants
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
		if (empty($settings['asset_type'])) return FALSE;
		if (empty($settings['attribute'])) return FALSE;

		if ((!$settings['inherit']) && (array_get_index($state, 'asset_type') != $settings['asset_type'])) {
			// definitely not the right type
			return FALSE;
		}

		if (empty($state['asset'])) {
			$state['asset'] = $GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
		}

		if (!isset($state['asset_type'])) {
			$state['asset_type'] = $state['asset']->type();
		}

		$value = $settings['value'];

		if ($settings['inherit']) {
			if ($state['asset'] instanceof $settings['asset_type']) {
				return ($state['asset']->attr($settings['attribute']) == $value) == (array_get_index($settings, 'operator', '=') == '=');
			}
		} else {
			if ($state['asset_type'] == $settings['asset_type']) {
				return ($state['asset']->attr($settings['attribute']) == $value) == (array_get_index($settings, 'operator', '=') == '=');
			}
		}
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
		$selected_type = array_get_index($settings, 'asset_type', 'page');
		$am = $GLOBALS['SQ_SYSTEM']->am;

		// asset type selector
		ob_start();
			if ($write_access) {
				asset_type_chooser($prefix.'[type_code]', FALSE, Array('type_code' => $selected_type), NULL, FALSE, FALSE);
				echo ' ( ';
				check_box($prefix.'[inherit]', 1, array_get_index($settings, 'inherit'), '', 'id="'.$prefix.'_inherit" style="margin: 0px"');
				echo ' ';
				label(translate('or_inherited_types'), $prefix.'_inherit');
				echo ') ';
			} else {
				echo '<b>'.$selected_type.'</b>';
				if (array_get_index($settings, 'inherit')) {
					echo ' '.translate('or_inherited_types').' ';
				}
			}
			$asset_type_component = ob_get_contents();
		ob_end_clean();

		// attribute selector
		ob_start();
			$attribute = array_get_index($settings, 'attribute', '');
			if ($selected_type == '') {
				echo '<b>['.translate('asset_type_not_selected').']</b>';
			} else {
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
			$attribute_component = ob_get_contents();
		ob_end_clean();

		// operator selector
		ob_start();
			$operator_options = Trigger_Condition_Attribute_Value::getOperatorOptions();
			$operator = array_get_index($settings, 'operator', '=');
			if ($write_access) {
				combo_box($prefix.'[operator]', $operator_options, FALSE, $operator);
			} else {
				echo '<b>'.$operator_options[$operator].'</b>';
			}
			$operator_component = ob_get_contents();
		ob_end_clean();


		// value selector
		ob_start();
			$value = array_get_index($settings, 'text_value', '');
			if ($write_access) {
				echo '<br /><br />';
				text_area($prefix.'[value]', $value, 80, 5);
			} else {
				echo '<b>'.$value.'</b>';
			}
			$value_component = ob_get_contents();
		ob_end_clean();

		return sprintf(translate('trigger_attribute_value'), $asset_type_component, $attribute_component, $operator_component, $value_component);

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
			return translate('trigger_input_data_error');
		}

		$asset_type = array_get_index($request_data, 'type_code', FALSE);
		if (!$asset_type) {
			return translate('trigger_type_code_not_supplied');
		}

		$settings['asset_type'] = $asset_type;
		$settings['inherit'] = !empty($request_data['inherit']);
		$settings['attribute'] = array_get_index($request_data, 'attribute', FALSE);
		$settings['operator'] = array_get_index($request_data, 'operator', '=');

		$value = array_get_index($request_data, 'value', FALSE);
		$settings['text_value'] = $value;

		// handle serialised attributes
		if (!empty($settings['asset_type']) && !empty($settings['attribute'])) {
			$GLOBALS['SQ_SYSTEM']->am->includeAsset($settings['asset_type']);
			$fake_asset = new $settings['asset_type'];
			$attr = $fake_asset->getAttribute($settings['attribute'], TRUE);
			if (!is_null($attr)) {
				if ($attr instanceof Asset_Attribute_Serialise) {
					$value = str_replace(';', '', $value); // avoid code injection
					eval('$value = '.$value.';');
				}
			}
		}
		$settings['value'] = $value;

		return FALSE;

	}//end processInterface()


}//end class

?>
