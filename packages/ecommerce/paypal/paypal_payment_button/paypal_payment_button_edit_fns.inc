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
* $Id: paypal_payment_button_edit_fns.inc,v 1.5 2012/08/30 00:58:34 ewang Exp $
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
* @version $Revision: 1.5 $
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
		if ($asset->attr('button_type') == '_view_cart') return TRUE;
		
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
		if ($asset->attr('button_type') == '_view_cart') return FALSE;
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
		if ($asset->attr('button_type') == '_view_cart') return TRUE;
		
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
		if ($asset->attr('button_type') == '_view_cart') return FALSE;
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
		if ($asset->attr('button_type') == '_view_cart') return TRUE;
		
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
		if ($asset->attr('button_type') == '_view_cart') return FALSE;
		if (!isset($_POST[$prefix.'_price'])) return FALSE;
		
		$price = $_POST[$prefix.'_price'];
		
		return $asset->setAttrValue('price', $price);
		
	}//end processPrice()
	
	
	/**
	* Paint the interface for currency
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintCurrency(&$asset, &$o, $prefix)
	{
		if ($asset->attr('button_type') == '_view_cart') return TRUE;
		
		$field_name = 'Currency';
		$note = 'The currency of the price.';
		
		$o->openField($field_name, '', $note);
		
		$attr = $asset->getAttribute('currency');
		
		$attr->paint($prefix.'_'.$attr->id, !$asset->writeAccess('attributes'));
		
		return TRUE;
		
	}//end paintCurrency()
	
	
	/**
	* Process the interface for currency
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processCurrency(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;
		if ($asset->attr('button_type') == '_view_cart') return FALSE;
		
		$attr = $asset->getAttribute('currency');
		
		$attr->process($prefix.'_'.$attr->id);
		
		return ($attr->processed && $asset->setAttrValue($attr->name, $attr->value));
		
	}//end processCurrency()
	
	
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
	* Paint the interface for custom variable
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintCustomVar(&$asset, &$o, $prefix)
	{
		if ($asset->attr('button_type') == '_view_cart') return TRUE;

		$field_name = 'Custom variable';
		$note = 'Pass-through variable that will return to Matrix after Paypal finishes processing. This value can be any extra information that Paypal will send back after a customer purchases an item. This value is never presented to customers so it can be used to send secret data under the format var1=value1,var2=value2. These values can be extracted by an IPN Receiver asset later. Globals keywords like %globals_user_assetid% can be used.';
		
		
		if ($asset->attr('button_type') == '_cart') {
			$note .= ' You should use the same value for this field in all <b>Add to Cart</b> buttons because the last clicked button\'s custom variable is returned and you will not know what it is.';
		}
		
		$o->openField($field_name, '', $note);
		
		$custom_var = $asset->attr('custom_var');
		if ($asset->writeAccess('attributes')) {
			text_box($prefix.'_custom_var', $custom_var, 60);
		} else {
			echo htmlspecialchars($custom_var);
		}
		
		return TRUE;
		
	}//end paintCustomVar()
	
	
	/**
	* Process the interface for custom variable
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processCustomVar(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;
		if ($asset->attr('button_type') == '_view_cart') return FALSE;
		if (!isset($_POST[$prefix.'_custom_var'])) return FALSE;
		
		$custom_var = $_POST[$prefix.'_custom_var'];
		
		return $asset->setAttrValue('custom_var', $custom_var);
		
	}//end processCustomVar()
	

	/**
	* Paint the interface for notify url
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintNotifyUrl(&$asset, &$o, $prefix)
	{
		if ($asset->attr('button_type') == '_view_cart') return TRUE;

		$field_name = 'Notify URL';
		$note = 'This is the URL that Paypal will POST an Instant Payment Notification (IPN) to. It must be the URL of an IPN Receiver asset.';
		
		
		if ($asset->attr('button_type') == '_cart') {
			$note .= ' You should use the same value for this field in all <b>Add to Cart</b> buttons because the last clicked button\'s notify URL is used by Paypal and you will not know what it is.';
		}
		
		$o->openField($field_name, '', $note);
		
		$notify_url = $asset->attr('notify_url');
		if ($asset->writeAccess('attributes')) {
			text_box($prefix.'_notify_url', $notify_url, 60);
		} else {
			echo htmlspecialchars($notify_url);
		}
		
		return TRUE;
		
	}//end paintNotifyUrl()
	
	
	/**
	* Process the interface for notify url
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processNotifyUrl(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;
		if ($asset->attr('button_type') == '_view_cart') return FALSE;
		if (!isset($_POST[$prefix.'_notify_url'])) return FALSE;
		
		$notify_url = $_POST[$prefix.'_notify_url'];
		
		return $asset->setAttrValue('notify_url', $notify_url);
		
	}//end processNotifyUrl()
	

	/**
	* Paint the interface for return url
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintReturnUrl(&$asset, &$o, $prefix)
	{
		if ($asset->attr('button_type') == '_view_cart') return TRUE;

		$field_name = 'Return URL';
		$note = 'The URL of the page that the user will be returned after completing a payment.';
		
		
		if ($asset->attr('button_type') == '_cart') {
			$note .= ' You should use the same value for this field in all <b>Add to Cart</b> buttons because the last clicked button\'s return URL is used by Paypal and you will not know what it is.';
		}
		
		$o->openField($field_name, '', $note);
		
		$return_url = $asset->attr('return_url');
		if ($asset->writeAccess('attributes')) {
			text_box($prefix.'_return_url', $return_url, 60);
		} else {
			echo htmlspecialchars($return_url);
		}
		
		return TRUE;
		
	}//end paintReturnUrl()
	
	
	/**
	* Process the interface for return url
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processReturnUrl(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;
		if ($asset->attr('button_type') == '_view_cart') return FALSE;
		if (!isset($_POST[$prefix.'_return_url'])) return FALSE;
		
		$return_url = $_POST[$prefix.'_return_url'];
		
		return $asset->setAttrValue('return_url', $return_url);
		
	}//end processReturnUrl()
	

	/**
	* Paint the interface for cancel url
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintCancelUrl(&$asset, &$o, $prefix)
	{
		if ($asset->attr('button_type') == '_view_cart') return TRUE;

		$field_name = 'Cancel URL';
		$note = 'The URL of the page that the user will be returned if a payment is cancelled.';
		
		
		if ($asset->attr('button_type') == '_cart') {
			$note .= ' You should use the same value for this field in all <b>Add to Cart</b> buttons because the last clicked button\'s cancel URL is used by Paypal and you will not know what it is.';
		}
		
		$o->openField($field_name, '', $note);
		
		$cancel_url = $asset->attr('cancel_return_url');
		if ($asset->writeAccess('attributes')) {
			text_box($prefix.'_cancel_return_url', $cancel_url, 60);
		} else {
			echo htmlspecialchars($cancel_url);
		}
		
		return TRUE;
		
	}//end paintCancelUrl()
	
	
	/**
	* Process the interface for cancel url
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processCancelUrl(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;
		if ($asset->attr('button_type') == '_view_cart') return FALSE;
		if (!isset($_POST[$prefix.'_cancel_return_url'])) return FALSE;
		
		$cancel_url = $_POST[$prefix.'_cancel_return_url'];
		
		return $asset->setAttrValue('cancel_return_url', $cancel_url);
		
	}//end processCancelUrl()
	
	
	/**
	* Paint the interface for shopping cart target window
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintCartTargetWindow(&$asset, &$o, $prefix)
	{
		if (($asset->attr('button_type') != '_cart') && ($asset->attr('button_type') != '_view_cart')) return TRUE;
		
		$field_name = 'Paypal Shopping Cart opens in';
		$note = 'This field specifies that the Paypal Shopping Cart will be opened in a separate browser window or opened in the same browser window that displays your website.';
		
		$o->openField($field_name, '', $note);
		
		$attr = $asset->getAttribute('cart_target_window');
		
		$attr->paint($prefix.'_'.$attr->id, !$asset->writeAccess('attributes'));
		
		return TRUE;
		
	}//end paintCartTargetWindow()
	
	
	/**
	* Process the interface for shopping cart target window
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processCartTargetWindow(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;
		if (($asset->attr('button_type') != '_cart') && ($asset->attr('button_type') != '_view_cart')) return FALSE;
		
		$attr = $asset->getAttribute('cart_target_window');
		
		$attr->process($prefix.'_'.$attr->id);
		
		return ($attr->processed && $asset->setAttrValue($attr->name, $attr->value));
		
	}//end processCartTargetWindow()
	
	
	/**
	* Paint the interface for shopping URL
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintShoppingURL(&$asset, &$o, $prefix)
	{
		if (($asset->attr('button_type') != '_cart') && ($asset->attr('button_type') != '_view_cart')) return TRUE;
		
		$field_name = 'Shopping URL';
		$note = 'This field specifies which page PayPal returns buyers to when they click the <b>Continue Shopping</b> button.';
		
		$o->openField($field_name, '', $note);
		
		$shopping_url = $asset->attr('shopping_url');
		if ($asset->writeAccess('attributes')) {
			text_box($prefix.'_shopping_url', $shopping_url, 60);
		} else {
			echo htmlspecialchars($shopping_url);
		}
		
		return TRUE;
		
	}//end paintShoppingURL()
	
	
	/**
	* Process the interface for shopping URL
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processShoppingURL(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;
		if (($asset->attr('button_type') != '_cart') && ($asset->attr('button_type') != '_view_cart')) return FALSE;
		if (!isset($_POST[$prefix.'_shopping_url'])) return FALSE;
		
		$shopping_url = $_POST[$prefix.'_shopping_url'];
		
		return $asset->setAttrValue('shopping_url', $shopping_url);
		
	}//end processShoppingURL()
	
	
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