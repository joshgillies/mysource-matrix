<?php

require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area.inc';

/**
* Design_Area_Colourise_Image
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve::packages::__core__
*/
class Design_Area_Colourise_Image extends Design_Area
{
	/**
	* Associative Array of colours that are going to be used in the converison process
	* @var Array();
	*/
	var $colours = Array();

	/**
	* Load the asset represented by the passed asset id
	*
	* @param int    $assetid      the asset id to be loaded
	*
	* @access public
	*/
	function load($assetid)
	{
		parent::load($assetid);
		if ($this->vars['colours']['value']) $this->colours = unserialize($this->vars['colours']['value']);
	}// end load()

	/**
	* Set the value for the passed variable to the passed value
	*
	* @param string	$name	the name of the attribute
	* @param string	$value	the new value of the attribute
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
			$this->colours[$name] = $value;
			$name = 'colours';
			$value = serialize($this->colours);
		}

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
		if (empty($this->colours[$name])) {
			return parent::attr($name);
		} else { 
			return $this->colours[$name];
		}
	}//end attr();


	/*
	* Outputs either the straight HTML or PHP code that is needed for this design area to be represented
	*
	* @param object Design	$design the design for which we are currently printing
	*
	* @access public
	*/
	function printArea(&$design) 
	{
		$edit_fns = $this->getEditFns();
		$image = $edit_fns->createImage($this, $design);
		if (!$image) return;

		$image = $this->type().'/'.$this->id.'/'.$image;

		// if they only want us to echo out the filename
		if ($this->attr('filename_only')) {
			echo '<', '?php echo $DATA_PATH; ?>/', $image;

		// else print out a full tag
		} else {
			echo '<img src="<', '?php echo $DATA_PATH; ?>/', $image, '" ', $this->attr('extra'), '>';

		}

	}//end printArea()

}//end class

?>
