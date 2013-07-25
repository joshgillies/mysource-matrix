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
* $Id: ecommerce_cart_processor.inc,v 1.30.4.2 2013/07/25 06:49:43 cupreti Exp $
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
* @version $Revision: 1.30.4.2 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/


class Ecommerce_Cart_Processor
{

	var $cart = NULL;


	/**
	* Constructor
	*/
	function Ecommerce_Cart_Processor($checkout_id = 0)
	{
		// set, but rely on external objects to initialise
		$this->setCartContainer($_SESSION['sq_cart_contents'][$checkout_id]);

		if (is_null($this->cart)) $this->initialiseCart();

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
		// remove any invalid cart items
		$this->validateCart();

		return $this->cart;

	}//end getCart()


	/**
	* Returns the sum-total of all the items currently in this cart
	* If taxation rate is specified then it will adjust product prices accordingly. You need this
	* parameter if you are charging tax on your products and you want accurate sub-totals.
	* The returned total does not include any delivery fees or flat charges.
	*
	* @param float	$taxation_cost	Taxation cost contains the total cost of tax
	* @param float	$taxation_rate	Taxation rate of each item
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
	* Returns the grand total of the cart.
	* Includes all tax considerations, all flat charges.
	* Does not include any delivery fees, this must be calculated separately.
	*
	* @param float		$taxation_rate	Taxation rate of each item
	* @param boolean	$remove_tax		Whether to remove tax from taxed products, i.e. if country is not defined as taxable
	* @param array		$flat_charges	Flat charges Array('Name' => 'Price')
	*
	* @return float
	* @access public
	*/
	function getGrandTotalExDelivery($taxation_rate, $remove_tax, $flat_charges)
	{
		$total_tax = $this->getTotalTax($taxation_rate);
		$flat_charges_total = $this->getFlatChargesTotal($flat_charges);

		if ($remove_tax) {
			$grand_total = ($this->getTotal($total_tax, $taxation_rate) + $flat_charges_total) - $total_tax;
		} else {
			$grand_total = $this->getTotal($total_tax, $taxation_rate) + $flat_charges_total;
		}

		return $grand_total;

	}//end getGrandTotalExDelivery()


	/**
	* Returns the total cost of all flat charges.
	*
	* @param array	$flat_charges	Flat charges Array('Name' => 'Price')
	*
	* @return float
	* @access public
	*/
	function getFlatChargesTotal($flat_charges)
	{
		$flat_charges_total = 0.0;
		foreach ($flat_charges as $name => $price) {
			$flat_charges_total += $price;
		}

		return $flat_charges_total;

	}//end getFlatChargesTotal()


