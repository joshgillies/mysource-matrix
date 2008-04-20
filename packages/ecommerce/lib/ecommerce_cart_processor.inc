<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +--------------------------------------------------------------------+
*
* $Id: ecommerce_cart_processor.inc,v 1.11.2.2 2008/04/20 23:06:47 bshkara Exp $
*
*/


/*
* Ecommerce Package Cart Processor Object
*
* This object handles the interactions with the ecommerce cart, either stored in
* $_SESSION (the default), or passed in by the caller
*
* @author Robert Howard <rhoward@squiz.net>
* @author Luke Wright <lwright@squiz.net>
* @author Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.11.2.2 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/


class Ecommerce_Cart_Processor
{

	var $cart = NULL;


	/**
	* Constructor
	*/
	function Ecommerce_Cart_Processor()
	{
		// set, but rely on external objects to initialise
		$this->setCartContainer($_SESSION['sq_cart_contents']);

	}//end constructor


	/**
	* Sets the container to use for the cart
	*
	* @param object	&$cart	the cart to set the processor's cart to
	*
	* @return void
	* @access public
	*/
	function setCartContainer(&$cart)
	{
		$this->cart =& $cart;

	}//end setCartContainer()


	/**
	* Sets the cart to use for the cart
	*
	* @param object	$cart	the cart to set the contents of the processor's cart to
	*
	* @return void
	* @access public
	*/
	function setCartContents($cart)
	{
		$this->cart = $cart;

	}//end setCartContents()


	/**
	* Prepare cart for first use, if the relevant session vars aren't there
	*
	* Return 1 if the session var had to be created (ie. cart needed to be init'ed),
	* or 0 if the cart already existed (-1 is reserved for an error condition)
	*
	* @return int
	* @access public
	*/
	function initialiseCart()
	{
		$ret_val = 0;

		if (is_null($this->cart)) {
			$this->cart = Array();
			$ret_val = 1;
		}

		return $ret_val;

	}//end initialiseCart()


	/**
	* Unsets the contents of this particular cart
	*
	* @return boolean
	* @access public
	*/
	function clearCart()
	{
		$this->cart = Array();
		return TRUE;

	}//end clearCart()


	/**
	* Returns the details of one item in the following format:
	*
	* Array(
	*	'quantity'	=> quantity,		// the number of items in the cart
	*	'price'		=> value,			// the cost of the item at the time of purchase
	* )
	*
	* @param string	$assetid	the asset to retrieve from the cart
	*
	* @return array
	* @access public
	*/
	function &getItem($assetid)
	{
		if (!empty($this->cart[$assetid])) {
			return $this->cart[$assetid];
		} else {
			return FALSE;
		}

	}//end getItem()


	/**
	* Returns the contents of the cart in the following format:
	* <pre>
	* Array(
	*	'assetid'	=> Array(
	*					'quantity'			=> quantity,	// the number of items in the cart
	*					'price'				=> value,		// the cost of the item at the time of purchase
	*					'name'				=> name,		// the name of the asset
	*					'taxable'			=> boolean		// whether to calculate tax for this item
	*					'value_has_tax'		=> boolean		// whether this item already has tax included in the price
	*				   ),
	* )
	* </pre>
	*
	* @return array
	* @access public
	*/
	function &getCart()
	{
		if (is_null($this->cart)) $this->initialiseCart();

		// remove any invalid cart items
		$this->validateCart();

		return $this->cart;

	}//end getCart()


	/**
	* Return the sum-total of all the items currently in this cart
	* If taxation rate is specified then it will adjust product prices accordingly. You need this
	* parameter if you are charging tax on your products and you want accurate sub-totals
	*
	* @param float	$taxation_cost	taxation cost contains the total cost of tax
	* @param float	$taxation_rate	taxation rate of each item
	*
	* @return float
	* @access public
	*/
	function getTotal($taxation_cost=0.0, $taxation_rate=0.0)
	{
		$total = 0.00;
		foreach ($this->cart as $assetid => $details) {
			// -1 == value not known
			if ($details['price'] != -1) {
				// adjust product prices
				$has_tax = array_get_index($details, 'has_tax', FALSE);
				$calculate_tax = array_get_index($details, 'taxable', FALSE);
				if ($calculate_tax) {
					$item_costs = $this->_calculateItemCosts($details['price'], $taxation_rate, $has_tax);
					$cost = $item_costs['tax_adjusted_cost'];
				} else {
					$cost = $details['price'];
				}

				$total += $details['quantity'] * $cost;
			}
		}

		$total += $taxation_cost;
		return $total;

	}//end getTotal()


	/**
	*
	* Return the sum of the taxation cost of all the items currently in this cart
	*
	* @param float	$taxation_rate	tax value
	*
	* @return float
	* @access public
	*/
	function getTotalTax($taxation_rate=0.0)
	{
		$totalTax = 0.00;
		foreach ($this->cart as $assetid => $details) {
			// -1 == value not known
			if (($details['price'] != -1) && ($details['taxable'])) {
				$has_tax = array_get_index($details, 'has_tax', FALSE);
				$item_costs = $this->_calculateItemCosts($details['price'], $taxation_rate, $has_tax);
				$tax_cost = $item_costs['tax_cost'];

				$totalTax += $details['quantity'] * $tax_cost;
			}
		}

		return $totalTax;

	}//end getTotalTax()


	/**
	*
	* Gets an individual item's tax cost
	*
	* @param float		$cost			cost of the item
	* @param float		$taxation_rate	tax value
	* @param boolean	$has_tax		whether this cost already includes tax
	*
	* @return array
	* @access public
	*/
	function _calculateItemCosts($cost, $taxation_rate, $has_tax)
	{
		// deduct tax or add tax depending on whether item currently has tax included in its price
		if ($has_tax) {
			// deduct tax and alter product cost as well to reflect this
			$tax_cost = $cost * $taxation_rate;
			$cost = $cost - ($cost * $taxation_rate);
		} else {
			// add tax
			$tax_cost = $cost * $taxation_rate;
		}
		$item_costs = Array(
						'tax_adjusted_cost'	=> $cost,
						'tax_cost'			=> $tax_cost,
					  );

		return $item_costs;

	}// end _calculateItemCosts()


	/**
	* Return the count of all the items currently in this cart
	*
	* @return float
	* @access public
	*/
	function getCount()
	{
		if (is_null($this->cart)) return 0;
		$count = 0;
		foreach ($this->cart as $assetid => $details) {
			$count += $details['quantity'];
		}

		return $count;

	}//end getCount()


	/**
	* Return the count of the unique items currently in this cart
	*
	* @return float
	* @access public
	*/
	function getUniqueCount()
	{
		return count($this->cart);

	}//end getUniqueCount()


	/**
	* Adds an item(s) to the cart
	*
	* If an item already exists in the cart, add the quantity to the existing quantity
	* Use addItem() for blindly adding items, rather than using updateItem()
	* Returns true if the cart was updated
	*
	* @param string	$assetid	The assetid of the item to add
	* @param int	$quantity	The quantity to add
	*
	* @return boolean
	* @access public
	*/
	function addItem($assetid, $quantity=1)
	{
		if ($quantity <= 0) {
			// quantity is less than zero, throw a hissy fit
			if ($quantity < 0) {
				trigger_localised_error('ECOM0004', E_USER_WARNING);
			}
			return FALSE;
		}

		// check that the passed item assetid is indeed valid and
		// able to be bought
		assert_valid_assetid($assetid);
		$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($assetid), '', FALSE, 'type_code');

		if (!$asset_info) {
			trigger_localised_error('SYS0087', E_USER_WARNING, $assetid);
			return FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->am->includeAsset($asset_info[$assetid]);
		if (!implements_interface($asset_info[$assetid], 'buyable')) {
			trigger_localised_error('ECOM0005', E_USER_WARNING, $assetid);
			return FALSE;
		}

		if (!isset($this->cart[$assetid])) {
			$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			if ($asset) {

				// ugly, but we need to work around the backwards way Files store names
				if (is_a($asset, 'File') || is_subclass_of($asset, 'File')) {
					$name = $asset->attr('title');
				} else {
					$name = $asset->name;
				}

				// only include a price if the object is a product
				// only include taxable boolean if the object is a product
				$taxable = FALSE;
				$has_tax = FALSE;
				if (is_a($asset, 'Product') || is_subclass_of($asset, 'Product')) {
					$price = $asset->attr('price');

					// get the taxable boolean
					$tmp_price_obj = $asset->attr('price_obj');
					if (!empty($tmp_price_obj) ) {
						if ((!empty($tmp_price_obj->value_has_tax)) && ($tmp_price_obj->value_has_tax == 1)) {
							$has_tax = TRUE;
						}
						if ((!empty($tmp_price_obj->calculate_tax)) && ($tmp_price_obj->calculate_tax == 1)) {
							$taxable = TRUE;
						}
					}
				} else {
					$price = -1;
				}

				$this->cart[$assetid] = Array(
											'price'		=> $price,
											'quantity'	=> 0,
											'name'		=> $name,
											'has_tax'	=> $has_tax,
											'taxable'	=> $taxable,
										);
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			} else {
				return FALSE;
			}
		}//end if

		$this->cart[$assetid]['quantity'] += $quantity;

		return TRUE;

	}//end addItem()


	/**
	* Updates the quantity of an item(s) in the cart
	*
	* Assumption: items were correctly added using the addItem() method
	* and therefore, only valid items are found in the cart (because of the
	* addItem() checking)
	* Returns true if the cart was updated
	*
	* @param string	$assetid	The assetid of the item to remove
	* @param int	$quantity	The new quantity of this item
	*
	* @return boolean
	* @access public
	*/
	function updateItem($assetid, $quantity)
	{
		// item doesn't exist in cart, then silently redirect to addItem()
		if (!isset($this->cart[$assetid])) {
			return $this->addItem($assetid, $quantity);
		}

		// if we are not updating anything, silently return
		if (array_get_index($this->cart[$assetid], 'quantity', 0) == $quantity) {
			return FALSE;
		}

		// Qty < 0, throw an error
		if ($quantity < 0) {
			trigger_localised_error('ECOM0010', E_USER_WARNING);
			return FALSE;
		} else if ($quantity == 0) {
			$this->removeItem($assetid);
		} else if (isset($this->cart[$assetid]['quantity']) && $this->cart[$assetid]['quantity'] == $quantity) {
			// not updating, so fail
			return FALSE;
		} else {
			$this->cart[$assetid]['quantity'] = $quantity;
		}

		return TRUE;

	}//end updateItem()


	/**
	* Removes item(s) from the cart
	*
	* Assumption: items were correctly added using the addItem() method
	* and therefore, only valid items are found in the cart (because of the
	* addItem() checking)
	* Returns true if the cart was updated
	*
	* @param string	$assetid	The assetid of the item to remove
	* @param int	$quantity	The quantity of this item to remove - leave blank to remove all of that item
	*
	* @return boolean
	* @access public
	*/
	function removeItem($assetid, $quantity=NULL)
	{
		// item doesn't exist in cart - if the quantity is zero (or ALL)
		// then silently return, otherwise error
		if (!isset($this->cart[$assetid])) {
			if ($quantity > 0) {
				trigger_localised_error('ECOM0006', E_USER_WARNING);
			}
			return FALSE;
		}

		// after this point we can assume that asset exists in the cart
		// quantity is null, remove everything
		if (is_null($quantity)) {
			$quantity = $this->cart[$assetid]['quantity'];
		}

		// qty == 0, silently return - Qty < 0, throw an error
		if ($quantity <= 0) {
			if ($quantity < 0) {
				trigger_localised_error('ECOM0007', E_USER_WARNING);
			}
			return FALSE;
		}

		if ($quantity > $this->cart[$assetid]['quantity']) {
			trigger_localised_error('ECOM0008', E_USER_WARNING);
			return FALSE;
		} else if ($quantity == $this->cart[$assetid]['quantity']) {
			unset($this->cart[$assetid]);
		} else {
			$this->cart[$assetid]['quantity'] -= $quantity;
		}

		return TRUE;

	}//end removeItem()


	/**
	* Takes the contents of the cart and hashes it
	* Used by the ecom checkout form
	* Returns the hashed form of the cart
	*
	* @return string
	* @access public
	*/
	function hashCart()
	{
		// don't grab by reference, because we're sorting and mucking around with the array
		$tmp_cart = $this->cart;
		ksort($tmp_cart);
		$hash = md5(serialize($tmp_cart));

		return $hash;

	}//end hashCart()


	/**
	*
	* Prints out the contents of the Ecommerce Cart
	*
	* @param string		$taxation_name	name of tax
	* @param int		$taxation_rate	tax value (0.1 is 10%, 0.2 is 20%)
	* @param array		$flat_charges	flat charges Array('Name' => 'Price')
	* @param boolean	$remove_tax		whether to remove tax from taxed products, i.e. if country is not defined as taxable
	*
	* @return void
	* @access public
	*/
	function printReceipt($taxation_name='GST', $taxation_rate=0, $flat_charges=Array(), $remove_tax=FALSE)
	{
		if (empty($this->cart)) return;

		$name_length = 0;
		$has_price   = FALSE;

		// formatting preparation
		foreach ($this->cart as $assetid => $details) {
			// determine longest item name

			$tmp_taxable = array_get_index($details, 'taxable', FALSE);
			$product_name = array_get_index($details, 'name', '');
			if (strlen($product_name) > $name_length) {
				$name_length = strlen(array_get_index($details, 'name', ''));
			}
			if ($tmp_taxable) {
				if (strlen('Excluded Tax ('.$taxation_name.')') > $name_length) {
					$name_length = strlen('Excluded Tax ('.$taxation_name.')');
				}
			}

			// determine whether all items have price
			if (!$has_price && array_get_index($details, 'price', -1) != -1) {
				$has_price = TRUE;
			}
		}

		// calculate the longest item name and calculate the number of flat_charges
		$flat_charges_total = 0.0;
		foreach ($flat_charges as $name => $price) {
			if (strlen($name) > $name_length) {
				$name_length = strlen($name);
			}
			$flat_charges_total += $price;

		}

		// standard format for the receipt header and rows
		$row_format = '%-'.$name_length.'s %10s'.($has_price ? ' %10s' : '')."\n";

		if ($has_price) {
			$header = sprintf($row_format, 'Item', 'Quantity', 'Price');
		} else {
			$header = sprintf($row_format, 'Item', 'Quantity');
		}

		$break = '';
		for ($ii = 1; $ii < strlen($header); $ii++) {
			$break .= '-';
		}
		$break .= "\n";

		echo $header;
		echo $break;

		// print each row
		foreach ($this->cart as $assetid => $details) {
			$cost = array_get_index($details, 'price', 0);
			// calculate tax for this item only if we are not removing tax
			$has_tax = array_get_index($details, 'has_tax', FALSE);
			$calculate_tax = array_get_index($details, 'taxable', FALSE);
			if ($calculate_tax) {
				$item_costs = $this->_calculateItemCosts($cost, $taxation_rate, $has_tax);
				$cost = $remove_tax ? $item_costs['tax_adjusted_cost'] : array_get_index($details, 'price', 0);
				$cost = $item_costs['tax_adjusted_cost'];
				$tax_cost = $item_costs['tax_cost'];
			} else {
				$tax_cost = 0;
			}

			if ($cost < 0) {
				$cost = '';
			} else {
				$cost = sprintf('$%.2f', $cost * array_get_index($details, 'quantity', 0));
				$tax_cost = sprintf('$%.2f', $tax_cost * array_get_index($details, 'quantity', 0));
			}

			if ($has_price) {
				printf($row_format, array_get_index($details, 'name', ''), array_get_index($details, 'quantity', 0), $cost);
			} else {
				printf($row_format, array_get_index($details, 'name', ''), array_get_index($details, 'quantity', 0));
			}
		}

		echo $break;

		// change the row format for the totals line
		$row_format = '%'.$name_length.'s %10s'.($has_price ? ' %10s' : '')."\n";

		if ($has_price) {
			$totalTax = $this->getTotalTax($taxation_rate);

			$sub_total = $remove_tax ? $this->getTotal($totalTax, $taxation_rate)-$totalTax : $this->getTotal(0, $taxation_rate);
			echo sprintf($row_format, 'Total', $this->getCount(), sprintf('$%.2f', $sub_total));

			if (($totalTax > 0) || ($flat_charges_total > 0.0)) {
				if ($totalTax > 0) {
					$row_title = $remove_tax ? 'Excluded Tax ('.$taxation_name.')' : 'Total ('.$taxation_name.')';
					echo sprintf($row_format, $row_title, $this->getCount(), sprintf('$%.2f', $totalTax));
				}
				foreach ($flat_charges as $name => $price) {
					echo sprintf($row_format, $name, 1, sprintf('$%.2f', $price));
				}
				echo $break;
				$grand_total = $remove_tax ? $this->getTotal($totalTax, $taxation_rate)+$flat_charges_total-$totalTax : $this->getTotal($totalTax, $taxation_rate)+$flat_charges_total;
				echo sprintf($row_format, 'Grand Total', $this->getCount(), sprintf('$%.2f', $grand_total));
			}
		} else {
			echo sprintf($row_format, 'Total', $this->getCount());
		}

		return;

	}//end printReceipt()


	/**
	* Check the current cart and if the cart item does not exist,
	* remove them from the cart
	*
	* @return void
	* @access public
	*/
	function validateCart()
	{
		if (empty($this->cart)) return;

		$asset_ids = array_keys($this->cart);
		$result = $GLOBALS['SQ_SYSTEM']->am->assetExists($asset_ids);

		// the array of existing assetids
		if (is_array($result)) {

			foreach ($asset_ids as $id) {
				if (!is_int(array_search($id, $result))) {
					$this->removeItem($id);
				}
			}

		// false, one item doesn't exist.
		} else if (is_bool($result) && !$result) {
			$this->removeItem($id);
		}

	}//end validateCart()


}//end class


?>
