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
* $Id: password.inc,v 1.17 2004/08/11 01:02:39 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_ATTRIBUTES_PATH.'/text/text.inc';

/**
* Password Text Asset Attribute
*
*
*
*
* <!-- Sample Edit Interface XML Node -->
*    <password width="8" min_length="6" />
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
* @subpackage attributes
*/
class Asset_Attribute_Password extends Asset_Attribute_Text
{


	/**
	* Constructor
	*
	* @param int	$attributeid	the attribute id to be loaded
	* @param mixed	$value			the current value for the element
	*
	*/
	function Asset_Attribute_Password($attributeid=0, $value=null)
	{
		$this->Asset_Attribute_Text($attributeid, $value);

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

		$this->_edit_params['min_length'] = isset($node->attributes['min_length']) ? (int) $node->attributes['min_length'] : 0;

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
		if ($read_only) {
			echo '<i>Password not shown</i>';
			return;
		}

		require_once SQ_LIB_PATH.'/html_form/html_form.inc';

		// NOTE: we deliberatly don't set the value
		password_box($prefix.'_'.$this->name.'_one', '', $this->_edit_params['width'], $this->_params['max_length']);
		echo '<br />';
		password_box($prefix.'_'.$this->name.'_two', '', $this->_edit_params['width'], $this->_params['max_length']);

		return true;

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
		if (!isset($_POST[$prefix.'_'.$this->name.'_one']) && !isset($_POST[$prefix.'_'.$this->name.'_two'])) return false;
		$this->processed = false;

		$one = $_POST[$prefix.'_'.$this->name.'_one'];
		$two = $_POST[$prefix.'_'.$this->name.'_two'];

		$this->validateValue($one);
		$this->validateValue($two);

		if (!$one) return;

		if ($one == $two) {
			if ($this->setValue($one)) {
				$this->processed = true;
			} else {
				$error_msg = 'Password validation failed';
				if ($this->_edit_params['min_length']) {
					$error_msg .= ' Please ensure your password is at least '.$this->_edit_params['min_length'].' characters long';
				}
				trigger_error($error_msg, E_USER_WARNING);
				$this->processed = false;
			}
		} else {
			trigger_error('Password and confirmation don\'t match', E_USER_WARNING);
			$this->processed = false;
		}

	}//end process()


	/**
	* Ensure the password being set is 'legal'
	*
	* @param mixed	&$value	value to be validated
	*
	* @return boolean
	* @access public
	*/
	function validateValue(&$value)
	{
		$value = trim($value);
		$new_value = (string)$value;
		if ($new_value != $value) return false;
		if (isset($this->_edit_params['min_length']) && $this->_edit_params['min_length']) {
			if (strlen($value) < $this->_edit_params['min_length']) return false;
		}
		return true;

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
		return Array();

	}//end getKeyData()


}//end class

?>