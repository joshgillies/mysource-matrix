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
* $Id: html_width.inc,v 1.3 2004/08/11 01:01:57 gsherwood Exp $
* $Name: not supported by cvs2svn $
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
* @version $Version$ - 1.0
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
			echo (int) $this->value . ($is_pct ? ' Percent' : ' Pixels');
			return;
		}

		require_once SQ_LIB_PATH.'/html_form/html_form.inc';
		text_box($prefix.'_'.$this->name, (int) $this->value, 4, 0, false, 'onFocus="javascript: this.__sq_init_value = this.value;" onChange="javascript: if (isNaN(parseInt(this.value))) { this.value = this.__sq_init_value; alert(\'This is an integer field.\\nUnable to accept non-digit characters in this field.\'); } else { this.value = parseInt(this.value); } "');
		combo_box($prefix.'_'.$this->name.'_pct', Array(0 => 'Pixels', 1 => 'Percent'), false, $is_pct, 0, $extras='', $maxlength=0);

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
		if (!isset($_POST[$prefix.'_'.$this->name])) return false;

		// percent
		if ($_POST[$prefix.'_'.$this->name.'_pct'] == 1) {
			$value = (int) $_POST[$prefix.'_'.$this->name] . '%';
		} else {
			$value = (int) $_POST[$prefix.'_'.$this->name];
		}

		if ((string)$this->value != $value && $this->setValue($value)) {
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


	/**
	* Retrieve a list of indexable values from this attribute
	*
	* The return value not only contains the indexable values, but also the number of
	* times they occur in this attributes value
	*
	* @return array(string => int)
	* @access public
	*/
	function getKeyData()
	{
		return Array($this->value => 1);

	}//end getKeyData()


}//end class

?>