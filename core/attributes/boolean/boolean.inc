<?php

require_once SQ_INCLUDE_PATH.'/asset_attribute.inc';

/**
* Boolean Asset Attribute
*
* 
*
*
* <!-- Sample Edit Interface XML Node -->
*    <boolean true_text="true" false_text="false" />
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
		$this->_edit_params['true_text']  = empty($node->attributes['true_text'])  ? 'True' : $node->attributes['true_text'];
		$this->_edit_params['false_text'] = empty($node->attributes['false_text']) ? 'False' : $node->attributes['false_text'];
		return true;
	}// end setEditParams()


	/**
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
		combo_box($prefix.'_'.$this->name, $options, false, $this->value, 0, $extras='', $maxlength=0);
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
		$value = $_POST[$prefix.'_'.$this->name];
		if ($this->value != $value && $this->setValue($value)) {
			$this->processed = true;
		} else {
			$this->processed = false;
		}
	}// end process()


	/**
	* Ensure the attribute being set is an integer
	*
	* @param mixed	&$value	value to be validated
	*
	* @return boolean
	* @access public
	*/
	function validateValue(&$value)
	{
		$new_value = (int) $value;
		if ($new_value != $value) {
			// check for strings that mean the same as true and false
			$new_value = strtolower((string)$value);
			
			if ($new_value == 'true' || 
				$new_value == 't' || 
				$new_value == 'yes' || 
				$new_value == 'y') $new_value = 1;

			if ($new_value == 'false' || 
				$new_value == 'f' || 
				$new_value == 'no' || 
				$new_value == 'n') $new_value = 0;

			// if new_value is not an integer, it didnt
			// pass our tests to become 1 or 0
			if (!is_int($new_value)) return false;
		}
		$value = $new_value;
		return true;
	}//end validateValue()

}//end class

?>