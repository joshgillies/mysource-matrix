<?php

require_once SQ_CORE_PACKAGE_PATH.'/designs/design_areas/menu/design_area_menu_type/design_area_menu_type.inc';

/**
* Design_Area_Menu_Recursive
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve::packages::__core__
*/
class Design_Area_Menu_Recursive extends Design_Area_Menu_Type
{

	/**
	* Set the value for the passed variable to the passed value
	*
	* @param string		$name				the name of the attribute
	* @param string		$value				the new value of the attribute
	* @param boolean	$dont_run_updated	if true this->_updated() won't be run (THIS IS ONLY EVER CALLED FROM Asset::create())
	*
	* @return boolean
	* @access public
	*/
	function setAttrValue($name, $value, $dont_run_updated=false)
	{
		// make sure the aligment is either left or right
		if ($name == 'alignment') {
			if ($value != 'right') $value = 'left';
		}// end if
		return parent::setAttrValue($name, $value, $dont_run_updated);
	}//end setAttrValue()

	/*
	* Outputs the var either as straight HTML or PHP code, depending on the type
	*
	* @param string	$var			the name of the var to print out
	* @param Array	$args			array of any extra arguments that existed in the print command
	* @param string	$value_prefix	PHP code to wrap put before the value before outputting it
	* @param string	$value_suffix	PHP code to wrap put after  the value before outputting it
	*
	* @access public
	*/
	function printVar($var, $args, $value_prefix='', $value_suffix='')
	{
		if ($var == 'page_short_name' && $this->attr('ellipsis_length') > 0) { 
			$value_prefix = 'ellipsisize('.$value_prefix;
			$value_suffix = $value_suffix.', '.$this->attr('ellipsis_length').' - (2 * $depth_count))';
		}// end if

		parent::printVar($var, $args, $value_prefix, $value_suffix);
	}//end printVar()

}//end class
?>
