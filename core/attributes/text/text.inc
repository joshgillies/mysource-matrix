<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: text.inc,v 1.13 2003/09/26 05:26:30 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


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
* @package MySource_Matrix
* @subpackage attributes
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
	function Asset_Attribute_Text($attributeid=0, $value=null) 
	{
		$this->Asset_Attribute($attributeid, $value, Array('max_length' => 0));
	}// end constructor

	/**
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

	/**
	* Prints the interface for filling in a value
	*
	* @param string		$prefix		prefix for the form element
	* @param boolean	$read_only	are we just printing the value
	*
	* @access public
	*/
	function paint($prefix, $read_only=false)
	{
		if ($read_only) {
			echo $this->value;
			return;
		}

		require_once SQ_LIB_PATH.'/html_form/html_form.inc';

		if($this->_edit_params['height'] > 1) {
			text_area($prefix.'_'.$this->name, $this->value, $this->_edit_params['width'], $this->_edit_params['height'], $this->_params['max_length']);
		} else {
			text_box($prefix.'_'.$this->name, $this->value, $this->_edit_params['width'], $this->_params['max_length']);
		}
	}// end paint()

	/**
	* Prints the interface for filling in a value
	*
	* @param string	prefix	prefix for the form element
	*
	* @access public
	*/
	function process($prefix)
	{
		if (!isset($_POST[$prefix.'_'.$this->name])) return false;
		$value = (string) $_POST[$prefix.'_'.$this->name];
		if ($this->value != $value && $this->setValue($value)) {
			$this->processed = true;
		} else {
			$this->processed = false;
		}
	}// end process()


	/**
	* Ensure the attribute being set is a string
	*
	* @param mixed	&$value	value to be validated
	*
	* @return boolean
	* @access public
	*/
	function validateValue(&$value)
	{
		$new_value = (string) $value;
		if ($new_value != $value) return false;
		$value = $new_value;
		return true;
	}//end validateValue()


}//end class

?>