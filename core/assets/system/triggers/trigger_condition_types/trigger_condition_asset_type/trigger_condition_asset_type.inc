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
* $Id: trigger_condition_asset_type.inc,v 1.11 2006/02/07 02:27:13 sdanis Exp $
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
* @version $Revision: 1.11 $
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
				return FALSE;
			}
		}

		if (empty($settings['asset_type'])) return FALSE;

		// check the condition
		if (array_get_index($settings, 'descendants', FALSE)) {
			$type_desc = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants($settings['asset_type'], TRUE);
			if (in_array($state['asset_type'], $type_desc)) {
				return TRUE;
			} else {
				return FALSE;
			}
		} else {
			if ($state['asset_type'] == $settings['asset_type']) {
				return TRUE;
			} else {
				return FALSE;
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
	* @return boolean
	* @access public
	*/
	function getInterface($settings, $prefix, $write_access=FALSE)
	{
		$selected_type = array_get_index($settings, 'asset_type', 'page');

		// asset type selector
		// get a list of all asset types in the system, sort in alphabetical order

		$is_descendant = array_get_index($settings, 'descendants', FALSE);

		ob_start();
			if ($write_access) {
				asset_type_chooser($prefix, FALSE, Array('type_code' => $selected_type, 'inherit' => $is_descendant), NULL, FALSE, TRUE);
			} else {
				echo '<b>';
				echo $selected_type;
				if ($is_descendant) {
					echo ' ('.translate('or_decendant_types').')';
				}
				echo '</b>';
			}

			$basic_part_1 = ob_get_contents();
		ob_end_clean();

		return translate('trigger_asset_type', $basic_part_1);

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
	function processInterface(&$settings, $request_data)
	{
		if (empty($request_data)) {
			return translate('trigger_input_data_error');
		}

		$asset_type = array_get_index($request_data, 'type_code', FALSE);
		if (!$asset_type) {
			return translate('trigger_type_code_not_supplied');
		}

		// descendants is set to either '1' or not at all
		$descendants = array_get_index($request_data, 'inherit', FALSE);

		$settings['asset_type'] = $asset_type;
		$settings['descendants'] = $descendants;

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
	function setHash(&$settings, &$hash)
	{
		if (isset($settings['asset_type']) && isset($settings['descendants'])) {
			$hash->setAssetType($settings['asset_type'], ($settings['descendants'] == '1' ? TRUE : FALSE));
		}
		return FALSE;

	}//end setHash()


}//end class

?>
