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
* $Id: email.inc,v 1.24 2012/08/30 01:09:17 ewang Exp $
*
*/


require_once SQ_ATTRIBUTES_PATH.'/text/text.inc';
require_once SQ_FUDGE_PATH.'/general/www.inc';

/**
* Email Asset Attribute
*
*
*
*
* <!-- Sample Edit Interface XML Node -->
*    <email width="40" />
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.24 $
* @package MySource_Matrix
* @subpackage attributes
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

	}//end constructor


	/**
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

	}//end setEditParams()


	/**
	* Prints the interface for filling in a value
	*
	* @param string	 $prefix	prefix for the form element
	* @param boolean $validate  Whether to validate the value
	*
	* @return boolean
	* @access public
	*/
	function process($prefix, $validate=TRUE)
	{
		if (!isset($_REQUEST[$prefix])) return false;
		$value = (string) $_REQUEST[$prefix];

		if ($this->value !== $value && $this->setValue($value, $validate)) {
			$this->processed = true;
		} else {
			$this->processed = false;
		}

	}//end process()


	/**
	* Ensure the email address is valid
	*
	* @param mixed	$value	value to be validated
	*
	* @access public
	*/
	function validateValue($value)
	{
		$value = trim($value);
		if ($value == '') return true;
		$new_value = (string)$value;
		if ($new_value != $value) return false;
		if (!valid_email($value)) return false;
		return true;

	}//end validateValue()


}//end class

?>
