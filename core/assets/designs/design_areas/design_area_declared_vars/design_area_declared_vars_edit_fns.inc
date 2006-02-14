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
* $Id: design_area_declared_vars_edit_fns.inc,v 1.25 2006/02/14 23:57:09 sdanis Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area_edit_fns.inc';

/**
* Design_Area_Declared_Vars_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.25 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Declared_Vars_Edit_Fns extends Design_Area_Edit_Fns
{


	/**
	* Loops through the contents array and creates the necessary objects
	*
	* @param object	&$asset		the asset to which we belong
	* @param array	$contents	the contents array we generated with _parseString()
	*
	* @return boolean
	* @access private
	*/
	function _processContents(&$asset, $contents)
	{
		$vars = Array();
		$declared_vars = Array();
		// firstly let's check to see if all the design areas have unique names
		foreach ($contents as $index => $element) {
			if ($element['_type'] != 'TAG' || $element['operation'] != 'declare') {
				continue;
			}

			// if there is a name and it's not the the same type as the current design area
			if (empty($element['attributes']['name']) || empty($element['attributes']['type'])) {
				trigger_localised_error('CORE0141', E_USER_WARNING);
				continue;
			}//end if

			if (!isset($element['attributes']['value'])) {
				$element['attributes']['value'] = '';
			}
			if (!isset($element['attributes']['description'])) {
				$element['attributes']['description'] = '';
			}

			$name = preg_replace('/[^a-z0-9_]/i', '_', $element['attributes']['name']);
			if ($name != $element['attributes']['name']) {
				trigger_localised_error('CORE0150', E_USER_WARNING, $element['attributes']['name'], $asset->attr('id_name'), $name);
			}

			// make sure this is a valid attribute type
			if (!$GLOBALS['SQ_SYSTEM']->am->validAttributeType($element['attributes']['type'])) {
				trigger_localised_error('CORE0152', E_USER_WARNING, $name, $asset->attr('id_name'), $element['attributes']['type']);
				return FALSE;
			}//end if

			// check that the value is valid
			require_once(SQ_ATTRIBUTES_PATH.'/'.$element['attributes']['type'].'/'.$element['attributes']['type'].'.inc');
			$class = 'Asset_Attribute_'.$element['attributes']['type'];
			$attr =& new $class();
			if (!$attr->validateValue($element['attributes']['value'])) {
				trigger_localised_error('CORE0151', E_USER_WARNING, $name, $asset->attr('id_name'), $element['attributes']['value'], $element['attributes']['type']);
				return FALSE;
			}//end if

			$declared_vars[$name] = Array(
										'type'			=> $element['attributes']['type'],
										'value'			=> $element['attributes']['value'],
										'description'	=> $element['attributes']['description'],
									);
		}//end foreach

		// now make sure we save our changes
		$GLOBALS['SQ_PROCESSED_DESIGN_AREAS'][$asset->attr('id_name')] = TRUE;
		return $asset->setAttrValue('declared_vars', $declared_vars);

		// NOTE: we deliberately don't set the 'contents' as is done in other design areas

	}//end _processContents()


	/**
	* Paint declared variables
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintDecVars(&$asset, &$o, $prefix)
	{
		$attrs = $asset->getDeclaredVarAttributes();
		foreach ($attrs as $name => $attr) {
			$o->openField($name);
			if ($asset->writeAccess('attributes')) {
				$attr->paint($prefix.'_'.$name);
			} else {
				echo $attr->value;
			}
			if ($asset->vars['declared_vars']['value'][$name]['description']) {
				$o->note($asset->vars['declared_vars']['value'][$name]['description']);
			}

		}// end foreach

	}//end paintDecVars()


	/**
	* Process declared variables
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processDecVars(&$asset, &$o, $prefix)
	{
		$save = FALSE;
		$attrs = $asset->getDeclaredVarAttributes();
		$declared_vars = $asset->attr('declared_vars');
		foreach ($attrs as $name => $attr) {
			$attr->process($prefix.'_'.$name);
			if ($attr->processed) {
				$declared_vars[$name]['value'] = $attr->value;
				$save = TRUE;
			}
		}

		if ($save) {
			// now make sure we save our changes
			$asset->setAttrValue('declared_vars', $declared_vars);
			return TRUE;
		} else {
			return FALSE;
		}

	}//end processDecVars()


}//end class
?>