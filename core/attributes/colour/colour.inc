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
* $Id: colour.inc,v 1.15 2004/08/11 01:01:22 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_attribute.inc';

/**
* Colour Asset Attribute
*
* parameters => Array(
*                     'allow_blanks' => [true|false]
*               )
*
*
* <!-- Sample Edit Interface XML Node -->
*    <colour />
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
* @subpackage attributes
*/
class Asset_Attribute_Colour extends Asset_Attribute
{


	/**
	* Constructor
	*
	* @param int	$attributeid	the attribute id to be loaded
	* @param mixed	$value			the current value for the element
	*
	*/
	function Asset_Attribute_Colour($attributeid=0, $value=null) 
	{
		$this->Asset_Attribute($attributeid, $value);
		if (!$attributeid) {
			// Set Some default params for when we are loaded without an ID
			$this->_params = Array('allow_blanks' => false);
		}
		
	}//end constructor


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
			?>
			<span style="background-color: <?php echo $this->value; ?>;">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</span>&nbsp;
			<?php
			echo strtoupper($this->value);
			return;
		}

		require_once SQ_LIB_PATH.'/html_form/html_form.inc';
		colour_box($prefix.'_'.$this->name, $this->value);

	}//end paint()


	/**
	* Processes the interface for filling in a value
	*
	* @param string	prefix	prefix for the form element
	*
	* @access public
	*/
	function process($prefix)
	{
		if (!isset($_POST[$prefix.'_'.$this->name])) return false;
		$value = $_POST[$prefix.'_'.$this->name];
		if ($this->value != $value && $this->setValue($value)) {
			$this->processed = true;
		} else {
			$this->processed = false;
		}

	}//end process()


	/**
	* Ensure the attribute being set is a valid colour
	*
	* @param mixed	&$value	value to be validated
	*
	* @return boolean
	* @access public
	*/
	function validateValue(&$value)
	{
		$value = trim($value);

		if (preg_match('/^#?[0-9a-f]{6}$/i', (string) $value)) {
			if ($value{0} != '#') $value = '#'.$value;
			return true;
		} else if (!empty($this->_params['allow_blanks']) && strlen($value) == 0) {
			return true;
		} else {
			return false;
		}

	}//end validateValue()


}//end class

?>