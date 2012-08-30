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
* $Id: css_dimension.inc,v 1.4 2012/08/30 01:09:17 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_attribute.inc';

/**
* HTML Width Asset Attribute
*
*
*
*
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix
* @subpackage attributes
*/
class Asset_Attribute_CSS_Dimension extends Asset_Attribute
{


	/**
	* Constructor
	*
	* @param int	$attributeid	the attribute id to be loaded
	* @param mixed	$value			the current value for the element
	*
	*/
	function Asset_Attribute_CSS_Dimension($attributeid=0, $value=null)
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
	function paint($prefix, $have_lock=false, $value)
	{
		$this->value = $value;
		$value = (int) $value;
		$unit = substr($this->value, strlen($value));

		$all_units =  Array('px' => 'px', '%' => '%', 'mm' => 'mm', 'pt' => 'pt', 'in' => 'in', 'em' => 'em', 'ex' => 'ex' );
		if (!$have_lock) {
			echo $this->value;
			return;
		}
		require_once SQ_LIB_PATH.'/html_form/html_form.inc';
		int_text_box($prefix, (int) $this->value, false, 5);
		combo_box($prefix.'_unit', $all_units, false, $unit, 0, $extras='', $maxlength=0);
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
		$value = Array();
		$value = $_REQUEST[$prefix];
		$unit = $_REQUEST[$prefix.'_unit'];
		$value .= $unit;
		if ($this->value != $value) {
			$this->setValue($value);
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
		if (empty($this->_params['allow_blanks']) && strlen($value) == 0) {
			return false;
		} else {
			return true;
		}

	}//end validateValue()


}//end class

?>
