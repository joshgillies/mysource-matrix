<?php

require_once SQ_INCLUDE_PATH.'/asset_attribute.inc';

/**
* Asset Attribute
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
*/
class Asset_Attribute_Text extends Asset_Attribute
{

	/**
	* Constructor
	*
	* @param int	$attributeid	the attribute id to be loaded
	* @param mixed	$value			the current value for the element
	*
	*/
	function Asset_Attribute_Text($attributeid, $value) 
	{
		$this->Asset_Attribute($attributeid, $value);

		if ($this->id) {
			// now just make sure some default values are set
			foreach(Array('height' => 1, 'width' => '10', 'max_length' => 0) as $name => $default) {
				if (!isset($this->_params[$name])) $this->_params[$name] = $default;
			}
		}// end if

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
		if(empty($this->_params['height']) || $this->_params['height'] == 1) {
			echo text_box($prefix.'['.$this->name.']', $this->value, $this->_params['width'], $this->_params['max_length']);
		} else {
			echo text_area($prefix.'['.$this->name.']', $this->value, $this->_params['width'], $this->_params['height'], $this->_params['max_length']);
		}
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
		return false;
	}


}//end class

?>