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
* $Id: design_area_colourise_image.inc,v 1.16 2003/12/12 17:01:29 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area.inc';

/**
* Design_Area_Colourise_Image
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Colourise_Image extends Design_Area
{

	/**
	* Set the value for the passed variable to the passed value
	*
	* @param string		$name				the name of the attribute
	* @param string		$value				the new value of the attribute
	*
	* @return boolean
	* @access public
	*/
	function setAttrValue($name, $value)
	{

		// if this is one of the colour attributes then let's grab the value, add it the 
		// $this->colours and then save that array
		if ($name == 'black_colour' || $name == 'white_colour' 
			|| stristr($name, 'from_colour') || stristr($name, 'to_colour')) {
			$new_value = $this->vars['colours']['value'];
			if(!is_array($new_value)) $new_value = Array();
			$new_value[$name] = $value;
			$name = 'colours';
			$value = $new_value;
		}

		if ($name == 'type' && $value != 'multiple_colours') $value = 'greyscale';

		return parent::setAttrValue($name, $value);

	}//end setAttrValue()


	/**
	* Easy way to get the value of an attribute
	*
	* @param string	$name
	*
	* @return mixed
	* @access public
	*/
	function attr($name) 
	{
		if (empty($this->vars['colours']['value'][$name])) {
			return parent::attr($name);
		} else { 
			return $this->vars['colours']['value'][$name];
		}
	}//end attr();


	/*
	* Outputs either the straight HTML or PHP code that is needed for this design area to be represented
	*
	* @param object Design	$design the design for which we are currently printing
	*
	* @return boolean
	* @access public
	*/
	function printArea(&$design) 
	{
		$edit_fns = $this->getEditFns();
		$image = $edit_fns->createImage($this, $design);
		if (empty($image)) return false;

		$image = 'assets/'.$this->type().'/'.$this->id.'/'.$image;

		// if they only want us to echo out the filename
		if ($this->attr('filename_only')) {
			echo '<', '?php echo $DATA_PATH; ?>/', $image;

		// else print out a full tag
		} else {
			echo '<img src="<', '?php echo $DATA_PATH; ?', '>/', $image, '" ', $this->attr('extra'), '>';
		}

		return true;

	}//end printArea()

}//end class

?>
