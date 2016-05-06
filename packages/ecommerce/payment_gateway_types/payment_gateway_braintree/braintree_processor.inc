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
*/


define('BRAINTREE_LOG_FILE', 'braintree_transactions');

require_once SQ_DATA_PATH.'/private/conf/tools.inc';
if (defined('SQ_TOOL_BRAINTREE_LIB_PATH')) {
	$braintree_lib_file = rtrim(SQ_TOOL_BRAINTREE_LIB_PATH, '/').'/lib/Braintree.php';
	if (is_file($braintree_lib_file)) {
		require_once $braintree_lib_file;
	}
}

/**
* Braintree_Processor
*
* Brintree payment processing object
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Braintree_Processor
{

	/**
	* Braintree account merchant ID
	*
	* @var string
	*/
	private $_merchant_id;


	/**
	* Braintree account private key
	*
	* @var string
	*/
	private $_private_key;


	/**
	* Braintree account public key
	*
	* @var string
	*/
	private $_public_key;


	/**
	* Braintree account merchant account ID to use for transaction
	*
	* @var string
	*/
	private $_merchant_account_id;


	/**
	* Whether to use "production" or "sandbox" enviroment
	*
	* @var string
	*/
	private $_test_mode;


	/**
	* Response details
	*
	* @param array
	*/
	private $_response;


	/**
	* Payment nonce returned by Client SDK
	*
	* @var string
	*/
	private $_payment_method_nonce;


	/**
	* Transaction amount
	*
	* @var string
	*/
	private $_transaction_amount;


	/**
	* Whether to do AVS check when processing the transaction
	*
	* @var boolean
	*/
	private $_verify_card;


	/**
	* Braintree billing address first name
	*
	* @var string
	*/
	private $_address_first_name;


	/**
	* Braintree billing address last name
	*
	* @var string
	*/
	private $_address_last_name;


	/**
	* Braintree billing address postal code
	*
	* @var string
	*/
	private $_address_postal_code;


	/**
	* Braintree billing address street
	*
	* @var string
	*/
	private $_address_street;


	/**
	* Braintree billing address country
	*
	* @var string
	*/
	private $_address_country;


	/**
	* Constructor
	*
	* @param boolean $test_mode
	* @param string  $merchant_id
	* @param string  $public_key
	* @param string  $private_key
	* @param string  $merchant_account_id
	*
	* @return void
	* @access public
	*/
	function __construct($test_mode, $merchant_id, $public_key, $private_key, $merchant_account_id)
	{
		$this->_test_mode = $test_mode;
		$this->_merchant_id = trim($merchant_id);
		$this->_public_key = trim($public_key);
		$this->_private_key = trim($private_key);
		$this->_merchant_account_id = trim($merchant_account_id);

	}//end constructor


	/**
	* Set Brantree API lib parameters
	*
	* @return boolean
	* @access private
	*/
	private function _setupBraintreeApi()
	{
		if (!class_exists('Braintree_Configuration')) {
			trigger_error(translate('The path set to the Braintree PHP lib is not valid.'));
			return FALSE;
		}

		if (!$this->parametersSet()) {
			trigger_error(translate('Required Braintree parameters are not set.'));
			return FALSE;
		}

		Braintree_Configuration::environment($this->_test_mode ? 'sandbox' : 'production');
		Braintree_Configuration::merchantId($this->_merchant_id);
		Braintree_Configuration::publicKey($this->_public_key);
		Braintree_Configuration::privateKey($this->_private_key);

		return TRUE;

	}//end _setupBraintreeLib()


	/**
	* Generate and return a new client token
	*
	* @return string
	* @access private
	*/
	private function _getClientToken()
	{
		if (!$this->_setupBraintreeApi()) {
			return FALSE;
		}

		$response = $this->_makeRequest('generate_clinet_token');

		if (!empty($response['error'])) {
			trigger_error($response['error']);
		}

		return $response['response'];

	}//end _getClientToken()


	/**
	* Make request to the Braintree API
	*
	* @param string $function    Type of Braintree API to call
	* @param array  $parameters  Parameters to pass to the API
	*
	* @return mixed
	* @access private
	*/
	private function _makeRequest($function, $parameters=Array())
	{
		$response = NULL;
		$error = NULL;

		try {
			switch($function) {
				case 'generate_clinet_token':
					$response = Braintree_ClientToken::generate();
				break;

				case 'transaction_sale':
					$response = Braintree_Transaction::sale($parameters);
				break;

				default:
					$error = translate('Unknown request type');
				break;
			}//end switch
		} catch (Braintree_Exception_Authentication $e) {
			$error = translate('Invalid Braintree account configuration.');

		} catch (Braintree_Exception_Authorization $e) {
			$error = translate('API keys being used is not authorised to perform the attempted action.');

		} catch (Braintree_Exception_Configuration $e) {
			$error = translate('Invalid Braintree account configuration.');

		} catch (Braintree_Exception_DownForMaintenance $e) {
			$error = translate('Request to Braintree timed out.');

		} catch (Braintree_Exception_ForgedQueryString $e) {
			$error = translate('Invalid hash in the query string.');

		} catch (Braintree_Exception_ServerError $e) {
			$error = translate('Unknown error occured at Braintree when processing the transaction.');

		} catch (Braintree_Exception_SSLCertificate $e) {
			$error = translate('Client library cannot verify the server\'s SSL certificate.');

		} catch (Braintree_Exception_Unexpected $e) {
			$error = translate('Unexpected error occured.');

		} catch (Braintree_Exception_UpgradeRequired $e) {
			$error = translate('Current version of Braintree lib is no longer supported.');

		} catch (Exception $e) {
			$error = $e->getMessage();
		}

		return Array(
				'response' => $response,
				'error' => $error
			);

	}//end _makeRequest()


	/**
	* Print the payment form SDK JS
	*
	* @param string  $custom_form_id      ID of the form when using custom integration
	* @param string  $msg_container_id    ID of the container for displaying processing error message (applicable to drop-in integratoin only)
	* @param string  $dropin_container_id ID of the container for the payment form when using the drop-in integraion
	* @param string  $paypal_container_id ID of the container for PyaPal button, applicable when using custom integration
	*
	* @return void
	* @access public
	*/
	public function printClinetSdkJs($custom_form_id, $msg_container_id, $dropin_container_id='', $paypal_container_id='')
	{
		// Only either of 'Custom' or 'Drop-in' integration can be used at a time
		// If the form/container id for both are provided, then we will use 'Drop-in'
		$client_token = $this->_getClientToken();
		if ($client_token) {
			?>
			<script src="https://js.braintreegateway.com/v2/braintree.js"></script>
			<script>
			<?php
			if (!empty($dropin_container_id)) {
				// Using "Drop-in" integration
				?>
				braintree.setup('<?php echo $client_token; ?>',  'dropin', {
					container: '<?php echo $dropin_container_id; ?>',
					id: '<?php echo $custom_form_id; ?>',
					onError: function(e) {
						if (typeof(e) === 'object' && e.message !== undefined) {
							var container = document.getElementById('<?php echo $msg_container_id; ?>');
							container.innerHTML = '<p>'+e.message+'</p>';
						}
					}//end onerror
				});
				<?php
			} else {
				// Using "Custom" integration
				$paypal_container_js = '';
				if (!empty($paypal_container_id)) {
					// Setup Braintree client for PayPal button if required
					$paypal_container_js = ',
									paypal: { "container": "'.$paypal_container_id.'"}';
				}
				// Setup Braintree client for the card payment
				?>
				braintree.setup('<?php echo $client_token; ?>',  'custom', {
					id: <?php echo "'".$custom_form_id."'".$paypal_container_js ?>
				});
			<?php
			}
			?>
			</script>
			<?php
		}

	}//end printClinetSdkJs()


	/**
	* Process the transaction
	*
	* @return mixed
	* @access public
	*/
	public function process()
	{
		$this->_logRequest();

		if (!$this->_setupBraintreeApi()) {
			return FALSE;
		}

		// Make the payment request
		$request_params = Array(
							'amount' => $this->_transaction_amount,
							'merchantAccountId' => $this->_merchant_account_id,
							'paymentMethodNonce' => $this->_payment_method_nonce,
						);
		if ($this->_verify_card) {
			$request_params['billing'] = Array(
											'firstName' => $this->_address_first_name,
											'lastName' => $this->_address_last_name,
											'postalCode' => $this->_address_postal_code,
											'streetAddress' => $this->_address_street,
											'countryCodeAlpha2' => $this->_address_country,
										);
		}

		$success = FALSE;
		$this->_response = Array('status' => 'Failure');
		$response = $this->_makeRequest('transaction_sale', $request_params);
		if (!empty($response['error'])) {
			$this->_response['message'] = $response['error'];
		} else {
			$this->_response = $response['response'];
			if (isset($this->_response->success)) {
				$success = $this->_response->success;
			}
		}

		$this->_logResponse();

		return $success;

	}//end process()


	/**
	* Check if all the necessary Braintree parameters are set
	*
	* @return boolean
	* @access public
	*/
	public function parametersSet()
	{
		return $this->_merchant_id && $this->_public_key && $this->_private_key && $this->_merchant_account_id;

	}//end parametersSet()


	/**
	* Reset all the currently set transaction parameters
	*
	* @return void
	* @access public
	*/
	public function resetPaymentParameters()
	{
		$this->_payment_method_nonce = '';
		$this->_transaction_amount = '';

		$this->_address_first_name = '';
		$this->_address_last_name = '';
		$this->_address_postal_code = '';
		$this->_address_street = '';
		$this->_address_country = '';

	}//end resetPaymentParameters()


	/**
	* Return the transaction result
	*
	* @return array
	* @access public
	*/
	public function getResponse()
	{
		return $this->_response;

	}//end getResponse()


	/**
	* Set the transaction amount
	*
	* @param string $val
	*
	* @return boolean
	* @access public
	*/
	public function setTransactionAmount($val)
	{
		if (empty($val) || !is_numeric($val) || $val <= 0) {
			return FALSE;
		}
		$val = number_format((float)$val, 2, '.', '');
		$this->_transaction_amount = $val;

		return TRUE;

	}//end setTransactionAmount()


	/**
	* Set the transaction payment nonce
	*
	* @param boolean $val
	*
	* @return string
	* @access public
	*/
	public function setPaymentNonce($val)
	{
		$this->_payment_method_nonce = $val;

	}//end setPaymentNonce()


	/**
	* Set the CVV/AVS check flag
	*
	* @param boolean $val
	*
	* @return string
	* @access public
	*/
	public function setVerifyCard($val)
	{
		$this->_verify_card = $val;

	}//end setVerifyCard()


	/**
	* Set billing address first name
	*
	* @return array
	* @access public
	*/
	public function setBillingAddressFirstName($val)
	{
		$val = trim($val);
		$this->_address_first_name = $val;

		return $val;

	}//end setBillingAddressFirstName()


	/**
	* Set billing address last name
	*
	* @return array
	* @access public
	*/
	public function setBillingAddressLastName($val)
	{
		$val = trim($val);
		$this->_address_last_name = $val;

		return $val;

	}//end setBillingAddressLastName()


	/**
	* Set billing address postal code
	*
	* @return array
	* @access public
	*/
	public function setBillingAddressPostalCode($val)
	{
		$val = trim($val);
		$this->_address_postal_code = $val;

		return $val;

	}//end setBillingAddressPostalCode()


	/**
	* Set billing address street
	*
	* @return array
	* @access public
	*/
	public function setBillingAddressStreet($val)
	{
		$val = trim($val);
		$this->_address_street = $val;

		return $val;

	}//end setBillingAddressStreet()


	/**
	* Set billing address country
	* Country name should be in alpha-2 code format
	*
	* @return array
	* @access public
	*/
	public function setBillingAddressCountry($val)
	{
		$val = trim(strtolower($val));
		require SQ_FUDGE_PATH.'/standards_lists/countries.inc';
		if (!isset($standards_lists_countries[$val])) {
			return FALSE;
		}

		$this->_address_country = strtoupper($val);

		return TRUE;

	}//end setBillingAddressCountry()


	/**
	* Get to Braintree Payment Nonce value from the submitted payment form
	* This field is injected by Braintree SDK
	*
	* @return string
	* @access public
	*/
	public static function getPaymentNonce()
	{
		return isset($_REQUEST['payment_method_nonce']) ? $_REQUEST['payment_method_nonce'] : '';

	}//end getPaymentNonce()


	/**
	* Get the list of the test card numbers
	*
	* @return array
	* @access public
	*/
	public static function getTestCardNumbers()
	{
		$test_card_numbers = Array(
								'378282246310005'  => 'Success - American Express',
								'6011111111111117' => 'Success - Discover',
								'3530111333300000' => 'Success - JCB',
								'6304000000000000' => 'Success - Maestro',
								'5555555555554444' => 'Success - MasterCard',
								'4111111111111111' => 'Success - Visa',
								'378734493671000'  => 'Failure - American Express',
								'6011000990139424' => 'Failure - Discover',
								'3566002020360505' => 'Failure - JCB',
								'5105105105105100' => 'Failure - MasterCard',
								'4000111111111115' => 'Failure - Visa',
							);

		return $test_card_numbers;

	}//end getTestCardNumbers()


	/**
	* Log the Request's information
	*
	* @return void
	* @access private
	*/
	private function _logRequest()
	{
		$message = "\nRequest:\n";
		$message .= 'Time: '.date('r')."\n";
		$message .= 'Transaction Ref: '.$this->_payment_method_nonce."\n";
		$message .= 'Transaction Amount: '.$this->_transaction_amount."\n";

		$this->_log($message);

	}//end _logRequest()


	/**
	* Log the Response's information
	*
	* @return void
	* @access private
	*/
	private function _logResponse()
	{
		$status = 'unknown';
		$transaction_id = '';
		$payment_type = '';
		$amount = '';
		$currency = '';
		$message = '';
		$date = date('r');
		$credit_card = '';
		$billing_name = '';
		$billing_address = '';
		$paypal_email = '';

		if (is_object($this->_response)) {
			if (is_object($this->_response->transaction)) {
				$status = $this->_response->transaction->status;
				$transaction_id = $this->_response->transaction->id;
				$payment_type = $this->_response->transaction->paymentInstrumentType;
				$amount = $this->_response->transaction->amount;
				$currency = $this->_response->transaction->currencyIsoCode;
				$message = $this->_response->transaction->processorResponseCode.': '.$this->_response->transaction->processorResponseText;
				$date = $this->_response->transaction->createdAt->format('r');

				if (!empty($this->_response->transaction->creditCard) && !empty($this->_response->transaction->creditCard['cardType'])) {
					$cc_details = $this->_response->transaction->creditCard;
					$credit_card = 'Card type: '.$cc_details['cardType'].'; '.'Last 4 digits: '.$cc_details['last4'];
				}
				if (!empty($this->_response->transaction->billing)) {
					$billing = $this->_response->transaction->billing;
					if (!empty($billing['firstName'])) {
						$billing_name = trim($billing['firstName'].' '.$billing['lastName']);
					}
					if (!empty($billing['streetAddress'])) {
						$billing_address .= $billing['streetAddress'].', ';
					}
					if (!empty($billing['postalCode'])) {
						$billing_address .= $billing['postalCode'].', ';
					}
					if (!empty($billing['countryCodeAlpha2'])) {
						$billing_address .= $billing['countryCodeAlpha2'].', ';
					}
					$billing_address = rtrim($billing_address, ', ');
				}
				if (!empty($this->_response->transaction->paypal) && !empty($this->_response->transaction->paypal['payerEmail'])) {
					$paypal_email = $this->_response->transaction->paypal['payerEmail'];
				}

			} else {
				if ($this->_response->errors) {
					$error_objs = $this->_response->errors->deepAll();
					foreach($error_objs as $error_obj) {
						$message .= $error_obj->__get('message')." ";
					}
				}
			}
		} else if (is_array($this->_response)) {
			$status = array_get_index($this->_response, 'status', $status);
			$message = array_get_index($this->_response, 'message', $message);
		}

		// Re-format our response data
		$this->_response = Array(
							'status' => $status,
							'braintree_ref' => $transaction_id,
							'payment_type' => $payment_type,
							'amount' => $amount,
							'currency' => $currency,
							'date' => $date,
							'message' => $message,
				);
		if (!empty($credit_card)) {
			$this->_response['credit_card'] = $credit_card;
		}
		if (!empty($billing_name)) {
			$this->_response['billing_name'] = $billing_name;
		}
		if (!empty($billing_address)) {
			$this->_response['billing_address'] = $billing_address;
		}
		if (!empty($paypal_email)) {
			$this->_response['paypal_email'] = $paypal_email;
		}

		$log_message = "\nResponse:\n";
		foreach($this->_response as $key => $val) {
			$log_message .= ucwords(str_replace('_', ' ', $key)).': '.$val."\n";
		}

		$this->_log($log_message);

	}//end _logResponse()


	/**
	* Log a message to the log file of the $this->_client DPS account
	*
	* @param string	$message	the message to log
	* @param int	$level		the error level of the message
	* @param bolean	$encode		the flag indicates if the message needs to be encoded
	*
	* @return void
	* @access private
	*/
	private function _log($message, $level = E_USER_NOTICE, $encode=FALSE)
	{
		$log_file = BRAINTREE_LOG_FILE.($this->_test_mode ? '_test' : '_live');
		log_write($message, $log_file, $level, $encode);

	}//end log()


}//end class

?>
