<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: ecommerce_common.inc,v 1.2 2005/07/18 04:07:13 rhoward Exp $
*
*/

/*
* Ecommerce Package Library Functions
*
* This file contains global functions that are useful throughout the ecommerce module
* (but not beyond)
* This library generally handles the interactions with the $_SESSION cart
*
* @author Robert Howard <rhoward@squiz.net>
* @author Luke Wright <lwright@squiz.net>
* @author Andrei Railean <arailena@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/

/**
*	Structure of SESSION vars
	$_SESSION[SQ_CART] = Array(
							'id'	=> Array(
										'quantity'	=> int,
										'price'		=> int,
									   ),
						   );
*/

define('SQ_CART', 'sq_cart_contents');


/**
* Prepare cart for first use, if the relevant session vars aren't there
*
* @return int	1 if session var needed to be created (ie. cart
*				needed to be init'ed), 0 if cart already existed
*				(-1 is reserved for an error condition)
* @access public
*/
function initialiseCart()
{
	$ret_val = 0;

	if (!isset($_SESSION[SQ_CART])) {
		$_SESSION[SQ_CART] = Array();
		$ret_val = 1;
	}

	return $ret_val;

}//end initialiseCart()


/**
* Unsets the contents of this particular cart
*
* @return boolean	always true (false is reserved for an error condition)
* @access public
*/
function clearCart()
{
	unset($_SESSION[SQ_CART]);
	return true;

}//end clearCart()


/**
* Returns the details of one item in the following format:
*
* Array(
*	'quantity'	=> quantity,		// the number of items in the cart
*	'price'		=> value,			// the cost of the item at the time of purchase
* )
*
* @param string	$assetid			// the asset to retrieve from the cart
*
* @return Array(string=>mixed)
* @access public
*/
function &getItem($assetid)
{
	if (!empty($_SESSION[SQ_CART][$assetid])) {
		return $_SESSION[SQ_CART][$assetid];
	} else {
		return false;
	}

}//end getItem()


/**
* Returns the contents of the cart in the following format:
*
*	Array(
*		'assetid'	=> Array(
*						'quantity'	=> quantity,				// the number of items in the cart
*						'price'		=> value,					// the cost of the item at the time of purchase
*					   ),
*	);
*
* @return Array(mixed)
* @access public
*/
function &getCart()
{
	if (!isset($_SESSION[SQ_CART])) initialiseCart();

	return $_SESSION[SQ_CART];

}//end getCart()


/**
* Return the sum-total of all the items currently in this cart
*
* @return double
* @access public
*/
function getTotal()
{
	$cart =& getCart();
	$total = 0.0;
	foreach ($cart as $assetid => $details) {
		// -1 == value not known
		if ($details['price'] != -1) {
			$total += $details['quantity'] * $details['price'];
		}
	}

	return $total;

}//end getTotal()


/**
* Return the count of all the items currently in this cart
*
* @return double
* @access public
*/
function getCount()
{
	$cart =& getCart();
	$count = 0;
	foreach ($cart as $product_assetid => $details) {
		$count += $details['quantity'];
	}

	return $count;

}//end getCount()


// TODO: Remove: @param double		$price				The price to record against that product
/**
* Adds an item(s) to the cart
* If an item already exists in the cart, add the quantity to the existing quantity
* Use addItem() for blindly adding items, rather than using updateItem()
*
* @param string		$assetid	The assetid of the product to add
* @param int		$quantity	The quantity to add
*
* @return boolean	indicating success
* @access public
*/
function addItem($assetid, $quantity=1)
{
	// quantity is equal to or less than zero, throw a hissy fit
	if ($quantity <= 0) {
		trigger_localised_error('ECOM0004', E_USER_WARNING);
		return false;
	}

	// check that the passed product assetid is indeed valid and
	// a valid product (or subclass of product)
	assert_valid_assetid($assetid);
	$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($assetid), 'product', false);

	if (!$asset_info) {
		trigger_localised_error('ECOM0005', E_USER_WARNING, $assetid);
		return false;
	}

	$cart =& getCart();
	if (!isset($cart[$assetid])) {
		$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if ($asset) {
			$cart[$assetid] = Array(
								'price'		=> $asset->attr('price'),
								'quantity'	=> 0,
							  );
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
		} else {
			return false;
		}
	}

	$cart[$assetid]['quantity'] += $quantity;

	return true;

}//end addItem()


/**
* Updates the quantity of an item(s) in the cart
*
* Assumption: product lines were correctly made using the addItem() method
* and therefore, only valid products are found in the cart (because of the
* addItem() checking).
*
* @param string		$assetid	The assetid of the product to remove
* @param int		$quantity			The new quantity of this product
*
* @return boolean	indicating success
* @access public
*/
function updateItem($assetid, $quantity)
{
	$cart =& getCart();

	// product doesn't exist in cart, then silently redirect to addItem()
	if (!isset($cart[$assetid])) {
		bam('   ...Redirecting to addItem ('.$assetid.', '.$quantity.')');
		return addItem($assetid, $quantity);// TODO: Remove trigger_localised_error('ECOM0009', E_USER_WARNING);
	}

	// if we are not updating anything, silently return success
	if (array_get_index($cart[$assetid], 'quantity', 0) == $quantity) {
		return true;
	}

	// Qty < 0, throw an error
	if ($quantity <= 0) {
		trigger_localised_error('ECOM0010', E_USER_WARNING);
		return false;
	} else if ($quantity == 0) {
		removeItem($assetid);
	} else {
		$cart[$assetid]['quantity'] = $quantity;
	}

	return true;

}//end updateItem()


/**
* Removes an item(s) from the cart
*
* Assumption: product lines were correctly made using the add() method
* and therefore, only valid products are found in the cart (because of the
* add() checking).
*
* @param string	$assetid	The assetid of the product to remove
* @param int		$quantity		The quantity of this product to remove -
*								leave blank to remove all of that product
*
* @return boolean	indicating success
* @access public
*/
function removeItem($assetid, $quantity=null)
{
	$cart =& getCart();

	// asset doesn't exist in cart - if the quantity is zero (or ALL)
	// then silently return, otherwise error
	if (!isset($cart[$assetid])) {
		if (is_null($quantity) || ($quantity == 0)) {
			return true;
		} else if ($quantity > 0) {
			trigger_localised_error('ECOM0006', E_USER_WARNING);
			return false;
		}
	}

	// after this point we can assume that asset exists in the cart
	// quantity is null, remove everything
	if (is_null($quantity)) {
		$quantity = $cart[$assetid]['quantity'];
	}

	// qty == 0, silently return - Qty < 0, throw an error
	if ($quantity == 0) {
		return true;
	} else if ($quantity < 0) {
		trigger_localised_error('ECOM0007', E_USER_WARNING);
		return false;
	}

	if ($quantity > $cart[$assetid]['quantity']) {
		trigger_localised_error('ECOM0008', E_USER_WARNING);
		return false;
	} else if ($quantity == $cart[$assetid]['quantity']) {
		unset($cart[$assetid]);
	} else {
		$cart[$assetid]['quantity'] -= $quantity;
	}

}//end removeItem()


/**
* Takes the contents of the cart and hashes it
* Used by the ecom checkout form
*
*
*/
function hashCart()
{
	// don't grab by reference, because we're sorting and mucking around with the array
	$cart = getCart();

	ksort($cart);
	$hash = md5(serialize($cart));

	return $hash;

}//end hashCart()


?>