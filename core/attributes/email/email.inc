<?php

require_once SQ_ATTRIBUTES_PATH.'/text/text.inc';

/**
* Email Asset Attribute
*
* 
*
*
* <!-- Sample Edit Interface XML Node -->
*    <text width="40" height="4" password="0|1" />
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve::attributes
*/
class Asset_Attribute_Email extends Asset_Attribute_Text
{

	/**
	* Constructor
	*
	* @param int	$attributeid	the attribute id to be loaded
	* @param mixed	$value			the current value for the element
	*
	*/
	function Asset_Attribute_Email($attributeid=0, $value=null) 
	{
		$this->Asset_Attribute_Text($attributeid, $value);
		$this->_params['max_length'] = 0; // can't have a max length for an email address
	}// end constructor

	/*
	* Takes an XML_Tree_Node from an edit interface XML file and creates
	* the params array from it
	*
	* @param object XML_Tree_Node	$node
	*
	* @return boolean
	* @access public
	* @see Asset_Attribute::setEditParams()
	*/
	function setEditParams(&$node)
	{
		if (!parent::setEditParams($node)) return false;
		$this->_edit_params['height'] = 1; // can only ever be one line high
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
		$this->processed = false;

		$old_value = $this->value;
		parent::process($prefix);
		if (!$this->processed) return;

		if ($this->value) {
			trigger_error('Do email validation here', E_USER_NOTICE);
			if (!true) {
				$this->processed = false;
				$this->value = $old_value;
			}
		}

	}// end process()

}//end class

?>