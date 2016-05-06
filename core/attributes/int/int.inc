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
* $Id: int.inc,v 1.41 2012/08/30 01:09:17 ewang Exp $
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
* @version $Revision: 1.41 $
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
		$this->_edit_params['empty_text'] = '';

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
		$this->_edit_params['width']  = (int) $node->attributes()->width;
		if (isset($node->attributes()->empty_text)) {
			$this->_edit_params['empty_text'] = (string) $node->attributes()->empty_text;
		}
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
		// we want to retain empty values retrieved from the DB
		// Postgres will return an empty string and Oracle will return a NULL value
		if (($this->value === '') || is_null($this->value)) {
			$this->value = '';
		} else {
			$this->value = (int)$this->value;
		}

		if ($read_only) {
			if ((empty($this->_edit_params['empty_text']) === FALSE) && ($this->value === '')) {
				echo '<em>'.$this->_edit_params['empty_text'].'</em>';
			} else {
				echo $this->value;
			}
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
		if (($value === '') || is_null($value)) {
			$allow_empty = array_get_index($this->_params, 'allow_empty', TRUE);
			if ($allow_empty && (($value === '') || is_null($value))) {
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
