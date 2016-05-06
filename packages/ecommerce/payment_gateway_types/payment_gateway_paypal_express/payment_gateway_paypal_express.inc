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
* $Id: payment_gateway_paypal_express.inc,v 1.2 2013/07/25 06:12:17 cupreti Exp $
*
*/


require_once dirname(__FILE__).'/../../payment_gateway/payment_gateway.inc';
require_once dirname(__FILE__).'/paypal_express_processor.inc';


/**
* Payment_Gateway_Paypal_Express
*
* @see Payment_Gateway
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Payment_Gateway_Paypal_Express extends Payment_Gateway
{

	/**
	* Perform any additional processing required during the creation of this asset
	*
	* Datacash Payment Gateway creates a bodycopy asset when they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	function _createAdditional(&$link)
	{
		if (!parent::_createAdditional($link)) return FALSE;

		return ($this->_createBodycopy('display_format', translate('%processing_error%
<div style="background-color: #CCC; padding: 5px">Confirm payment <strong>$%transaction_amount%</strong></div>
<table cellpadding="5">
	<tr>
		<td>&nbsp;</td>
		<td>%submit_button% %reset_button% %cancel_button%</td>
	</tr>
</table>')));


	}//end _createAdditional()


	/**
	* Return the available keywords
	*
	* The return value is in the form:
	* <pre>
	* Array(
	*     'keyword' => 'name',
	* )
	* </pre>
	*
	* @return array
	* @access public
	*/
	function getAvailableKeywords()
	{
		$keywords = parent::getAvailableKeywords();

		// Form fields keywords
		$keywords['delivery_address_first_name'] = 'Delivery address first name';
		$keywords['delivery_address_last_name'] = 'Delivery address last name';
		$keywords['delivery_address_street'] = 'Delivery address street (street number, name and type)';
		$keywords['delivery_address_city'] = 'Delivery address city';
		$keywords['delivery_address_state'] = 'Delivery address state';
		$keywords['delivery_address_postcode'] = 'Delivery address postcode';
		$keywords['delivery_address_country'] = 'Delivery address country';

		$keywords['submit_button'] = 'Submit Button';
		$keywords['cancel_button'] = 'Cancel Button';
		$keywords['reset_button'] = 'Reset Button';

		// Transaction keywords
		$keywords['transaction_amount'] = 'Transaction Amount';
		$keywords['transaction_currency'] = 'Transaction Currency';
		$keywords['processing_error'] = 'Processing Error (empty if no error)';

		return $keywords;

	}//end getAvailableKeywords()


	/**
	* Basic output function. root of output process
	*
	* @return void
	* @access public
	*/
	function printBody()
	{
		//this page is redirected to by the child iframe with unknown reference error
		if (array_get_index($_GET, 'unknown_referer', FALSE)) {
			trigger_error('Unknown caller reference');
			exit;
		}

		//this page is redirected to by the child iframe with an error message
		if (array_get_index($_GET, 'error_message', FALSE)) {
			$this->_setError($_GET['error_message']);
		}

		$submit_text = $this->attr('submit_text');
		if (empty($submit_text)) $submit_text = 'Submit';
		$cancel_text = $this->attr('cancel_text');
		if (empty($cancel_text)) $cancel_text = 'Cancel';

		// Get the PayPal Transaction Token from the session
		$transaction_token = array_get_index($_SESSION, 'PAYPAL_TRANSACTION_TOKEN', '');

		// After user logins and confirms the amount at the Paypal,
		// user will be redirected to the gateway with the "token" and "PayerID"
		$get_var_transaction_token = array_get_index($_REQUEST, 'token', '');
		$payer_id = array_get_index($_REQUEST, 'PayerID', '');

		$valid_token = TRUE;

		if (!empty($payer_id) && !empty($get_var_transaction_token) && $get_var_transaction_token == $transaction_token) {
			$this->appendPendingOrderAuditLog(translate('User authenticated successfully at PayPal'));

			// Redirected from PayPal after successful user authentication
			$_SESSION['PAYPAL_PAYER_ID'] = $payer_id;
		} else if (!empty($get_var_transaction_token) && $get_var_transaction_token != $transaction_token) {
			// Token in the session is not valid anymore
			$valid_token = FALSE;
		}

		$action = array_get_index($_REQUEST, $this->getPrefix().'_action', FALSE);

		if (!$valid_token || empty($transaction_token) || (empty($get_var_transaction_token) && !$action)) {
			// Transaction token not present in session means we have not initated the transaction process
			// This will get the PayPal transaction token and then redirect user to the PayPal
			$this->initiateTransaction();
		}

		$success = FALSE;
		$amount = array_get_index($_SESSION,'SQ_ECOM_AMOUNT', 0);

		if ($action == $cancel_text) {
			$this->returnToCaller('CANCEL');
		} else if (empty($amount)) {
			unset($_SESSION['SQ_ECOM_AMOUNT']);
			//clear pass through variables
			if (isset($_SESSION['SQ_ECOM_PASS_THROUGH_VARS'])) {
				unset($_SESSION['SQ_ECOM_PASS_THROUGH_VARS']);
			}
			$this->appendPendingOrderAuditLog(translate('Payment has no amount'));

			$this->returnToCaller('SUCCESS');

		} else if ($action == $submit_text) {
			$this->appendPendingOrderAuditLog(translate('Payment Attempted (gateway contacted)'));

			$success = $this->processPayment();
		}

		//if transaction is accepted, redirect the user to the success page
		if ($success) {
			unset($_SESSION['SQ_ECOM_AMOUNT']);
			unset($_SESSION['PAYPAL_TRANSACTION_TOKEN']);
			unset($_SESSION['PAYPAL_CORRELATION_ID']);
			unset($_SESSION['PAYPAL_PAYER_ID']);

			//clear pass through variables
			if (isset($_SESSION['SQ_ECOM_PASS_THROUGH_VARS'])) {
				unset($_SESSION['SQ_ECOM_PASS_THROUGH_VARS']);
			}
			//broadcast payment completed trigger event before return
			if (isset($_SESSION['SQ_ECOM_PAYMENT_COMPLETED_BROADCASTER_ASSETID'])) {
				$broadcaster = $GLOBALS['SQ_SYSTEM']->am->getAsset($_SESSION['SQ_ECOM_PAYMENT_COMPLETED_BROADCASTER_ASSETID']);
				if (!empty($broadcaster)) {
					//put all returned variables in $_SESSION['SQ_ECOM_RESPONSE'] to $_POST array so that the listening trigger can use them
					$response = array_get_index($_SESSION, 'SQ_ECOM_RESPONSE', FALSE);
					if ($response) {
						$_POST['transaction_reference'] = $response['MERCHANT_REFERENCE'];
						$_POST['transaction_time'] = $response['TIME'];
						$_POST['transaction_status'] = $response['STATUS'];
						$_POST['transaction_card_number'] = 'n/a';
						$_POST['transaction_amount'] = $response['AMOUNT'];
						$_POST['transaction_currency'] = $response['CURRENCY'];
					}
					$GLOBALS['SQ_SYSTEM']->broadcastTriggerEvent('trigger_event_ecommerce_payment_completed', $broadcaster);
				}
			}

			$this->appendPendingOrderAuditLog(translate('Payment is successful'));


			$this->returnToCaller('SUCCESS');
		} else if ($action == $submit_text) {
				$this->appendPendingOrderAuditLog(translate('Payment Declined'));

		}

		//print default bodycopy
		$this->printBodycopy('display_format');

	}//end printBody()


	/**
	* Prints the front-end interface for this asset
	*
	* @return void
	* @access public
	*/
	function printBodycopy($link_value)
	{
		$bodycopy = $this->getBodycopy($link_value);
		if (is_null($bodycopy)) return;

		$keywords = $bodycopy->getKeywords();
		$replacements = Array();
		foreach ($keywords as $word) {
			$replacements[$word] = $this->getKeywordReplacement($word);
		}
		$bodycopy->setKeywordReplacements($replacements);

		if ($link_value == 'display_format') {
			//the default body copy with card input
			if ($this->attr('test_mode')) {
				echo '<h2 class="gateway-test-mode" style="color: white; background-color: #C00; padding: 5px">TEST MODE</h2>';
			}

			// Load the JS 'Loader' object to print the overlay when payment form is submitted
			$this->includeLoaderJsObject($this->getPrefix().'_card');
			?>
			<form method="post" action="<?php echo $this->getURL(); ?>" onsubmit="Loader.show();">
			<?php
			$bodycopy->printBody();
			echo '</form>';
		} else {
			//the card holder verification bodycopy
			$bodycopy->printBody();
		}

	}//end printBodycopy()


	/**
	* Process the express checkout payment
	*
	* @return mixed	return FALSE if the card is not valid or transaction fails, return 1 if SUCCESS, return 2 if need to REDIRECT in 3-D Secure check
	* @access public
	*/
	function processPayment()
	{
		$transaction_token = array_get_index($_SESSION, 'PAYPAL_TRANSACTION_TOKEN', '');
		$correlation_id = array_get_index($_SESSION, 'PAYPAL_CORRELATION_ID', '');
		$payer_id = array_get_index($_SESSION, 'PAYPAL_PAYER_ID', '');

		// Check that transaction token matches with the one in the session
		if (empty($transaction_token) || empty($correlation_id) || empty($payer_id)){
			$this->_setError(translate('PayPal authentication unsuccessful'));

			return FALSE;
		}

		$processor = new Paypal_Express_Processor($this->attr('test_mode'));

		// Merchand reference should be set here instead of default_delivery_method.inc or elsewhere
		// since we will not have "token" before the initial request (see self::initiateTransaction())
		$_SESSION['SQ_ECOM_REF_NO'] = $transaction_token;

		// Set the PayPal account paramaters, transaction amount and currency
		$this->setPaypalApiParameters($processor);

		// Set the transaction token and payer id
		$processor->setTransactionToken($transaction_token);
		$processor->setPayerId($payer_id);

		// Set the transaction method
		$processor->setTransactionMethod('DoExpressCheckoutPayment');

		// Check for any error before sending request to the PayPal
		if ($this->isError()) {
			return FALSE;
		}

		$this->appendPendingOrderAuditLog(translate('Sending request to PayPal to finialise the transaction'));

		// Do the processing
		$success = $processor->process();

		// Set the delivery address
		// NOTE: Delivery address is not relevent to this gateway, since its all handled by PayPal
		// The delivery address set here is merely used for Matrix Ecom Order logging purpose
		$this->_setDeliveryAddress();

		// Get the response
		$response = $processor->getResponse();

		// Set the results
		$this->_setTransactionResult($success, $response);

		return $success;

	}//end processPayment()


	/**
	* Requests the transaction token from PayPal API and
	* then redirects user to PayPal login page
	*
	* @return void|boolean
	* @access private
	*/
	function initiateTransaction()
	{
		$processor = new Paypal_Express_Processor($this->attr('test_mode'));

		// Set the PayPal account paramaters, transaction amount and currency
		$this->setPaypalApiParameters($processor);

		// The "token" generated after intial request will be used as referece id later
		$_SESSION['SQ_ECOM_REF_NO'] = 'n/a';

		// Set "success" and "cancel" urls
		$cancel_text = $this->attr('cancel_text');
		if (empty($cancel_text)) $cancel_text = 'Cancel';
		$success_url = $this->getUrl();
		$cancel_url = $this->getUrl().'?'.$this->getPrefix().'_action='.$cancel_text;
		$processor->setReturnUrlSuccess($success_url);
		$processor->setReturnUrlCancel($cancel_url);

		$processor->setTransactionMethod('SetExpressCheckout');

		// Check for any error before sending request to the PayPal
		if ($this->isError()) {
			return FALSE;
		}

		$this->appendPendingOrderAuditLog(translate('Transaction Token request sent to PayPal'));


		$success = $processor->process();
		$response = $processor->getResponse();

		if (empty($response['CORRELATIONID']) || empty($response['TOKEN']) || empty($response['ACK']) || $response['ACK'] != 'Success') {
			$this->_setError(translate('Expected transaction paramaters not obtained from the PayPal'));

			if (!empty($response['PAYPAL_ERROR'])) {
				$this->_setError($response['PAYPAL_ERROR']);
			}
			return FALSE;
		}

		// Store the info in the session
		$_SESSION['PAYPAL_TRANSACTION_TOKEN'] = $response['TOKEN'];
		$_SESSION['PAYPAL_CORRELATION_ID'] = $response['CORRELATIONID'];
		$_SESSION['SQ_ECOM_REF_NO'] = $response['TOKEN'];

		$this->appendPendingOrderAuditLog(translate('Token obtained. Redirecting user to PayPal'));


		// Everything good so far, now redirect user to the Paypal
		$processor->redirectToPaypal($response['TOKEN']);

		exit;

	}// end initiateTransaction()


	/**
	* Set the PayPal API parameters, transaction amount and currency
	*
	* @param Object $processor
	*
	* @return void
	* @access private
	*/
	private function setPaypalApiParameters(&$processor)
	{
		$amount = array_get_index($_SESSION,'SQ_ECOM_AMOUNT', 0);
		if ($amount && is_numeric($amount) && $amount > 0) {
			$processor->setTransactionAmount($amount);
		} else {
			$this->_setError(translate('Transaction amount is invalid'));

		}
		$currency = $this->attr('currency');
		$processor->setTransactionCurrency($currency);

		$api_username = $this->attr('username');
		if ($api_username) {
			$processor->setApiUsername($api_username);
		} else {
			$this->_setError(translate('PayPal API Username not specified'));

		}

		$api_password = $this->attr('password');
		if ($api_password) {
			$processor->setApiPassword($api_password);
		} else {
			$this->_setError(translate('PayPal API Password not specified'));

		}

		$api_signature = $this->attr('signature');
		if ($api_signature) {
			$processor->setApiSignature($api_signature);
		} else {
			$this->_setError(translate('PayPal API Signature not specified'));

		}

		$timeout = $this->attr('timeout');
		if ($timeout) {
			$processor->setRequestTimeout($timeout);
		}

		// Set the Payment action type
		$processor->setPaymentAction('Sale');

	}//end setPaypalApiParameters()


	/**
	* Set the delivery address to the response
	* This info is not used when processing the transaction at PayPal
	*
	* @return void
	* @access private
	*/
	private function _setDeliveryAddress()
	{
		$address_info = array_get_index($_REQUEST, $this->getPrefix().'_address', FALSE);
		if (!$address_info) {
			return;
		}
		// Clean the post values
		foreach($address_info as $key => $val) {
			$address_info[$key] = htmlentities($val);
		}

		$name = array_get_index($address_info, 'first_name', '').' '.array_get_index($address_info, 'last_name', '');
		$address = array_get_index($address_info, 'street', '').', '.array_get_index($address_info, 'city', '').' '.array_get_index($address_info, 'state', '').' '.array_get_index($address_info, 'postcode', '').', '.array_get_index($address_info, 'country', '');

		$this->_response['DELIVERY_NAME'] = $name;
		$this->_response['DELIVERY_ADDR'] = $address;

	}//end setDeliveryAddress()


	/**
	* Returns the "Checkout Button" image url for this gateway
	*
	* @return string
	* @access public
	*/
	public function checkoutButtonImageUrl()
	{
		return $this->attr('button_image');

	}//end checkoutButtonImageUrl()


	/**
	 * Set the result of the transaction. If transaction fails, set its error message.
	 * If transaction success, set the Session variable SQ_ECOM_RESPONSE
	 *
	 * @param boolean	$success		the reason of the transaction. It is either 1 or FALSE
	 * @param array		$response		the response array from the Datacash Processor
	 *
	 * @return void
	 * @access private
	 */
	private function _setTransactionResult($success, $response)
	{
		if (!$success) {
			$this->_setError($response['PAYPAL_ERROR']);
		} else {
			if ($this->attr('test_mode')) {
				$response['STATUS'] = '!!!ATTENTION!!! TEST MODE (transaction not performed) -- '.$response['STATUS'];
			}

			// PayPal will have the card and billing info
			$response['AMOUNT'] = $_SESSION['SQ_ECOM_AMOUNT'];
			$response['CURRENCY'] = $this->attr('currency');
			$response['CARDNO'] = 'n/a';
			$response['BILLING_ADDR'] = 'n/a';
			$response['BILLING_NAME'] = 'n/a';
			$response['DELIVERY_NAME'] = $this->_response['DELIVERY_NAME'];
			$response['DELIVERY_ADDR'] = $this->_response['DELIVERY_ADDR'];

			$_SESSION['SQ_ECOM_RESPONSE']   = $response;
		}

	}//end _setTransactionResult()


	/**
	* Returns back to e-commerce form, which called gateway
	*
	* @param string	$state	state to return
	*
	* @return void
	* @access public
	*/
	function returnToCaller($state='SUCCESS')
	{
		$back_url = NULL;

		if ($state == 'SUCCESS') {
			$back_url = array_get_index($_SESSION, 'SQ_ECOM_SUCCESS_URL');
		} else if ($state == 'CANCEL') {
			$back_url = array_get_index($_SESSION, 'SQ_ECOM_CANCEL_URL');
		}

		unset($_SESSION['SQ_ECOM_SUCCESS_URL']);
		unset($_SESSION['SQ_ECOM_CANCEL_URL']);

		if (isset($_SESSION['PAYPAL_TRANSACTION_TOKEN'])) {
			unset($_SESSION['PAYPAL_TRANSACTION_TOKEN']);
		}
		if (isset($_SESSION['PAYPAL_CORRELATION_ID'])) {
			unset($_SESSION['PAYPAL_CORRELATION_ID']);
		}
		if (isset($_SESSION['PAYPAL_PAYER_ID'])) {
			unset($_SESSION['PAYPAL_PAYER_ID']);
		}

		if (is_null($back_url)) {
			trigger_error('Unknown caller reference');
		} else {
			header('Location: '.$back_url);
			exit;
		}

	}//end returnToCaller()


	/**
	* Transaction Amount keyword replacement
	*
	* @return string
	* @access public
	*/
	function getTransactionAmountKeywordReplacement()
	{
		$amount = array_get_index($_SESSION,'SQ_ECOM_AMOUNT', 0);

		return $amount;

	}//end getTransactionAmountKeywordReplacement()


	/**
	* Transaction Currency keyword replacement
	*
	* @return string
	* @access public
	*/
	function getTransactionCurrencyKeywordReplacement()
	{
		return $this->attr('currency');

	}//end getTransactionAmountKeywordReplacement()


	/**
	* Processing Error keyword replacement
	*
	* @return string
	* @access public
	*/
	function getProcessingErrorKeywordReplacement()
	{
		if ($this->isError()) {
			return '<span class="payment-gateway-transaction-error">'.$this->getErrorMessage().'</span>';
		}

		return '';

	}//end getProcessingErrorKeywordReplacement()


	/**
	* Submit Button keyword replacement
	*
	* @return string
	* @access public
	*/
	function getSubmitButtonKeywordReplacement()
	{
		$button_text = $this->attr('submit_text');
		if (empty($button_text)) $button_text = 'Submit';

		return '<input type="submit" name="'.$this->getPrefix().'_action" value="'.$button_text.'" />';

	}//end getSubmitButtonKeywordReplacement()


	/**
	* Reset Button keyword replacement
	*
	* @return string
	* @access public
	*/
	function getResetButtonKeywordReplacement()
	{
		$button_text = $this->attr('reset_text');
		if (empty($button_text)) $button_text = 'Reset';

		return '<input type="reset" value="'.$button_text.'" />';

	}//end getResetButtonKeywordReplacement()


	/**
	* Cancel Button keyword replacement
	*
	* @return string
	* @access public
	*/
	function getCancelButtonKeywordReplacement()
	{
		$button_text = $this->attr('cancel_text');
		if (empty($button_text)) $button_text = 'Cancel';

		return '<input type="submit" name="'.$this->getPrefix().'_action" value="'.$button_text.'" />';

	}//end getCancelButtonKeywordReplacement()


	/**
	* Delivery Address First Name keyword replacement
	*
	* @return string
	* @access private
	*/
	function getDeliveryAddressFirstNameKeywordReplacement()
	{
		ob_start();
		$field_name = $this->getPrefix().'_address';
		text_box($field_name.'[first_name]', $this->_getDefaultInputFromPassThroughAttribute('first_name_var_name'));

		return ob_get_clean();

	}//end getDeliveryAddressFirstNameKeywordReplacement()


	/**
	* Delivery Address First Name keyword replacement
	*
	* @return string
	* @access private
	*/
	function getDeliveryAddressLastNameKeywordReplacement()
	{
		ob_start();
		$field_name = $this->getPrefix().'_address';
		text_box($field_name.'[last_name]', $this->_getDefaultInputFromPassThroughAttribute('last_name_var_name'));

		return ob_get_clean();

	}//end getDeliveryAddressLastNameKeywordReplacement()


	/**
	* Delivery Address Street keyword replacement
	*
	* @return string
	* @access private
	*/
	function getDeliveryAddressStreetKeywordReplacement()
	{
		ob_start();
		$field_name = $this->getPrefix().'_address';
		text_box($field_name.'[street]', $this->_getDefaultInputFromPassThroughAttribute('addr_street_var_name'));

		return ob_get_clean();

	}//end getDeliveryAddressStreetKeywordReplacement()


	/**
	* Delivery Address City keyword replacement
	*
	* @return string
	* @access private
	*/
	function getDeliveryAddressCityKeywordReplacement()
	{
		ob_start();
		$field_name = $this->getPrefix().'_address';
		text_box($field_name.'[city]', $this->_getDefaultInputFromPassThroughAttribute('city_var_name'));

		return ob_get_clean();

	}//end getDeliveryAddressCityKeywordReplacement()


	/**
	* Delivery Address Postcode keyword replacement
	*
	* @return string
	* @access private
	*/
	function getDeliveryAddressPostcodeKeywordReplacement()
	{
		ob_start();
		$field_name = $this->getPrefix().'_address';
		text_box($field_name.'[postcode]', $this->_getDefaultInputFromPassThroughAttribute('postcode_var_name'));

		return ob_get_clean();

	}//end getDeliveryAddressPostcodeKeywordReplacement()


	/**
	* Delivery Address State keyword replacement
	*
	* @return string
	* @access private
	*/
	function getDeliveryAddressStateKeywordReplacement()
	{
		ob_start();
		$field_name = $this->getPrefix().'_address';
		text_box($field_name.'[state]', $this->_getDefaultInputFromPassThroughAttribute('state_var_name'));

		return ob_get_clean();

	}//end getDeliveryAddressStateKeywordReplacement()


	/**
	* Delivery Address Country keyword replacement
	*
	* @return string
	* @access private
	*/
	function getDeliveryAddressCountryKeywordReplacement()
	{
		global $standards_lists_countries;
		require_once SQ_FUDGE_PATH.'/standards_lists/countries.inc';

		ob_start();
		$field_name = $this->getPrefix().'_address';
		combo_box($field_name.'[country]', $standards_lists_countries, FALSE, $this->_getDefaultInputFromPassThroughAttribute('country_var_name'));

		return ob_get_clean();

	}//end getDeliveryAddressCountryKeywordReplacement()


}//end class

?>
