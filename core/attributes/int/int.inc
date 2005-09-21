<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: int.inc,v 1.33 2005/09/21 04:14:09 bcaldwell Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_attribute.inc';

/**
* Integer Asset Attribute
*
* parameters => Array(
* 					'allow_negative'	=> [true|false],
*					'allow_empty'		=> [true|false],
* 					'range_lower'		=> [int],				// optional
* 					'range_upper'		=> [int],				// optional
*               )
*
*
* <!-- Sample Edit Interface XML Node -->
*    <int width="10" />
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.33 $
* @package MySource_Matrix
* @subpackage attributes
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

		// apply default params where necessary
		if (!isset($this->_params['allow_negative'])) {
			$this->_params['allow_negative'] = true;
		}
		if (!isset($this->_params['allow_empty'])) {
			$this->_params['allow_empty'] = false;
		}
		if (!isset($this->_params['range_lower'])) {
			$this->_params['range_lower'] = false;
		}
		if (!isset($this->_params['range_upper'])) {
			$this->_params['range_upper'] = false;
		}

		// set default edit parameters
		$this->_edit_params['width']  = 0;

	}//end constructor


	/**
	* Takes an XML_Tree_Node from an edit interface XML file and creates
	* the params array from it
	*
	* @param object	&$node	XML_Tree_Node object to generate params from.
	*
	* @return boolean
	* @access public
	*/
	function setEditParams(&$node)
	{
		if (!parent::setEditParams($node)) return false;
		$this->_edit_params['width']  = (int) $node->attributes['width'];

		return true;

	}//end setEditParams()


	/**
	* Prints the interface for filling in a value
	*
	* @param string		$prefix		prefix for the form element
	* @param boolean	$read_only	are we just printing the value
	*
	* @return void
	* @access public
	*/
	function paint($prefix, $read_only=false)
	{
		if ($this->value !== '') {
			$this->value = (int)$this->value;
		}
		if ($read_only) {
			echo $this->value;
			return;
		}

		$range_lower = $this->_params['range_lower'];
		$range_upper = $this->_params['range_upper'];

		// We don't want to inforce a range on the integer field.
		if (!is_int($range_lower)) $range_lower = null;
		if (!is_int($range_upper)) $range_upper = null;

		$allow_negative = array_get_index($this->_params, 'allow_negative', true);

		require_once SQ_LIB_PATH.'/html_form/html_form.inc';
		int_text_box($prefix, $this->value, $allow_negative, $this->_edit_params['width'], $range_lower, $range_upper, 0, false, '');
		//text_box($prefix, $this->value, $this->_edit_params['width'], 0, false, $javascript);

	}//end paint()


	/**
	* Prints the interface for filling in a value
	*
	* @param string	$prefix	prefix for the form element
	*
	* @return void
	* @access public
	*/
	function process($prefix)
	{
		if (!isset($_REQUEST[$prefix])) return false;
		$value = $_REQUEST[$prefix];
		if ($value !== '') $value = (int)$value;
		if ($this->value != $value && $this->setValue($value)) {
			$this->processed = true;
		} else {
			$this->processed = false;
		}

	}//end process()


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
		$value = trim($value);

		$allow_negative = array_get_index($this->_params, 'allow_negative', true);
		$allow_empty = array_get_index($this->_params, 'allow_empty', true);

		if ($allow_empty && ($value === '')) {
			return true;
		}

		if (!$allow_negative && $value < 0) {
			$value = $this->_default_value;
		}

		$new_value = (int)$value;

		// we are comparing by string here, reason is that passing in a string as $value
		// becomes comparing $value with zero which is TRUE. So instead we would be comparing
		// it with a string '0', which is FALSE and what we want for it to invalidate.
		// Only genuine ints will pass through.
		if ((string)$new_value != $value) return false;
		return true;

	}//end validateValue()


}//end class

?>
