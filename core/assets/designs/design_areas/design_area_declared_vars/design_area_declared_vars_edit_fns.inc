<?php

require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area_edit_fns.inc';

/**
* Design_Area_Declared_Vars_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve::packages::__core__
*/
class Design_Area_Declared_Vars_Edit_Fns extends Design_Area_Edit_Fns
{

	/**
	* Loops through the contents array and creates the necessary objects
	*
	* @param object Asset	$asset		the asset to which we belong
	* @param array			$contents	the contents array we generated with _parseString()
	*
	* @access private
	*/
	function _processContents(&$asset, &$contents) 
	{
		$vars = Array();
		// firstly let's check to see if all the design areas have unique names
		foreach($contents as $index => $element) {
			if ($element['_type'] != 'TAG' || $element['operation'] != 'declare') continue;

			// if there is a name and it's not the the same type as the current design area
			if (empty($element['attributes']['name']) || empty($element['attributes']['value']) || empty($element['attributes']['type'])) {
				trigger_error('All declared variables need to have a name, a value and a type', E_USER_WARNING);
				continue;
			}//end if

			if (!isset($element['attributes']['description'])) $element['attributes']['description'] = '';

			trigger_error('NEED TO MAKE SURE THAT THE TYPE IS VALID HERE', E_USER_NOTICE);

			$asset->declared_vars[$element['attributes']['name']] = Array(
																'type'        => $element['attributes']['type'],
																'value'       => $element['attributes']['value'],
																'description' => $element['attributes']['description'],
																);
		}// end foreach

		// now make sure we save our changes
		$asset->setAttrValue('declared_vars', serialize($asset->declared_vars));
	
	}// end _processContents()

}//end class
?>
