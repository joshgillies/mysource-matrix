<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: password.inc,v 1.11 2003/09/26 05:26:30 brobertson Exp $
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
*    <password width="8" />
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
	}// end constructor


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
				trigger_error('Password validation failed', E_USER_WARNING);
				$this->processed = false;
			}
		} else {
			trigger_error('Password and confirmation don\'t match', E_USER_WARNING);
			$this->processed = false;
		}
	}// end process()


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