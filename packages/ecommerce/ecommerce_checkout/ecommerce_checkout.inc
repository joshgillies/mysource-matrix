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
* $Id: ecommerce_checkout.inc,v 1.47 2006/12/07 05:26:28 bcaldwell Exp $
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
* @version $Revision: 1.47 $
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
	* Constructor
	*
	* @param int	assetid	Id of this asset
	*
	* @return void
	* @access public
	*/
	function Ecommerce_Checkout($assetid=0)
	{
		// initialise the temporary order container so as to avoid extra checking
		if (!isset($_SESSION['sq_local_cart_contents'])) {
			$_SESSION['sq_local_cart_contents'] = Array();
		}

		$this->Page($assetid);

	}//end constructor


	/**
	* Returns a list of available contexts
	*
	* @return array		available contexts
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
		$allowed_links[SQ_LINK_NOTICE]['ecommerce_delivery_method'] = Array('card' => 'M', 'exclusive' => false);
		$allowed_links[SQ_LINK_NOTICE]['ecommerce_order'] = Array('card' => 'M', 'exclusive' => false);

		return $allowed_links;

	}//end _getAllowedLinks()


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the short name or the full name
	*
	* @see Asset::_getName()
	* @access private
	*/
	function _getName($short_name=false)
	{
		return $short_name ? $this->attr('short_name') : $this->attr('name');

	}//end _getName()


	/**
	* Called by the design to print the body of this asset
	*
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
		if (array_get_index($this->_tmp, 'delete_all_orders_after_printing', false)) {

			if (!isset($_SESSION['sq_local_cart_contents']['editing_order'])) {
				// clear global cart
				$cart =& new Ecommerce_Cart_Processor();
				$cart->clearCart();
				unset($cart);
			}

			// clear local cart
			$_SESSION['sq_local_cart_contents'] = Array();
		}

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

	}//end printBody()


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
			$order =& $GLOBALS['SQ_SYSTEM']->am->getAsset($order_id);

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
					$_SESSION['sq_local_cart_contents']['editing_order'] = true;

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
			if (strcmp($this->print_context, 'checkout') == 0) {
				$global_cart =& new Ecommerce_Cart_Processor();

				// unset previous order id and delivery information
				unset($_SESSION['sq_local_cart_contents']['order_id']);
				unset($_SESSION['sq_local_cart_contents']['delivery_id']);
				unset($_SESSION['sq_local_cart_contents']['delivery_state']);
				unset($_SESSION['sq_local_cart_contents']['editing_order']);

				$local_cart_empty = empty($_SESSION['sq_local_cart_contents']['cart_contents']);

				$need_update = false;
				if ($local_cart_empty) {
					// if global is not empty, update
					if ($global_cart->getCount() != 0) {
						$need_update = true;
					}
				} else {
					// if global and local carts are different, update
					$local_cart =& new Ecommerce_Cart_Processor();
					$local_cart->setCartContainer($_SESSION['sq_local_cart_contents']['cart_contents']);

					if (strcmp($global_cart->hashCart(), $local_cart->hashCart()) != 0) {
						$need_update = true;
					}
				}

				// update the local cart
				if ($need_update) {
					$_SESSION['sq_local_cart_contents']['cart_contents'] = $global_cart->getCart();
				}

			}
			return;

		}

		$action = key($request_action);
		$success = true;
		switch ($action) {
			case 'go_to_confirmation':
				$delivery_method_id = array_get_index($_REQUEST, 'sq_delivery_method_id');

				// if we're already at confirmation, why are we going again?
				if (isset($_SESSION['sq_local_cart_contents']['delivery_state'])) {
					if ($_SESSION['sq_local_cart_contents']['delivery_state']['last_context'] == 'confirmation') {
						$this->print_context = 'confirmation';
						break;
					}
				}

				if (!empty($delivery_method_id) && !empty($_SESSION['sq_local_cart_contents']['cart_contents'])) {

					$delivery_method =& $GLOBALS['SQ_SYSTEM']->am->getAsset($delivery_method_id);
					if (empty($delivery_method)) {
						$this->print_context = 'checkout';
						return;
					}

					// delivery method state has cart contents
					$_SESSION['sq_local_cart_contents']['delivery_state']['cart_contents'] = $_SESSION['sq_local_cart_contents']['cart_contents'];
					$delivery_method->setState($_SESSION['sq_local_cart_contents']['delivery_state']);

					if (!$delivery_method->processInputInterface()){
						$this->_tmp['delivery_method_errors'] = $delivery_method->getErrors();
						$success = false;
					}


					// create a temporary order
					$_SESSION['sq_local_cart_contents']['delivery_id']		= $delivery_method_id;
					$_SESSION['sq_local_cart_contents']['delivery_state']	= $delivery_method->getState();
					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($delivery_method);
				}

				// if cart is empty, there's no point in checking out
				if ($success && empty($_SESSION['sq_local_cart_contents']['cart_contents'])) {
					$this->_tmp['delivery_method_errors'][] = 'The ecommerce cart is empty, cannot checkout';
					$success = false;
				}
				if ($success) {
					$this->print_context = 'confirmation';
				} else {
					$this->print_context = 'checkout';
				}

				if (isset($_SESSION['sq_local_cart_contents']['editing_order'])) {
					$_SESSION['sq_local_cart_contents']['editing_order'] = true;
				}

			break;

			case 'return_to_checkout':
				$this->print_context = 'checkout';
			break;

			case 'confirm_order':
				// components should be processed here
				// delivery processing
				$delivery_method =& $this->_getActiveDeliveryMethod();
				if (empty($delivery_method)){
					$this->_tmp['delivery_method_errors'][] = 'Delivery method not found during the confirmation';
					$this->print_context = 'failure';
					break;
				}
				// check whether we're editing an order or creating a new one
				if (isset($_SESSION['sq_local_cart_contents']['order_id'])) {
					$order_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($_SESSION['sq_local_cart_contents']['order_id']);
				}

				if (empty($order_asset)) {
					$order_asset =& $this->_createOrder($_SESSION['sq_local_cart_contents']['cart_contents']);
					if (is_null($order_asset)){
						$this->_tmp['delivery_method_errors'][] = 'Unable to create Ecommerce Order';
						$this->print_context = 'failure';
						break;
					}
				}

				$delivery_method->setState($_SESSION['sq_local_cart_contents']['delivery_state']);

				$delivery_processing_status = $delivery_method->processDelivery($order_asset);

				$_SESSION['sq_local_cart_contents']['delivery_state'] = $delivery_method->getState();

				// set the order summary
				$order_summary     = $this->_getCartSummary();
				$order_summary    .= $delivery_method->getReceipt();
				$internal_summary  = $delivery_method->getInternalReceipt();

				$order_asset->setAttrValue('summary',			$order_summary);
				$order_asset->setAttrValue('internal_summary',	$internal_summary);
				$order_asset->setAttrValue('delivery_id',		$_SESSION['sq_local_cart_contents']['delivery_id']);
				$order_asset->setAttrValue('delivery_state',	$_SESSION['sq_local_cart_contents']['delivery_state']);

				// delivery failed
				if (!$delivery_processing_status) {
					$order_asset->setAttrValue('status', SQ_ECOM_ORDER_STATUS_FAILED);
					$order_asset->setAttrValue('status_message', translate('ecom_order_status_failed'));

					$this->_tmp['failure_summary'] = $delivery_method->getFailureSummary();
					$this->print_context = 'failure';
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
					$this->_tmp['delete_all_orders_after_printing'] = true;
					$GLOBALS['SQ_SYSTEM']->broadcastTriggerEvent('trigger_event_ecommerce_order_submitted', $order_asset);
				}
			break;

		}

		// set the last context we saw to this one
		$_SESSION['sq_local_cart_contents']['delivery_state']['last_context'] = $this->print_context;
		if (isset($delivery_method)) {
			$delivery_method->setState($_SESSION['sq_local_cart_contents']['delivery_state']);
		}

		return;

	}//end _processInterface()


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
		$bodycopy_folder_link  = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'folder', true, 'bodycopies');
		if (empty($bodycopy_folder_link)) {
			trigger_localised_error('ECOM0017', E_USER_WARNING);
			return;
		}

		$content_bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLink($bodycopy_folder_link['minorid'], SQ_LINK_TYPE_2, 'bodycopy', true, $this->print_context);
		$content_bodycopy =& $GLOBALS['SQ_SYSTEM']->am->getAsset($content_bodycopy_link['minorid'], $content_bodycopy_link['minor_type_code']);
		if (is_null($content_bodycopy)) {
			trigger_localised_error('ECOM0018', E_USER_WARNING);
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
			echo '<form action="'.$_SERVER['PHP_SELF'].'" method="post">';
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
	* @access private
	* @return boolean
	*/
	function _createAdditional(&$link)
	{
		if (!parent::_createAdditional($link)) return false;

		$success = true;

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('folder');

		$sub_assets = Array(
						'bodycopies'	=> 'folder',
						'orders'		=> 'folder',
					  );

		$bodycopy_folder = null;
		foreach ($sub_assets as $link_value => $type) {
			$asset =& new $type();
			$copy_link = Array(
							'asset'			=> &$this,
							'value'			=> $link_value,
							'link_type'		=> SQ_LINK_TYPE_2,
							'is_dependant'	=> 1,
							'is_exclusive'	=> 1,
						 );

			$asset->setAttrValue('name', ucwords(str_replace('_',' ', $link_value)));
			if (!$asset->create($copy_link)) $success = false;

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
				$asset =& new Bodycopy();

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
	* Perform any additional processing required during the creation of this asset
	*
	* Create bopycopy and folder assets when they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @access private
	* @return Ecommerce_Order
	*/
	function &_createOrder($cart_items)
	{
		// error flag
		$success = true;

		$am =& $GLOBALS['SQ_SYSTEM']->am;

		// create ecommerce order under orders folder
		$order_folder_link  = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'folder', true, 'orders');
		if (empty($order_folder_link)) return null;
		$order_folder_id = $order_folder_link['minorid'];

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// set runlevel
		$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);

		$orders_folder =& $am->getAsset($order_folder_id);
		$ecommerce_order =& new Ecommerce_Order();

		$order_link = Array(
						'asset'		=> &$orders_folder,
						'link_type'	=> SQ_LINK_TYPE_2,
					  );

		if (!$ecommerce_order->create($order_link)) {
			$success = false;
		}

		// add products from the session(cart) to ecommerce order
		if ($success && !empty($cart_items)) {
			$ecommerce_order->setAttrValue('products', $cart_items);
		} else {
			$success = false;
		}

		// notice link from Checkout_Form to Ecommerce_Order
		if ($success && ($am->createAssetLink($this, $ecommerce_order, SQ_LINK_NOTICE, 'checkout') == 0)) {
			$success = false;
		}

		// set order status
		if ($success) {
			$ecommerce_order->setAttrValue('status', SQ_ECOM_ORDER_STATUS_PROCESSING);
			$ecommerce_order->setAttrValue('status_message', translate('ecom_order_status_processing'));
		}

		// save attributes
		if (!$am->acquireLock($ecommerce_order->id, 'attributes')) {
			$success = false;
		}
		if ($success) {
			if (!$ecommerce_order->saveAttributes()) {
				$success = false;
			}
		}
		$am->releaseLock($ecommerce_order->id, 'attributes');

		// restore runlevel
		$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

		if (!$success) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return null;
		}
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return $ecommerce_order;

	}//end _createOrder()


	/**
	* Get the asset IDs for all the linked delivery methods
	*
	* @return array	asset IDs
	* @access private
	*/
	function _getDeliveryAssetIds()
	{
		if (!isset($this->_tmp['delivery_ids'])) {
			$this->_tmp['delivery_ids'] = Array();

			$delivery_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_NOTICE, '', false, 'major', 'delivery_method');
			foreach ($delivery_links as $one_link) {
				$this->_tmp['delivery_ids'][] = $one_link['minorid'];
			}
		}

		return $this->_tmp['delivery_ids'];

	}//end _getDeliveryAssetIds()


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
		$this->_tmp['form_required'] = true;

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
		return array_get_index($this->_tmp, 'form_required', false);

	}//end _isFormRequired()


	/**
	* Get The currently active delivery method asset by reference
	*
	* Looks at the current state of the asset to determine the active delivery method asset
	*
	* @return Ecommerce_Delivery_Method
	* @access public
	*/
	function &_getActiveDeliveryMethod()
	{
		$delivery_method = null;

		$delivery_method_id = $this->_getActiveDeliveryMethodId();
		if (!is_null($delivery_method_id)) {
			$delivery_method =& $GLOBALS['SQ_SYSTEM']->am->getAsset($delivery_method_id);
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
	* @return Ecommerce_Delivery_Method
	* @access public
	*/
	function _getActiveDeliveryMethodId()
	{
		$delivery_method_id = null;

		// TODO: this does not seem to be coming through...
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
	* Return a cart summary for the ecommer cart
	*
	* @param $html	if it's true, the summary string is wrapped by <pre> tags
	*
	* @return string
	* @access private
	*/
	function _getCartSummary($html=true)
	{
		$cart =& new Ecommerce_Cart_Processor();
		$cart->setCartContainer($_SESSION['sq_local_cart_contents']['cart_contents']);
		ob_start();
			if ($html) echo '<pre>';
			echo $cart->printReceipt();
			if ($html) echo '</pre>';
		$cart_summary = ob_get_clean();

		unset($cart);
		return  $cart_summary;

	}//end _getCartSummary()


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
		if (empty($keyword)) return '';

		$replacement = "%$keyword%";

		$tmp_keyword = strtr($keyword, '_', ' ');
		$tmp_keyword = ucwords($tmp_keyword);
		$tmp_keyword = preg_replace('/\s+/', '', $tmp_keyword);
		$func_name = 'get'.$tmp_keyword.'KeywordReplacement';
		if (method_exists($this, $func_name)) {
			$replacement = $this->$func_name();
		} else {

			if (isset($_SESSION['sq_local_cart_contents']['editing_order']) && !empty($_SESSION['sq_local_cart_contents']['editing_order'])) {
				$orderid = $_SESSION['sq_local_cart_contents']['order_id'];
				$order =& $GLOBALS['SQ_SYSTEM']->am->getAsset($orderid);

				if (!empty($order)) {
					if (strcmp('asset_assetid', $keyword) == 0) {
						$replacement = $order->id;
					} else if (0 === strpos($keyword, 'asset_attribute_')) {
						$attr_name = substr($keyword, strlen('asset_attribute_'));
						if (array_key_exists($attr_name, $order->vars)) {
							$attr = &$order->getAttribute($attr_name);
							if (!$attr->is_admin) {
								$replacement = $this->attr($attr_name);
							}
						}
					} else {
						$mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager();
						$dummy_array = Array($keyword);
						$mm_keyword = $mm->generateKeywordReplacements($order, $dummy_array, false);
						if (isset($mm_keyword[$keyword])) {
							$replacement = $mm_keyword[$keyword];
						}
					}
				} else {
					$replacement = '';
				}

			} else {
				$replacement = '';
			}
		}

		return $replacement;

	}//end getKeywordReplacement()


	/**
	* Add valid keywords for this asset to an array of keywords when asked
	*
	* @param &object Asset	$asset	the asset that triggered the event
	* @param Array			$vars	the vars that get submitted by the broadcaster
	*								we add keywords to the $vars['keywords'] array
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

		$bodycopy_folder_link  = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'folder', true, 'bodycopies');
		// not our bodycopy, just bail out
		if (empty($bodycopy_folder_link)) {
			return;
		} else {
			$bodycopy_folder_id = $bodycopy_folder_link['minorid'];
		}

		$broadcaster_parents = $GLOBALS['SQ_SYSTEM']->am->getParents($broadcaster->id, 'bodycopy', true);
		$content_bodycopy_value = null;
		foreach (array_keys($broadcaster_parents) as $bodycopy_id) {
			$content_bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($bodycopy_folder_id, $bodycopy_id);
			if (!empty($content_bodycopy_link) && !empty($content_bodycopy_link['value'])){
				$bodycopy_context = $content_bodycopy_link['value'];
				break;
			}
		}

		// check if the found bodycopy context is recognised
		$contexts = $this->_getAvailableContexts();
		if (empty($bodycopy_context) || !isset($contexts[$bodycopy_context])) {
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
	* @access private
	* @return array
	*/
	function _getCheckoutContextKeywords()
	{
		$keywords = parent::getAvailableKeywords();

		$keywords['delivery_selector']			= translate('ecom_checkout_keyword_delivery_selector');
		$keywords['delivery_interface'] 		= translate('ecom_checkout_keyword_delivery_interface');
		$keywords['error_messages']				= translate('ecom_checkout_keyword_error_messages');
		$keywords['cart_summary']				= translate('ecom_checkout_keyword_cart_summary');
		$keywords['go_to_confirmation_button']	= translate('ecom_checkout_keyword_go_to_confirmation_button');
		$keywords['items_count']				= translate('ecom_checkout_keyword_items_count');
		$keywords['items_total_count']			= translate('ecom_checkout_keyword_total_items_count');

		return $keywords;

	}//end _getCheckoutContextKeywords()


	/**
	* Get keywords for 'Confirmation' context
	*
	* @access private
	* @return array
	*/
	function _getConfirmationContextKeywords()
	{
		$keywords['cart_summary']				= translate('ecom_checkout_keyword_cart_summary');
		$keywords['confirmation_summary']		= translate('ecom_checkout_keyword_confirmation_summary');
		$keywords['confirm_order_button']		= translate('ecom_checkout_keyword_confirm_order_button');
		$keywords['return_to_checkout_button']	= translate('ecom_checkout_keyword_return_to_checkout_button');
		$keywords['items_count']				= translate('ecom_checkout_keyword_items_count');
		$keywords['items_total_count']			= translate('ecom_checkout_keyword_total_items_count');

		return $keywords;

	}//end _getConfirmationContextKeywords()


	/**
	* Get keywords for 'Receipt' context
	*
	* @access private
	* @return array
	*/
	function _getReceiptContextKeywords()
	{
		$keywords['receipt_summary']	= translate('ecom_checkout_keyword_receipt_summary');

		return $keywords;

	}//end _getReceiptContextKeywords()


	/**
	* Get keywords for 'Receipt' context
	*
	* @access private
	* @return array
	*/
	function _getFailureContextKeywords()
	{
		$keywords['failure_summary']			= translate('ecom_checkout_keyword_failure_summary');
		$keywords['return_to_checkout_button']	= translate('ecom_checkout_keyword_return_to_checkout_button');

		return $keywords;

	}//end _getFailureContextKeywords()


	//// -- Replacements


	/**
	* Get Keyword Replacement for 'delivery_selector' keyword
	*
	* @return string	replacement
	* @access public
	*/
	function getDeliverySelectorKeywordReplacement()
	{
		$delivery_ids = $this->_getDeliveryAssetIds();
		$multiple_asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($delivery_ids);


		$selection_contents = Array(); //Array('' => '-- '.translate('ecom_checkout_select_delivery_method').' --');
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
			combo_box('sq_delivery_method_id', $selection_contents, false, $active_delivery_method_id, 0, 'onchange="handleDeliverySelectChange(this)"');

		return ob_get_clean();

	}//end getDeliverySelectorKeywordReplacement()


	/**
	* Get Keyword Replacement for 'delivery_interface' keyword
	*
	* @return string	replacement
	* @access public
	*/
	function getDeliveryInterfaceKeywordReplacement()
	{

		$delivery_ids = $this->_getDeliveryAssetIds();
		$active_delivery_method =& $this->_getActiveDeliveryMethod();

		if (is_null($active_delivery_method )) {
			$active_delivery_method_id = array_shift($delivery_ids);
		} else {
			$active_delivery_method_id = $active_delivery_method->id;
		}

		$this->_registerFormRequirement();
		ob_start();
		foreach ($this->_getDeliveryAssetIds() as $one_assetid) {
			$delivery_method =& $GLOBALS['SQ_SYSTEM']->am->getAsset($one_assetid);

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
	* Get Keyword Replacement for 'confirm_order_button' keyword
	*
	* @return string	replacement
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
	* @return string	replacement
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
	* @return string	replacement
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
	* @return string	replacement
	* @access public
	*/
	function getConfirmationSummaryKeywordReplacement()
	{
		ob_start();
			$delivery_method =& $this->_getActiveDeliveryMethod();
			if (!is_null($delivery_method)) {
				echo $delivery_method->getConfirmationSummary();
			}
		return ob_get_clean();

	}//end getCheckoutSummaryKeywordReplacement()


	/**
	* Get Keyword Replacement for 'receipt_summary' keyword
	*
	* @return string	replacement
	* @access public
	*/
	function getReceiptSummaryKeywordReplacement()
	{
		ob_start();
			$delivery_method =& $this->_getActiveDeliveryMethod();
			if (!is_null($delivery_method)) {
				echo $delivery_method->getReceiptSummary();
			}
		return ob_get_clean();

	}//end getReceiptSummaryKeywordReplacement()


	/**
	* Get Keyword Replacement for 'cart_summary' keyword
	*
	* @return string	replacement
	* @access public
	*/
	function getCartSummaryKeywordReplacement()
	{
		return $this->_getCartSummary(true);

	}//end getCartSummaryKeywordReplacement()


	/**
	* Get Keyword Replacement for 'error_messages' keyword
	*
	* @return string	replacement
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
	* @return string	replacement
	* @access public
	*/
	function getFailureSummaryKeywordReplacement()
	{
		ob_start();
			$delivery_method =& $this->_getActiveDeliveryMethod();
			if (!is_null($delivery_method)) {
				echo $delivery_method->getFailureSummary();
			}
		return ob_get_clean();

	}//end getFailureSummaryKeywordReplacement()


	/**
	* Get Keyword Replacement for 'items_count' keyword
	*
	* @return string
	* @access public
	*/
	function getItemsCountKeywordReplacement()
	{
		$cart =& new Ecommerce_Cart_Processor();
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
		$cart =& new Ecommerce_Cart_Processor();
		$cart->setCartContainer($_SESSION['sq_local_cart_contents']['cart_contents']);
		return $cart->getCount();

	}//end getItemsTotalCountKeywordReplacement()



}//end class
?>
