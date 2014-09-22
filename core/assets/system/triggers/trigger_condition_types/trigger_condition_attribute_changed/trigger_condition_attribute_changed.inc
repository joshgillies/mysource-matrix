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
* $Id: trigger_condition_attribute_changed.inc,v 1.9 2012/08/30 01:09:14 ewang Exp $
*
*/

require_once dirname(dirname(__FILE__)).'/trigger_condition_asset_type/trigger_condition_asset_type.inc';

/**
* Trigger Condition
*
* Checks whether the specified attribute belonging to the asset
* firing the event has been modified.
*
*
* @author   Robert Howard <rhoward@squiz.net>
* @version $Revision: 1.9 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Condition_Attribute_Changed extends Trigger_Condition_Asset_Type
{


	/**
	* Evaluate this condition
	*
	* The settings used by this condition are in the form:
	* <pre>
	* Array(
	*		'asset_type'	=> string   // the asset type_code of the asset to match
	*		'descendants'	=> boolean  // determines whether to match just the asset_type, or the asset_type and all the descendants
	*		'attribute'		=> string	// check whether the value of this attribute has changed
	* );
	* </pre>
	*
	* Expects parameters in the following format:
	* <pre>
	* Array(
	*   'attr_name'	=> Array(
	*					'type'		=> 'text',
	*					'old_value'	=> 'the old value',
	*					'new_value'	=> 'the new value',
	*				   ),
	* )
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

		$changed_attr = $state['event']['data'];

		if ($settings['inherit']) {
			if ($state['asset'] instanceof $settings['asset_type']) {
				return isset($changed_attr[$settings['attribute']]);
			}
		} else {
			if ($state['asset_type'] == $settings['asset_type']) {
				return isset($changed_attr[$settings['attribute']]);
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
		$am =& $GLOBALS['SQ_SYSTEM']->am;

		// asset type selector
		ob_start();
			if ($write_access) {
				asset_type_chooser($prefix.'[type_code]', FALSE, Array('type_code' => $selected_type), NULL, TRUE, FALSE);
				echo ' ( ';
				check_box($prefix.'[inherit]', 1, array_get_index($settings, 'inherit'), '', 'id="'.$prefix.'_inherit" style="margin: 0px"');
				echo ' ';
				label(translate('or inherited types'), $prefix.'_inherit');

				echo ') ';
			} else {
				echo '<b>'.$selected_type.'</b>';
				if (array_get_index($settings, 'inherit')) {
					echo ' '.translate('or inherited types').' ';

				}
			}
			$asset_type_component = ob_get_contents();
		ob_end_clean();

		// attribute selector
		ob_start();
			$attribute = array_get_index($settings, 'attribute', '');
			if ($selected_type == '') {
				echo '<b>['.translate('asset type not selected').']</b>';

			} else {
				$attrs = $GLOBALS['SQ_SYSTEM']->am->getAssetTypeAttributes($selected_type, Array('name'));
				if (empty($attrs)) {
					echo '<b>['.translate('no attributes found for asset type').']</b>';

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

		return sprintf(translate('If the value of the %1$s attribute %2$s changes<br /><br /><strong>Note:</strong> This condition will cause the trigger to fail if the firing event is not "Attributes Changed".'), $asset_type_component, $attribute_component);


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

		$asset_type = array_get_index($request_data, 'type_code', FALSE);
		if (!$asset_type) {
			return translate('Asset type code has not been supplied');

		}

		$settings['asset_type'] = $asset_type;
		$settings['inherit'] = !empty($request_data['inherit']);
		$settings['attribute'] = array_get_index($request_data, 'attribute', FALSE);

		return FALSE;

	}//end processInterface()


}//end class

?>
