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
* $Id: password.inc,v 1.34 2013/07/25 23:23:49 lwright Exp $
*
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
*    <password width="8" min_length="6" autocomplete="off" />
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.34 $
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
	{// TODO TOF TEST
		if (!parent::setEditParams($node)) return false;

		$this->_edit_params['min_length'] = isset($node->attributes()->min_length) ? (int) $node->attributes()->min_length : 0;
        if(isset($node->attributes()->autocomplete)){
			$this->_edit_params['autocomplete'] = $node->attributes()->autocomplete;
        }

		// Whether to include an extra password feild to confirm the input
		$this->_edit_params['confirm_field'] = isset($node->attributes()->confirm_field) ? (int) $node->attributes()->confirm_field : 1;

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
			echo translate('password_not_shown');
			return;
		}

		require_once SQ_LIB_PATH.'/html_form/html_form.inc';
		// NOTE: we deliberatly don't set the value
		password_box($prefix.'_one', '', $this->_edit_params['width'], $this->_params['max_length'],isset($this->_edit_params['autocomplete']) ? 'autocomplete="'.$this->_edit_params['autocomplete'].'"' : '');
		if (isset($this->_edit_params['confirm_field']) && $this->_edit_params['confirm_field']) {
			echo '<br />';
			password_box($prefix.'_two', '', $this->_edit_params['width'], $this->_params['max_length'],isset($this->_edit_params['autocomplete']) ? 'autocomplete="'.$this->_edit_params['autocomplete'].'"' : '');
		}

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
		if (isset($this->_edit_params['confirm_field']) && !$this->_edit_params['confirm_field'] && isset($_REQUEST[$prefix.'_one'])) {
			$one = $_REQUEST[$prefix.'_one'];
			$this->validateValue($one);
			$this->processed = $one && $this->setValue($one);

			return;
		}

		if (!isset($_REQUEST[$prefix.'_one']) && !isset($_REQUEST[$prefix.'_two'])) {
			return false;
		}
		$this->processed = false;

		$one = $_REQUEST[$prefix.'_one'];
		$two = $_REQUEST[$prefix.'_two'];

		$this->validateValue($one);
		$this->validateValue($two);

		if (!$one) return;

		if ($one == $two) {
			if ($this->setValue($one)) {
				$this->processed = true;
			} else {
				$this->processed = false;
			}
		} else {
			trigger_localised_error('SYS0017', E_USER_WARNING);
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
		return true;

	}//end validateValue()


}//end class

?>
