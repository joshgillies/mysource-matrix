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
* $Id: float.inc,v 1.16 2004/07/23 00:23:55 lwright Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_attribute.inc';

/**
* Floar/Real/Double Asset Attribute
*
*
*
*
* <!-- Sample Edit Interface XML Node -->
*    <float width="40" />
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
* @subpackage attributes
*/
class Asset_Attribute_Float extends Asset_Attribute
{

	/**
	* Constructor
	*
	* @param int	$attributeid	the attribute id to be loaded
	* @param mixed	$value			the current value for the element
	*
	*/
	function Asset_Attribute_Float($attributeid=0, $value=null)
	{
		$this->Asset_Attribute($attributeid, (float) $value, Array('decimal_places' => '2'));

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
		$this->_edit_params['width'] = (int) $node->attributes['width'];

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
		$v = number_format($this->value, $this->_params['decimal_places'], '.', '');

		if ($read_only) {
			echo $v;
			return;
		}

		require_once SQ_LIB_PATH.'/html_form/html_form.inc';
		text_box($prefix.'_'.$this->name, $v, $this->_edit_params['width'], 0, false, 'onFocus="javascript: this.__sq_init_value = this.value;" onChange="javascript: if (isNaN(parseFloat(this.value))) { this.value = this.__sq_init_value; alert(\'This is an numerical field.\\nUnable to accept non-digit characters in this field.\'); } else { this.value = number_format(this.value, '.$this->_params['decimal_places'].', \'.\', \'\'); } "');

	}//end paint()


	/**
	* Prints the interface for filling in a value
	*
	* @param object Asset	$asset	the asset that owns the aprefix for the form element
	* @param string	prefix	prefix for the form element
	*
	* @access public
	*/
	function process($prefix)
	{
		if (!isset($_POST[$prefix.'_'.$this->name])) return false;
		$value = (float) $_POST[$prefix.'_'.$this->name];
		if ($this->value != $value && $this->setValue($value)) {
			$this->processed = true;
		} else {
			$this->processed = false;
		}

	}//end process()


	/**
	* Ensure the attribute being set is a float
	*
	* @param mixed	&$value	value to be validated
	*
	* @return boolean
	* @access public
	*/
	function validateValue(&$value)
	{
		$value = trim($value);
		$new_value = (float)$value;

		// we are comparing by string here, reason is that passing in a string as $value
		// becomes comparing $value with zero which is TRUE. So instead we would be comparing
		// it with a string '0', which is FALSE and what we want for it to invalidate.
		// Only genuine ints will pass through.
		if ((string)$new_value != $value) return false;
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
		if (empty($this->value)) return Array();
		return Array($this->value => 1);

	}//end getKeyData()


}//end class

?>