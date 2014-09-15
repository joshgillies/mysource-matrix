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
* $Id: trigger_condition_keyword.inc,v 1.2 2012/08/30 01:09:15 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_condition_types/trigger_condition_asset_type/trigger_condition_asset_type.inc';

/**
* Trigger Condition
*
* Checks whether an asset is of a particular type
*
*
* @author   Maki Maruyama <mmaruyama@squiz.co.uk>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Condition_Keyword extends Trigger_Condition_Asset_Type
{

	/**
	* Get the options for the matching options to use for the value comparison
	*
	* @return array
	* @access public
	*/
	protected static function getMatchOptions()
	{
		return Array(
				'exact'		=> translate('exact_match'),
				'partial'	=> translate('partial_match'),
				'regex'		=> translate('regex_match'),
			   );

	}//end getMatchOptions()


	/**
	* Evaluate this condition
	*
	* The settings used by this condition are in the form:
	* <PRE>
	* Array(
	*       'keyword'		=> string   // keyword to apply (asset/globals)
	*       'replacement_value'	=> boolean  // compare string > after the keyword above is replaced, the replacement value will be compared
	*						with this value
	*	'match_type'		=> string   // how the values are compared - exact match, partial match or regex
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
		if (empty($settings['keyword'])) return FALSE;
		if (empty($settings['match_type'])) return FALSE;

		if (empty($state['asset'])) {
			if (empty($state['assetid'])) return FALSE;
			$state['asset'] = $GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
		}

		// try replacing the asset keyword
		// if the keyword is a globals keyword, this function simply returns the globals keyword as it is
		// note: we are currently allowing only one keyword to be specified here, so, we strip '%' from the
		//       specified keyword, in case multiple keywords are set here
		$keyword = str_replace('%', '', $settings['keyword']);
		$asset_keyword_replacement = $state['asset']->getKeywordReplacement($keyword);

		// now try replacing the globals keyword
		// we need to make sure that the triggering asset is used to repalce global keywords,
		// so, temporarily switch the 'current asset' to the triggering asset
		$original_current_asset = $GLOBALS['SQ_SYSTEM']->getGlobalDefine('CURRENT_ASSET', NULL);
		$GLOBALS['SQ_SYSTEM']->setGlobalDefine('CURRENT_ASSET', $state['asset']);
		replace_global_keywords($asset_keyword_replacement);

		// we are done with the temporary current asset, so, let's put it back to the original state
		if (is_null($original_current_asset)) {
			$GLOBALS['SQ_SYSTEM']->unsetGlobalDefine('CURRENT_ASSET');
		} else {
			$GLOBALS['SQ_SYSTEM']->setGlobalDefine('CURRENT_ASSET', $original_current_asset);
		}
		// now we have a replacement value - test the replacement value against the value specified to compare
		switch ($settings['match_type']) {
			case 'exact':
				return ($asset_keyword_replacement === $settings['comparison_value']);
			case 'partial':
				return (strpos($asset_keyword_replacement, $settings['comparison_value']) !== FALSE);
			case 'regex':
				return (preg_match('/'.str_replace('/', '\/', $settings['comparison_value']).'/', $asset_keyword_replacement) > 0);
		}

		// we shouldn't come this far but if we did, return FALSE so that the trigger action wouldn't run on this asset
		return FALSE;

	}//end evaluate()


	/**
	* Get the HTML editing interface for this condition. Returns the string instead of printing it
	*
	* @param array		$settings	settings that this condition save in processInterface()
	* @param string		$prefix		unique prefix
	* @param boolean	$write_access	write access flag
	*
	* @return boolean
	* @access public
	*/
	public static function getInterface($settings, $prefix, $write_access=FALSE)
	{
		// check if we already have values set so that we can pre-set the values where possible below
		$keyword = isset($settings['keyword']) ? $settings['keyword'] : '';
		$replacement_value = isset($settings['comparison_value']) ? $settings['comparison_value'] : '';
		$operator = array_get_index($settings, 'match_type', 'exact');

		$am = $GLOBALS['SQ_SYSTEM']->am;

		// keyword input box
		ob_start();
			if ($write_access) {
				text_box($prefix.'[keyword]', $keyword, '40', '', false, 'style="margin: 0 0 5px 0;"');
			} else {
				echo '<b>'.$keyword.'</b>';
			}
			$keyword_component = ob_get_contents();
		ob_end_clean();

		// value to compare
		ob_start();
			if ($write_access) {
				text_box($prefix.'[comparison_value]', $replacement_value , '50px');
			} else {
				echo '<b>'.$replacement_value.'</b>';
			}

			$replacement_value_component = ob_get_contents();
		ob_end_clean();

		// operator selector
		ob_start();
			$operator_options = Trigger_Condition_Keyword::getMatchOptions();

			if ($write_access) {
				combo_box($prefix.'[match_type]', $operator_options, FALSE, $operator);
			} else {
				echo '<b>'.$operator_options[$operator].'</b>';
			}
			$operator_component = ob_get_contents();
		ob_end_clean();


		return sprintf(translate('trigger_condition_keyword'), $keyword_component, $operator_component, $replacement_value_component);

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
			return translate('trigger_input_data_missing');
		}

		$settings['keyword'] = array_get_index($request_data, 'keyword', '');
		$settings['comparison_value'] = array_get_index($request_data, 'comparison_value', '');
		$settings['match_type'] = array_get_index($request_data, 'match_type', 'exact');

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
