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
* $Id: ecommerce_cart.inc,v 1.4 2005/03/21 06:27:12 gsherwood Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';

/**
* Ecommerce_Cart
*
* Purpose
*   A simple shopping cart used for ecommerce functions
*
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Ecommerce_Cart extends Page
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
		$this->Page($assetid);

	}//end constructor


	/**
	* Returns an array of all the permitted link type, the type asset and
	* the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks()
	{
		$page_links = parent::_getAllowedLinks();
		$page_links[SQ_LINK_TYPE_1]['folder'] = Array('card' => 'M');
		$page_links[SQ_LINK_TYPE_1]['product'] = Array('card' => 'M');
		return $page_links;

	}//end _getAllowedLinks()


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
	* @return int	1 if session var needed to be created (ie. cart
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
		unset($_SESSION['sq_cart_contents']);
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
		if (!isset($cart[$product_assetid])) {
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
	* Updtes the quantity of an item(s) in the cart
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
	function update($product_assetid, $quantity)
	{
		$cart =& $this->get();

		// if we are not updating anything, silently return succes
		if (array_get_index($cart, $product_assetid, 0) == $quantity) return true;
		if (array_get_index($cart[$product_assetid], 'quantity', 0) == $quantity) return true;

		// product doesn't exist in cart, then silently return, otherwise error
		if (!isset($cart[$product_assetid])) {
			trigger_error('Attempt to update() an item in a cart when there is nothing to update', E_USER_WARNING);
			return false;
		}

		// Qty < 0, throw an error
		if ($quantity < 0) {
			trigger_error('Quantity to update() an item with a negative quantity', E_USER_WARNING);
			return false;
		} else if ($quantity == 0) {
			unset($cart[$product_assetid]);
		} else {
			$cart[$product_assetid]['quantity'] = $quantity;
		}

		return true;

	}//end update()


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
		return $_SESSION['sq_cart_contents'];

	}//end get()


	/**
	* Return the sum-total of all the items currently in this cart
	*
	* @return double
	* @access public
	*/
	function total()
	{
		$cart =& $this->get();
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
		$cart =& $this->get();
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
		?><div align="center"><form action="<?php echo $this->getURL() ?>" method="post"><?php

		if (!is_null(array_get_index($_POST,'update_quantity',null))) {
			foreach($_POST['update_quantity'] as $product_assetid => $new_quantity) {
				if (is_numeric($new_quantity)) {
					$this->update($product_assetid, (int)$new_quantity);
				}
			}
			?><strong>Product quantities changed.</strong><?php
		}

		require_once SQ_LIB_PATH.'/html_form/html_form.inc';

		$cart = $this->get();
		?>
		<table width="50%">
		<thead><tr>
		<th>Product Name</th>
		<th>Qty</th><?php
		if ($this->attr('show_price')) {
			?><th>Price</th><?php
			?><th>Subtotal</th><?php
		}
		?></thead>
		<?php if ($this->attr('show_price')) {
			// the W3C recommendations say that TFOOT must come before TBODY
			// for some reason, so here it is
			?><tfoot><tr><?php
			?><td style="text-align:right"><strong>Total</strong></td><?php
			?><td style="text-align:right"><strong><?php echo $this->count(); ?></strong></td><?php
			?><td colspan="2" style="text-align:right"><strong><?php echo '$'.number_format($this->total(),2); ?></strong></td><?php
			?></tr>
			</tfoot><?php
		}
		?><tbody><?php
		if (empty($cart)) {
			?><tr><td colspan="<?php echo $this->attr('show_price') ? 4 : 2 ?>" style="text-align:center">There are no items in your cart</td></tr><?php
		} else {
			foreach($cart as $product_assetid => $details) {
				?><tr><?php
				$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($product_assetid));
				?><td><a href="<?php echo $GLOBALS['SQ_SYSTEM']->am->getAssetURL($product_assetid) ?>"><?php echo $asset_info[$product_assetid]['name'] ?></a></td><?php
				?><td style="text-align:right"><?php text_box('update_quantity['.$product_assetid.']', $details['quantity'], 3) ?></td><?php
				if ($this->attr('show_price')) {
					?><td style="text-align:right"><?php echo '$'.number_format($details['price'],2) ?></td><?php
					?><td style="text-align:right"><?php echo '$'.number_format($details['quantity'] * $details['price'],2) ?></td><?php
				}
				?></tr><?php
			}
		}
		?></tbody>
		</table>

		<?php if (!empty($cart)) { ?>
			<input type="submit" value="Update Quantity">
			<?php 
			$form_id = $this->attr('ecom_form_id');
			if (!empty($form_id)) {
				$ecom_form_url = $GLOBALS['SQ_SYSTEM']->am->getAssetURL($form_id); ?>
			<input type="button" onclick="location.href = '<?php echo addslashes($ecom_form_url); ?>'" value="<?php echo $this->attr('checkout_text'); ?>">
			<?php } ?>
		<?php } ?>
		</form></div>
		<?php

	}//end printBody()


}//end class

?>