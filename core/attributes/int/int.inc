<?php

require_once SQ_INCLUDE_PATH.'/asset_attribute.inc';

/**
* Integer Asset Attribute
*
* 
*
*
* <!-- Sample Edit Interface XML Node -->
*    <int width="10" />
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve::attributes
*/
class Asset_Attribute_Int extends Asset_Attribute
{

	/**
	* Constructor
	*
	* @param int	$attributeid	the attribute id to be loaded
	* @param mixed	$value			the current value for the element
	*
	*/
	function Asset_Attribute_Int($attributeid=0, $value=null) 
	{
		$this->Asset_Attribute($attributeid, $value);
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
		$this->_edit_params['width']  = (int) $node->attributes['width'];

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
		require_once SQ_LIB_PATH.'/html_form/html_form.inc';
		text_box($prefix.'_'.$this->name, (int) $this->value, $this->_edit_params['width'], 0, 'onFocus="javascript: this.__sq_init_value = this.value;" onChange="javascript: if (isNaN(parseInt(this.value))) { this.value = this.__sq_init_value; alert(\'This is an integer field.\\nUnable to accept non-digit characters in this field.\'); } else { this.value = parseInt(this.value); } "');
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
		$this->value = (int) $_POST[$prefix.'_'.$this->name];
		$this->processed = true;
	}

}//end class

?>