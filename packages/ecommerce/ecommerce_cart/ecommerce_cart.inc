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
* $Id: ecommerce_cart.inc,v 1.2 2005/01/20 13:22:21 brobertson Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Ecommerce_Cart
*
* Purpose
*   A simple shopping cart used for ecommerce functions
*
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Ecommerce_Cart extends Asset
{

	
	/**
	* Constructor
	*
	* @param string	$assetid	the assetid of the asset to load into this object
	*
	* @return void
	* @access public
	*/
	function Ecommerce_Cart($assetid=0)
	{
		$this->Asset($assetid);
		
	}//end Ecommerce_Cart()
	
	
	/**
	* Create this asset
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return mixed (int|boolean)	the link id if the asset is created
	*							successfully, or bool(false) if create failed
	* @access public
	* @see Asset::create()
	*/
	function create(&$link)
	{
		$name = trim($this->attr('name'));
		if ($name == '') {
			trigger_error('Unable to create a '.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name').' with a blank name', E_USER_WARNING);
			return false;
		}

		// if the short name hasn't been set then short and long names are the same to begin with
		if (trim($this->attr('short_name')) == '') $this->setAttrValue('short_name', $name);

		return parent::create($link);

	}//end create()
	
	
	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the short name or the full name
	*
	* @see Asset::_getName()
	* @return string
	* @access private
	*/
	function _getName($short_name=false)
	{
		return (($short_name) ? $this->attr('short_name') : $this->attr('name'));

	}//end _getName()


	/**
	* Prepare cart for first use, if the relevant session vars aren't there
	*
	* @return int	1 if either session var needed to be created (ie. cart
	*				needed to be init'ed), 0 if cart already existed
	*				(-1 is reserved for an error condition)
	* @access public
	*/
	function initialise()
	{
		$ret_val = 0;
		
		if (!isset($_SESSION['sq_cart_contents'])) {
			$_SESSION['sq_cart_contents'] = Array();
			$ret_val = 1;
		}
		
		if (!isset($_SESSION['sq_cart_contents'][$this->id])) {
			$_SESSION['sq_cart_contents'][$this->id] = Array();
			$ret_val = 1;
		}
		
		return $ret_val;
		
	}//end initialise()
	
	
	/**
	* Unsets the contents of this particular cart
	*
	* If this is the last cart being stored in the session cookie, unset the
	* whole cart contents session variable as well
	*
	* @return boolean	always true (false is reserved for an error condition)
	* @access public
	*/
	function clear()
	{
		unset($_SESSION['sq_cart_contents'][$this->id]);
		
		// this is the last cart
		if (empty($_SESSION['sq_cart_contents'])) {
			unset($_SESSION['sq_cart_contents']);
		}
		
		return true;
		
	}//end clear()

	
	/**
	* Adds an item(s) to the cart
	*
	* @param string	$product_assetid	The assetid of the product to add
	* @param double	$price			The price to record against that product
	* @param int		$quantity		The quantity to add
	*
	* @return boolean	indicating success
	* @access public
	*/
	function add($product_assetid, $price, $quantity=1)
	{
		// quantity is ZERO, assume that this is valid and silently return
		// quantity is LESS THAN ZERO, throw an error
		if ($quantity == 0) {
			return true;
		} else if ($quantity < 0) {
			trigger_error('Quantity to add() to cart cannot be less than zero', E_USER_WARNING);
			return false;
		}
		
		// check that the passed product assetid is indeed valid and
		// a valid product (or subclass of product)
		assert_valid_assetid($product_assetid);
		$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($product_assetid), 'product', false);
		
		if (empty($asset_info)) {
			trigger_error('Assetid sent to cart add() function is not a Product asset', E_USER_WARNING);
			return false;
		}
		
		$cart =& $this->get();
		if (!isset($cart[$product_assetid])) {
			$cart[$product_assetid] = Array('price' => $price, 'quantity' => 0);
		}
		
		$cart[$product_assetid]['quantity'] += $quantity;
		
	}//end add()
	
	
	/**
	* Removes an item(s) from the cart
	*
	* Assumption: product lines were correctly made using the add() method
	* and therefore, only valid products are found in the cart (because of the
	* add() checking).
	*
	* @param string	$product_assetid	The assetid of the product to remove
	* @param int		$quantity		The quantity of this product to remove -
	*								leave blank to remove all of that product
	*
	* @return boolean	indicating success
	* @access public
	*/
	function remove($product_assetid, $quantity=null)
	{
		$cart =& $this->get();
		
		// product doesn't exist in cart - if the quantity is zero (or ALL)
		// then silently return, otherwise error
		if (!isset($cart[$product_assetid]))
		{
			if (is_null($quantity) || ($quantity == 0)) {
				return true;
			} else if ($quantity > 0) {
				trigger_error('Attempt to remove() items from cart when there is none of that product', E_USER_WARNING);
				return false;
			}
		}
		
		// after this point we can assume that product exists in the cart
		// Quantity is null, remove everything
		if (is_null($quantity)) {
			$quantity = $cart[$product_assetid]['quantity'];
		}
		
		// Qty == 0, silently return - Qty < 0, throw an error
		if ($quantity == 0) {
			return true;
		} else if ($quantity < 0) {
			trigger_error('Quantity to remove() from cart cannot be less than zero', E_USER_WARNING);
			return false;
		}
		
		if ($quantity > $cart[$product_assetid]['quantity']) {
			trigger_error('Quantity to remove() from cart cannot be more than how many of that product are actually in the cart', E_USER_WARNING);
			return false;
		} else if ($quantity == $cart[$product_assetid]['quantity']) {
			unset($cart[$product_assetid]);
		} else {
			$cart[$product_assetid]['quantity'] -= $quantity;
		}
		
	}//end remove()
	
	
	/**
	* Returns a reference to the session variable that holds this cart
	*
	* This function calls initialise() so the cart contents array will always
	* be there when this is called.
	*
	* @return Array
	* @access public
	*/
	function &get()
	{
		$this->initialise();
		return $_SESSION['sq_cart_contents'][$this->id]; 
		
	}//end get()
	
	
	/**
	* Return the sum-total of all the items currently in this cart
	*
	* @return double
	* @access public
	*/
	function total()
	{
		$cart = $this->get();
		$total = 0.0;
		foreach($cart as $product_assetid => $details) {
			$total += $details['quantity'] * $details['price'];
		}
		
		return $total;
		
	}//end total()
	
	
	/**
	* Return the count of all the items currently in this cart
	*
	* @return double
	* @access public
	*/
	function count()
	{
		$cart = $this->get();
		$count = 0;
		foreach($cart as $product_assetid => $details) {
			$count += $details['quantity'];
		}
		
		return $count;
		
	}//end count()
	
	
	/**
	* Paints the asset to the page
	*
	* @return void
	* @access public
	*/
	function printBody()
	{
		echo $this->count();
		echo ($this->count() == 1) ? ' item' : ' items';
		echo '<br/>';
		echo '$'.number_format($this->total(),2);
		
	}//end printBody()
	
	
}//end class

?>