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
* $Id: trigger_condition_asset_type.inc,v 1.13.2.1 2006/07/17 01:11:46 skim Exp $
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
* @version $Revision: 1.13.2.1 $
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
	*       'asset_type'        => Array(string)   // (ARRAY OF) asset type_code to match
	*       'descendants'       => Array(boolean)  // (ARRAY OF) descentdant flag for each type code. indexed the same way as 'asset_type'
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


		$selected_types = array_get_index($settings, 'asset_type', Array('page'));
		// backwards compatibility. previously you could only specify one type
		if (!is_array($selected_types)) {
			$selected_types = Array($selected_types);
		}

		$descendant_flags = array_get_index($settings, 'descendants', Array(FALSE));
		if (!is_array($descendant_flags)) {
			$descendant_flags = Array($descendant_flags);
		}

		foreach ($selected_types as $key => $type) {
			if ($descendant_flags[$key]) {
				$type_desc = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants($type, TRUE);
				if (in_array($state['asset_type'], $type_desc)) {
					return TRUE;
				}
			} else {
				if ($type == $state['asset_type']) return TRUE;
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
	function getInterface($settings, $prefix, $write_access=FALSE)
	{
		$selected_types = array_get_index($settings, 'asset_type', Array('page'));

		// backwards compatibility. previously you could only specify one type
		if (!is_array($selected_types)) {
			$selected_types = Array($selected_types);
		}

		$descendant_flags = array_get_index($settings, 'descendants', Array(FALSE));
		if (!is_array($descendant_flags)) {
			$descendant_flags = Array($descendant_flags);
		}

		ob_start();
			if ($write_access) {
				asset_type_chooser($prefix, TRUE, Array('type_code' => $selected_types, 'inherit' => $descendant_flags), NULL, FALSE, TRUE);
			} else {
				foreach ($selected_types as $key => $type) {
					echo '<b>';
					echo $type;
					if ($descendant_flags[$key]) {
						echo '&nbsp;('.translate('or_decendant_types').')';
					}
					echo '</b><br />';
				}
			}

			$interface = ob_get_contents();
		ob_end_clean();

		return translate('trigger_asset_type', $interface);

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
			foreach ($settings['asset_type'] as $key => $type) {
				if (empty($type)) continue;
				$hash->setAssetType($type, ($settings['descendants'][$key] == '1' ? TRUE : FALSE));
			}
		}

		return FALSE;

	}//end setHash()


}//end class

?>
