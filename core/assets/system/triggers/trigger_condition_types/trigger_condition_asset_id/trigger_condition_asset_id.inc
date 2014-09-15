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
* $Id: trigger_condition_asset_id.inc,v 1.10 2012/08/30 01:09:14 ewang Exp $
*
*/


/**
* Trigger Condition
*
* Parent Condition that all trigger conditions extend.
*
*
* @author   Andrei Railean <arailean@squiz.net>
* @author   Robert Howard <rhoward@squiz.net>
* @version $Revision: 1.10 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Condition_Asset_ID extends Trigger_Condition
{


	/**
	* Evaluate this condition
	*
	* The settings used by this condition are in the form:
	* <PRE>
	* Array(
	*       'assetid'       => int  // the assetid of the asset to match
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
		// grab the data we need to check the condition, if it's not already cached
		// note that new state is modified and new data is available to other conditions
		if (!isset($state['assetid'])) {
			if (isset($state['asset'])) {
				$state['assetid'] = $state['asset']->id();
			} else {
				return FALSE;
			}
		}

		// check the condition
		// no need to check if $settings['assetid'] is pointing to a valid asset, $state takes care of that for us
		if ($state['assetid'] == $settings['assetid']) {
			return TRUE;
		} else {
			return FALSE;
		}

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
		$selected_assetid = array_get_index($settings, 'assetid', 0);

		// check if the selected asset is valid
		if ($selected_assetid && !$GLOBALS['SQ_SYSTEM']->am->assetExists($selected_assetid)) {
			$selected_assetid = 0;
		}

		if (!$write_access) {
			$form_element_extras = 'disabled="disabled"';
		} else {
			$form_element_extras = '';
		}

		// begin buffering basic options
		ob_start();
			if ($write_access) {
				asset_finder($prefix.'[assetid]', $selected_assetid);
			} else {
				if ($selected_assetid) {
					$selected_assetname = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($selected_assetid), Array(), TRUE, 'name');
					text_box($prefix.'[assetid]', $selected_assetname[$selected_assetid].' (#'.$selected_assetid.')', '', '', FALSE, $form_element_extras);
				} else {
					text_box($prefix.'[assetid]', '', '', '', FALSE, $form_element_extras);
				}
			}

			$basic_part_1 = ob_get_contents();
		ob_end_clean();

		return sprintf(translate('trigger_asset_id'), $basic_part_1);

	}//end getInterface()


	/**
	* Function that handles the conversion of interface to settings
	*
	* @param array	&$settings		a container for any data the condition might want to save
	* @param array	$request_data	array of data corresponding to this condition as specified by the $prefix in the getInterface.
	*
	* @return mixed string|boolean
	* @access public
	*/
	public static function processInterface(&$settings, $request_data)
	{
		if (empty($request_data)) {
			return 'There was an error with input data';
		}

		if (empty($request_data['assetid']) || empty($request_data['assetid']['assetid'])) {
			return 'Asset id is not specified';
		}

		$assetid = $request_data['assetid']['assetid'];
		$settings['assetid'] = $assetid;

		return FALSE;

	}//end processInterface()


	/**
	* Sets the hash using the settings from the condition
	* Consult the hash object to see the list of setter functions
	*
	* @param array	&$settings	a container for any data the condition might want to save
	* @param object	&$hash		hash object that takes data that helps to speed up trigger firing
	*
	* @return mixed string|boolean
	* @access public
	*/
	public static function setHash(&$settings, &$hash)
	{
		if (isset($settings['assetid'])) {
			$hash->setAssetID($settings['assetid']);
		}
		return FALSE;

	}//end setHash()


}//end class

?>
