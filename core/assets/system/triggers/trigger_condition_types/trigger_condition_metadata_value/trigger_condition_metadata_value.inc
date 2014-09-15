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
* $Id: trigger_condition_metadata_value.inc,v 1.11 2013/10/17 05:11:21 cupreti Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_condition/trigger_condition.inc';

/**
* Trigger Condition
*
* Checks whether the specified metadata holds a specified value
*
*
* @author  Anh Ta <ata@squiz.co.uk>
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.11 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Condition_Metadata_Value extends Trigger_Condition
{


	/**
	* Evaluate this condition
	*
	* The settings used by this condition are in the form:
	* <pre>
	* Array(
	*		'asset_id'		=> string	// the asset id of the metadata field
	*		'match'			=> string	// the match type (exact, partial, and regex)
	*		'value'			=> string	// the value to compare the metadata field value with
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
		if (!isset($settings['value']) || ($settings['value'] == '')) return FALSE;
		if (!isset($state['asset']) || is_null($state['asset'])) $state['asset'] = $GLOBALS['SQ_SYSTEM']->am->getasset($state['assetid']);

		$compared_value = $settings['value'];
		replace_global_keywords($compared_value);

		$unfiltered_metadata_value = $GLOBALS['SQ_SYSTEM']->getMetadataManager()->getMetadataValueByAssetid($state['assetid'], $settings['asset_id'], TRUE);
		$keywords = extract_keywords($unfiltered_metadata_value);

		$replacement = Array();
		foreach ($keywords as $keyword) {
			$replacement[$keyword] = $state['asset']->getKeywordReplacement($keyword);
		}

		$metadata_value = replace_keywords( $unfiltered_metadata_value, $replacement);

		if (!is_null($metadata_value)) {
			switch (array_get_index($settings, 'match', 'exact')) {
				case 'exact':
					return ($compared_value == $metadata_value);
				case 'partial':
					if (empty($compared_value)) {
						return ($compared_value == $metadata_value);
					}
					return (strpos($metadata_value, $compared_value) !== FALSE);
				case 'regex':
					return (preg_match('/'.str_replace('/', '\/', $compared_value).'/', $metadata_value) > 0);
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
							'regex'		=> translate('regex_match'),
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

		// match field
		ob_start();
			if ($write_access) {
				combo_box($prefix.'[match]', $type_of_search, FALSE, $settings['match']);
			} else {
				echo translate($settings['match'].'_match');
			}
			$match_component = ob_get_contents();
		ob_end_clean();

		// value field
		ob_start();
			if ($write_access) {
				text_box($prefix.'[value]', array_get_index($settings, 'value'), 50, '', false, 'style="margin-top: 5px;"');
			} else {
				echo $settings['value'];
			}
			$value_component = ob_get_contents();
		ob_end_clean();

		return sprintf(translate('trigger_metadata_value'), $asset_id_component, $match_component, $value_component);

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
