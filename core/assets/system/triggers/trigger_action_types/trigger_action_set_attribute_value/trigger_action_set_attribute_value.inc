<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: trigger_action_set_attribute_value.inc,v 1.9 2006/11/03 03:18:35 skim Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';

/**
* Trigger_Action_Set_Attribute_Value
*
* A trigger action that sets a the value of an asset attribute
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.9 $
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
	*       );
	* </PRE>
	*
	* @param array	$settings	the stored settings for this action
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return mixed array|boolean
	* @access public
	*/
	function execute($settings, &$state)
	{
		// check required settings
		if (empty($settings['attribute'])) return FALSE;

		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset.
			if (empty($state['assetid'])) {
				return FALSE;
			} else {
				$state['asset'] =& $GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}

		if (is_null($state['asset'])) return FALSE;

		$value = $settings['value'];

		// handle serialised attributes
		if (is_scalar($value)) {
			$attr =& $state['asset']->getAttribute($settings['attribute']);
			if (is_null($attr)) return FALSE;

			switch (get_class($attr)) {
				case 'asset_attribute_serialise' :
				case 'asset_attribute_parameter_map' :
				case 'asset_attribute_email_format' :
					// set $value to be the contents of the array
					eval('$value = '.$value);
				break;
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
	* Get the HTML editing interface for this action. Returns the string instead of printing it
	*
	* @param array		$settings		settings that this condition saves in processInterface()
	* @param string		$prefix			unique prefix
	* @param boolean	$write_access	determines whether the interface is editable
	*
	* @return boolean
	* @access public
	*/
	function getInterface($settings, $prefix, $write_access=FALSE)
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
				text_area($prefix.'[value]', $value, 80, 5);
			} else {
				echo '<b>'.$value.'</b>';
			}

			$basic_part_3 = ob_get_contents();
		ob_end_clean();

		$output = translate('trigger_set_attribute_value_basic', $basic_part_1, $basic_part_2, $basic_part_3);

		if (!empty($selected_type) && isset($attrs[$attribute])) {
			if (in_array($attrs[$attribute], Array('serialise', 'parameter_map'))) {
				$output .= '<br /><i>'.translate('trigger_set_attribute_value_serialise_note').'</i>';
			}
		}

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
	function processInterface(&$settings, $request_data)
	{
		$type_code = array_get_index($request_data, 'asset_type', FALSE);
		if (!$type_code) {
			return translate('asset_type_not_specified');
		}
		$settings['asset_type'] = $type_code;

		$settings['attribute'] = array_get_index($request_data, 'attribute', '');
		$settings['value'] = array_get_index($request_data, 'value', '');

		// sanity checking
		$attrs = $GLOBALS['SQ_SYSTEM']->am->getAssetTypeAttributes($settings['asset_type'], Array('name'));
		$attribute = array_get_index($settings, 'attribute', '');
		if ($attribute != '' && !in_array($attribute, $attrs)) {
			// unset selected attribute if it is not found for the selected asset type
			$settings['attribute'] = '';
		}

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
	function getLocks($settings, &$state)
	{
		return Array($state['assetid'] => Array('attributes'));

	}//end getLocks()


}//end class

?>
