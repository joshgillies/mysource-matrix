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
* $Id: product_price.inc,v 1.2 2005/02/03 15:22:50 brobertson Exp $
*
*/


/**
* Product_Price
*
*     Provides an extensible method for representing the price in a Product
*
* @author  Blair Robertson
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Product_Price extends MySource_Object
{
	/**
	* Name for this price
	* @var string
	*/
	var $name = '';

	/**
	* Current value for this price, if -1 then the value is not known. Can be zero for "free".
	* @var float
	*/
	var $value = -1.0;

	/**
	* Value includes this tax or not
	* @var boolean
	*/
	var $tax_inclusive = false;

	/**
	* Constructor
	*
	* @return void
	* @access public
	*/
	function Product_Price()
	{
		$this->MySource_Object();

	}//end Product_Price()


	/**
	* Prints out the backend interface for the price
	*
	* @param object Backend_Outputter	$o
	* @param string						$prefix	name prefix for all form field names
	* @param boolean					$wa		write access, whether we have write access or not
	*
	* @access public
	* @return boolean	true if editing interface printed
	*/
	function paintBackend(&$o, $prefix, $wa)
	{
		$o->openSection();

			foreach(Array('name', 'value', 'tax_inclusive') as $var) {
				$this->_paintBackendField($o, $prefix, $wa, $var);
			}

		$o->closeSection();

	}//end paintBackend()


	/**
	* Paint the individual fields that are used in paintBackend()
	*
	* @param object Backend_Outputter	&$o
	* @param string						$prefix	name prefix for all form field names
	* @param boolean					$wa		write access, whether we have write access or not
	* @param string						$var	the name of the var whose field we need to print
	*
	* @access public
	* @return boolean	true if editing interface printed
	*/
	function _paintBackendField(&$o, $prefix, $wa, $var)
	{
		switch($var) {
			case 'name' :
				$o->openField('Name');
					if ($wa) text_box($prefix.'_name', $this->name, 15);
					else echo $this->name;
				$o->closeField();
				break;
			case 'value' :
				$o->openField('Value');
					if ($wa) text_box($prefix.'_value', ($this->value == -1) ? '' : $this->value, 10);
					else echo ($this->value == -1) ? '&nbsp;' : $this->value;
					$o->note('Zero = Free<br>Blank = Value Not Known');
				$o->closeField();
				break;
			case 'tax_inclusive' :
				$o->openField('Includes Tax');
					$options = Array(0 => 'No', 1 => 'Yes');
					if ($wa) combo_box($prefix.'_tax_inclusive', $options, false, (int) $this->tax_inclusive);
					else echo $options[(int) $this->tax_inclusive];
				$o->closeField();
				break;
		}

	}//end _paintBackendField()


	/**
	* Process the backend interface for the price
	*
	* @param object Backend_Outputter	$o
	* @param string						$prefix	name prefix for all form field names
	*
	* @access public
	* @return boolean	true if everything OK and changes were made
	*/
	function processBackend(&$o, $prefix)
	{
		$changes = false;

		foreach(Array('name', 'value', 'tax_inclusive') as $var) {
			if ($this->_processBackendField($o, $prefix, $field_name)) $changes = true;
		}// end foreach

		return $changes;

	}//end processBackend()


	/**
	* Process the individual fields that are used in processBackend()
	*
	* @param object Backend_Outputter	&$o
	* @param string						$prefix	name prefix for all form field names
	* @param string						$var	the name of the var whose field we need to print
	*
	* @access public
	* @return boolean	true if editing interface printed
	*/
	function _processBackendField(&$o, $prefix, $var)
	{
		$changes = false;

		if (!isset($_POST[$prefix.'_'.$var])) continue;
		$val = trim($_POST[$prefix.'_'.$var]);
		switch($var) {
			case 'value' :
				if ($val == '') {
					$val = -1.0;
				} else {
					$val = floatval($val);
					if ($val < 0) {
						trigger_error('Price Value cannot be negative', E_USER_WARNING);
						return false;
					}
				}
				break;

			case 'tax_inclusive' :
				$val = (bool) $val;
				break;

		}// end switch

		if ($this->$var != $val) {
			$this->$var = $val;
			$changes = true;
		}

		return $changes;

	}//end processBackend()


}//end class
?>
