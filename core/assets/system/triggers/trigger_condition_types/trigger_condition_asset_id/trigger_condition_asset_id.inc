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
* $Id: trigger_condition_asset_id.inc,v 1.3 2005/03/01 00:25:31 arailean Exp $
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
* @version $Revision: 1.3 $
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
	* @param array(string=>mixed)   $settings   the stored settings for this condition
	* @param array(string=>mixed)   &$state     the state that describes the asset the condition is executing on
	*
	* @return boolean
	* @access public
	*/
	function evaluate($settings, &$state)
	{
		// grab the data we need to check the condition, if it's not already cached
		// note that new state is modified and new data is available to other conditions
		if (!isset($state['assetid'])) {
			if (isset($state['asset'])) {
				$state['assetid'] = $state['asset']->id();
			} else {
				return false;
			}
		}

		// check the condition
		// no need to check if $settings['assetid'] is pointing to a valid asset, $state takes care of that for us
		if ($state['assetid'] == $settings['assetid']) {
			return true;
		} else {
			return false;
		}

	}//end evaluate()


	/**
	* Get the HTML editing interface for this condition. Returns the string instead of printing it
	*
	* @param Array      $settings  settings that this condition save in processInterface()
	* @param string     $prefix    unique prefix
	* @param boolean    $write_access   write access flag
	*
	* @access public
	* @return boolean
	*/
	function getInterface($settings, $prefix, $write_access=false)
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

		ob_start();
		echo 'Asset ID is ';
		if ($write_access) {
			asset_finder($prefix.'[assetid]', $selected_assetid);
		} else {
			if ($selected_assetid) {
				$selected_assetname = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($selected_assetid), Array(), true, 'name');
				text_box($prefix.'[assetid]', $selected_assetname[$selected_assetid].' (#'.$selected_assetid.')', '', '', false, $form_element_extras);
			} else {
				text_box($prefix.'[assetid]', '', '', '', false, $form_element_extras);
			}
		}

		return ob_get_clean();

	}//end getInterface()


	/**
	* Function that handles the conversion of interface to settings
	* together with settings it is expected to populate the hash object
	* Consult the hash object to see the list of setter functions
	*
	* @param Array          $settings       a container for any data the condition might want to save
	* @param object Hash    $hash           hash object that takes data that helps to speed up trigger firing
	* @param array          $request_data   array of data corresponding to this condition as specified by the $prefix in the getInterface.
	*
	* @access public
	* @return boolean
	*/
	function processInterface(&$settings, &$hash, $request_data)
	{
		if (empty($request_data) || empty($request_data['assetid']) || empty($request_data['assetid']['assetid'])) {
			return false;
		}

		$assetid = $request_data['assetid']['assetid'];
		$settings['assetid'] = $assetid;
		$hash->setAssetID($assetid);

		return true;

	}//end processInterface()


}//end class

?>
