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
* $Id: html_width.inc,v 1.12 2012/08/30 01:09:17 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_attribute.inc';

/**
* HTML Width Asset Attribute
*
*
*
*
* <!-- Sample Edit Interface XML Node -->
*    <html_width />
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.12 $
* @package MySource_Matrix
* @subpackage attributes
*/
class Asset_Attribute_HTML_Width extends Asset_Attribute
{


	/**
	* Constructor
	*
	* @param int	$attributeid	the attribute id to be loaded
	* @param mixed	$value			the current value for the element
	*
	*/
	function Asset_Attribute_HTML_Width($attributeid=0, $value=null)
	{
		$this->Asset_Attribute($attributeid, $value);

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
		$is_pct = (strpos($this->value, '%') !== false);

		if ($read_only) {
			echo (int) $this->value.($is_pct ? ' '.translate('percent') : ' '.translate('pixels'));
			return;
		}

		require_once SQ_LIB_PATH.'/html_form/html_form.inc';
		text_box($prefix, (int) $this->value, 4, 0, false, 'onFocus="javascript: this.__sq_init_value = this.value;" onChange="javascript: if (isNaN(parseInt(this.value))) { this.value = this.__sq_init_value; alert(js_translate(\'cannot_accept_non_digit_characters\')); } else { this.value = parseInt(this.value); } "');
		combo_box($prefix.'_pct', Array(0 => 'Pixels', 1 => 'Percent'), false, $is_pct, 0, $extras='', $maxlength=0);

	}//end paint()


	/**
	* Prints the interface for filling in a value
	*
	* @param string	prefix	prefix for the form element
	*
	* @access public
	*/
	function process($prefix)
	{

		if (!isset($_REQUEST[$prefix])) return false;

		if ($_REQUEST[$prefix.'_pct'] == 1) {

			$value = (int) $_REQUEST[$prefix].'%';
		} else {
			$value = (int) $_REQUEST[$prefix];
		}

		if ((string)$this->value != (string) $value && $this->setValue($value)) {
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
		if (preg_match('/^[0-9a-f]+%?$/i', (string) $value)) {
			return true;
		} else if (!empty($this->_params['allow_blanks']) && strlen($value) == 0) {
			return true;
		} else {
			return false;
		}

	}//end validateValue()


}//end class

?>