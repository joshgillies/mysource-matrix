<?php

require_once SQ_INCLUDE_PATH.'/asset_attribute.inc';

/**
* Boolean Asset Attribute
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
class Asset_Attribute_Boolean extends Asset_Attribute
{

	/**
	* Constructor
	*
	* @param int	$attributeid	the attribute id to be loaded
	* @param mixed	$value			the current value for the element
	*
	*/
	function Asset_Attribute_Boolean($attributeid=0, $value=null) 
	{
		$this->Asset_Attribute($attributeid, (($value) ? 1 : 0));
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
		$this->_edit_params['true_text']  = empty($node->attributes['true_text'])  ? 'True' : $node->attributes['true_text'];
		$this->_edit_params['false_text'] = empty($node->attributes['false_text']) ? 'False' : $node->attributes['false_text'];
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
		$options = Array(0 => $this->_edit_params['false_text'], 
						 1 => $this->_edit_params['true_text']);
		combo_box($prefix.'_'.$this->name, $options, false, $this->value, 0, $extras='', $maxlength=0) 
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
		$value = ($_POST[$prefix.'_'.$this->name]) ? 1 : 0;
		if ($this->value != $value) {
			$this->value = $value;
			$this->processed = true;
		} else {
			$this->processed = false;
		}
	}// end process()

}//end class

?>