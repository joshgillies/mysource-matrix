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
* $Id: ecommerce_checkout.inc,v 1.95 2013/10/23 01:11:15 cupreti Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';
require_once SQ_PACKAGES_PATH.'/ecommerce/lib/ecommerce_cart_processor.inc';
require_once SQ_PACKAGES_PATH.'/ecommerce/ecommerce_order/ecommerce_order.inc';


/**
* Ecommerce_Checkout
*
* Purpose
*
*
* @author  Andrei Railean	<arailean@squiz.net>
* @author  Scott Kim		<skim@squiz.net>
* @version $Revision: 1.95 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Ecommerce_Checkout extends Page
{

	/**
	* Screen Context
	*
	* Variable holding context to be printed by 'printContext' function
	* Context can change based on the logic of this asset.
	*
	*/
	var $print_context;


	/**
	* Whether there are insufficient product quantities
	* @var boolean
	*/
	var $insufficient_quantities = FALSE;


	/**
	* Constructor
	*
	* @param int	assetid	Id of this asset
	*
	* @return void
	* @access public
	*/
	function __construct($assetid=0)
	{
		// initialise the temporary order container so as to avoid extra checking
		if (!isset($_SESSION['sq_local_cart_contents'])) {
			$_SESSION['sq_local_cart_contents'] = Array();
		}

		$this->_ser_attrs = TRUE;

		parent::__construct($assetid);

	}//end constructor


	/**
	* Returns a list of available contexts
	*
	* @return array
	* @access private
	*/
	function _getAvailableContexts()
	{
		return Array(
				'checkout'		=> 'Checkout',
				'confirmation'	=> 'Confirmation',
				'failure'		=> 'Failure',
				'receipt'		=> 'Receipt',
			   );

	}//end _getAvailableContexts()


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	* In the form
	*
	*   Array('[link_type]' => Array('[type_code]' => Array('card' => [cardinality], 'exclusive' => [exclusive])));
	* Where:
	*   link_type   = SQ_LINK_TYPE_1|SQ_LINK_TYPE_2|SQ_LINK_TYPE_3|SQ_LINK_NOTICE
	*   cardinality = [max number of links]|'M'
	*   exclusive   = true|false
	*
	* @return array
	* @access private
	*/
	function _getAllowedLinks()
	{
		$allowed_links = parent::_getAllowedLinks();
		$allowed_links[SQ_LINK_NOTICE]['ecommerce_delivery_method'] = Array('card' => 'M', 'exclusive' => FALSE);
		$allowed_links[SQ_LINK_NOTICE]['ecommerce_order'] = Array('card' => 'M', 'exclusive' => FALSE);

		return $allowed_links;

	}//end _getAllowedLinks()


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	* @param int		$contextid	what context to return the name from
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	protected function _getName($short_name=FALSE, $contextid=NULL)
	{
		// No context specified, using the current context
		if ($contextid === NULL) {
			$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();
		}//end if

		// Obtain the attribute value for Name from the specified Context
		$attr = ($short_name) ? 'short_name' : 'name';
		$values = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName($attr, $this->type(), Array($this->id), $contextid);
		if (empty($values) === TRUE) {
			return parent::_getName($short_name, $contextid);
		} else {
			return $values[$this->id];
		}

	}//end _getName()


	/**
	* Called by the design to print the body of this asset
	*
	* @return void
	* @access public
	*/
	function printBody()
	{
		// since we are doing a lot of editing stuff with the checkout page, it's
		// probably best if we tie everything together using db2
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');

		// process interface. this determines the print context
		$this->_processInterface();

		// error occured in processing interface, just return
		if (empty($this->print_context)) {
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return;
		}

		// print context
		$this->_printContext();
		if (array_get_index($this->_tmp, 'delete_all_orders_after_printing', FALSE)) {

			if (!isset($_SESSION['sq_local_cart_contents']['editing_order'])) {
				// clear global cart
				$cart = new Ecommerce_Cart_Processor($this->id);
				$cart->clearCart();
				unset($cart);
			}

			// clear local cart
			$_SESSION['sq_local_cart_contents'] = Array();
		}

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

	}//end printBody()


	/**
	* Retrieve the order from the current session data
	*
	* @return object
	* @access private
	*/
	function _retrieveOrder()
	{
		$order_id = array_get_index($_SESSION['sq_local_cart_contents'], 'pending_order_id', FALSE);
		if (!$order_id && isset($_SESSION['sq_local_cart_contents']['editing_order']) && $_SESSION['sq_local_cart_contents']['editing_order']) {
			// ok are we editing an existing order
			$order_id = array_get_index($_SESSION['sq_local_cart_contents'], 'order_id', FALSE);
		}

		if (isset($order_id) && $order_id) {
			return $GLOBALS['SQ_SYSTEM']->am->getAsset($order_id);
		} else {
			return NULL;
		}

	}//_retrieveOrder()


	/**
	* Log the current ecommerce order's actions
	*
	* @param string		$action	Description of the activity to log
	* @param object		$order	Order being logged
	*
	* @return void
	* @access private
	*/
	function _logOrderAction($action, &$order=NULL)
	{
		if (is_null($order)) {
			$order = $this->_retrieveOrder();
			if (is_null($order)) {
				return;
			}
		}

		$order_history = $order->attr('order_history');
		$index = date('r');
		while (isset($order_history[$index])) {
			$index .= ' ';
		}
		$order_history[$index] = $action;
		$order->setAttrValue('order_history', $order_history);

		$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
		$order->saveAttributes();
		$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

	}//_logOrderAction()


	/**
	* Process the interface
	*
	* This is the main controller function. It determines the context asset is in based on the request
	* and state variables
	*
	* @return void
	* @access private
	*/
	function _processInterface()
	{
		// edit Ecommerce Order. Check permission and lock order
		// if the user doesn't have permission or lock, return without setting
		// print_context
		$order_id = array_get_index($_REQUEST, 'edit_order_id', 0);

		if (!empty($order_id)) {
			$order = $GLOBALS['SQ_SYSTEM']->am->getAsset($order_id);

			$this->_logOrderAction(translate('Existing order assetid provided in URL parameter for editing'), $order);


			// order exists, and has write access
			if (!empty($order) && $order->writeAccess()) {

				if ($GLOBALS['SQ_SYSTEM']->am->acquireLock($order->id, 'attribute')) {
					$_SESSION['sq_local_cart_contents']['order_id']			= $order->id;
					$_SESSION['sq_local_cart_contents']['delivery_id']		= $order->attr('delivery_id');
					$_SESSION['sq_local_cart_contents']['delivery_state']	= $order->attr('delivery_state');
					$_SESSION['sq_local_cart_contents']['cart_contents']	= $order->attr('products');

					$this->print_context = 'checkout';
					$_SESSION['sq_local_cart_contents']['delivery_state']['last_context'] = 'checkout';

					// set edit flag true
					$_SESSION['sq_local_cart_contents']['editing_order'] = TRUE;

				} else {
					echo 'This order is currently being edited';
				}

			} else {
				echo 'You cannot edit this order';
			}
			return;

		}

		// default print context is'checkout'
		$this->print_context = 'checkout';

		$request_action = array_get_index($_REQUEST, $this->getPrefix().'_action');

		// if context is checkout, compare the global and local carts,
		// if they are different, update the local cart
		if (empty($request_action)) {
			if ($this->print_context === 'checkout') {
				$global_cart = new Ecommerce_Cart_Processor($this->id);

				// unset previous order id and delivery information
				unset($_SESSION['sq_local_cart_contents']['order_id']);
				unset($_SESSION['sq_local_cart_contents']['delivery_id']);
				unset($_SESSION['sq_local_cart_contents']['delivery_state']);
				unset($_SESSION['sq_local_cart_contents']['editing_order']);
				unset($_SESSION['sq_local_cart_contents']['success_order_id']);
				unset($_SESSION['sq_local_cart_contents']['promotion_code']);

				$local_cart_empty = empty($_SESSION['sq_local_cart_contents']['cart_contents']);

				$need_update = FALSE;
				if ($local_cart_empty) {
					// if global is not empty, update
					if (!$global_cart->isEmpty()) {
						$need_update = TRUE;
					}
				} else {
					// if global and local carts are different, update
					$local_cart = new Ecommerce_Cart_Processor();
					$local_cart->setCartContainer($_SESSION['sq_local_cart_contents']['cart_contents']);

					if (strcmp($global_cart->hashCart(), $local_cart->hashCart()) != 0) {
						$need_update = TRUE;
					}
				}

				// update the local cart
				if ($need_update) {
					$_SESSION['sq_local_cart_contents']['cart_contents'] = $global_cart->getCart();
				}

			}//end if
			return;
		}

		if (is_array($request_action)) {
			$action = key($request_action);
		} else {
			// cater for GET vars
			$action = $request_action;
		}

		$success = TRUE;

		$this->_logOrderAction(sprintf(translate('<em>Action: %s</em>'), ucwords(str_replace('_',' ', $action))));


		switch ($action) {
			case 'apply_promotional_code':
				$cart = new Ecommerce_Cart_Processor();
				$cart->setCartContainer($_SESSION['sq_local_cart_contents']['cart_contents']);
				$promo_code = array_get_index($_REQUEST, $this->getPrefix().'_promotional_code', '');

				// We will need "order" to apply the promotions
				if (isset($_SESSION['sq_local_cart_contents']['editing_order']) && $_SESSION['sq_local_cart_contents']['editing_order'] && isset($_SESSION['sq_local_cart_contents']['order_id'])) {
					$order = $GLOBALS['SQ_SYSTEM']->am->getAsset($_SESSION['sq_local_cart_contents']['order_id']);
				} else if (isset($_SESSION['sq_local_cart_contents']['pending_order_id'])) {
					$order = $GLOBALS['SQ_SYSTEM']->am->getAsset($_SESSION['sq_local_cart_contents']['pending_order_id']);
				} else {
					// Pending order is not created yet. So create the pending order
					$order = $this->_createPendingOrder();
					// Since we are just creating the order, the above "action" log obviously didnt got logged
					$this->_logOrderAction(sprintf(translate('<em>Action: %s</em>'), ucwords(str_replace('_',' ', $action))));

				}

				// Apply promotion to the items in the cart
				$remove_tax = isset($_SESSION['sq_region_specific']['remove_tax']) ? $_SESSION['sq_region_specific']['remove_tax'] : FALSE;
				if ($cart->applyPromotionalCode($promo_code, $order, $this->attr('taxation_rate')/100, $remove_tax)) {
					// Store the successful promotional code in the session
					$_SESSION['sq_local_cart_contents']['promotion_code'] = $promo_code;
				} else {
					$_SESSION['sq_local_cart_contents']['promotion_code'] = FALSE;
				}

				// Update global cart in the session
				$_SESSION['sq_cart_contents'][$this->id] = $cart->cart;

				// We intentially don't "break" here

			case 'go_to_confirmation':
				$delivery_method_id = array_get_index($_REQUEST, 'sq_delivery_method_id');

				// if we're already at confirmation, why are we going again?
				if (isset($_SESSION['sq_local_cart_contents']['delivery_state'])) {
					if ($_SESSION['sq_local_cart_contents']['delivery_state']['last_context'] == 'confirmation') {
						$this->print_context = 'confirmation';
						break;
					}
				}

				// if no delivery method is specified, we just use default one
				if(empty($delivery_method_id)) {
				    // get delivery method links
				    $delivery_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_NOTICE, '', FALSE, 'major', 'delivery_method');
				    if(isset($delivery_links[0]['minorid'])) {
					$delivery_method_id = $delivery_links[0]['minorid'];
				    }
				}

				if (!empty($delivery_method_id) && !empty($_SESSION['sq_local_cart_contents']['cart_contents'])) {

					$delivery_method = $GLOBALS['SQ_SYSTEM']->am->getAsset($delivery_method_id);
					if (empty($delivery_method)) {
						$this->print_context = 'checkout';
						return;
					}

					// delivery method state has cart contents
					$_SESSION['sq_local_cart_contents']['delivery_state']['cart_contents'] = $_SESSION['sq_local_cart_contents']['cart_contents'];
					$delivery_method->setState($_SESSION['sq_local_cart_contents']['delivery_state']);

					// process the form questions in the delivery interface and place into session var
					// also retrieve a more comprehensive version returned by the same function
					$form_answers = $delivery_method->processInputInterface();
					if ($form_answers === FALSE) {
						$this->_tmp['delivery_method_errors'] = $delivery_method->getErrors();
						$success = FALSE;
					}

					// set any region specific options
					$country_question = $this->attr('country_question');
					$taxable_countries = $this->attr('taxable_countries');

					// find the country code they have selected as their address
					if (isset($form_answers[$country_question])) {
						$country_code = $form_answers[$country_question]['value'];
						if (is_array($country_code)) {
							// need a scalar
							$country_code = $country_code[0];
						}
					}

					// only set region specific options if we have a country code to compare against
					$has_country_code = isset($country_code);
					if ($has_country_code || $this->attr('accept_empty_country_question')) {
						// place all region specific options here to evaluate later
						$_SESSION['sq_region_specific'] = Array();
						// if customer is not defined as a taxable country then exclude tax
						if ($has_country_code) {
							if (!in_array($country_code, $taxable_countries)) {
								$_SESSION['sq_region_specific']['remove_tax'] = TRUE;
							} else {
								$_SESSION['sq_region_specific']['remove_tax'] = FALSE;
							}
						}//end if

						// delivery prices per weight metric
						if ($has_country_code) {
							$delivery_weight_fees = $this->attr('delivery_weight_fee_mapping');
							foreach ($delivery_weight_fees as $key => $data) {
								if (in_array($country_code, $data['countries'])) {
									// found a valid mapping for this country so record the fee
									$_SESSION['sq_region_specific']['delivery_weight_fee']['fee'] = $data['price'];
									// only find the first occurrence of the country
									break;
								}
							}
						}//end if

						// record the item weight source
						$selector_type = $this->attr('item_weight_selector');
						switch ($selector_type) {
							case 'metadata':
								$fieldid = $this->attr('item_weight_metadata_source');
								if (!empty($fieldid)) {
									$_SESSION['sq_region_specific']['delivery_weight_fee']['metadata_fieldid'] = $fieldid;
								}
							break;

							case 'attribute':
								$attribute_name = $this->attr('item_weight_attribute_source');
								if (!empty($attribute_name)) {
									$_SESSION['sq_region_specific']['delivery_weight_fee']['attribute_name'] = $attribute_name;
								}
							break;
						}

						//if there is no custom fee, use the default fee
						if (!isset($_SESSION['sq_region_specific']['delivery_weight_fee']['fee'])) {
							// use default value price per weight metric
							$default_price_weight = $this->attr('default_delivery_weight_fee');
							$_SESSION['sq_region_specific']['delivery_weight_fee']['fee'] = $default_price_weight;
						}

						//if there is default maximum number of product quantity, add it to the session variable
						$default_max_product_quantity = $this->attr('default_max_product_quantity');
						if (!empty($default_max_product_quantity)) {
							$_SESSION['sq_region_specific']['delivery_weight_fee']['max_quantity'] = $default_max_product_quantity;
						}

						// additional international delivery fee
						if ($has_country_code) {
							$exempt_countries = $this->attr('international_delivery_fee_exempt_countries');
							if (!in_array($country_code, $exempt_countries)) {
								// you are going to be charged with this fee
								$international_fee = $this->attr('international_delivery_fee');
								if (!empty($international_fee)) {
									$_SESSION['sq_region_specific']['international_delivery_fee'] = $international_fee;
								}
							}
						}//end if

						// additional conditional delivery fee
						if ($has_country_code) {
							$conditional_fees = $this->attr('conditional_delivery_fees');
							$comparison_data = Array();
							if (!empty($conditional_fees)) {
								foreach ($conditional_fees as $key => $data) {
									if (in_array($country_code, $data['countries'])) {
										unset($data['countries']);
										$comparison_data[] = $data;
									}
								}
								if (!empty($comparison_data)) {
									$_SESSION['sq_region_specific']['conditions'] = $comparison_data;
								}
							}
						}//end if

						//if maximum delivery fee is set (!= 0), add it to the session variable
						$max_delivery_fee = $this->attr('max_delivery_fee');
						if ($max_delivery_fee != 0) {
							$_SESSION['sq_region_specific']['max_delivery_fee'] = $max_delivery_fee;
						}

					} else if (isset($_SESSION['sq_region_specific'])) {
						unset($_SESSION['sq_region_specific']);
					}//end if

					// create a temporary order
					$_SESSION['sq_local_cart_contents']['delivery_id']		= $delivery_method_id;
					$_SESSION['sq_local_cart_contents']['delivery_state']	= $delivery_method->getState();
					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($delivery_method);
				}//end if

				// if cart is empty, there's no point in checking out
				if ($success && empty($_SESSION['sq_local_cart_contents']['cart_contents'])) {
					$this->_tmp['delivery_method_errors'][] = 'The cart is empty.  We cannot perform a checkout.';
					$success = FALSE;
				}
				if ($success) {
					$this->print_context = 'confirmation';
				} else {
					$this->print_context = 'checkout';
				}

				if (isset($_SESSION['sq_local_cart_contents']['editing_order'])) {
					$_SESSION['sq_local_cart_contents']['editing_order'] = TRUE;
				}

			break;

			case 'return_to_checkout':
				// determine if taxable by comparing the default value of the question with taxable countries
				$country_question = $this->attr('country_question');
				if (!empty($country_question)) {
					$question = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->attr('country_question'));
					$default_value = $question->attr('default');
					if (isset($default_value[0]) && in_array($default_value[0], $this->attr('taxable_countries'))) {
						$_SESSION['sq_region_specific']['remove_tax'] = FALSE;
					} else {
						$_SESSION['sq_region_specific']['remove_tax'] = TRUE;
					}
				} else {
					$_SESSION['sq_region_specific']['remove_tax'] = FALSE;
				}

				$this->print_context = 'checkout';
			break;

			case 'payment_gateway_successful':
			case 'confirm_order':
				// components should be processed here
				// delivery processing
				$delivery_method = $this->_getActiveDeliveryMethod();

				if (empty($delivery_method)) {
					$this->_tmp['delivery_method_errors'][] = 'Delivery method not found during the confirmation';
					$this->print_context = 'failure';
					break;
				}
				// create an order if there are none pending, or the pending order does not exist
				if ((!isset($_SESSION['sq_local_cart_contents']['pending_order_id']) || !$GLOBALS['SQ_SYSTEM']->am->assetExists($_SESSION['sq_local_cart_contents']['pending_order_id']))) {
					// also check if we are editing an existing order
					if (!(isset($_SESSION['sq_local_cart_contents']['editing_order']) && $_SESSION['sq_local_cart_contents']['editing_order'])) {
						// create the pending order
						$this->_createPendingOrder();
					}
				}

				// let the delivery method do its thing
				$this->_processOrder($delivery_method);

			break;

		}//end switch

		// set the last context we saw to this one
		$_SESSION['sq_local_cart_contents']['delivery_state']['last_context'] = $this->print_context;
		if (isset($delivery_method)) {
			$delivery_method->setState($_SESSION['sq_local_cart_contents']['delivery_state']);
		}

		return;

	}//end _processInterface()


	/**
	* Create a "pending order" asset
	*
	* @return Object
	* @access private
	*/
	function _createPendingOrder()
	{
		// create the pending order
		$order_asset = $this->_createOrder($_SESSION['sq_local_cart_contents']['cart_contents']);

		// so everyone knows that there is an existing pending order and to ensure that we do not create new ones unnecessarily
		$_SESSION['sq_local_cart_contents']['pending_order_id'] = $order_asset->id;

		// Save initial order details here so orders can be tracked in case of the following events:
		// 1. Failure to contact the remote gateway
		// 2. Failure to return from the remote gateway with "success"
		//
		// This ensures that disputed orders can be manually reconciled between Matrix and gateway records if required

		$delivery_method = $this->_getActiveDeliveryMethod();
		$this->_saveOrderSummary($order_asset, $delivery_method);
		$this->_logOrderAction(translate('Order and customer details saved in Pending Order'), $order_asset);


		return $order_asset;

	}//end _createPendingOrder()


	/**
	* Calls the delivery method and asks it to process our order
	*
	* @param object &$delivery_method	The delivery method asset passed by reference
	*
	* @return void
	* @access private
	*/
	function _processOrder(&$delivery_method)
	{
		// check whether we're editing an old order or completing an existing one
		if (isset($_SESSION['sq_local_cart_contents']['order_id'])) {
			$order_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($_SESSION['sq_local_cart_contents']['order_id']);

			$this->_logOrderAction(translate('Processing of existing order commencing'), $order_asset);

		}

		//prepare the addresses XML to write to the Ecommerce Order asset
		$payment_gateway_xml = '';

		// does this order need to be reviewed, i.e. placed in the Orders To Review folder
		$review_order = FALSE;

		if (empty($order_asset)) {
			// have we processed this order with a payment gateway yet?
			$payment_response = array_get_index($_SESSION, 'SQ_ECOM_RESPONSE', FALSE);

			// check quantities before completing this order
			// don't check here if we have just paid
			if (!$payment_response && $this->attr('enforce_quantities')) {
				// do we have enough products to complete an order?
				if (!$this->_updateQuantities(FALSE)) {
					$this->print_context = 'failure';
					return;
				} else {
					$this->insufficient_quantities = FALSE;
				}
			}

			if (!$payment_response) {

				// If the user have selected the gateway
				$selected_gateway = trim(array_get_index($_REQUEST, $this->getPrefix().'_selected_gw', ''));

				// process the payment gateway if one is present and if we haven't done so already
				$payment_gateway_attr   =& $delivery_method->getAttribute('payment_gateways', TRUE);
				$payment_gateway_id     = NULL;
				// Only try to find payment gateway if this delivery method support payment gateway, some methods might not.
				if (!is_null($payment_gateway_attr)) {
					$payment_gateway_ids = $delivery_method->attr('payment_gateways');
					if (!empty($payment_gateway_ids)) {
						// Make sure the supplied gateway is the valid
						if (!empty($selected_gateway) && in_array($selected_gateway, $payment_gateway_ids)) {
							$payment_gateway_id = $selected_gateway;
						} else {
							// No valid gateway selected. Get the first gateway from configured list
							$payment_gateway_id = $payment_gateway_ids[0];
						}
					}
				}//end if

				if (!empty($payment_gateway_id)) {
					// get cart total cost
					$cart = new Ecommerce_Cart_Processor();
					$cart->setCartContainer($_SESSION['sq_local_cart_contents']['cart_contents']);
					// whether we are removing tax from this order
					$remove_tax = isset($_SESSION['sq_region_specific']['remove_tax']) ? $_SESSION['sq_region_specific']['remove_tax'] : FALSE;
					$total_cost = $cart->getGrandTotalExDelivery($this->attr('taxation_rate')/100, $remove_tax, $this->getFlatCharges());

					$delivery_tax = 0;
					$delivery_cost = $cart->getTotalDeliveryCosts($total_cost, $this->attr('taxation_rate')/100, $this->attr('apply_tax_on_delivery'), $this->attr('delivery_includes_tax'), $delivery_tax);
					if (!$remove_tax) {
						$delivery_cost += $delivery_tax;
					}

					// Substract any promotional discounts
					$total_discount = $cart->getTotalDiscount();
					$grand_total = $total_cost + $delivery_cost - $total_discount;
					if ($grand_total < 0) {
						$grand_total = 0;
					}

					// For '0' amount the payment gateway will mark the order as 'has no amount' and then redirect back to the "success" url
					// without setting the response 'SQ_ECOM_RESPONSE'. So basically we will get back here again for '0' order amount
					if ($grand_total != 0 || array_get_index($_REQUEST, $this->getPrefix().'_action', '') != 'payment_gateway_successful') {
						// customer pays here!
						$this->_logOrderAction(translate('Payment Gateway page displayed'));

						$delivery_method->processPaymentGateway(round($grand_total, $this->attr('float_precision')), $payment_gateway_id, $this->id);
						return;
					}

				} else if ($delivery_method->attr('gateway_required')){
					$this->_tmp['delivery_method_errors'][] = translate('No Payment Gateway specified for the Delivery Method');

					$this->print_context = 'failure';
					return;
				}
			}

			// we only come here if customer has just paid for this order
			// complete the order whatever happens and flag if quantity was incorrect in the end
			// e.g. customer pays for order, in the meantime quantity changes, by the time payment goes through, the quantity may be incorrect
			// but we don't want the order to fail because the customer has already paid

			// unset this because the payment experience was just completed for this session
			unset($_SESSION['SQ_ECOM_RESPONSE']);

			// check quantities
			if ($this->attr('enforce_quantities')) {
				if (!$this->_updateQuantities(FALSE)) {
					// product quantity was negative
					// set keyword replacement message for email(s) to be sent
					ob_start();
					foreach ($this->insufficient_quantity_assets as $quantity => $asset_name) {
						echo $asset_name.":\t -> \t".abs($quantity)."\n";
					}
					$neg_quantity_list = ob_get_clean();
					$review_order = TRUE;
					// the order has to go through because it has been paid for
					$this->print_context = 'receipt';

					$this->_logOrderAction(translate('[Review Order] Insufficient product quantities but processed by Payment Gateway'));

				}
			}

			// the order is completed so move it into the correct folder
			$pending_order_id = array_get_index($_SESSION['sq_local_cart_contents'], 'pending_order_id', NULL);
			$dest_folder_name = $review_order ? 'Orders To Review' : 'Completed Orders';
			if (!is_null($pending_order_id)) {
				$order_asset = $this->_moveOrder($pending_order_id, $review_order);

				$this->_logOrderAction(sprintf(translate('Ecommerce order asset moved to \'%s\' folder'), $dest_folder_name), $order_asset);


				// Record the reference number returned from the Gateway
				if (isset($_SESSION['SQ_ECOM_REF_NO']) && !empty($_SESSION['SQ_ECOM_REF_NO'])) {
					$order_asset->setAttrValue('ecom_ref_no', $_SESSION['SQ_ECOM_REF_NO']);
					$this->_logOrderAction(translate('SecurePay transaction ID recorded'), $order_asset);

				} else {
					$this->_logOrderAction(translate('SecurePay transaction ID was NOT recorded'), $order_asset);

				}

				//customer has just paid for this order
				if ($payment_response) {
					//set ecommerce order's details
					$order_asset->setAttrValue('transaction_id', array_get_index($payment_response, 'TRANSACTION', ''));
					$order_asset->setAttrValue('billing_name', array_get_index($payment_response, 'BILLING_NAME', ''));
					$order_asset->setAttrValue('billing_addr', array_get_index($payment_response, 'BILLING_ADDR', ''));
					$order_asset->setAttrValue('delivery_name', array_get_index($payment_response, 'DELIVERY_NAME', ''));
					$order_asset->setAttrValue('delivery_addr', array_get_index($payment_response, 'DELIVERY_ADDR', ''));
					$xml = '<RESPONSE>'.array_to_xml($payment_response).'</RESPONSE>';
					$order_asset->setAttrValue('transaction_response', $xml);
					$payment_gateway_xml = array_get_index($payment_response, 'PAYMENT_GATEWAY_XML', '');
				}

			}

			if (!isset($order_asset) || is_null($order_asset)) {
				$this->_tmp['delivery_method_errors'][] = sprintf(translate('Unable to move ecommerce order asset to folder: "%s"'), $dest_folder_name);

				$this->print_context = 'failure';
				return;
			} else {
				// order moved and the pending order is no more
				unset($_SESSION['sq_local_cart_contents']['pending_order_id']);
			}

		} else if ($_SESSION['sq_local_cart_contents']['editing_order']) {
			// editing an existing order, so we do not process it via the payment gateway
			$this->_logOrderAction(translate('Edited ecommerce order asset not processed by Payment Gateway'), $order_asset);

		}

		$delivery_method->setState($_SESSION['sq_local_cart_contents']['delivery_state']);

		// get the arguments for customised receipts if set
		$format_bodycopy = NULL;
		$contents_bodycopy = NULL;
		if ($this->attr('customise_receipt')) {
			$format_bodycopy = $this->getBodycopy('receipt_format', TRUE);
			$contents_bodycopy = $this->getBodycopy('receipt_contents', TRUE);
		}

		$this->_logOrderAction(translate('Delivery method commencing processing'), $order_asset);


		// process the delivery and provide the tax name and tax rate
		$delivery_processing_status = $delivery_method->processDelivery($order_asset, $contents_bodycopy, $format_bodycopy, $this->attr('currency_symbol'), $this->attr('float_precision'), $this->attr('taxation_name'), $this->attr('taxation_rate')/100.0, $this->getFlatCharges(), isset($neg_quantity_list) ? $neg_quantity_list : '', isset($neg_quantity_list) ? $delivery_method->attr('negative_quantity_message') : '', $this->id);

		$this->_logOrderAction(translate('Delivery method processing completed'), $order_asset);


		$_SESSION['sq_local_cart_contents']['delivery_state'] = $delivery_method->getState();

		$this->_saveOrderSummary($order_asset, $delivery_method, $payment_gateway_xml, $payment_response);

		// delivery failed
		if (!$delivery_processing_status) {
			$order_asset->setAttrValue('status', SQ_ECOM_ORDER_STATUS_FAILED);
			$order_asset->setAttrValue('status_message', translate('Failed'));


			$this->_tmp['failure_summary'] = $delivery_method->getFailureSummary();
			$this->print_context = 'failure';
		} else {
			// delivery succeeded and we can update product quantities
			if ($this->attr('enforce_quantities')) {
				$this->_updateQuantities();

				$this->_logOrderAction(translate('Product quantities updated'), $order_asset);

			}
		}

		// Save attributes, LOCKING
		$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
		if (!$order_asset->saveAttributes()) {
			$this->print_context = 'failure';
		}
		$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
		if ($this->print_context !== 'failure') {

			// order processing successful!
			$this->print_context = 'receipt';
			$_SESSION['sq_local_cart_contents']['success_order_id'] = $order_asset->id;
			$this->_tmp['delete_all_orders_after_printing'] = TRUE;
			$GLOBALS['SQ_SYSTEM']->broadcastTriggerEvent('trigger_event_ecommerce_order_submitted', $order_asset);
		}

		// Anything else that needs to be done after the order is processed
		$this->finishCheckout($order_asset);

	}//end _processOrder()


	/**
	* Validates or updates the quantities of items stored in the current cart
	*
	* @param boolean	$update	Whether to update the product quantity values
	*
	* @return boolean
	* @access public
	*/
	function _updateQuantities($update=TRUE)
	{
		$local_cart_contents = $_SESSION['sq_local_cart_contents']['cart_contents'];
		$ok = TRUE;
		foreach ($local_cart_contents as $assetid => $info) {
			if ($GLOBALS['SQ_SYSTEM']->am->assetExists($assetid)) {
				// determine which to use: metadata field or attribute name
				$source = $this->attr('quantity_selector');
				switch ($source) {
					case 'metadata':
						$quantity_field = $this->attr('quantity_field');
						if (!empty($quantity_field)) {
							$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
							$current_quantity = $mm->getMetadataValueByAssetid($assetid, $quantity_field);
							$new_quantity = $current_quantity - $info['quantity'];
							if ($new_quantity < 0) {
								// we cannot put this order through because there are not enough items
								$this->insufficient_quantities = TRUE;
								$ok = FALSE;
								// add name of asset to error list for displaying on frontend
								$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
								$this->insufficient_quantity_assets[$new_quantity] = $asset->name;
							}
							if ($update) {
								$metadata = Array(
												$quantity_field	=> Array(
																	Array(
																		'value'	=> $new_quantity,
																	),
																   ),
											);
								$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
								if (!$mm->setMetadata($assetid, $metadata) || !$mm->generateContentFile($assetid)) {
									trigger_localised_error('ECOM0024', sprintf(translate('Unable to update product quantity information for asset with Id: #%s'), $assetid), E_USER_WARNING);
									$this->print_context = 'failure';
								}
								$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
							}
						}
					break;

					case 'attribute':
						$quantity_attribute = $this->attr('quantity_attribute');
						if (!empty($quantity_attribute)) {
							$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
							$current_quantity = $asset->attr($quantity_attribute);
							$new_quantity = $current_quantity - $info['quantity'];
							if ($new_quantity < 0) {
								// we cannot put this order through because there are not enough items
								$this->insufficient_quantities = TRUE;
								$ok = FALSE;
								// add name of asset to error list for displaying on frontend
								$this->insufficient_quantity_assets[$new_quantity] = $asset->name;
							}
							if ($update) {
								$asset->setAttrValue($quantity_attribute, $new_quantity);
								$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
								if (!$asset->saveAttributes()) {
									trigger_localised_error('ECOM0024', sprintf(translate('Unable to update product quantity information for asset with Id: #%s'), $assetid), E_USER_WARNING);
									$this->print_context = 'failure';
								}
								$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
							}
						}
					break;
				}//end switch
			}//end if
		}//end foreach

		return $ok;

	}//end _updateQuantities()


	/**
	* Print the context bodycopy
	*
	* Provided that the context has already been determined, this function simply find an appropriate
	* bodycopy and prints it, providing it with keywords
	*
	* @return void
	* @access public
	*/
	function _printContext()
	{
		$bodycopy_folder_link  = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'folder', TRUE, 'bodycopies');
		if (empty($bodycopy_folder_link)) {
			trigger_localised_error('ECOM0017', translate('Ecommerce Checkout Bodycopy Folder is missing'), E_USER_WARNING);
			return;
		}

		$content_bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLink($bodycopy_folder_link['minorid'], SQ_LINK_TYPE_2, 'bodycopy', TRUE, $this->print_context);
		$content_bodycopy = $GLOBALS['SQ_SYSTEM']->am->getAsset($content_bodycopy_link['minorid'], $content_bodycopy_link['minor_type_code']);
		if (is_null($content_bodycopy)) {
			trigger_localised_error('ECOM0018', translate('Unable to create Ecommerce Order'), E_USER_WARNING);
			return;
		}

		require_once SQ_FUDGE_PATH.'/general/text.inc';
		$keywords = $content_bodycopy->getKeywords();
		$replacements = Array();
		foreach ($keywords as $keyword) {
			$replacements[$keyword] = $this->getKeywordReplacement($keyword);
		}

		$form_req = $this->_isFormRequired();

		if ($form_req) {

			$active_delivery_method_id = $this->_getActiveDeliveryMethodId();

			if (is_null($active_delivery_method_id )) {
				$delivery_ids = $this->_getDeliveryAssetIds();
				$active_delivery_method_id = array_shift($delivery_ids);
			}

			if ($active_delivery_method_id) {
				$delivery_method = $GLOBALS['SQ_SYSTEM']->am->getAsset($active_delivery_method_id);
				$delivery_form = $delivery_method ? $delivery_method->getForm() : NULL;

				$onsubmit = $delivery_form && $delivery_form->attr('client_side') ? 'onSubmit="return beforeSubmit_'.$delivery_form->getPrefix().'(this)"' : '';
			} else {
				$onsubmit = '';
			}

			echo '<form action="'.current_url(TRUE, TRUE, FALSE).'" method="post" '.$onsubmit.'>';
		}

		$content_bodycopy->setKeywordReplacements($replacements);
		$content_bodycopy->printBody();

		if ($form_req) echo '</form>';

	}//end _printContext()


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* Create bopycopy and folder assets when they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	function _createAdditional(&$link)
	{
		if (!parent::_createAdditional($link)) return FALSE;

		$success = TRUE;

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('folder');

		$sub_assets = Array(
						'bodycopies'		=> 'folder',
						'pending_orders'	=> 'folder',
						'completed_orders'	=> 'folder',
						'orders_to_review'	=> 'folder',
					  );

		$bodycopy_folder = NULL;
		foreach ($sub_assets as $link_value => $type) {
			$asset = new $type();
			$copy_link = Array(
							'asset'			=> &$this,
							'value'			=> $link_value,
							'link_type'		=> SQ_LINK_TYPE_2,
							'is_dependant'	=> 1,
							'is_exclusive'	=> 1,
						 );

			$asset->setAttrValue('name', ucwords(str_replace('_',' ', $link_value)));
			if (!$asset->create($copy_link)) $success = FALSE;

			if ($link_value == 'bodycopies') {
				$bodycopy_folder = $asset;
			}

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			unset($asset);

			if (!$success) break;
		}

		if ($success) {
			// now create context bodycopies
			$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');
			foreach ($this->_getAvailableContexts() as $context_link_value => $context_bodycopy_name) {
				$asset = new Bodycopy();

				$copy_link = Array(
								'asset'			=> &$bodycopy_folder,
								'value'			=> $context_link_value,
								'link_type'		=> SQ_LINK_TYPE_2,
								'is_dependant'	=> 1,
								'is_exclusive'	=> 1,
							 );

				$asset->setAttrValue('name', $context_bodycopy_name);

				$success = $asset->create($copy_link);
				unset($asset);
				if (!$success) break;
			}
		}

		return $success;

	}//end _createAdditional()


	/**
	* Create a pending ecommerce order
	*
	* @param array	$cart_items	Information used to create the initial link
	*
	* @return object
	* @access private
	*/
	function _createOrder($cart_items)
	{
		// error flag
		$success = TRUE;

		$am = $GLOBALS['SQ_SYSTEM']->am;

		// create ecommerce order
		$order_folder_link = $am->getLink($this->id, SQ_LINK_TYPE_2, 'folder', TRUE, 'pending_orders');
		if (empty($order_folder_link)) return NULL;
		$order_folder_id = $order_folder_link['minorid'];

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// set runlevel
		$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);

		$orders_folder = $am->getAsset($order_folder_id);
		$ecommerce_order = new Ecommerce_Order();

		$order_link = Array(
						'asset'		=> &$orders_folder,
						'link_type'	=> SQ_LINK_TYPE_2,
					  );

		if (!$ecommerce_order->create($order_link)) {
			$success = FALSE;
		}

		// add products from the session (cart) to ecommerce order
		if ($success && !empty($cart_items)) {
			$ecommerce_order->setAttrValue('products', $cart_items);

			$this->_logOrderAction(translate('Pending ecommerce order asset created'), $ecommerce_order);


		} else {
			$success = FALSE;
		}

		// notice link from Checkout to Ecommerce_Order
		if ($success && ($am->createAssetLink($this, $ecommerce_order, SQ_LINK_NOTICE, 'checkout') == 0)) {
			$success = FALSE;
		}

		// set order status and delivery id
		if ($success) {
			$ecommerce_order->setAttrValue('status', SQ_ECOM_ORDER_STATUS_PROCESSING);
			$ecommerce_order->setAttrValue('status_message', translate('Processing'));

			$ecommerce_order->setAttrValue('delivery_id', $this->_getActiveDeliveryMethodId());
		}

		// save attributes
		if (!$am->acquireLock($ecommerce_order->id, 'attributes')) {
			$success = FALSE;
		}
		if ($success) {
			if (!$ecommerce_order->saveAttributes()) {
				$success = FALSE;
			}
		}
		$am->releaseLock($ecommerce_order->id, 'attributes');

		// restore runlevel
		$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

		if (!$success) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return NULL;
		}
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return $ecommerce_order;

	}//end _createOrder()


	/**
	* Move and complete a pending ecommerce order
	*
	* @param int		$order_id	The assetid of the existing pending ecommerce order
	* @param boolean	$review		Whether this ecommerce order is to be placed in the review folder
	*
	* @return object
	* @access private
	*/
	function _moveOrder($order_id, $review)
	{
		// error flag
		$success = TRUE;

		$am = $GLOBALS['SQ_SYSTEM']->am;

		if (empty($order_id)) return NULL;
		$ecommerce_order = $am->getAsset($order_id);

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// set runlevel
		$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);

		// get linkid of the ecommerce order to move
		$pending_orders_link = $am->getLink($this->id, SQ_LINK_TYPE_2, 'folder', TRUE, 'pending_orders');
		$order_link_info = $am->getLinkByAsset($pending_orders_link['minorid'], $order_id);

		// get destination folder assetid
		$dest_link_value = $review ? 'orders_to_review' : 'completed_orders';
		$order_folder_link = $am->getLink($this->id, SQ_LINK_TYPE_2, 'folder', TRUE, $dest_link_value);

		// now move the ecommerce order to its final resting place
		if (!$am->moveLink($order_link_info['linkid'], $order_folder_link['minorid'], SQ_LINK_TYPE_2, 0)) {
			$success = FALSE;
		}

		// restore runlevel
		$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

		if (!$success) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return NULL;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return $ecommerce_order;

	}//end _moveOrder()


	/**
	* Get the asset IDs for all the linked delivery methods
	*
	* @return array
	* @access private
	*/
	function _getDeliveryAssetIds()
	{
		if (!isset($this->_tmp['delivery_ids'])) {
			$this->_tmp['delivery_ids'] = Array();

			$delivery_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_NOTICE, '', FALSE, 'major', 'delivery_method');
			foreach ($delivery_links as $one_link) {
				$this->_tmp['delivery_ids'][] = $one_link['minorid'];
			}
		}

		return $this->_tmp['delivery_ids'];

	}//end _getDeliveryAssetIds()


	/**
	* Check if the field is already registered
	*
	* @param string	$field_name	Name of the form field
	*
	* @return boolean
	* @access private
	*/
	function _fieldRegistered($field_name)
	{
		return in_array($field_name, $this->_tmp['registered_form_fields']);

	}//end _fieldRegistered()


	/**
	* Register a form field name
	*
	* This helps determine whether the output should be wrapped in a form
	*
	* @param string	$field_name	Name of the form field
	*
	* @return void
	* @access private
	*/
	function _registerFormField($field_name)
	{
		if (!empty($field_name)) {
			$this->_tmp['registered_form_fields'][] = $field_name;
			$this->_registerFormRequirement();
		}

	}//end _registerFormField()


	/**
	* Register Form Requirement
	*
	* If an element of the asset's interface requires a form, this will register that requirement
	*
	* @return void
	* @access private
	*/
	function _registerFormRequirement()
	{
		$this->_tmp['form_required'] = TRUE;

	}//end _registerFormRequirement()


	/**
	* Is Form Required?
	*
	* This function can reply to the question whether the form tags are required in asset's interface
	* Form is required when _registerFormRequirement() was called
	*
	* @return boolean
	* @access private
	*/
	function _isFormRequired()
	{
		return array_get_index($this->_tmp, 'form_required', FALSE);

	}//end _isFormRequired()


	/**
	* Get The currently active delivery method asset
	*
	* Looks at the current state of the asset to determine the active delivery method asset
	*
	* @return object
	* @access public
	*/
	function _getActiveDeliveryMethod()
	{
		$delivery_method = NULL;

		$delivery_method_id = $this->_getActiveDeliveryMethodId();
		if (!is_null($delivery_method_id)) {
			$delivery_method = $GLOBALS['SQ_SYSTEM']->am->getAsset($delivery_method_id);
			$delivery_method_state = array_get_index($_SESSION['sq_local_cart_contents'], 'delivery_state', Array());
			$delivery_method->setState($delivery_method_state);
		}

		return $delivery_method;

	}//end _getActiveDeliveryMethod()


	/**
	* Get Active Delivery Method Asset ID
	*
	* Looks at the current state of the asset to determine the active delivery method asset id
	*
	* @return int
	* @access public
	*/
	function _getActiveDeliveryMethodId()
	{
		return array_get_index($_SESSION['sq_local_cart_contents'], 'delivery_id');

	}//end _getActiveDeliveryMethodId()


	/**
	* Return a receipt for the product section
	*
	* @return string
	* @access public
	*/
	function _getProductReceipt()
	{
		return;

	}//end _getProductReceipt()


	/**
	* It returns an Array of the Flat Charges Array ('name of the charge' => 'value of the charge')
	*
	* @return array
	* @access public
	*/
	function getFlatCharges()
	{
		$flat_charges = $this->attr('flat_charge_assetid');
		$flat_charge_array = Array();
		$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);

		foreach ($flat_charges as $charge_id) {
			$tmp_charge = $GLOBALS['SQ_SYSTEM']->am->getAsset($charge_id);
			$price = $tmp_charge->attr('price');
			if ($price <= 0) $price = 0;

			$flat_charge_array = array_merge($flat_charge_array, Array($tmp_charge->name => $price));
		}

		$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
		return $flat_charge_array;

	}//end getFlatCharges()


	/**
	* Return a cart summary for the ecommer cart
	*
	* @param boolean	$html	if it's true, the summary string is wrapped by <pre> tags
	*
	* @return string
	* @access private
	*/
	function _getCartSummary($html=TRUE)
	{
		$cart = new Ecommerce_Cart_Processor();
		$cart->setCartContainer($_SESSION['sq_local_cart_contents']['cart_contents']);

		ob_start();
			if ($html) echo '<pre>';

			// if products are taxed, remove tax from final amount
			$remove_tax = isset($_SESSION['sq_region_specific']['remove_tax']) ? $_SESSION['sq_region_specific']['remove_tax'] : FALSE;
			echo $cart->printReceipt($this->attr('taxation_name'), ($this->attr('taxation_rate')/100.0), $this->getFlatCharges(), $remove_tax, $this->attr('currency_symbol'), $this->attr('float_precision'), $this->attr('refund_info_text'), $this->attr('apply_tax_on_delivery'), $this->attr('delivery_includes_tax'));

			if ($html) echo '</pre>';
		$cart_summary = ob_get_clean();
		unset($cart);
		return  $cart_summary;

	}//end _getCartSummary()


	/**
	* Get the XML representation of the cart to store in the order_xml attribute of Ecommerce Order asset
	*
	* @return string
	* @access private
	*/
	function _getCartXml()
	{
		$cart = new Ecommerce_Cart_Processor();
		$cart->setCartContainer($_SESSION['sq_local_cart_contents']['cart_contents']);

		// if products are taxed, remove tax from final amount
		$remove_tax = isset($_SESSION['sq_region_specific']['remove_tax']) ? $_SESSION['sq_region_specific']['remove_tax'] : FALSE;
		$promo_code = $this->_getPromotionCodeUsed();

		return $cart->getCartXml($this->attr('taxation_name'), ($this->attr('taxation_rate')/100.0), $this->getFlatCharges(), $remove_tax, $this->attr('currency_symbol'), $this->attr('float_precision'), $this->attr('apply_tax_on_delivery'), $this->attr('delivery_includes_tax'), $promo_code);

	}//end _getCartXml()


	/**
	* Return a cart total of ecommerce cart
	*
	* @param boolean	$html	if it's true, the summary string is wrapped by <pre> tags
	*
	* @return string
	* @access private
	*/
	function _getCartTotal($html=TRUE)
	{
		$cart = new Ecommerce_Cart_Processor();
		$cart->setCartContainer($_SESSION['sq_local_cart_contents']['cart_contents']);

		ob_start();
			if ($html) echo '<pre>';

			// if products are taxed, remove tax from final amount
			$remove_tax = isset($_SESSION['sq_region_specific']['remove_tax']) ? $_SESSION['sq_region_specific']['remove_tax'] : FALSE;
			$total_cost = $cart->getGrandTotalExDelivery($this->attr('taxation_rate')/100, $remove_tax, $this->getFlatCharges());
			// Substract the discount
			$total_discount = $cart->getTotalDiscount();
			$total_cost -= $total_discount;
			if ($total_cost < 0) {
				$total_cost = 0;
			}

			// customer pays here!
			echo $total_cost;

			if ($html) echo '</pre>';
		$total_cost = ob_get_clean();
		unset($cart);
		return  $total_cost;

	}//end _getCartTotal()


	//// -- Keywords


	/**
	* Get bodycopy keyword replacement
	*
	* Provides a mechanism for providing keyword replacement functions by
	* looking up the function named 'getKeywordNameKeywordReplacement'
	* so an asset wishing to add support for 'best_ever' custom keyword
	* should implement a function getBestEverKeywordReplacement()
	* which returns a replacement string
	*
	* @param string	$keyword	Keyword to find a replacement for
	*
	* @return string
	* @access public
	*/
	function getKeywordReplacement($keyword)
	{
		$keyword_replaced = FALSE;

		if (isset($_SESSION['sq_local_cart_contents']['editing_order']) && !empty($_SESSION['sq_local_cart_contents']['editing_order'])) {
			$orderid = $_SESSION['sq_local_cart_contents']['order_id'];
			$order = $GLOBALS['SQ_SYSTEM']->am->getAsset($orderid);

			if (!empty($order)) {
				if (strcmp('asset_assetid', $keyword) == 0) {
					$replacement = $order->id;
					$keyword_replaced = TRUE;
				} else if (0 === strpos($keyword, 'asset_attribute_')) {
					$attr_name = substr($keyword, strlen('asset_attribute_'));
					if (array_key_exists($attr_name, $order->vars)) {
						$attr = $order->getAttribute($attr_name);
						if (!$attr->is_admin) {
							$replacement = $this->attr($attr_name);
							$keyword_replaced = TRUE;
						}
					}
				} else {
					$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
					$dummy_array = Array($keyword);
					$mm_keyword = $mm->generateKeywordReplacements($order, $dummy_array, FALSE);
					if (isset($mm_keyword[$keyword])) {
						$replacement = $mm_keyword[$keyword];
						$keyword_replaced = TRUE;
					}
				}
			}//end if (!empty($order))
		}//end if

		if (!$keyword_replaced) {
			// Keyword replacement for "Confirm Order" button for the specified gateway
			if (strpos($keyword, 'confirm_order_button_gateway_') !== FALSE) {
				$replacement = '';
				$gateway_id = trim(substr($keyword, 29));
				$replacement = $this->printGatewayCheckoutButton($gateway_id);
			} else {
				$replacement = parent::getKeywordReplacement($keyword);
			}
		}

		return $replacement;

	}//end getKeywordReplacement()


	/**
	* Add valid keywords for this asset to an array of keywords when asked
	*
	* @param object	&$broadcaster	$asset	the asset that triggered the event
	* @param array	$vars			the vars that get submitted by the broadcaster we add keywords to the $vars['keywords'] array
	*
	* @return boolean
	* @access private
	*/
	function onRequestKeywords(&$broadcaster, $vars=Array())
	{
		$keywords = Array();

		if (!is_array($vars['keywords'])) {
			$vars['keywords'] = Array();
		}

		$bodycopy_folder_link  = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'folder', TRUE, 'bodycopies');
		// not our bodycopy, just bail out
		if (empty($bodycopy_folder_link)) {
			return;
		} else {
			$bodycopy_folder_id = $bodycopy_folder_link['minorid'];
		}

		$customise_receipt = $this->attr('customise_receipt');

		$broadcaster_parents = $GLOBALS['SQ_SYSTEM']->am->getParents($broadcaster->id, 'bodycopy', TRUE);
		foreach (array_keys($broadcaster_parents) as $bodycopy_id) {
			// get the bodycopy context for bodycopies linked underneath the Bodycopies folder
			$content_bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($bodycopy_folder_id, $bodycopy_id);
			if (!empty($content_bodycopy_link) && !empty($content_bodycopy_link['value'])) {
				$bodycopy_context = $content_bodycopy_link['value'];
				break;
			}

			// get the bodycopy context for bodycopies linked underneath the Customised Receipt Bodycopies folder
			if ($customise_receipt) {
				$customised_receipt_folder_link = $GLOBALS['SQ_SYSTEM']->am->getLink($bodycopy_folder_id, SQ_LINK_TYPE_2, 'folder', TRUE, 'customised_receipt_folder');
				$content_bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($customised_receipt_folder_link['minorid'], $bodycopy_id);
				if (!empty($content_bodycopy_link) && !empty($content_bodycopy_link['value'])) {
					$bodycopy_context = $content_bodycopy_link['value'];
					break;
				}
			}
		}

		// check if the found bodycopy context is recognised
		$receipt_contexts = Array(
								'receipt_format',
								'receipt_contents',
							);

		$contexts = $this->_getAvailableContexts();
		if (empty($bodycopy_context) || !isset($contexts[$bodycopy_context]) && !in_array($bodycopy_context, $receipt_contexts)) {
			return;
		}

		require_once SQ_FUDGE_PATH.'/general/text.inc';

		$context_kw_fn = '_get'.ucwords_no_space($bodycopy_context).'ContextKeywords';
		if (method_exists($this, $context_kw_fn)) {
			$vars['keywords'] = array_merge($vars['keywords'], $this->$context_kw_fn());
		}

	}//end onRequestKeywords()


	/**
	* Get keywords for 'Content' context
	*
	* @return array
	* @access private
	*/
	function _getCheckoutContextKeywords()
	{
		if ($this->attr('customise_receipt')) {
			$keywords['customised_receipt'] = translate('Customised Receipt');

		}

		$keywords['cart_summary']				= translate('Cart Summary');

		$keywords['delivery_selector']			= translate('Delivery Method Selector');

		$keywords['delivery_interface'] 		= translate('Delivery Method Interface');

		$keywords['error_messages']				= translate('Error Messages');

		$keywords['go_to_confirmation_button']	= translate('Go To Confirmation Button');

		$keywords['items_count']				= translate('Number Of Items');

		$keywords['items_total_count']			= translate('Number Of Total Items');


		return $keywords;

	}//end _getCheckoutContextKeywords()


	/**
	* Get keywords for 'Confirmation' context
	*
	* @return array
	* @access private
	*/
	function _getConfirmationContextKeywords()
	{
		if ($this->attr('customise_receipt')) {
			$keywords['customised_receipt'] = translate('Customised Receipt');

		}

		$keywords['cart_summary']				= translate('Cart Summary');

		$keywords['confirmation_summary']		= translate('Confirmation Summary');

		$keywords['gateway_selector_dropdown']	= translate('Gateway Selector Dropdown Button');

		$keywords['gateway_selector_radio']		= translate('Gateway Selector Radio Button');

		$keywords['confirm_order_button']		= translate('Confirm Order Button');

		$keywords['confirm_order_button_gateway_X'] = translate('Confirm Order Button for Gateway assetid \'X\'');

		$keywords['return_to_checkout_button']	= translate('Return to Checkout Button');

		$keywords['items_count']				= translate('Number Of Items');

		$keywords['items_total_count']			= translate('Number Of Total Items');

		$keywords['promotional_code_field']			= translate('Promotional code text field');

		$keywords['apply_promotional_code_button']	= translate('Promotional code submit button');


		return $keywords;

	}//end _getConfirmationContextKeywords()


	/**
	* Get keywords for 'Receipt' context
	*
	* @return array
	* @access private
	*/
	function _getReceiptContextKeywords()
	{
		if ($this->attr('customise_receipt')) {
			$keywords['customised_receipt'] = translate('Customised Receipt');

		}

		$keywords['receipt_summary']	= translate('Receipt Summary');

		$keywords['cart_summary']		= translate('Cart Summary');

		$keywords['order_xml']			= translate('Order XML');


		return $keywords;

	}//end _getReceiptContextKeywords()


	/**
	* Get keywords for 'Failure' context
	*
	* @return array
	* @access private
	*/
	function _getFailureContextKeywords()
	{
		$keywords['failure_summary']						= translate('Failure Summary');

		if ($this->attr('enforce_quantities')) {
			$keywords['insufficient_quantity_message']			= translate('Insufficient Product Quantity Message');

		}
		$keywords['return_to_checkout_button']				= translate('Return to Checkout Button');


		return $keywords;

	}//end _getFailureContextKeywords()


	/**
	* Get keywords for 'Receipt Format' customised receipt context
	*
	* @return array
	* @access private
	*/
	function _getReceiptFormatContextKeywords()
	{
		$keywords['item_quantity_added']	= translate('Item Quantity Added To Cart');

		$keywords['item_price_incl_tax']	= translate('Individual Item Price Including Tax');

		$keywords['item_tax']				= translate('Individual Item Tax Amount');

		$keywords['item_price_excl_tax']		= translate('Individual Item Price Excluding Tax');

		$keywords['total_item_price_excl_tax']	= translate('Item Price Total Excluding Tax');

		$keywords['total_item_price']		= translate('Item Price Total');

		$keywords['total_item_tax']			= translate('Item Tax Total');

		return $keywords;

	}//end _getReceiptFormatContextKeywords()


	/**
	* Get keywords for 'Receipt Contents' customised receipt context
	*
	* @return array
	* @access private
	*/
	function _getReceiptContentsContextKeywords()
	{
		$keywords['receipt_listing']	= translate('Receipt Listing');


		$keywords['total_tax']			= translate('Total Tax');

		$keywords['tax_name']			= translate('Tax Name');


		$keywords['flat_fees_table']	= translate('All Flat Fees Listed');

		$keywords['flat_fees_total']	= translate('Total Flat Charges');

		$keywords['sub_total']			= translate('Sub Total (Grand Total Excluding Tax and Delivery)');

		$keywords['delivery_total']		= translate('Delivery Total');

		$keywords['grand_total']		= translate('Grand Total');

		$keywords['total_discount']		= translate('Total Discount');

		$keywords['grand_total_ex_discount'] = translate('Grand Total Before Discount');


		return $keywords;

	}//end _getReceiptContentsContextKeywords()

	/**
	* Print the "Confrim Order' button for the specifed gateway
	*
	* @param int $gateway_id
	*
	* @return string
	* @access public
	*/
	function printGatewayCheckoutButton($gateway_id)
	{
		$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($gateway_id, 'payment_gateway', FALSE, 'name');
		if (empty($asset_info[$gateway_id])) {
			return '';
		}

		// If the gateway has their own button image
		$gateway = $GLOBALS['SQ_SYSTEM']->am->getAsset($gateway_id);
		$image_url = '';
		if (method_exists($gateway, 'checkoutButtonImageUrl')) {
			$image_url = $gateway->checkoutButtonImageUrl();
		}
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($gateway);

		$field_name = $this->getPrefix().'_action[confirm_order]';
		$field_id = $this->getPrefix().'_action_confirm_order';
		$gateway_field = $this->getPrefix().'_selected_gw';

		$this->_registerFormField($field_name);

		// Set the gateway id based on the
		$onclick = 'onClick="document.getElementById(\''.$gateway_field.'\').value='.$gateway_id.'"';
		ob_start();
			// Print this field only once in the output
			if (!$this->_fieldRegistered($gateway_field)) {
				hidden_field($gateway_field, '', 'id="'.$gateway_field.'"');
				$this->_registerFormField($gateway_field);
			}
			if (empty($image_url)) {
				submit_button($field_name, $this->attr('confirm_order_button_text'), '', ' id="'.$field_id.'" '.$onclick);
			} else {
				echo '<input type="image" name="'. $field_name.'"  src="'.$image_url.'" id="'.$field_id.'" '.$onclick.'/>';
			}
		return ob_get_clean();

	}//end printGatewayCheckoutButton()


	//// -- Replacements


	/**
	* Get Keyword Replacement for 'delivery_selector' keyword
	*
	* @return string
	* @access public
	*/
	function getDeliverySelectorKeywordReplacement()
	{
		$delivery_ids = $this->_getDeliveryAssetIds();
		$multiple_asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($delivery_ids);

		$selection_contents = Array(); // Array('' => '-- '.translate('Select Delivery Method').' --');

		foreach ($multiple_asset_info as $assetid => $asset_info) {
			$selection_contents[$assetid] = $asset_info['name'];
		}

		$active_delivery_method_id = $this->_getActiveDeliveryMethodId();

		if (is_null($active_delivery_method_id )) {
			$active_delivery_method_id = array_shift($delivery_ids);
		}

		$this->_registerFormRequirement();
		ob_start();
			?>
			<script type="text/javascript">
				//<![CDATA[
				var current_method_id = <?php echo $active_delivery_method_id; ?> ;

				function handleDeliverySelectChange(field) {
					selected_id = field.options[field.selectedIndex].value;

					if ( current_method_id != null ) {
						document.getElementById('sq_delivery_method_' + current_method_id).style.display = 'none';
					}
					current_method_id = selected_id;
					document.getElementById('sq_delivery_method_' + current_method_id).style.display = 'block';

				}
				//]]>
			</script>

			<?php
			combo_box('sq_delivery_method_id', $selection_contents, FALSE, $active_delivery_method_id, 0, 'onchange="handleDeliverySelectChange(this)"');

		return ob_get_clean();

	}//end getDeliverySelectorKeywordReplacement()


	/**
	* Get Keyword Replacement for 'delivery_interface' keyword
	*
	* @return string
	* @access public
	*/
	function getDeliveryInterfaceKeywordReplacement()
	{

		$delivery_ids = $this->_getDeliveryAssetIds();
		$active_delivery_method = $this->_getActiveDeliveryMethod();

		if (is_null($active_delivery_method )) {
			$active_delivery_method_id = array_shift($delivery_ids);
		} else {
			$active_delivery_method_id = $active_delivery_method->id;
		}

		$this->_registerFormRequirement();
		ob_start();
		foreach ($this->_getDeliveryAssetIds() as $one_assetid) {
			$delivery_method = $GLOBALS['SQ_SYSTEM']->am->getAsset($one_assetid);

			if ($active_delivery_method_id == $one_assetid) {
				$display = 'block';
			} else {
				$display = 'none';
			}

			echo "\n".'<div id="sq_delivery_method_'.$one_assetid.'" style="display:'.$display.'" >'."\n".$delivery_method->getInputInterface()."\n</div>\n";

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($delivery_method);
		}

		return ob_get_clean();

	}//end getDeliveryInterfaceKeywordReplacement()


	/**
	* Get Keyword Replacement for 'gateway_selector_dropdown' button keyword
	*
	* @return string
	* @access public
	*/
	function getGatewaySelectorDropdownKeywordReplacement()
	{
		$delivery_method = $this->_getActiveDeliveryMethod();
		if (!$delivery_method->getAttribute('payment_gateways', TRUE)) {
			return '';
		}
		$payment_gateways = $delivery_method->attr('payment_gateways');
		if (empty($payment_gateways)) {
			return '';
		}
		$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($payment_gateways, 'payment_gateway', FALSE, 'name');

		$replacement = '';
		$field_name = $this->getPrefix().'_selected_gw';
		$this->_registerFormField($field_name);
		ob_start();
			combo_box($field_name, $asset_info);
			$replacement = ob_get_contents();
		ob_end_clean();

		return $replacement;

	}//end getGatewaySelectorRadioKeywordReplacement()


	/**
	* Get Keyword Replacement for 'gateway_selector_radio' button keyword
	*
	* @return string
	* @access public
	*/
	function getGatewaySelectorRadioKeywordReplacement()
	{
		$delivery_method = $this->_getActiveDeliveryMethod();
		if (!$delivery_method->getAttribute('payment_gateways', TRUE)) {
			return '';
		}
		$payment_gateways = $delivery_method->attr('payment_gateways');
		if (empty($payment_gateways)) {
			return '';
		}
		$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($payment_gateways, 'payment_gateway', FALSE, 'name');

		$replacement = '';
		$field_name = $this->getPrefix().'_selected_gw';
		$this->_registerFormField($field_name);
		ob_start();
			foreach($asset_info as $assetid => $name) {
				radio_button($field_name, $assetid);
				echo ' '.$name;
			}
			$replacement = ob_get_contents();
		ob_end_clean();

		return $replacement;

	}//end getGatewaySelectorRadioKeywordReplacement()


	/**
	* Get Keyword Replacement for 'confirm_order_button' keyword
	*
	* @return string
	* @access public
	*/
	function getConfirmOrderButtonKeywordReplacement()
	{
		$field_value = $this->getPrefix().'_action[confirm_order]';
		$field_id = $this->getPrefix().'_action_confirm_order';

		$this->_registerFormField($field_value);
		ob_start();
			submit_button($field_value, $this->attr('confirm_order_button_text'), '', ' id="'.$field_id.'"');
		return ob_get_clean();

	}//end getConfirmOrderButtonKeywordReplacement()


	/**
	* Get Keyword Replacement for 'return_to_checkout_button' keyword
	*
	* @return string
	* @access public
	*/
	function getReturnToCheckoutButtonKeywordReplacement()
	{
		$field_value = $this->getPrefix().'_action[return_to_checkout]';
		$field_id = $this->getPrefix().'_action_return_to_checkout';
		$this->_registerFormField($field_value);
		ob_start();
			submit_button($field_value, $this->attr('return_to_checkout_button_text'), '', ' id="'.$field_id.'"');
		return ob_get_clean();

	}//end getReturnToCheckoutButtonKeywordReplacement()


	/**
	* Get Keyword Replacement for 'go_to_confirmation_button' keyword
	*
	* @return string
	* @access public
	*/
	function getGoToConfirmationButtonKeywordReplacement()
	{
		$field_value = $this->getPrefix().'_action[go_to_confirmation]';
		$field_id = $this->getPrefix().'_action_go_to_confirmation';
		$this->_registerFormField($field_value);
		ob_start();
			submit_button($field_value, $this->attr('go_to_confirmation_button_text'), '', ' id="'.$field_id.'"');
		return ob_get_clean();

	}//end getGoToConfirmationButtonKeywordReplacement()


	/**
	* Get Keyword Replacement for 'get_checkout_summary' keyword
	*
	* Checkout Summary consists of
	*
	* @return string
	* @access public
	*/
	function getConfirmationSummaryKeywordReplacement()
	{
		ob_start();
			$delivery_method = $this->_getActiveDeliveryMethod();
			if (!is_null($delivery_method)) {
				echo $delivery_method->getConfirmationSummary();
			}
		return ob_get_clean();

	}//end getConfirmationSummaryKeywordReplacement()


	/**
	* Get Keyword Replacement for 'receipt_summary' keyword
	*
	* @return string
	* @access public
	*/
	function getReceiptSummaryKeywordReplacement()
	{
		ob_start();
			$delivery_method = $this->_getActiveDeliveryMethod();
			if (!is_null($delivery_method)) {
				echo $delivery_method->getReceiptSummary();
			}
		return ob_get_clean();

	}//end getReceiptSummaryKeywordReplacement()


	/**
	* Get Keyword Replacement for 'cart_summary' keyword
	*
	* @return string
	* @access public
	*/
	function getCartSummaryKeywordReplacement()
	{
		// essentially the cart's current receipt
		return $this->_getCartSummary(TRUE);

	}//end getCartSummaryKeywordReplacement()


	/**
	* Get order_xml keyword replacement for the success order
	*
	* @return string
	* @access public
	*/
	function getOrderXmlKeywordReplacement()
	{
		$order = NULL;
		if (isset($_SESSION['sq_local_cart_contents']['success_order_id'])) {
			$order = $GLOBALS['SQ_SYSTEM']->am->getAsset($_SESSION['sq_local_cart_contents']['success_order_id']);
		}

		return is_null($order)? '' : $order->attr('order_xml');

	}//end getOrderXmlKeywordReplacement()



	/**
	* Transaction Response keyword replacement
	*
	* @return string
	* @access private
	*/
	function getTransactionResponseKeywordReplacement()
	{
		$order = NULL;
		if (isset($_SESSION['sq_local_cart_contents']['success_order_id'])) {
			$order = $GLOBALS['SQ_SYSTEM']->am->getAsset($_SESSION['sq_local_cart_contents']['success_order_id']);
		}

		return is_null($order)? '' : $order->attr('transaction_response');

	}//end getTransactionResponseKeywordReplacement()

	
	/**
	* Get Keyword Replacement for 'error_messages' keyword
	*
	* @return string
	* @access public
	*/
	function getErrorMessagesKeywordReplacement()
	{
		ob_start();
			if (!empty($this->_tmp['delivery_method_errors'])) {
				echo '<ul>';
				foreach ($this->_tmp['delivery_method_errors'] as $message) {
					echo '<li>'.$message.'</li>';
				}
				echo '</ul>';
			}
		return ob_get_clean();

	}//end getErrorMessagesKeywordReplacement()


	/**
	* Get Keyword Replacement for 'failure_message' keyword
	*
	* @return string
	* @access public
	*/
	function getFailureSummaryKeywordReplacement()
	{
		ob_start();
			$delivery_method = $this->_getActiveDeliveryMethod();
			if (!is_null($delivery_method)) {
				echo $delivery_method->getFailureSummary();
			}
		return ob_get_clean();

	}//end getFailureSummaryKeywordReplacement()


	/**
	* Get Keyword Replacement for 'insufficient_quantity_message' keyword
	*
	* @return string
	* @access public
	*/
	function getInsufficientQuantityMessageKeywordReplacement()
	{
		if ($this->insufficient_quantities) {
			ob_start();
			echo $this->attr('quantity_message_checkout');
			echo '<ul>';
			foreach ($this->insufficient_quantity_assets as $asset_name) {
				echo '<li>';
				echo $asset_name;
				echo '</li>';
			}
			echo '</ul>';
			return ob_get_clean();
		}

	}//end getInsufficientQuantityMessageKeywordReplacement()


	/**
	* Get Keyword Replacement for 'items_count' keyword
	*
	* @return string
	* @access public
	*/
	function getItemsCountKeywordReplacement()
	{
		$cart = new Ecommerce_Cart_Processor();
		$cart->setCartContainer($_SESSION['sq_local_cart_contents']['cart_contents']);
		return $cart->getUniqueCount();

	}//end getItemsCountKeywordReplacement()


	/**
	* Get Keyword Replacement for 'items_total_count' keyword
	*
	* @return string
	* @access public
	*/
	function getItemsTotalCountKeywordReplacement()
	{
		$cart = new Ecommerce_Cart_Processor();
		$cart->setCartContainer($_SESSION['sq_local_cart_contents']['cart_contents']);
		return $cart->getCount();

	}//end getItemsTotalCountKeywordReplacement()


	/**
	* Get Keyword Replacement for 'customised_receipt' keyword
	*
	* @return string
	* @access public
	*/
	function getCustomisedReceiptKeywordReplacement()
	{
		if ($this->attr('customise_receipt')) {
			$cart = new Ecommerce_Cart_Processor();
			$cart->setCartContainer($_SESSION['sq_local_cart_contents']['cart_contents']);

			ob_start();

				// if products are taxed, remove tax from final amount
				$remove_tax = isset($_SESSION['sq_region_specific']['remove_tax']) ? $_SESSION['sq_region_specific']['remove_tax'] : FALSE;
				// get the customised receipt bodycopies
				$contents_bodycopy = $this->getBodycopy('receipt_contents', TRUE);
				$format_bodycopy = $this->getBodycopy('receipt_format', TRUE);
				echo $cart->printCustomisedReceipt($contents_bodycopy, $format_bodycopy, $this->attr('taxation_name'), ($this->attr('taxation_rate')/100.0), $this->getFlatCharges(), $remove_tax, $this->attr('currency_symbol'), $this->attr('float_precision'), $this->attr('apply_tax_on_delivery'), $this->attr('delivery_includes_tax'));

			$customised_receipt = ob_get_clean();
			unset($cart);

			return $customised_receipt;
		}

	}//end getCustomisedReceiptKeywordReplacement()


	/**
	* Get Keyword Replacement for promotional code field
	*
	* @return string
	* @access public
	*/
	function getPromotionalCodeFieldKeywordReplacement()
	{
		$field_id = $this->getPrefix().'_promotional_code';
		$size = $this->attr('promotional_code_field_size');
		if (empty($size)) {
			$size = 20;
		}

		$this->_registerFormField($field_id);
		ob_start();
			text_box($field_id, '', $size, '', FALSE, ' id="'.$field_id.'"');
		return ob_get_clean();

	}//end getPromotionalCodeFieldKeywordReplacement()


	/**
	* Get Promotion Code string used in the checkout, if any
	*
	* @return string|boolean
	* @access private
	*/
	function _getPromotionCodeUsed()
	{
		return array_get_index($_SESSION['sq_local_cart_contents'], 'promotion_code', FALSE);

	}//end _getPromotionCodeUsed()


	/**
	* Get Keyword Replacement for apply promotional code submit button
	*
	* @return string
	* @access public
	*/
	function getApplyPromotionalCodeButtonKeywordReplacement()
	{
		$field_value = $this->getPrefix().'_action[apply_promotional_code]';
		$field_id = $this->getPrefix().'_action_apply_promotional_code';
		$button_value = $this->attr('promotional_code_button_text');
		if (empty($button_value)) {
			$button_value = 'Apply Promotion';
		}

		$this->_registerFormField($field_value);
		ob_start();
			submit_button($field_value, $button_value, '', ' id="'.$field_id.'"');
		return ob_get_clean();

	}//end getApplyPromotionalCodeButtonKeywordReplacement()


	/**
	* Returns a reference to a bodycopy based of the name of the bodycopy
	*
	* @param string		$name			the name of the bodycopy
	* @param boolean	$receipt_folder	whether the bodycopy is inside the attached Customised Receipt Bodycopies folder
	*
	* @return object
	* @access public
	*/
	function getBodycopy($name='', $receipt_folder=FALSE)
	{
		$null = NULL;

		if (!$name) return $null;
		$am = $GLOBALS['SQ_SYSTEM']->am;

		$folder_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'folder', TRUE, 'bodycopies');
		if (empty($folder_link)) return $null;

		$folder = $am->getAsset($folder_link['minorid'], $folder_link['minor_type_code']);
		if (is_null($folder)) return $null;

		if ($receipt_folder) {
			$folder_link = $GLOBALS['SQ_SYSTEM']->am->getLink($folder->id, SQ_LINK_TYPE_2, 'folder', TRUE, 'customised_receipt_folder');
			if (empty($folder_link)) return $null;
			$folder = $am->getAsset($folder_link['minorid'], $folder_link['minor_type_code']);
			if (is_null($folder)) return $null;
		}

		$bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLink($folder->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, $name);
		if (empty($bodycopy_link)) return $null;

		$bodycopy = $am->getAsset($bodycopy_link['minorid'], $bodycopy_link['minor_type_code']);

		return $bodycopy;

	}//end getBodycopy()


	/**
	* Save attributes set with setAttrValue to the database
	*
	* @param boolean	$dont_run_updated	if TRUE this->_updated() won't be run
	*										(THIS IS ONLY EVER CALLED FROM Asset::create())
	*
	* @return boolean
	* @access public
	* @see setAttrValue()
	*/
	function saveAttributes($dont_run_updated=FALSE)
	{
		if (!parent::saveAttributes($dont_run_updated)) {
			return FALSE;
		}

		// create customised receipt bodycopies
		if ($this->id) {
			$customise_receipt = $this->attr('customise_receipt');
			if ($customise_receipt) {
				$bodycopies_folder_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'folder', TRUE, 'bodycopies');
				$customised_receipt_folder_link = $GLOBALS['SQ_SYSTEM']->am->getLink($bodycopies_folder_link['minorid'], SQ_LINK_TYPE_2, 'folder', TRUE, 'customised_receipt_folder');
				// create folder and bodycopies
				if (empty($customised_receipt_folder_link)) {
					$GLOBALS['SQ_SYSTEM']->am->includeAsset('folder');

					$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
					$db = $GLOBALS['SQ_SYSTEM']->db;

					$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

					// create a folder for the bodycopies
					$GLOBALS['SQ_SYSTEM']->am->includeAsset('folder');
					$folder = new Folder();
					$folder->setAttrValue('name', 'Customised Receipt Bodycopies');

					$bodycopies_folder = $GLOBALS['SQ_SYSTEM']->am->getAsset($bodycopies_folder_link['minorid']);

					$folder_link = Array('asset' => &$bodycopies_folder, 'link_type' => SQ_LINK_TYPE_2, 'value' => 'customised_receipt_folder', 'is_dependant' => 1, 'is_exclusive' => 1);

					if (!$folder->create($folder_link)) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
						return FALSE;
					}

					$bodycopies = Array(
									'Receipt Format',
									'Receipt Contents',
								  );
					foreach ($bodycopies as $name) {
						// create a link to the folder so we can put the bodycopies in it
						$copy_link = Array('asset' => &$folder, 'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1, 'value' => strtolower(str_replace(' ', '_', $name)));

						$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');
						$bodycopy = new Bodycopy();
						$bodycopy->setAttrValue('name', $name);

						if (!$bodycopy->create($copy_link)) {
							$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
							$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
							return FALSE;
						}
					}

					$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

				}
			}
		}

		return TRUE;

	}//end saveAttributes()


 	/**
	* Store initial product and customer information details against the Order
	*
	* @param object &$order_asset	The ECommerce_Order asset
	* @param object &$delivery_method	The Delivery Method asset used
	* @param string $payment_gateway_xml	An XML-formatted response from the Payment Gateway
	* @param array  $payment_response	Payment gateway response data
	*
	* @returns void
	* @access private
	*/
	private function _saveOrderSummary(ECommerce_Order &$order_asset, ECommerce_Delivery_Method &$delivery_method, $payment_gateway_xml='', $payment_response=Array())
	{
		// set the order summary
		$order_summary = 'Order ID: '.$order_asset->id;
		$order_summary .= $this->_getCartSummary();
		//get cart XML
		$cart_xml = $this->_getCartXml();

		$order_summary .= $delivery_method->getReceipt();
		//get delivery method XML
		$delivery_method_xml = $delivery_method->getDeliveryMethodXML();

		//get the default character set of Matrix System
		$default_char_set = SQ_CONF_DEFAULT_CHARACTER_SET;

		// Payment response info
		$payment_response_xml = '';
		if ($payment_response) {
			$payment_response_xml .= '  	<gateway_reference>'.array_get_index($payment_response, 'TRANSACTION', '').'</gateway_reference>'."\n";
			$payment_response_xml .= '  	<merchant_reference>'.array_get_index($_SESSION, 'SQ_ECOM_REF_NO', '').'</merchant_reference>'."\n";
			$payment_response_xml .= '  	<payment_date>'.array_get_index($payment_response, 'TIME', date('r')).'</payment_date>'."\n";
			$payment_response_xml .= '  	<gateway_assetid>'.array_get_index($_SESSION, 'SQ_ECOM_PAYMENT_GATEWAY_ASSETID', '').'</gateway_assetid>'."\n";
		}

		//create order xml
		$order_xml = '<'.'?xml version="1.0" encoding="'.$default_char_set.'"?'.">\n".
					 "<order>\n".
					 '	<id>'.$order_asset->id."</id>\n".
					 $payment_response_xml.
					 $cart_xml."\n".
					 $delivery_method_xml.$payment_gateway_xml."\n".
					 "</order>";

		// if there has been a negative quantity order placed (can only happen if payment gateway is used), append the message
		if (isset($neg_quantity_list)) {
			$order_summary .= '<br />'.$delivery_method->attr('negative_quantity_message');
			$order_summary .= $neg_quantity_list;
		}

		$total_cost = $this->_getCartTotal(FALSE);

		$order_asset->setAttrValue('summary',			$order_summary, FALSE);
		$order_asset->setAttrValue('order_xml',			$order_xml, FALSE);
		$order_asset->setAttrValue('delivery_id',		$_SESSION['sq_local_cart_contents']['delivery_id']);
		$order_asset->setAttrValue('delivery_state',	$_SESSION['sq_local_cart_contents']['delivery_state']);
		$order_asset->setAttrValue('order_grand_total', $total_cost);

	}//end _saveOrderSummary()


	/**
	* Stuff that needs to be done at the end of the checkout
	*
	* @param object $order_asset
	*
	* @return boolean
	* @access public
	*/
	public function finishCheckout(&$order_asset)
	{
		// Execute the checkout's completion actions configured for this checkout page
		$success = $this->_performCheckoutCompletionActions($order_asset);

		unset($_SESSION['SQ_ECOM_PAYMENT_GATEWAY_ASSETID']);

		return $success;

	}// end finishCheckout()


	/**
	* Executes the specifed actions configured, if any
	*
	* @param object $order_asset
	*
	* @return boolean
	* @accesss private
	*/
	private function _performCheckoutCompletionActions(&$order_asset)
	{
		$actions = $this->attr('actions');
		$success = TRUE;
		foreach($actions as $action) {
			$action_type = $action['type_code'];
			$GLOBALS['SQ_SYSTEM']->am->includeAsset($action_type);

			if ($action['active'] && call_user_func(Array($action_type, 'isValid'), $this, $action['settings'])) {
				$success &= call_user_func(Array($action_type, 'execute'), $this, $order_asset, $action['settings']);
			}
		}

		return $success;

	}//end _performCheckoutCompletionActions()


}//end class
?>
