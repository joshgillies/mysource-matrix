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
* @package Resolve_Packages
* @subpackage __core__
*/
class Design_Area_Declared_Vars_Edit_Fns extends Design_Area_Edit_Fns
{

	/**
	* Loops through the contents array and creates the necessary objects
	*
	* @param object Asset	$asset		the asset to which we belong
	* @param array			$contents	the contents array we generated with _parseString()
	*
	* @return boolean
	* @access private
	*/
	function _processContents(&$asset, $contents) 
	{
		$vars = Array();
		$declared_vars = Array();
		// firstly let's check to see if all the design areas have unique names
		foreach($contents as $index => $element) {
			if ($element['_type'] != 'TAG' || $element['operation'] != 'declare') continue;

			// if there is a name and it's not the the same type as the current design area
			if (empty($element['attributes']['name']) || empty($element['attributes']['type'])) {
				trigger_error('All declared variables need to have a name and a type', E_USER_WARNING);
				continue;
			}//end if

			if (!isset($element['attributes']['value']))		$element['attributes']['value'] = '';
			if (!isset($element['attributes']['description']))	$element['attributes']['description'] = '';

			$name = preg_replace('/[^a-z0-9_]/i', '_', $element['attributes']['name']);
			if ($name != $element['attributes']['name']) {
				trigger_error('Declared Variable "'.$element['attributes']['name'].'" in "'.$asset->attr('id_name').'" contained illegal characters and has been changed to "'.$name.'"', E_USER_WARNING);
			}

			// make sure this is a valid attribute type
			if (!$GLOBALS['SQ_SYSTEM']->am->validAttributeType($element['attributes']['type'])) {
				trigger_error('Declared Variable "'.$name.'" in "'.$asset->attr('id_name').'" is specified as unknown in Attribute Type "'.$element['attributes']['type'].'"', E_USER_WARNING);
				return false;
			}// end if
			
			// check that the value is valid
			require_once(SQ_ATTRIBUTES_PATH.'/'.$element['attributes']['type'].'/'.$element['attributes']['type'].'.inc');
			$class = 'Asset_Attribute_'.$element['attributes']['type'];
			$attr = new $class();
			if (!$attr->validateValue($element['attributes']['value'])) {
				trigger_error('Declared Variable "'.$name.'" in "'.$asset->attr('id_name').'" has an invalid value "'.$element['attributes']['value'].'" for an "'.$element['attributes']['type'].'" attribute type', E_USER_WARNING);
				return false;
			}// end if

			$declared_vars[$name] = Array(
											'type'        => $element['attributes']['type'],
											'value'       => $element['attributes']['value'],
											'description' => $element['attributes']['description'],
											);
		}// end foreach

		// now make sure we save our changes
		return $asset->setAttrValue('declared_vars', $declared_vars);

		// NOTE: we deliberately don't set the 'contents' as is done in other design areas
	
	}// end _processContents()

	/**
	* Creates a string that would be able to be parsed to recreate this design area in its current form
	*
	* @param object Asset	$asset	the asset whose interface we are painting
	*
	* @return boolean
	* @access public
	*/
	function createParseString(&$asset)
	{
		$text = '';
		foreach($asset->vars['declared_vars']['value'] as $name => $data) {
			$text .= '<'.$this->tag_name.'_DECLARE name="'.htmlentities($name).'"';
			foreach($data as $k => $v) $text .= ' '.$k.'="'.htmlentities($v).'"';
			$text .= " />\n";
		}// end foreach

		return $text;
	}// end createParseString()
	
	/**
	*
	* @param object Asset				$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintDecVars(&$asset, &$o, $prefix) 
	{
		$attrs = $asset->getDeclaredVarAttributes();
		foreach($attrs as $name => $attr) {
			$o->openField($name);
			if ($asset->writeAccess()) {
				$attr->paint($prefix);
			} else {
				echo $attr->value;
			}
			if ($asset->vars['declared_vars']['value'][$name]['description']) $o->note($asset->vars['declared_vars']['value'][$name]['description']);

		}// end foreach

	}// end paintSpecialisation()

	/**
	*
	* @param object Asset				$asset	the asset to which we belong
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processDecVars(&$asset, &$o, $prefix) 
	{
		$save = false;
		$attrs = $asset->getDeclaredVarAttributes();
		$declared_vars = $asset->attr('declared_vars');
		foreach($attrs as $name => $attr) {
			$attr->process($prefix);
			if ($attr->processed) {
				$declared_vars[$name]['value'] = $attr->value;
				$save = true;
			}
		}

		if ($save) {
			// now make sure we save our changes
			$asset->setAttrValue('declared_vars', $declared_vars);
			return true;
		} else {
			return false;
		}

	}// end processDecVars()

}//end class
?>
