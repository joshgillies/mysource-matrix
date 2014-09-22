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
* $Id: paypal_ipn_receiver.inc,v 1.4 2012/08/30 00:58:34 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';

define('LOG_FILE_PREFIX', 'paypal_transactions');
define('FAIL_LOG_FILE', LOG_FILE_PREFIX.'_failure');

/**
* Paypal_IPN_Receiver
*
* Purpose
*
*
* @author  Anh Ta <ata@squiz.co.uk>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Paypal_IPN_Receiver extends Page
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
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	public function _getAllowedLinks()
	{
		$allowed_links = parent::_getAllowedLinks();

		$allowed_links[SQ_LINK_NOTICE]['asset'] = Array('card' => 'M', 'exclusive' => FALSE);

		return $allowed_links;

	}//end _getAllowedLinks()


	/**
	* Prints out the Frontend for this asset
	*
	* @return void
	* @access public
	*/
	public function printFrontend()
	{
		$payment_info = Array();
		$payment_info['request_server'] = "There is a {$_SERVER['REQUEST_METHOD']} Request from {$_SERVER['REMOTE_ADDR']}";

		//only process if the request method is POST
		if ($_SERVER['REQUEST_METHOD'] == 'POST') {
			$payment_info['txn_type'] = array_get_index($_POST, 'txn_type', '');
			$payment_info['txn_id'] = array_get_index($_POST, 'txn_id', '');
			$payment_info['test_ipn'] = array_get_index($_POST, 'test_ipn', '0');
			$payment_info['payment_status'] = array_get_index($_POST, 'payment_status', '');
			$payment_info['payment_date'] = array_get_index($_POST, 'payment_date', '');

			//do not proceed if there is no transaction type, transaction ID or payment_status
			if (empty($payment_info['txn_type']) || empty($payment_info['txn_id']) || empty($payment_info['payment_status'])) {
				$payment_info['error'] = 'THERE IS NO TRANSACTION TYPE, TRANSACTION ID OR PAYMENT STATUS.';
				$this->_returnPageNotFound($payment_info);
			}

			if ($payment_info['payment_status'] == 'Pending') {
				$payment_info['incomplete_reason'] = array_get_index($_POST, 'pending_reason', '');
			} else {
				$payment_info['incomplete_reason'] = array_get_index($_POST, 'reason_code', '');
			}

			$payment_info['custom'] = array_get_index($_POST, 'custom', '');
			//if there is no custom parameter, it must not be a Paypal POST request (our payment button always sends custom variable)
			if (empty($payment_info['custom'])) {
				$payment_info['error'] = 'CUSTOM VARIABLE IS NOT SENT BACK FROM PAYPAL.';
				$this->_returnPageNotFound($payment_info);
			}

			$GLOBALS['SQ_SYSTEM']->am->includeAsset('paypal_payment_button');

			list($custom_hash, $button_id, $custom) = explode(Paypal_Payment_Button::$CUSTOM_PARAM_SPLIT_STR, $payment_info['custom']);

			$payment_info['button_id'] = $button_id;
			if (empty($button_id)) {
				$payment_info['error'] = 'BUTTON ID IS NOT SPECIFIED.';
				$this->_returnPageNotFound($payment_info);
			}

			$button = $GLOBALS['SQ_SYSTEM']->am->getAsset($button_id);

			$paypal_config_id = $button->attr('paypal_config_id');
			if (empty($paypal_config_id)) {
				$payment_info['error'] = "PAYPAL CONFIGURATION IS NOT SPECIFIED FOR THE BUTTON #$button_id.";
				$this->_returnPageNotFound($payment_info);
			}

			$paypal_config = $GLOBALS['SQ_SYSTEM']->am->getAsset($paypal_config_id);

			//check if there is any malicious changes to our custom parameter
			$custom_var = $button_id.Paypal_Payment_Button::$CUSTOM_PARAM_SPLIT_STR.$custom;

			if (!empty($custom)) {
				$custom = urldecode($custom);
			}

			$notify_url_query = array_get_index($_SERVER, 'QUERY_STRING', '');

			$payment_info['real_custom_var'] = $custom;
			$payment_info['notify_url_query'] = $notify_url_query;

			if ($custom_hash != md5($paypal_config->attr('custom_param_secret_str').$custom_var.$paypal_config->attr('custom_param_secret_str').$notify_url_query)) {
				//someone tampered with the custom paramater
				$payment_info['error'] = 'THE CHECKSUM OF CUSTOM VARIABLES AND NOTIFY URL QUERY STRING IS INVALID.';
				$this->_returnPageNotFound($payment_info);
			}

			$paypal_account_id = $button->attr('paypal_account_id');
			if (empty($paypal_account_id)) {
				$payment_info['error'] = "PAYPAL BUSINESS ACCOUNT IS NOT SPECIFIED FOR THE BUTTON #$button_id.";
				$this->_returnPageNotFound($payment_info);
			}

			$paypal_account = $GLOBALS['SQ_SYSTEM']->am->getAsset($paypal_account_id);

			$payment_info['business'] = array_get_index($_POST, 'business', '');
			//check if the payment is made to correct merchant
			if ($payment_info['business'] != $paypal_account->attr('account_id')) {
				$payment_info['error'] = "THE PAYMENT'S MERCHANT ID ({$payment_info['business']}) DOES NOT MATCH THE MERCHANT ID ({$paypal_account->attr('account_id')}) OF THE PAYPAL ACCOUNT #$paypal_account_id.";
				$this->_returnPageNotFound($payment_info);
			}

			//THE PAYMENT IS VALID

			//get other information of this payment to write to the log file
			$payment_info['mc_currency'] = array_get_index($_POST, 'mc_currency', '');
			$payment_info['mc_gross'] = array_get_index($_POST, 'mc_gross', 0);
			if ($payment_info['txn_type'] == 'cart') {
				$payment_info['num_cart_items'] = array_get_index($_POST, 'num_cart_items', 0);
				for ($i = 1; $i <= $payment_info['num_cart_items']; $i++) {
					$payment_info['item_name'.$i] = array_get_index($_POST, 'item_name'.$i, '');
					$payment_info['item_number'.$i] = array_get_index($_POST, 'item_number'.$i, '');
					$payment_info['quantity'.$i] = array_get_index($_POST, 'quantity'.$i, 1);
					$payment_info['mc_gross_'.$i] = array_get_index($_POST, 'mc_gross_'.$i, 0);
					$payment_info['price_'.$i] = $payment_info['mc_gross_'.$i]/$payment_info['quantity'.$i];
				}
			} else {
				$payment_info['item_name'] = array_get_index($_POST, 'item_name', '');
				$payment_info['item_number'] = array_get_index($_POST, 'item_number', '');
				$payment_info['quantity'] = array_get_index($_POST, 'quantity', 1);
				$payment_info['price'] = $payment_info['mc_gross']/$payment_info['quantity'];
			}

			//split custom parameters
			$params = Array();
			$custom_splits = explode(',', $custom);
			foreach ($custom_splits as $name_value) {
				list($name, $value) = explode('=', $name_value);
				$params[trim($name)] = trim($value);
			}

			//set all custom parameters to $_POST array so that triggers can use them later
			foreach ($params as $key => $value) {
				$_POST[$key] = $value;
			}

			//get target asset
			$assetid = '';
			$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, '', FALSE, 'target_asset');
			if (!empty($link)) {
				$assetid = $link['minorid'];
			}

			if (empty($assetid)) {
				//get assetid from custom parameter
				$assetid = array_get_index($_POST, $this->attr('assetid_param_name'), '');
			}

			if (empty($assetid)) {
				$payment_info['target_assetid'] = "No target asset ID specified.";
			} else {
				$payment_info['target_assetid'] = $assetid;
				$target_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);

				//if payment is completed, broadcast IPN Completed event
				if ($payment_info['payment_status'] == 'Completed') {
					if (empty($target_asset)) {
						$payment_info['ipn_event'] = "No IPN Completed event is broadcasted because there is no asset with ID #$assetid.";
					} else {
						$GLOBALS['SQ_SYSTEM']->broadcastTriggerEvent('trigger_event_paypal_ipn_completed', $target_asset);
						$payment_info['ipn_event'] = "An IPN Completed event was broadcasted to the asset #$assetid.";
					}
				} else {
						$payment_info['ipn_event'] = "No IPN Completed event is broadcasted because payment status is {$payment_info['payment_status']}.";
				}
			}

			$paypal_account->addPaypalOrder($payment_info, $message);
			$payment_info['paypal_order'] = $message;

			//log message
			$this->_logIncomingRequestInfo(LOG_FILE_PREFIX.'_account_'.$paypal_account_id, $payment_info);

		} else {
			//trick that there is no asset at this URL because the URL of IPN Receiver should not be revealed
			$this->_returnPageNotFound($payment_info);
		}

	}//end printFrontend()


	/**
	 * This function sends an HTTP status code 404 (Page Not Found) to the client to trick it that there is no page at
	 * this requested URL
	 *
	 * @param string $payment_info	The message to write to log file
	 *
	 * @return void
	 * @access private
	 */
	private function _returnPageNotFound($payment_info) {
		$protocol = current_protocol();
		$url = strip_url(current_url(FALSE, TRUE));
		if (!headers_sent()) {
			header('HTTP/1.0 404 Not Found');
		}

		//log message before triggering an error
		$payment_info['status_code'] = "A 404 status code was sent back to the client.";
		$this->_logIncomingRequestInfo(FAIL_LOG_FILE, $payment_info);

		trigger_localised_error('SYS0218', sprintf(translate('URL "%s" not found'), $protocol.'://'.$url), E_USER_NOTICE)

		exit(1);

	}//end _returnPageNotFound()


	/**
	 * Log message to the $file_name
	 *
	 * @param string $file_name		The name of the log file
	 * @param string $payment_info	The data to write down
	 *
	 * @return void
	 * @access private
	 */
	private function _logIncomingRequestInfo($file_name, $payment_info) {
		$data = '';
		foreach ($payment_info as $name => $message) {
			if ($message == '') {
				$message = '[Empty]';
			}
			$data .= "$name=$message\n";
		}
		log_write($data, $file_name, E_USER_NOTICE, FALSE);

	}//end _logIncomingRequest()


}//end class

?>