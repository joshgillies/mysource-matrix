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
* $Id: int.inc,v 1.35.2.1 2006/05/01 06:16:10 tbarrett Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_attribute.inc';

/**
* Integer Asset Attribute
*
* parameters => Array(
* 					'allow_negative'	=> [TRUE|FALSE],
*					'allow_empty'		=> [TRUE|FALSE],
* 					'range_lower'		=> [int],				// optional
* 					'range_upper'		=> [int],				// optional
*               )
*
*
* <!-- Sample Edit Interface XML Node -->
*    <int width="10" />
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.35.2.1 $
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
	function Asset_Attribute_Int($attributeid=0, $value=NULL)
	{
		$this->Asset_Attribute($attributeid, $value);

		// apply default params where necessary
		if (!isset($this->_params['allow_negative'])) {
			$this->_params['allow_negative'] = TRUE;
		}
		if (!isset($this->_params['allow_empty'])) {
			$this->_params['allow_empty'] = FALSE;
		}
		if (!isset($this->_params['range_lower'])) {
			$this->_params['range_lower'] = FALSE;
		}
		if (!isset($this->_params['range_upper'])) {
			$this->_params['range_upper'] = FALSE;
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
		if (!parent::setEditParams($node)) return FALSE;
		$this->_edit_params['width']  = (int) $node->attributes['width'];

		return TRUE;

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
	function paint($prefix, $read_only=FALSE)
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
		if (!is_int($range_lower)) $range_lower = NULL;
		if (!is_int($range_upper)) $range_upper = NULL;

		$allow_negative = array_get_index($this->_params, 'allow_negative', TRUE);

		require_once SQ_LIB_PATH.'/html_form/html_form.inc';
		int_text_box($prefix, $this->value, $allow_negative, $this->_edit_params['width'], $range_lower, $range_upper, 0, FALSE, '');

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
		if (!isset($_REQUEST[$prefix])) return FALSE;
		$value = $_REQUEST[$prefix];
		if ($value !== '') $value = (int)$value;
		if ($this->value !== $value && $this->setValue($value)) {
			$this->processed = TRUE;
		} else {
			$this->processed = FALSE;
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
		// empty string is OK if enabled
		if ($value === '') {
			$allow_empty = array_get_index($this->_params, 'allow_empty', TRUE);
			if ($allow_empty && ($value === '')) {
				return TRUE;
			} else {
				// we'll be nice and turn this into int zero rather than erroring
				$value = (int)$value;
			}
		}

		$range_lower = array_get_index($this->_params, 'range_lower', FALSE);
		if (!empty($range_lower) && ($value < $range_lower)) {
			return FALSE;
		}

		$range_upper = array_get_index($this->_params, 'range_upper', FALSE);
		if (!empty($range_upper) && ($value > $range_upper)) {
			return FALSE;
		}

		// unless one of the range checks failed, zero is OK
		if ($value === 0) return TRUE;
		if ($value === '0') return TRUE;

		// anything else that equates to int zero is not OK
		if ($value == 0) return FALSE;

		$allow_negative = array_get_index($this->_params, 'allow_negative', TRUE);
		if (!$allow_negative && $value < 0) {
			return FALSE;
		}

		return TRUE;

	}//end validateValue()


}//end class

?>