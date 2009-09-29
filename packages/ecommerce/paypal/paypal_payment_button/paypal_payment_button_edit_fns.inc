<?php 
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: paypal_payment_button_edit_fns.inc,v 1.3 2009/09/29 11:23:47 ata Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page_edit_fns.inc';

/**
* Paypal_Payment_Button_Edit_Fns
*
* Purpose
*
*
* @author  Anh Ta <ata@squiz.co.uk>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Paypal_Payment_Button_Edit_Fns extends Page_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();
		$this->static_screens['details']['force_unlock'] = FALSE;
		
	}//end constructor

	
	/**
	* Paint the interface for item name
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintItemName(&$asset, &$o, $prefix)
	{
		if ($asset->attr('button_type') == '_donations') {
			$field_name = 'Organization name/service';
			$note = 'The name of your organization or the purpose for the donation. Global keywords can be used here.';
		} else {
			$field_name = 'Item name';
			$note = 'The name of the item or service that you wish to sell. Global keywords can be used here.';
		}
		
		$o->openField($field_name, '', $note);
		
		$item_name = $asset->attr('item_name');
		if ($asset->writeAccess('attributes')) {
			text_box($prefix.'_item_name', $item_name, 60);
		} else {
			echo htmlspecialchars($item_name);
		}
		
		return TRUE;
		
	}//end paintItemName()
	
	
	/**
	* Process the interface for item name
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processItemName(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;
		if (!isset($_POST[$prefix.'_item_name'])) return FALSE;
		
		$item_name = $_POST[$prefix.'_item_name'];
		
		return $asset->setAttrValue('item_name', $item_name);
		
	}//end processItemName()
	
	
	/**
	* Paint the interface for item ID
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintItemID(&$asset, &$o, $prefix)
	{
		if ($asset->attr('button_type') == '_donations') {
			$field_name = 'Donation ID';
			$note = 'A value to help identify different kinds of contribution payments, for example, the name of your current fund-raising campaign';
		} else {
			$field_name = 'Item ID';
			$note = 'Pass-through variable that will return to Matrix after Paypal finishes processing. This value is usually an asset ID and can be viewed by customers. Thus, it should not be used if the asset ID is secret to customers.';
		}
		
		$o->openField($field_name, '', $note);
		
		$item_id = $asset->attr('item_id');
		if ($asset->writeAccess('attributes')) {
			text_box($prefix.'_item_id', $item_id, 60);
		} else {
			echo htmlspecialchars($item_id);
		}
		
		return TRUE;
		
	}//end paintItemID()
	
	
	/**
	* Process the interface for item ID
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processItemID(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;
		if (!isset($_POST[$prefix.'_item_id'])) return FALSE;
		
		$item_id = $_POST[$prefix.'_item_id'];
		
		return $asset->setAttrValue('item_id', $item_id);
		
	}//end processItemID()
	

	/**
	* Paint the interface for price
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintPrice(&$asset, &$o, $prefix)
	{
		if ($asset->attr('button_type') == '_donations') {
			$field_name = 'Amount';
			$note = 'The fixed contribution amount. If you leave the field blank, donors enter their own contribution amount after they click the Donate button. Global keywords can be used here.';
		} else {
			$field_name = 'Price';
			$note = 'The fixed price for your item. If you leave the field blank, buyers "name their own price" after they click the Buy Now button. Global keywords can be used here.';
		}
		
		$o->openField($field_name, '', $note);
		
		$price = $asset->attr('price');
		if ($asset->writeAccess('attributes')) {
			text_box($prefix.'_price', $price, 40);
		} else {
			echo htmlspecialchars($price);
		}
		
		return TRUE;
		
	}//end paintPrice()
	
	
	/**
	* Process the interface for price
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processPrice(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;
		if (!isset($_POST[$prefix.'_price'])) return FALSE;
		
		$price = $_POST[$prefix.'_price'];
		
		return $asset->setAttrValue('price', $price);
		
	}//end processPrice()
	
	
	/**
	* Paint the interface for undefined quantity setting field
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintUndefinedQuantity(&$asset, &$o, $prefix)
	{
		if ($asset->attr('button_type') != '_xclick') return TRUE;
		
		$field_name = 'Undefined quantity?';
		$note = 'Yes, if buyers can update the item quantity in payment process.';
		
		$o->openField($field_name, '', $note);
		
		$options = Array(
							'1' => 'Yes',
							'0' => 'No',
						);
		
		$undefined_quantity = $asset->attr('undefined_quantity');
		if ($asset->writeAccess('attributes')) {
			combo_box($prefix.'_undefined_quantity', $options, FALSE, $undefined_quantity);
		} else {
			echo $options[$undefined_quantity];
		}
		
		return TRUE;
		
	}//end paintUndefinedQuantity()
	
	
	/**
	* Process the interface for undefined quantity setting field
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processUndefinedQuantity(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;
		if ($asset->attr('button_type') != '_xclick') return FALSE;
		if (!isset($_POST[$prefix.'_undefined_quantity'])) return FALSE;
		
		$undefined_quantity = $_POST[$prefix.'_undefined_quantity'];
		
		return $asset->setAttrValue('undefined_quantity', $undefined_quantity);
		
	}//end processUndefinedQuantity()
	
	
	/**
	* Paint the interface for extra variables
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintExtraVars(&$asset, &$o, $prefix)
	{
		$current_vars = $asset->attr('extra_vars');
		
		if ($asset->writeAccess('attributes')) {
			text_area($prefix.'_extra_vars', $current_vars, 80, 30);
		} else {
			if ($current_vars == '') {
				echo '<b>[Empty]</b>';
			} else {
				echo '<b>'.str_replace("\n", '<br />', $current_vars).'</b>';
			}
		}
		
		return TRUE;
		
	}//end paintExtraVars()


	/**
	* Process the interface for extra variables
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processExtraVars(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;
		if (!isset($_POST[$prefix.'_extra_vars'])) return FALSE;
		
		$extra_vars = $_POST[$prefix.'_extra_vars'];
		$extra_vars = trim($extra_vars);
		//replace Windows newline character with Unix newline character because Paypal decryption only accept the Unix format
		$extra_vars = str_replace("\r\n", "\n", $extra_vars);
		
		return $asset->setAttrValue('extra_vars', $extra_vars);
		
	}//end processExtraVars()
	

}//end class
?>