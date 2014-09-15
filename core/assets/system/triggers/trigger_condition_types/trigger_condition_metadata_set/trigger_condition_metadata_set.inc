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
* $Id: trigger_condition_metadata_set.inc,v 1.4 2013/10/17 05:11:01 cupreti Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_condition/trigger_condition.inc';

/**
* Trigger Condition
*
* Checks whether the specified metadata holds a specified value
*
*
* @author  Ashish Karelia <akarelia@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Condition_Metadata_Set extends Trigger_Condition
{


	/**
	* Evaluate this condition
	*
	* The settings used by this condition are in the form:
	* <pre>
	* Array(
	*		'asset_id'		=> string	// the asset id of the metadata field
	* );
	* </pre>
	*
	* @param array	$settings	the stored settings for this condition
	* @param array	&$state		the state that describes the asset the condition is executing on
	*
	* @return boolean
	* @access public
	*/
	public static function evaluate($settings, &$state)
	{
		if (empty($settings['asset_id'])) return FALSE;

		$metadata_value = $GLOBALS['SQ_SYSTEM']->getMetadataManager()->getMetadataValueByAssetid($state['assetid'], $settings['asset_id'], TRUE);
		$default_value = $GLOBALS['SQ_SYSTEM']->getMetadataManager()->getMetadataFieldDefaultValue($settings['asset_id']);

		//if the metadatavalue is empty or same as default, that means its not set
		if ( $metadata_value == '' || ($metadata_value === $default_value)) return FALSE;

		// if we have metadata set for this field give mission GO!!!
		return TRUE;

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
		$selected_asset_id = array_get_index($settings, 'asset_id', 0);
		$am = $GLOBALS['SQ_SYSTEM']->am;
		if (!empty($selected_asset_id)) {
			$selected_asset = $am->getAsset($selected_asset_id);
		} else {
			$selected_asset = '';
		}

		// asset finder - to pick the metadata field
		ob_start();
			if ($write_access) {
				asset_finder($prefix.'[asset_id]', $selected_asset_id, Array('metadata_field' => 'D'));
			} else {
				if (!empty($selected_asset_id)) {
					echo get_asset_tag_line($selected_asset_id);
				} else {
					echo 'None selected';
				}
			}
			$asset_id_component = ob_get_contents();
		ob_end_clean();

		return sprintf(translate('trigger_metadata_set'), $asset_id_component);

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

		$asset_id = array_get_index($request_data, 'asset_id', FALSE);
		if (!$asset_id) {
			return translate('trigger_asset_not_supplied');
		}

		if (!empty($asset_id['assetid'])) {
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($asset_id['assetid']);
			$settings['asset_id'] = $asset_id['assetid'];
		}

		return FALSE;

	}//end processInterface()


	/**
	* Determines whether a condition is allowed to be used multiple times in a trigger
	* Default to false, children can override if necessary
	*
	* @return boolean
	* @access public
	*/
	public static function allowMultiple()
	{

		return TRUE;

	}//end allowMultiple()


}//end class

?>
