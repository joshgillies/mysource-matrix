<?php

require_once SQ_INCLUDE_PATH.'/asset_attribute.inc';

/**
* Colour Asset Attribute
*
* 
*
*
* <!-- Sample Edit Interface XML Node -->
*    <colour />
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve::attributes
*/
class Asset_Attribute_Colour extends Asset_Attribute
{

	/**
	* Constructor
	*
	* @param int	$attributeid	the attribute id to be loaded
	* @param mixed	$value			the current value for the element
	*
	*/
	function Asset_Attribute_Colour($attributeid=0, $value=null) 
	{
		$this->Asset_Attribute($attributeid, $value);
	}// end constructor

	/*
	* Prints the interface for filling in a value
	*
	* @param string	$prefix	prefix for the form element
	*
	* @access public
	*/
	function paint($prefix) 
	{
		require_once SQ_LIB_PATH.'/html_form/html_form.inc';
		colour_box($prefix.'_'.$this->name, $this->value);
	}

	/*
	* Prints the interface for filling in a value
	*
	* @param string	prefix	prefix for the form element
	*
	* @access public
	*/
	function process($prefix)
	{
		$value = preg_replace('/[^0-9a-f]/', '0', (string) $_POST[$prefix.'_'.$this->name]);
		if ($this->value != $value) {
			$this->value = $value;
			$this->processed = true;
		} else {
			$this->processed = false;
		}
	}// end process()

}//end class

?>