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
* @package MySource_Matrix
* @subpackage attributes
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
	* @param string		$prefix		prefix for the form element
	* @param boolean	$read_only	are we just printing the value
	*
	* @access public
	*/
	function paint($prefix, $read_only=false)
	{
		require_once SQ_LIB_PATH.'/html_form/html_form.inc';
		$options = Array(0 => $this->_edit_params['false_text'], 
						 1 => $this->_edit_params['true_text']);

		if ($read_only) {
			echo $options[$this->value];
			return;
		}

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
		if (!isset($_POST[$prefix.'_'.$this->name])) return false;
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
		if ((string) $new_value != (string) $value) {
			// check for strings that mean the same as true and false
			switch(strtolower((string) $value)) {
				case 'true' :
				case 't'    :
				case 'yes'  :
				case 'y'    :
					$new_value = 1;
					break;

				case 'false' :
				case 'f'     :
				case 'no'    :
				case 'n'     :
					$new_value = 0;
					break;
			}
		}
		$value = (empty($new_value)) ? 0 : 1;
		return true;
	}//end validateValue()

}//end class

?>