	/**
	* Returns the total delivery costs for this cart.
	*
	* @param float	$grand_total	Grand total of contents of cart
	*
	* @return float
	* @access public
	*/
	function getTotalDeliveryCosts($grand_total)
	{
		$total_delivery_cost = 0.00;
		if (isset($_SESSION['sq_region_specific'])) {
			foreach ($this->cart as $assetid => $details) {
				//if the asset (product) is not a donation, calculate delivery prices per weight metric
				if (!array_get_index($details, 'is_donation', FALSE) && isset($_SESSION['sq_region_specific']['delivery_weight_fee'])) {
					$delivery_weight_fee = $_SESSION['sq_region_specific']['delivery_weight_fee'];
					// it will either be a metadata field or an asset attribute that we have to get weight info from
					if (isset($_SESSION['sq_region_specific']['delivery_weight_fee']['metadata_fieldid'])) {
						$fieldid = $_SESSION['sq_region_specific']['delivery_weight_fee']['metadata_fieldid'];
						$mm =& $GLOBALS['SQ_SYSTEM']->getMetadataManager();
						$weight = (float) $mm->getMetadataValueByAssetid($assetid, $fieldid);
					} else if (isset($_SESSION['sq_region_specific']['delivery_weight_fee']['attribute_name'])) {
						$attribute_name = $_SESSION['sq_region_specific']['delivery_weight_fee']['attribute_name'];
						$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
						$weight = (float) $asset->attr($attribute_name);
					}

					if (isset($weight) && !empty($weight)) {
						$product_quantity = array_get_index($details, 'quantity', 0);
						
						if (isset($delivery_weight_fee['max_quantity']) && ($delivery_weight_fee['max_quantity'] < $product_quantity)) {
							$product_quantity = $delivery_weight_fee['max_quantity'];
						}
						
						$total_delivery_cost += $weight * $delivery_weight_fee['fee'] * $product_quantity;
					}
				}//end if
			}//end for
		}//end if

		// calculate international delivery fee
		if (isset($_SESSION['sq_region_specific']['international_delivery_fee'])) {
			$total_delivery_cost += $_SESSION['sq_region_specific']['international_delivery_fee'];
		}

		// calculate additional conditional delivery fees
		// based on total of cart contents plus all existing delivery fees
		if (isset($_SESSION['sq_region_specific']['conditions'])) {
			$grand_total_plus_delivery = $grand_total + $total_delivery_cost;
			$conditions = $_SESSION['sq_region_specific']['conditions'];
			foreach ($conditions as $key => $data) {
				// evaluate each condition
				$operator = print_r($data['operator'], TRUE);
				if (eval('$satisfied = $grand_total_plus_delivery'.$operator.'$data[\'comparison_price\']; return $satisfied;')) {
					if ($data['remove_fees']) {
						$total_delivery_cost = 0;
					} else {
						$total_delivery_cost += $data['additional_cost'];
					}
					// evaluate only the first condition satisfied
					break;
				}
			}
		}

		//cap the delivery fee if the maximum delivery fee is set
		if (isset($_SESSION['sq_region_specific']['max_delivery_fee']) && ($_SESSION['sq_region_specific']['max_delivery_fee'] < $total_delivery_cost)) {
			$total_delivery_cost = $_SESSION['sq_region_specific']['max_delivery_fee'];
		}
		
		return $total_delivery_cost;

	}//end getTotalDeliveryCosts()


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
		$total_tax = 0.00;
		foreach ($this->cart as $assetid => $details) {
			// -1 == value not known
			if (($details['price'] != -1) && ($details['taxable'])) {
				$has_tax = array_get_index($details, 'has_tax', FALSE);
				$item_costs = $this->_calculateItemCosts($details['price'], $taxation_rate, $has_tax);
				$tax_cost = $item_costs['tax_cost'];

				$total_tax += $details['quantity'] * $tax_cost;
			}
		}

