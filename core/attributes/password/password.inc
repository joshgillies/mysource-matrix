<?php

require_once SQ_ATTRIBUTES_PATH.'/text/text.inc';

/**
* Password Text Asset Attribute
*
* 
*
*
* <!-- Sample Edit Interface XML Node -->
*    <text width="40" height="4" password="0|1" />
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve::attributes
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

	/*
	* Prints the interface for filling in a value
	*
	* @param string	$prefix	prefix for the form element
	*
	* @access public
	*/
	function paint($prefix) 
	{
		require_once SQ_LIB_PATH.'/html_form/html_form.inc';

		// NOTE: we deliberatly don't set the value
		password_box($prefix.'_'.$this->name.'_one', '', $this->_edit_params['width'], $this->_params['max_length']);
		echo '<br />';
		password_box($prefix.'_'.$this->name.'_two', '', $this->_edit_params['width'], $this->_params['max_length']);

		return true;
	}

	/*
	* Prints the interface for filling in a value
	*
	* @param string	prefix	prefix for the form element
	*
	* @access public
	*/
	function process($prefix)
	{
		$this->processed = false;

		$one = (string) $_POST[$prefix.'_'.$this->name.'_one'];
		$two = (string) $_POST[$prefix.'_'.$this->name.'_two'];

		if (!trim($one)) return;

		if ($one == $two) {
			$this->value = $one;
			$this->processed = true;
		} else {
			trigger_error('Password and confirmation don\'t match', E_USER_WARNING);
			$this->processed = false;
		}
	}// end process()

}//end class

?>