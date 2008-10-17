<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: trigger_condition_metadata_value.inc,v 1.4 2008/10/17 06:05:12 ewang Exp $
*
*/

require_once dirname(dirname(__FILE__)).'/trigger_condition_asset_type/trigger_condition_asset_type.inc';

/**
* Trigger Condition
*
* Checks whether the specified metadata holds a specified value
*
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Condition_Metadata_Value extends Trigger_Condition_Asset_Type
{


	/**
	* Evaluate this condition
	*
	* The settings used by this condition are in the form:
	* <pre>
	* Array(
	*		'asset_type'	=> string	// the asset type_code of the asset to match
	*		'inherit'	=> boolean  // determines whether to match just the asset_type, or the asset_type and all the descendants
	*		'attribute'		=> string	// check whether the value of this attribute has changed
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
		if (empty($settings['value'])) return FALSE;
		if (empty($settings['match'])) {
			$settings['match'] = 'exact';
		}

		$metadata_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($settings['asset_id']);
		$metadata_value = $settings['value'];

		$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
		
		$object_value = $mm->getMetadataFieldValues($state['assetid']);

		if (array_key_exists($metadata_asset->name, $object_value)) {
			if ($settings['match'] == 'exact') {
				if ($metadata_value == $object_value[$metadata_asset->name]) {
					return TRUE;
				}
			} else {
				if (strpos($object_value[$metadata_asset->name], $metadata_value) !== FALSE) {
					return TRUE;
				}
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
		$selected_asset_id = array_get_index($settings, 'asset_id', 0);
		$am = $GLOBALS['SQ_SYSTEM']->am;
		if (!empty($selected_asset_id)) {
			$selected_asset = $am->getAsset($selected_asset_id);
		} else {
			$selected_asset = '';
		}

		$type_of_search = Array(
							'exact'		=> translate('exact_match'),
							'partial'	=> translate('partial_match'),
						  );

		// Check if not set
		if (!array_key_exists('match', $settings)) {
			$settings['match'] = 'exact';
		}

		// Check if not set
		if (!array_key_exists('value', $settings)) {
			$settings['value'] = '';
		}

		// asset finder - to pick the metadata field
		ob_start();
			if ($write_access) {
				$restricted_types['metadata_field_text'] = 'I';
				$restricted_types['metadata_field_thesaurus'] = 'I';
				$restricted_types['metadata_field_select'] = 'I';
				asset_finder($prefix.'[asset_id]', $selected_asset_id, $restricted_types);
			} else {
				if (!empty($selected_asset_id)) {
					echo get_asset_tag_line($selected_asset_id);
				} else {
					echo 'None selected';
				}
			}
			$asset_id_component = ob_get_contents();
		ob_end_clean();

		// match field
		ob_start();
			if ($write_access) {
				combo_box($prefix.'[match]', $type_of_search, FALSE, $settings['match']);
			} else {
				if ($settings['match'] == 'exact') {
					echo translate('exact_match');
				} else {
					echo translate('partial_match');
				}
			}
			$match_component = ob_get_contents();
		ob_end_clean();

		// value field
		ob_start();
			if ($write_access) {
				text_box($prefix.'[value]', array_get_index($settings, 'value'));
			} else {
				echo $settings['value'];
			}
			$value_component = ob_get_contents();
		ob_end_clean();

		return translate('trigger_metadata_value', $asset_id_component, $match_component, $value_component);

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
			$settings['asset_type'] = $asset->type();
			$settings['match'] = array_get_index($request_data, 'match', FALSE);
			$settings['value'] = array_get_index($request_data, 'value', FALSE);
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
		// default to to disallow
		return TRUE;

	}//end allowMultiple()


}//end class

?>
