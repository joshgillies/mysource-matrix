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
* $Id: product_price.inc,v 1.11 2012/08/30 00:58:34 ewang Exp $
*
*/


/**
* Product_Price
*
*     Provides an extensible method for representing the price in a Product
*
* @author  Blair Robertson
* @version $Revision: 1.11 $
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
	* Whether to calculate tax during checkout
	* @var boolean
	*/
	var $calculate_tax = false;

	/**
	* Whether this value already includes tax in it
	* @var boolean
	*/
	var $value_has_tax = false;

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

			foreach(Array('name', 'value', 'value_has_tax', 'calculate_tax') as $var) {
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
				$o->openField(translate('Name'));

					if ($wa) text_box($prefix.'_name', $this->name, 15);
					else echo $this->name;
				$o->closeField();
				break;
			case 'value' :
				$o->openField(translate('Value'));

					if ($wa) text_box($prefix.'_value', ($this->value == -1) ? '' : $this->value, 10);
					else echo ($this->value == -1) ? '&nbsp;' : $this->value;
					$o->note(translate('Zero = Free <br/> Blank = Value Not Known'));

				$o->closeField();
				break;
			case 'value_has_tax' :
				$o->openField(translate('Product Price Already Includes Tax'));

					$options = Array(0 => 'No', 1 => 'Yes');
					if ($wa) combo_box($prefix.'_value_has_tax', $options, false, (int) $this->value_has_tax);
					else echo $options[(int) $this->value_has_tax];
					$o->note(translate('Select this if the the above price value already includes the tax for this product.'));

				$o->closeField();
				break;
			case 'calculate_tax' :
				$o->openField(translate('Calculate Tax During Checkout'));

					$options = Array(0 => 'No', 1 => 'Yes');
					if ($wa) combo_box($prefix.'_calculate_tax', $options, false, (int) $this->calculate_tax);
					else echo $options[(int) $this->calculate_tax];
					$o->note(translate('If selected, tax will be calculated and be displayed during the checkout process.'));

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

		foreach(Array('name', 'value', 'calculate_tax', 'value_has_tax') as $var) {
			if ($this->_processBackendField($o, $prefix, $var)) $changes = true;
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
		if (!isset($_POST[$prefix.'_'.$var])) return false;

		$changes = false;
		$val = trim($_POST[$prefix.'_'.$var]);
		switch($var) {
			case 'value' :
				if ($val == '') {
					$val = -1.0;
				} else {
					$val = floatval($val);
					if ($val < 0) {
						trigger_localised_error('ECOM0001', translate('Price Value cannot be negative'), E_USER_WARNING)
						return false;
					}
				}
				break;

			case 'value_has_tax' :
			case 'calculate_tax' :
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
