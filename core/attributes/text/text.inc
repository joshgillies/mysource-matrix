<?php

require_once SQ_INCLUDE_PATH.'/asset_attribute.inc';

/**
* Text Asset Attribute
*
* 
*
*
* <!-- Sample Edit Interface XML Node -->
*    <text width="40" height="4" />
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve::attributes
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
		$this->Asset_Attribute($attributeid, $value, Array('max_length' => 0));
	}// end constructor

	/*
	* Takes an XML_Tree_Node from an edit interface XML file and creates
	* the params array from it
	*
	* @param object XML_Tree_Node	$node
	*
	* @return boolean
	* @access public
	*/
	function setEditParams(&$node)
	{
		if (!parent::setEditParams($node)) return false;

		$this->_edit_params['width']    = isset($node->attributes['width'])  ? (int) $node->attributes['width']  : 0;
		$this->_edit_params['height']   = isset($node->attributes['height']) ? (int) $node->attributes['height'] : 1;

		return true;
	}

	/*
	* Prints the interface for filling in a value
	*
	* @param string	$prefix	prefix for the form element
	*
	* @access public
	*/
	function paint($prefix) 
	{
		include_once SQ_LIB_PATH.'/html_form/html_form.inc';

		if($this->_edit_params['height'] > 1) {
			text_area($prefix.'_'.$this->name, $this->value, $this->_edit_params['width'], $this->_edit_params['height'], $this->_params['max_length']);
		} else {
			text_box($prefix.'_'.$this->name, $this->value, $this->_edit_params['width'], $this->_params['max_length']);
		}
		return true;
	}

	/*
	* Prints the interface for filling in a value
	*
	* @param string	prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function process($prefix)
	{
		$this->value = (string) $_POST[$prefix.'_'.$this->name];
		$this->processed = true;
	}

}//end class

?>