		return $total_tax;

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
			$tax_cost = $cost - ($cost / (1 + $taxation_rate));
			$cost = $cost - $tax_cost;
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
			if (!array_get_index($details, 'is_donation', FALSE)) {
				$count += $details['quantity'];
			}
		}

		return $count;

	}//end getCount()


	/**
	* Returns TRUE if the cart is empty
	*
	* @return float
	* @access public
	*/
	function isEmpty()
	{
		return is_null($this->cart) || (count($this->cart) <= 0);

	}//end isEmpty()


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
		$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (is_null($asset)) {
			return FALSE;
		}
		
		if (!($asset instanceof Donation) && !($asset instanceof Buyable)) {
			trigger_localised_error('ECOM0005', E_USER_WARNING, $assetid);
			return FALSE;
		}
		
		if ($quantity <= 0) {
			if ($asset instanceof Donation) {
				trigger_localised_error('ECOM0025', E_USER_WARNING);
			} else if ($quantity < 0) { // quantity is less than zero, throw a hissy fit
				trigger_localised_error('ECOM0004', E_USER_WARNING);
			}
			return FALSE;
		}


		if (!isset($this->cart[$assetid])) {
			// ugly, but we need to work around the backwards way Files store names
			if ($asset instanceof File) {
				$name = $asset->attr('title');
			} else {
				$name = $asset->name;
			}

			// only include a price if the object is a product
			// only include taxable boolean if the object is a product
			$taxable = FALSE;
			$has_tax = FALSE;
			$is_donation = FALSE;
			$refund_type = 0;
			$product_code = '';
			$financial_code = '';

			if ($asset instanceof Donation) {
				$price = 1;
				$is_donation = TRUE;
			} else if ($asset instanceof Product) {
				$price = $asset->attr('price');

				$refund_type = $asset->attr('refund_type');
				$product_code = $asset->attr('product_code');
				$financial_code = $asset->attr('financial_code');

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
										'price'				=> $price,
										'quantity'			=> $quantity,
										'name'				=> $name,
										'has_tax'			=> $has_tax,
										'taxable'			=> $taxable,
										'is_donation'		=> $is_donation,
										'refund_type' 	    => $refund_type,
										'product_code'		=> $product_code,
										'financial_code'	=> $financial_code,
									);
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			
		} else {
			if ($asset instanceof Donation) {
				$this->cart[$assetid]['quantity'] = $quantity;
			} else {
				$this->cart[$assetid]['quantity'] += $quantity;
			}
		}//end if

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
	* @param string		$taxation_name		name of tax
	* @param int		$taxation_rate		tax value (0.1 is 10%, 0.2 is 20%)
	* @param array		$flat_charges		flat charges Array('Name' => 'Price')
	* @param boolean	$remove_tax			whether to remove tax from taxed products, i.e. if country is not defined as taxable
	* @param string		$currency_symbol	symbol to use as the currency for all prices
	* @param int		$float_precision	precision of the floating numbers used for prices	
	* @param string	    $refund_info		refund info to include if some product in cart are non-refundable
	*
	* @return void
	* @access public
	*/
	function printReceipt($taxation_name='GST', $taxation_rate=0, $flat_charges=Array(), $remove_tax=FALSE, $currency_symbol='$', $float_precision=2, $refund_info='')
	{
		if (empty($this->cart)) return;

		// ensure the float precision is an int
		$float_precision = (int) $float_precision;
		$price_format = $currency_symbol.'%.'.$float_precision.'f';
		
		$name_length = 0;
		$has_price = FALSE;
		$has_products = FALSE;
        
        // TRUE if any item in cart is non-refundable
        $non_refundable_flag = FALSE;
        
		// formatting preparation
		foreach ($this->cart as $assetid => $details) {
			// determine longest item name

			$tmp_taxable = array_get_index($details, 'taxable', FALSE);
			$product_name = array_get_index($details, 'name', '');
			
			if (!$non_refundable_flag) {
                $refund_type = array_get_index($details, 'refund_type', 'NonRefundable');
                $non_refundable_flag = ($refund_type === 'NonRefundable') ? TRUE : FALSE;
			}
			
			if (strlen($product_name) > $name_length) {
				$name_length = strlen(array_get_index($details, 'name', ''));
			}
			if ($tmp_taxable) {
				if (strlen('Excluded Tax ('.$taxation_name.')') > $name_length) {
					$name_length = strlen('Excluded Tax ('.$taxation_name.')');
				}
			}

			// determine whether all items have price
			$price = array_get_index($details, 'price', -1);
			if (!$has_price && ($price != -1)) {
				$has_price = TRUE;
			}
			
			// check if there are not only donations
			if (!$has_products && !array_get_index($details, 'is_donation', FALSE) && ($price != -1)) {
				$has_products = TRUE;
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
		$row_format = '%-'.$name_length.'s %10s'.($has_price && $has_products ? ' %10s' : '')."\n";

		if ($has_price) {
			if ($has_products) {
				$header = sprintf($row_format, 'Item', 'Quantity', 'Price');
			} else { // there are donations only
				$header = sprintf($row_format, 'Item', 'Amount');
			}
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
				$cost = $item_costs['tax_adjusted_cost'];
				$tax_cost = $item_costs['tax_cost'];
			} else {
				$tax_cost = 0;
			}

			$product_quantity = array_get_index($details, 'quantity', 0);

			if ($cost < 0) {
				$cost = '';
			} else {
				$cost = sprintf($price_format, $cost * $product_quantity);
				$tax_cost = sprintf($price_format, $tax_cost * $product_quantity);
			}

			if ($has_price) {
				if ($has_products) {
					if (array_get_index($details, 'is_donation', FALSE)) {
						$product_quantity = '';
					}
					printf($row_format, array_get_index($details, 'name', ''), $product_quantity, $cost);
				} else {
					printf($row_format, array_get_index($details, 'name', ''), $cost);
				}
			} else {
				printf($row_format, array_get_index($details, 'name', ''), $product_quantity);
			}
		}//end foreach

		echo $break;

		// change the row format for the totals line
		$row_format = '%'.$name_length.'s %10s'.($has_price && $has_products ? ' %10s' : '')."\n";

		if ($has_price) {
			$total_tax = $this->getTotalTax($taxation_rate);

			$sub_total = $remove_tax ? $this->getTotal($total_tax, $taxation_rate)-$total_tax : $this->getTotal(0, $taxation_rate);
			
			if ($has_products) {
				echo sprintf($row_format, 'Total', $this->getCount(), sprintf($price_format, $sub_total));
			} else {
				echo sprintf($row_format, 'Total', sprintf($price_format, $sub_total));
			}
			
			$grand_total_ex_delivery = $this->getGrandTotalExDelivery($taxation_rate, $remove_tax, $flat_charges);
			$total_delivery_cost = $this->getTotalDeliveryCosts($grand_total_ex_delivery);

			if (($total_tax > 0) || ($flat_charges_total > 0.0) || $total_delivery_cost > 0) {
				if ($total_tax > 0) {
					$row_title = $remove_tax ? 'Excluded Tax ('.$taxation_name.')' : 'Total ('.$taxation_name.')';
					echo sprintf($row_format, $row_title, $this->getCount(), sprintf($price_format, $total_tax));
				}
				echo $break;
				foreach ($flat_charges as $name => $price) {
					echo sprintf($row_format, $name, '', sprintf($price_format, $price));
				}


				if ($total_delivery_cost > 0) {
					echo sprintf($row_format, 'Delivery Fee', '', sprintf($price_format, $total_delivery_cost));
				}

				// figure out whether to print this last line break
				if (($total_delivery_cost > 0) || ($flat_charges_total > 0)) {
					echo $break;
				}
				echo sprintf($row_format, 'Grand Total', '', sprintf($price_format, $grand_total_ex_delivery + $total_delivery_cost));
			}
            if (!empty($refund_info) && $non_refundable_flag) {
                echo "\n".$refund_info."\n";
            }
            
		} else {
			echo sprintf($row_format, 'Total', $this->getCount());
		}

		return;

	}//end printReceipt()


	/**
	*
	* Prints out the contents of the Ecommerce Cart adhering to the Receipt Format bodycopy
	*
	* @param object		&$contents_bodycopy	receipt contents bodycopy
	* @param object		&$format_bodycopy	receipt format bodycopy
	* @param string		$taxation_name		name of tax
	* @param int		$taxation_rate		tax value (0.1 is 10%, 0.2 is 20%)
	* @param array		$flat_charges		flat charges Array('Name' => 'Price')
	* @param boolean	$remove_tax			whether to remove tax from taxed products, i.e. if country is not defined as taxable
	* @param string		$currency_symbol	symbol to use as the currency for all prices
	* @param int		$float_precision	precision of the floating numbers used for prices
	*
	* @return void
	* @access public
	*/
	function printCustomisedReceipt(&$contents_bodycopy, &$format_bodycopy, $taxation_name='GST', $taxation_rate=0, $flat_charges=Array(), $remove_tax=FALSE, $currency_symbol='$', $float_precision=2)
	{
		if (empty($this->cart)) return;

		// ensure the float precision is an int
		$float_precision = (int) $float_precision;
		$price_format = $currency_symbol.'%.'.$float_precision.'f';

		$content_keywords = $contents_bodycopy->getKeywords();
		$content_replacements = Array();

		// print each asset in the cart using the receipt type format
		$format_keywords = $format_bodycopy->getKeywords();
		ob_start();
		foreach ($this->cart as $assetid => $details) {
			$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			$format_replacements = Array();

			// replace all asset based keywords
			foreach ($format_keywords as $keyword) {
				$format_replacements[$keyword] = $asset->getKeywordReplacement($keyword);
			}

			$product_quantity = array_get_index($details, 'quantity', 0);

			// calculate tax and costs for this item
			$cost = array_get_index($details, 'price', 0);
			// calculate tax for this item only if we are not removing tax
			$has_tax = array_get_index($details, 'has_tax', FALSE);
			$calculate_tax = array_get_index($details, 'taxable', FALSE);
			if ($calculate_tax) {
				$item_costs = $this->_calculateItemCosts($cost, $taxation_rate, $has_tax);
				$cost = $item_costs['tax_adjusted_cost'];
				$tax_cost = $item_costs['tax_cost'];
			} else {
				$tax_cost = 0;
			}

			// replace all ecommerce specific keywords
			if (array_get_index($details, 'is_donation', FALSE)) {
				$format_replacements['item_quantity_added'] = '';
				$format_replacements['item_price_incl_tax'] = sprintf($price_format, $product_quantity);
			} else {
				$format_replacements['item_quantity_added'] = $product_quantity;
				$format_replacements['item_price_incl_tax'] = sprintf($price_format, $cost);
			}

			$format_replacements['item_tax'] = sprintf($price_format, $tax_cost);

			$format_replacements['total_item_price'] = sprintf($price_format, $cost * $product_quantity);
			$format_replacements['total_item_tax'] = sprintf($price_format, $tax_cost * $product_quantity);

			$format_bodycopy->setKeywordReplacements($format_replacements);
			$format_bodycopy->printBody();
		}
		
		// receipt contents bodycopy keyword replacements
		$content_replacements['receipt_listing'] = ob_get_clean();

		$total_tax = $this->getTotalTax($taxation_rate);
		$content_replacements['total_tax'] = $remove_tax ? sprintf($price_format, 0) : sprintf($price_format, $total_tax);
		$content_replacements['tax_name'] = $taxation_name;

		ob_start();
		$flat_charges_total = 0.0;
		echo '<table>';
		foreach ($flat_charges as $name => $price) {
			echo '<tr><td>'.$name.':</td><td>'.sprintf($price_format, $price).'</td></tr>';
			$flat_charges_total += $price;
		}
		echo '</table>';
		$content_replacements['flat_fees_table'] = ob_get_clean();
		$content_replacements['flat_fees_total'] = sprintf($price_format, $flat_charges_total);

		$sub_total = $remove_tax ? $this->getTotal($total_tax, $taxation_rate)-$total_tax : $this->getTotal(0, $taxation_rate);
		$content_replacements['sub_total'] = sprintf($price_format, $sub_total);

		$grand_total_ex_delivery = $this->getGrandTotalExDelivery($taxation_rate, $remove_tax, $flat_charges);
		$total_delivery_cost = $this->getTotalDeliveryCosts($grand_total_ex_delivery);
		$content_replacements['delivery_total'] = sprintf($price_format, $total_delivery_cost);
		$content_replacements['grand_total'] = sprintf($price_format, $grand_total_ex_delivery + $total_delivery_cost);

		$contents_bodycopy->setKeywordReplacements($content_replacements);

		ob_start();
		$contents_bodycopy->printBody();
		$customised_receipt = ob_get_clean();

		return $customised_receipt;

	}//end printCustomisedReceipt()


	/**
	*
	* Get the XML representation of this cart to store in the order_xml attribute of Ecommerce Order asset
	*
	* @param string		$taxation_name		name of tax
	* @param int		$taxation_rate		tax value (0.1 is 10%, 0.2 is 20%)
	* @param array		$flat_charges		flat charges Array('Name' => 'Price')
	* @param boolean	$remove_tax			whether to remove tax from taxed products, i.e. if country is not defined as taxable
	* @param string		$currency_symbol	symbol to use as the currency for all prices
	* @param int		$float_precision	precision of the floating numbers used for prices
	*
	* @return void
	* @access public
	*/
	function getCartXml($taxation_name='GST', $taxation_rate=0, $flat_charges=Array(), $remove_tax=FALSE, $currency_symbol='$', $float_precision=2)
	{
		if (empty($this->cart)) return '';

		// ensure the float precision is an int
		$float_precision = (int) $float_precision;
		$price_format = '%.'.$float_precision.'f';

		//get Metadata Manager
		$mm =& $GLOBALS['SQ_SYSTEM']->getMetadataManager();
		
		$products_str = '';
		foreach ($this->cart as $assetid => $details) {
			$quantity = array_get_index($details, 'quantity', 0);

			// calculate tax and costs for this item
			$cost = array_get_index($details, 'price', 0);
			if (array_get_index($details, 'taxable', FALSE)) {
				$item_costs = $this->_calculateItemCosts($cost, $taxation_rate, array_get_index($details, 'has_tax', FALSE));
				$cost = $item_costs['tax_adjusted_cost'];
				$tax_cost = $item_costs['tax_cost'];
			} else {
				$tax_cost = 0;
			}

			//information of product is different if it is a donation
			if (array_get_index($details, 'is_donation', FALSE)) {
				$donation = 'yes';
				$cost = $quantity;
				$quantity = 1;
			} else {
				$donation = 'no';
			}

			$item_price = sprintf($price_format, $cost);
			$item_price_incl_tax = sprintf($price_format, $cost + $tax_cost);
			$total_item_price = sprintf($price_format, $cost * $quantity);
			$total_item_price_incl_tax = sprintf($price_format, ($cost + $tax_cost) * $quantity);
			$total_item_tax = sprintf($price_format, $tax_cost * $quantity);

			//get metadata of the product/donation asset
			$metadata_values = $mm->getMetadataFieldValues($assetid);
			$metadata_values_str = '';
			foreach ($metadata_values as $name => $value) {
				$metadata_values_str .= <<<HEREDOC

					<field>
						<name><![CDATA[$name]]></name>
						<value><![CDATA[$value]]></value>
					</field>
HEREDOC;
			}
			
			$metadata_str = '';
			if ($metadata_values_str != '') {
				$metadata_str = <<<HEREDOC

				<metadata>$metadata_values_str
				</metadata>
HEREDOC;
			}
			
			$products_str .= <<<HEREDOC

			<product>
				<id>$assetid</id>
				<name><![CDATA[{$details['name']}]]></name>
				<is_donation>$donation</is_donation>
				<quantity>$quantity</quantity>
				<item_price>$item_price</item_price>
				<total_item_price>$total_item_price</total_item_price>
				<item_price_incl_tax>$item_price_incl_tax</item_price_incl_tax>
				<total_item_price_incl_tax>$total_item_price_incl_tax</total_item_price_incl_tax>
				<item_tax>$tax_cost</item_tax>
				<total_item_tax>$total_item_tax</total_item_tax>$metadata_str
			</product>
HEREDOC;
		}
		
		//get total tax
		$total_tax = $this->getTotalTax($taxation_rate);
		$total_tax = $remove_tax ? sprintf('-'.$price_format, $total_tax) : sprintf($price_format, $total_tax);
		
		//get flat charges string
		$flat_charge_values_str = '';
		$flat_charges_total = 0.0;
		foreach ($flat_charges as $name => $price) {
			$flat_charge_values_str .= <<<HEREDOC

			<flat_charge>
				<name><![CDATA[$name]]></name>
				<price>$price</price>
			</flat_charge>
HEREDOC;
			$flat_charges_total += $price;
		}
		
		$flat_charges_str = '';
		if ($flat_charge_values_str != ''){
			$flat_charges_str = <<<HEREDOC

		<flat_charges>$flat_charge_values_str
		</flat_charges>
HEREDOC;
			
		}
		
		$flat_fees_total = sprintf($price_format, $flat_charges_total);

		$sub_total = sprintf($price_format, $this->getTotal(0, $taxation_rate));

		$grand_total_ex_delivery = $this->getGrandTotalExDelivery($taxation_rate, $remove_tax, $flat_charges);
		$delivery_total = $this->getTotalDeliveryCosts($grand_total_ex_delivery);
		
		$grand_total = sprintf($price_format, $grand_total_ex_delivery + $delivery_total);
		$grand_total_ex_delivery = sprintf($price_format, $grand_total_ex_delivery);
		$delivery_total = sprintf($price_format, $delivery_total);
		
		$cart_xml = <<<HEREDOC
	<cart>
		<currency_symbol><![CDATA[$currency_symbol]]></currency_symbol>
		<sub_total>$sub_total</sub_total>
		<taxation_name><![CDATA[$taxation_name]]></taxation_name>
		<total_tax>$total_tax</total_tax>
		<flat_fees_total>$flat_fees_total</flat_fees_total>
		<delivery_total>$delivery_total</delivery_total>
		<grand_total_ex_delivery>$grand_total_ex_delivery</grand_total_ex_delivery>
		<grand_total>$grand_total</grand_total>
		<products>$products_str
		</products>$flat_charges_str
	</cart>
HEREDOC;
		
		return $cart_xml;

	}//end getCartXml()
	
	
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
