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
* $Id: trigger_condition_asset_type.inc,v 1.7 2005/04/14 04:58:12 rhoward Exp $
*
*/


/**
* Trigger Condition
*
* Checks whether an asset is of a particular type
*
*
* @author   Andrei Railean <arailean@squiz.net>
* @author   Robert Howard <rhoward@squiz.net>
* @version $Revision: 1.7 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Condition_Asset_Type extends Trigger_Condition
{


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
	function evaluate($settings, &$state)
	{
		// grab the data we need to check the condition, if it's not already cached
		// note that new state is modified and new data is available to other conditions
		if (!isset($state['asset_type'])) {
			if (isset($state['asset'])) {
				$state['asset_type'] = $state['asset']->type();
			} else {
				return false;
			}
		}

		if (empty($settings['asset_type'])) return false;

		// check the condition
		if (array_get_index($settings, 'descendants', false)) {
			$type_desc = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants($settings['asset_type'], true);
			if (in_array($state['asset_type'], $type_desc)) {
				return true;
			} else {
				return false;
			}
		} else {
			if ($state['asset_type'] == $settings['asset_type']) {
				return true;
			} else {
				return false;
			}
		}

	}//end evaluate()


	/**
	* Get the HTML editing interface for this condition. Returns the string instead of printing it
	*
	* @param array		$settings		settings that this condition save in processInterface()
	* @param string		$prefix			unique prefix
	* @param boolean	$write_access	write access flag
	*
	* @access public
	* @return boolean
	*/
	function getInterface($settings, $prefix, $write_access=false)
	{
		$selected_type = array_get_index($settings, 'asset_type', 'page');

		// asset type selector
		// get a list of all asset types in the system, sort in alphabetical order

		$is_descendant = array_get_index($settings, 'descendants', false);

		ob_start();
		echo 'Asset is of type ';
		if ($write_access) {
			asset_type_chooser($prefix, false, Array('type_code' => $selected_type, 'inherit' => $is_descendant), null, false, true);
		} else {
			echo '<b>';
			echo $selected_type;
			if ($is_descendant) echo ' (or descendant types)';
			echo '</b>';
		}

		return ob_get_clean();

	}//end getInterface()


	/**
	* Function that handles the conversion of interface to settings
	* together with settings it is expected to populate the hash object
	* Consult the hash object to see the list of setter functions
	*
	* @param array			$settings		a container for any data the condition might want to save
	* @param object Hash	$hash			hash object that takes data that helps to speed up trigger firing
	* @param array			$request_data	array of data corresponding to this condition
	* 										as specified by the $prefix in the getInterface.
	*
	* @access public
	* @return false|string
	*/
	function processInterface(&$settings, &$hash, $request_data)
	{
		if (empty($request_data)) {
			return 'There was an error with input data';
		}

		$asset_type = array_get_index($request_data, 'type_code', false);
		if (!$asset_type) {
			return 'Asset type code has not been supplied';
		}

		// descendants is set to either '1' or not at all
		$descendants = array_get_index($request_data, 'inherit', false);

		$settings['asset_type'] = $asset_type;
		$settings['descendants'] = $descendants;
		$hash->setAssetType($asset_type, ($descendants == '1' ? true : false));

		return false;

	}//end processInterface()


}//end class

?>
