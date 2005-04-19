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
* $Id: trigger_action_set_attribute_value.inc,v 1.2 2005/04/19 04:51:30 gsherwood Exp $
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
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Set_Attribute_Value extends Trigger_Action
{


	/**
	* Execute this action
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
	* @return boolean
	* @access public
	*/
	function execute($settings, &$state)
	{
		// check required settings
		if (empty($settings['attribute'])) return false;

		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset.
			if (empty($state['assetid'])) {
				return false;
			} else {
				$state['asset'] = &$GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}

		if (is_null($state['asset'])) return false;

		$value = $settings['value'];

		// handle serialised attributes
		$attr = &$state['asset']->getAttribute($settings['attribute']);
		switch (get_class($attr)) {
			case 'asset_attribute_serialise' :
			case 'asset_attribute_parameter_map' :
				// set $value to be the contents of the array
				eval('$value = '.$value);
			break;
		}

		// dont change anything if the attribute already has the required value
		$current_value = $state['asset']->attr($settings['attribute']);
		if ($current_value == $value) return true;

		if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_LOCKING)) {
			// lock the asset
			if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($state['assetid'], 'attributes')) {
				return false;
			}
		}

		// set attribute value here
		if (!$state['asset']->setAttrValue($settings['attribute'], $value)) {
			return false;
		}
		if (!$state['asset']->saveAttributes()) return false;

		if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_LOCKING)) {
			// release the attributes lock, but dont fail if we cant unlock
			$GLOBALS['SQ_SYSTEM']->am->releaseLock($state['assetid'], 'attributes');
		}

		return true;

	}//end execute()


	/**
	* Get the HTML editing interface for this action. Returns the string instead of printing it
	*
	* @param array		$settings		settings that this condition saves in processInterface()
	* @param string		$prefix			unique prefix
	* @param boolean	$write_access	determines whether the interface is editable
	*
	* @access public
	* @return boolean
	*/
	function getInterface($settings, $prefix, $write_access=false)
	{
		ob_start();

		$selected_type = array_get_index($settings, 'asset_type', '');
		echo 'Set the value of the ';
		if ($write_access) {
			asset_type_chooser($prefix.'[asset_type]', false, Array($selected_type), true);
		} else {
			echo '<b>'.$selected_type.'</b>';
		}

		echo ' attribute ';
		if ($selected_type == '') {
			echo '<b>[asset type not selected]</b';
		} else {
			$attribute = array_get_index($settings, 'attribute', '');
			$attrs = $GLOBALS['SQ_SYSTEM']->am->getAssetTypeAttributes($selected_type, Array('name'));
			if (empty($attrs)) {
				echo '<b>[no attributes found for asset type]</b>';
			} else {
				if ($write_access) {
					$attr_options = Array('' => '');
					foreach ($attrs as $attr_name) {
						$attr_options[$attr_name] = $attr_name;
					}
					combo_box($prefix.'[attribute]', $attr_options, false, $attribute);
				} else {
					echo '<b>'.$attribute.'</b>';
				}
			}
		}

		echo ' to the value<br/>';
		$value = array_get_index($settings, 'value', '');
		if ($write_access) {
			text_area($prefix.'[value]', $value, 80, 5);
		} else {
			echo '<b>'.$value.'</b>';
		}

		return ob_get_clean();

	}//end getInterface()


	/**
	* Function that handles the conversion of interface to settings
	* together with settings it is expected to populate the hash object
	*
	* @param array	$settings		a container for any data the action might want to save
	* @param string	$request_data	array of data corresponding to this action as
	* 								specified by the $prefix in getInterface.
	*
	* @access public
	* @return boolean
	*/
	function processInterface(&$settings, $request_data)
	{
		$type_code = array_get_index($request_data, 'asset_type', false);
		if (!$type_code) return 'Asset type not specified';
		$settings['asset_type'] = $type_code;

		$settings['attribute'] = array_get_index($request_data, 'attribute', '');
		$settings['value'] = array_get_index($request_data, 'value', '');

		return false;

	}//end processInterface()


}//end class

?>
