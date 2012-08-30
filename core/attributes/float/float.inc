<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: float.inc,v 1.26 2012/08/30 01:09:17 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_attribute.inc';

/**
* Floar/Real/Double Asset Attribute
*
*
* parameters => Array(
* 					'range_lower'		=> [int|float],				// optional
* 					'range_upper'		=> [int|float],				// optional
*               )
*
* <!-- Sample Edit Interface XML Node -->
*    <float width="40" />
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.26 $
* @package MySource_Matrix
* @subpackage attributes
*/
class Asset_Attribute_Float extends Asset_Attribute
{


	/**
	* Constructor
	*
	* @param int	$attributeid	the attribute id to be loaded
	* @param mixed	$value			the current value for the element
	*
	*/
	function Asset_Attribute_Float($attributeid=0, $value=null)
	{
		$this->Asset_Attribute($attributeid, (float) $value, Array('decimal_places' => '2'));
			
		// set default edit parameters
		$this->_edit_params['width'] = 5;

	}//end constructor


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
		$this->_edit_params['width'] = (int) $node->attributes()->width;

		return true;

	}//end setEditParams()


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
		$v = number_format($this->value, $this->_params['decimal_places'], '.', '');

		if ($read_only) {
			echo $v;
			return;
		}

		$js_validate = '';
		if (isset($this->_params['range_upper'])){
			$js_validate = ' else if (parseFloat(this.value) > parseFloat('.(float)$this->_params['range_upper'].')) { this.value = this.__sq_init_value;}';
		}
		if (isset($this->_params['range_lower'])){
			$js_validate .= ' else if (parseFloat(this.value) < parseFloat('.(float)$this->_params['range_lower'].')) { this.value = this.__sq_init_value;}';
		}

		require_once SQ_LIB_PATH.'/html_form/html_form.inc';
		text_box($prefix, $v, $this->_edit_params['width'], 0, false, 'onFocus="javascript: this.__sq_init_value = this.value;" onChange="javascript: if (isNaN(parseFloat(this.value))) { this.value = this.__sq_init_value; alert(js_translate(\'cannot_accept_non_digit_characters\')); } '.$js_validate.' else {this.value = number_format(this.value, '.$this->_params['decimal_places'].', \'.\', \'\'); } "');

	}//end paint()


	/**
	* Prints the interface for filling in a value
	*
	* @param object Asset	$asset	the asset that owns the aprefix for the form element
	* @param string	prefix	prefix for the form element
	*
	* @access public
	*/
	function process($prefix)
	{
		if (!isset($_REQUEST[$prefix])) return false;
		$value = (float) $_REQUEST[$prefix];
		if ($this->value != $value && $this->setValue($value)) {
			$this->processed = true;
		} else {
			$this->processed = false;
		}

	}//end process()


	/**
	* Ensure the attribute being set is a float
	*
	* @param mixed	&$value	value to be validated
	*
	* @return boolean
	* @access public
	*/
	function validateValue(&$value)
	{
		$value = trim($value);
		$new_value = (float)$value;

		// we are comparing by string here, reason is that passing in a string as $value
		// becomes comparing $value with zero which is TRUE. So instead we would be comparing
		// it with a string '0', which is FALSE and what we want for it to invalidate.
		// Only genuine ints will pass through.
		if ((string)$new_value != $value) return false;
		
	
		if(isset($this->_params['range_upper'])) {
			(float)$range_upper = $this->_params['range_upper'];
			if ($new_value > $range_upper) {
				return FALSE;
			}	
		}
				

		if(isset($this->_params['range_lower'])) {
			(float)$range_lower = $this->_params['range_lower'];
			if ($new_value < $range_lower) {
				return FALSE;
			}	
		}
			
		return TRUE;

	}//end validateValue()


}//end class

?>