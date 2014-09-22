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
* $Id: paypal_business_account.inc,v 1.5 2012/08/30 00:58:34 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';
require_once SQ_PACKAGES_PATH.'/ecommerce/paypal/paypal_order/paypal_order.inc';

/**
* Paypal_Business_Account
*
* Purpose
*
*
* @author  Anh Ta <ata@squiz.co.uk>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Paypal_Business_Account extends Asset
{


	/**
	* Constructor
	*
	* @param string	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Perform any pre-validation required during the creation of this asset
	*
	* Paypal Business Accounts ensure a name has been set before they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	protected function _preCreateCheck(Array &$link)
	{
		if (!parent::_preCreateCheck($link)) return FALSE;

		$name = trim($this->attr('name'));
		if ($name == '') {
			trigger_localised_error('CORE0083', sprintf(translate('Unable to create a %s with a blank name'), $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name')), E_USER_WARNING)
			return FALSE;
		}

		return TRUE;

	}//end _preCreateCheck()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowedLinks()
	*/
	function _getAllowedLinks()
	{
		$allowed_links = parent::_getAllowedLinks();

		$allowed_links[SQ_LINK_TYPE_2]['folder']   					= Array('card' => 'M', 'exclusive' => FALSE);
		$allowed_links[SQ_LINK_NOTICE]['paypal_order'] 			= Array('card' => 'M', 'exclusive' => FALSE);
		//for better organization structure
		$allowed_links[SQ_LINK_TYPE_1]['paypal_payment_button']		= Array('card' => 'M', 'exclusive' => FALSE);
		$allowed_links[SQ_LINK_TYPE_2]['paypal_payment_button']		= Array('card' => 'M', 'exclusive' => FALSE);
		$allowed_links[SQ_LINK_TYPE_1]['paypal_ipn_receiver']		= Array('card' => 'M', 'exclusive' => FALSE);
		$allowed_links[SQ_LINK_TYPE_2]['paypal_ipn_receiver']		= Array('card' => 'M', 'exclusive' => FALSE);

		return $allowed_links;

	}//end _getAllowedLinks()


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* Create Pending Orders and Completed Orders folder assets when this asset is created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	function _createAdditional(&$link)
	{
		if (!parent::_createAdditional($link)) return FALSE;
		if (!$this->makeAndSaveInitialWebPath(strtolower($this->attr('name')), $link)) return FALSE;

		// add a bodycopy to this page when creating
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('folder');

		$sub_assets = Array(
							'pending_orders'	=> 'folder',
							'completed_orders'	=> 'folder',
						   );

		foreach ($sub_assets as $name => $type) {
			$asset = new $type();
			$copy_link = Array('asset' => $this, 'value' => $name ,'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);

			$asset->setAttrValue('name', ucwords(str_replace('_',' ', $name)));
			if (!$asset->create($copy_link)) return FALSE;

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			unset($asset);
		}

		return TRUE;

	}//end _createAdditional()


	/**
	 * Add a new Paypal Order if it does not exist; otherwise, move it from Pending folder to Completed folder
	 *
	 * @param array		$payment_info	The payment information from IPN Receiver asset
	 * @param string	$message 		The message shows the status of the add/move process
	 *
	 * @return bool		Return TRUE if success; otherwise, return FALSE
	 * @access public
	 */
	public function addPaypalOrder($payment_info, &$message)
	{
		$success = FALSE;
		$am = $GLOBALS['SQ_SYSTEM']->am;
		$order_link = $am->getLink($this->id, SQ_LINK_NOTICE, 'paypal_order', TRUE, $payment_info['txn_id']);
		if (empty($order_link)) {
			//order does not exist, create it
			$new_order = $this->_createOrder($payment_info);
			if ($new_order === FALSE) {
				$message = "CAN NOT CREAT A NEW PAYPAL ORDER FOR THE PAYPAL TRANSACTION #{$payment_info['txn_id']}.";
			} else {
				$message = "A NEW PAYPAL ORDER (#{$new_order->id}) IS CREATED.";
				$success = TRUE;
			}

		} else {
			//order exist
			if ($payment_info['payment_status'] == 'Completed') {
				$order = $am->getAsset($order_link['minorid']);
				if ($order->attr('payment_status') != 'Completed') {
					$success = $this->_moveOrder($order, $payment_info);
					if ($success) {
						$message = "THE PAYPAL ORDER #{$order->id} HAS BEEN MOVED TO THE COMPLETED FOLDER.";
						$success = TRUE;
					} else {
						$message = "THE PAYPAL ORDER #{$order->id} COULD NOT BE MOVED TO THE COMPLETED FOLDER.";
					}

				} else {
					$message = "THE TRANSACTION #{$payment_info['txn_id']} WAS ALREADY COMPLETED.";
					$success = TRUE;
				}

			} else {
				//update the status if it is different
				if ($order->attr('payment_status') != $payment_info['payment_status']) {
					$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);

					$paypal_order->setAttrValue('payment_status', $payment_info['payment_status']);
					$paypal_order->setAttrValue('incomplete_reason', $payment_info['incomplete_reason']);
					$paypal_order->setAttrValue('custom_info', $this->_getCustomInfo($payment_info));
					if ($paypal_order->saveAttributes()) {
						$message = "NEW PAYMENT STATUS {$payment_info['payment_status']} IS UPDATED TO THE PAYPAL ORDER #{$order->id}.";
						$success = TRUE;
					} else {
						$message = "NEW PAYMENT STATUS {$payment_info['payment_status']} COULD NOT BE UPDATED TO THE PAYPAL ORDER #{$order->id}.";
					}

					$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

				} else {
					$message = "THE PAYMENT STATUS {$payment_info['payment_status']} OF THE PAYPAL ORDER #{$order->id} HAS NOT BEEN CHANGED.";
					$success = TRUE;
				}
			}
		}

		//if an error occurs, send emails to inform
		if (!$success) {
			$this->sendEmailOnError("An error occurs under the Paypal Business Account #{$this->id}.\nERROR MESSAGE: $message");
		}

		return $success;

	}//end addPaypalOrder()


	/**
	* Create a new Paypal Order under Pending or Completed folder
	*
	* @param array $payment_info	The payment information from IPN Receiver asset
	*
	* @return bool		Return TRUE if success; otherwise, return FALSE
	* @access private
	*/
	private function _createOrder($payment_info)
	{
		$am = $GLOBALS['SQ_SYSTEM']->am;

		//get folder
		$payment_completed = $payment_info['payment_status'] == 'Completed';
		$folder_link_value = $payment_completed? 'completed_orders' : 'pending_orders';
		$order_folder_link = $am->getLink($this->id, SQ_LINK_TYPE_2, 'folder', TRUE, $folder_link_value);
		if (empty($order_folder_link)) return FALSE;

		$order_folder_id = $order_folder_link['minorid'];
		$order_folder = $am->getAsset($order_folder_id);

		$success = FALSE;

		$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$paypal_order = new Paypal_Order();

		$order_link = Array(
								'asset'		=> $order_folder,
								'link_type'	=> SQ_LINK_TYPE_2,
						   );

		if ($paypal_order->create($order_link)) {
			$paypal_order->setAttrValue('txn_id', $payment_info['txn_id']);
			$paypal_order->setAttrValue('payment_status', $payment_info['payment_status']);
			$paypal_order->setAttrValue('incomplete_reason', $payment_info['incomplete_reason']);
			$paypal_order->setAttrValue('total_amount', $payment_info['mc_gross']);
			$paypal_order->setAttrValue('currency', $payment_info['mc_currency']);
			$paypal_order->setAttrValue('custom_info', $this->_getCustomInfo($payment_info));

			$products = Array();

			if ($payment_info['txn_type'] == 'cart') {
				for ($i = 1; $i <= $payment_info['num_cart_items']; $i++) {
					$product = Array(
										'name' 		=> $payment_info['item_name'.$i],
										'number'	=> $payment_info['item_number'.$i],
										'quantity'	=> $payment_info['quantity'.$i],
										'price'		=> $payment_info['price_'.$i],
										'subtotal'	=> $payment_info['mc_gross_'.$i],
									 );

					$products[] = $product;
				}
			} else {
				$product = Array(
									'name' 		=> $payment_info['item_name'],
									'number'	=> $payment_info['item_number'],
									'quantity'	=> $payment_info['quantity'],
									'price'		=> $payment_info['price'],
									'subtotal'	=> $payment_info['mc_gross'],
								 );

				$products = Array($product);
			}

			$paypal_order->setAttrValue('products', $products);

			if ($paypal_order->saveAttributes()) {
				//create notice link from this Paypal Account asset to the new Paypal_Order asset
				if ($am->createAssetLink($this, $paypal_order, SQ_LINK_NOTICE, $payment_info['txn_id']) != 0) {
					$success = TRUE;
				}
			}
		}

		if ($success) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		}
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		// restore runlevel
		$GLOBALS['SQ_SYSTEM']->restoreRunLevel();


		return ($success? $paypal_order : FALSE);

	}//end _createOrder()


	/**
	* Move a pending Paypal Order to the Completed folder
	*
	* @param Paypal_Order	$paypal_order	The Payal Order asset to move
	* @param array		 	$payment_info	The payment information from IPN Receiver asset
	*
	* @return boolean	Return TRUE if success; otherwise, return FALSE
	* @access private
	*/
	private function _moveOrder($paypal_order, $payment_info)
	{
		$am = $GLOBALS['SQ_SYSTEM']->am;
		// get linkid of the paypal order to move
		$pending_orders_link = $am->getLink($this->id, SQ_LINK_TYPE_2, 'folder', TRUE, 'pending_orders');
		if (empty($pending_orders_link)) return FALSE;
		$order_link_info = $am->getLinkByAsset($pending_orders_link['minorid'], $paypal_order->id);

		// get destination folder assetid
		$order_folder_link = $am->getLink($this->id, SQ_LINK_TYPE_2, 'folder', TRUE, 'completed_orders');

		$success = FALSE;

		$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// now move the Paypal order to its final resting place
		if ($am->moveLink($order_link_info['linkid'], $order_folder_link['minorid'], SQ_LINK_TYPE_2, 0)) {
			$paypal_order->setAttrValue('payment_status', $payment_info['payment_status']);
			$paypal_order->setAttrValue('incomplete_reason', $payment_info['incomplete_reason']);
			$paypal_order->setAttrValue('custom_info', $this->_getCustomInfo($payment_info));

			if ($paypal_order->saveAttributes()) {
				$success = TRUE;
			}
		}

		if ($success) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		}
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		// restore runlevel
		$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

		return $success;

	}//end _moveOrder()


	/**
	 * Get custom information string from $payment_info array
	 *
	 * @param array		$payment_info	The payment information from IPN Receiver asset
	 *
	 * @return string	The string of custom information
	 * @access private
	 */
	private function _getCustomInfo($payment_info)
	{
		$custom_indexes = Array('txn_type', 'test_ipn', 'payment_date', 'custom', 'button_id', 'real_custom_var', 'notify_url_query', 'business', 'target_assetid', 'ipn_event');
		$custom_string = '';
		foreach ($custom_indexes as $index) {
			$message = empty($payment_info[$index])? '[Empty]' : $payment_info[$index];
			$custom_string .= "$index=$message\n";
		}

		return $custom_string;

	}//end _getCustomInfo()

	/**
	 * Send email to inform when an error occurs
	 *
	 * @return void
	 * @access public
	 */
	public function sendEmailOnError($message)
	{
		if ($this->attr('send_email_on_error')) {
			$to = $this->attr('error_email');
			if (empty($to) && SQ_CONF_TECH_EMAIL) {
				$to = SQ_CONF_TECH_EMAIL;
			}

			if ($to) {
				$subject = 'Matrix - '.SQ_CONF_SYSTEM_NAME.' - An error occurs when processing Payal Orders';
				mail($to, $subject, $message, 'From: '.SQ_CONF_TECH_EMAIL."\r\n");
			}
		}

	}//end sendEmailOnError()


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
		$values = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName('name', $this->type(), Array($this->id), $contextid);
		if (empty($values) === TRUE) {
			return parent::_getName($short_name, $contextid);
		} else {
			return $values[$this->id];
		}

	}//end _getName()


}//end class
?>
