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
* $Id: colour.inc,v 1.20 2012/08/30 01:09:17 ewang Exp $
*
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
* @version $Revision: 1.20 $
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
			<span class="sq-colour-picker-preview" style="background-color: <?php echo $this->value; ?>;">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</span>&nbsp;
			<?php
			echo strtoupper($this->value);
			return;
		}

		require_once SQ_LIB_PATH.'/html_form/html_form.inc';
		colour_box($prefix, $this->value);

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
		if (!isset($_REQUEST[$prefix])) return false;
		$value = $_REQUEST[$prefix];
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