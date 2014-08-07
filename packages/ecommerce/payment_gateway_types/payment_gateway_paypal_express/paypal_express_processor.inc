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
* $Id: paypal_express_processor.inc,v 1.1 2013/06/20 07:40:28 cupreti Exp $
*
*/


define('PAYPAL_EXPRESS_LOG_FILE', 'paypal_express_transactions');

/**
* Paypal_Express_Processor
*
* Paypal Express payment processing object
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Paypal_Express_Processor
{

	/**
	* Supported currency types
	*
	* @var array
	*/
	private static $_currency_types = Array(
										'GBP' => 'GBP',
										'AUD' => 'AUD',
										'USD' => 'USD',
										'EUR' => 'EUR',
										'JPY' => 'JPY',
										'CAD' => 'CAD',
									);

	/**
	* Live and test server address for PayPal Express Checkout
	*
	* @var array
	*/
	private $_servers = Array(
						'live' => 'https://api-3t.paypal.com/nvp',
						'sandbox' => 'https://api-3t.sandbox.paypal.com/nvp',
					);


	/**
	* Live and test server address for PayPal login pages
	*
	* @var array
	*/
	private $_paypal_login = Array(
								'live' => 'https://www.paypal.com/cgi-bin/webscr',
								'sandbox' => 'https://www.sandbox.paypal.com/cgi-bin/webscr',
							);

	/**
	* Response details
	*
	* @param array
	*/
	private $_response;


	/**
	* Paypal transaction token
	*
	* @var string
	*/
	private $_transaction_token;


	/**
	* Transaction amount
	*
	* @var string
	*/
	private $_transaction_amount;


	/**
	* Transaction currency
	*
	* @var string
	*/
	private $_transaction_currency;


	/**
	* Transaction parameter list
	*
	* @var string
	*/
	private $_parm_list;


	/**
	* Request timeout value in seconds
	*
	* @var string
	*/
	private $_timeout;


	/**
	* Request timeout value in seconds
	*
	* @var string
	*/
	private $_test_mode;


	/**
	* API action method type
	*
	* @var string
	*/
	private $_action_method;


	/**
	* Constructor
	* @param boolean $test_mode
	*/
	function __construct($test_mode)
	{
		// PayPal API to use is hardcoded here
     	$this->_parm_list = 'VERSION=89.0';

		// Transaction token is obtained after first request 'SetExpressCheckout' to PayPal
		$this->_transaction_token = '';

		// Default connection timeout value
		$this->_timeout = '60';

		$this->_response = Array();

		if ($test_mode) {
			$this->_test_mode = TRUE;
		} else {
			$this->_test_mode = FALSE;
		}

	}//end constructor


	/**
	* Process the transaction
	*
	* @return mixed
	* @access public
	*/
	public function process()
	{
		$paypal_server = $this->_test_mode ? $this->_servers['sandbox'] : $this->_servers['live'];

		$request_time = time();
		// Set curl headers and options
		$curl_options = Array(
						'TIMEOUT'			=> $this->_timeout,
						'FOLLOWLOCATION' 	=> 0,
						'RETURNTRANSFER'	=> 1,
						'SSL_VERIFYPEER' 	=> 1,
						'SSL_VERIFYHOST'	=> 2,
						'POST'				=> 1,
						'POSTFIELDS'		=> $this->_parm_list,
						'FORBID_REUSE'		=> 1,
						'USERAGENT'			=> $_SERVER['HTTP_USER_AGENT'],
					);
		$result = fetch_url($paypal_server, $curl_options);

		if (!empty($result['response'])) {
			$response = urldecode($result['response']);
			$this->_response['RESPONSE_STR'] = $response;
			$name_value = explode('&', $response);
			foreach($name_value as $pair) {
				if (strpos($pair, '=') === FALSE) continue;
				list($key, $val) = explode('=', $pair);
				$this->_response[$key] = $val;
			}//end foreach
		}

		$ack = array_get_index($this->_response, 'ACK', '');
		$response_time = array_get_index($this->_response, 'TIMESTAMP', date('c'));

		$this->_response['TIME'] = date('r', strtotime($response_time));

		$this->_logRequest($request_time);
		$this->_logResponse();

		$this->_response['STATUS'] = $ack;
		$this->_response['AMOUNT'] = array_get_index($this->_response, 'PAYMENTINFO_0_AMT', 'n/a');
		$this->_response['CURRENCY'] = array_get_index($this->_response, 'PAYMENTINFO_0_CURRENCYCODE', 'n/a');
		$this->_response['TRANSACTION'] = array_get_index($this->_response, 'PAYMENTINFO_0_TRANSACTIONID', 'n/a');
		$this->_response['PAYMENT_STATUS'] = array_get_index($this->_response, 'PAYMENTINFO_0_PAYMENTSTATUS', 'n/a');

		// If any error in response
		$this->setResponseErrorMessage();

		return $ack == 'Success';

	}//end process()


	/**
	* Setup error message in the response array, if any
	*
	* @return void
	* @accesss private
	*/
	private function setResponseErrorMessage()
	{
		$error_msg = '';
		$error_index = 0;
		while (isset($this->_response['L_ERRORCODE'.$error_index])) {

			$error_msg .= 'PayPal Error #'.$this->_response['L_ERRORCODE'.$error_index].': ';
			$error_msg .= $this->_response['L_SHORTMESSAGE'.$error_index];
			$error_msg .= $this->_response['L_LONGMESSAGE'.$error_index];

			$error_index++;
			if (isset($this->_response['L_ERRORCODE'.$error_index])) {
				$error_msg .= '<br/>';
			}
		}

		$this->_response['PAYPAL_ERROR'] = $error_msg;

		if (array_get_index($this->_response, 'ACK', '') != 'Success' && empty($error_msg)) {
			$this->_response['PAYPAL_ERROR'] = 'Unknown error occured at PayPal when processing the transaction';
		}

	}//end setResponseErrorMessage()


	/**
	* After getting the "transaction token" redirect to Paypal
	*
	* @param $transaction_token
	*
	* @return void
	* @access public
	*/
	public function redirectToPaypal($transaction_token)
	{
		$paypal_url = ($this->_paypal_login[$this->_test_mode ? 'sandbox' : 'live']).'?cmd=_express-checkout&token='.$transaction_token;
		header('Location: '.$paypal_url);

		exit;

	}//end redirectToPaypal()


	/**
	* Return the transaction result
	*
	* @return array
	* @access public
	*/
	public function getResponse()
	{
		return $this->_response;

	}//end _getResponse


	/**
	* Set API Username
	*
	* @param string $val
	*
	* @return string
	* @access public
	*/
	public function setApiUsername($val)
	{
		$this->_parm_list .= '&USER='.urlencode($val);

	}//end setApiUsername()


	/**
	* Set API Username
	*
	* @param string $val
	*
	* @return string
	* @access public
	*/
	public function setApiPassword($val)
	{
		$this->_parm_list .= '&PWD='.urlencode($val);

	}//end setApiPassword()


	/**
	* Set API Signature
	*
	* @param string $val
	*
	* @return string
	* @access public
	*/
	public function setApiSignature($val)
	{
		$this->_parm_list .= '&SIGNATURE='.urlencode($val);

	}//end setApiSignature()


	/**
	* Set the transaction amount
	*
	* @param string $val
	*
	* @return string
	* @access public
	*/
	public function setTransactionAmount($val)
	{
		// Needs to be in two decimal place format
		// For example amount '10' should be '10.00'
		$val = number_format((float)$val, 2, '.', '');
		$this->_transaction_amount = $val;

		$this->_parm_list .= '&PAYMENTREQUEST_0_AMT='.$val;

	}//end setTransactionAmount()


	/**
	* Set the currency
	*
	* @param string $val
	*
	* @return string
	* @access public
	*/
	public function setTransactionCurrency($val)
	{
		if (!isset(self::$_currency_types[$val])) {
			$val = 'AUD';
		}

		$this->_transaction_currency = $val;

		$this->_parm_list .= '&PAYMENTREQUEST_0_CURRENCYCODE='.$val;

	}//end setTransactionCurrency()


	/**
	* Set return success url
	*
	* @param string $val
	*
	* @return string
	* @access public
	*/
	public function setReturnUrlSuccess($val)
	{
		$this->_parm_list .= '&RETURNURL='.urlencode($val);

	}//end setReturnUrlSuccess()


	/**
	* Set return cancel url
	*
	* @param string $val
	*
	* @return string
	* @access public
	*/
	public function setReturnUrlCancel($val)
	{
		$this->_parm_list .= '&CANCELURL='.urlencode($val);

	}//end setReturnUrlCancel()


	/**
	* Set API paymeny action
	*
	* @param string $val
	*
	* @return string
	* @access public
	*/
	public function setPaymentAction($val)
	{
		$this->_parm_list .= '&PAYMENTREQUEST_0_PAYMENTACTION='.$val;

	}//end setPaymentAction()


	/**
	* Set API transaction method
	*
	* @param string $val
	*
	* @return string
	* @access public
	*/
	public function setTransactionMethod($val)
	{
		$this->_parm_list .= '&METHOD='.$val;

	}//end setTransactionMethod()


	/**
	* Set transaction token
	*
	* @param string $val
	*
	* @return string
	* @access public
	*/
	public function setTransactionToken($val)
	{
		$this->_transaction_token = $val;
		$this->_parm_list .= '&TOKEN='.urlencode($val);

	}//end setTransactionToken()


	/**
	* Set payer id
	*
	* @param string $val
	*
	* @return string
	* @access public
	*/
	public function setPayerId($val)
	{
		$this->_parm_list .= '&PAYERID='.urlencode($val);

	}//end setPayerId()


	/**
	* Set the connection timeout
	*
	* @param string $val
	*
	* @return string
	* @access public
	*/
	public function setRequestTimeout($val)
	{
		if (is_numeric($val)) {
			$this->_timeout = $val;
		}

	}//end setRequestTimeout()


	/**
	* Get the list of the supported currecy types
	*
	* @return array
	* @access public
	*/
	public static function getCurrencyTypes()
	{
		return self::$_currency_types;

	}//end getCurrencyTypes()


	/**
	* Log the Request's information
	*
	* @param string $request_time
	*
	* @return void
	* @access private
	*/
	private function _logRequest($request_time)
	{
		$method = (strpos($this->_parm_list, 'METHOD=DoExpressCheckoutPayment') !== FALSE) ? 'DoExpressCheckoutPayment' : 'SetExpressCheckout';

		$message = "\nRequest:\n";
		$message .= 'Method: '.$method."\n";
		$message .= 'Time: '.date('r', $request_time)."\n";
		$message .= 'Transaction Token: '.$this->_transaction_token."\n";
		$message .= 'Amount: '.$this->_transaction_amount."\n";
		$message .= 'Currency: '.$this->_transaction_currency."\n";
		$message .= 'Resquest String: '.$this->_parm_list."\n";

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
		$method = (strpos($this->_parm_list, 'METHOD=DoExpressCheckoutPayment') !== FALSE) ? 'DoExpressCheckoutPayment' : 'SetExpressCheckout';

		$message = "\nResponse:\n";
		$message .= 'Method: '.$method."\n";
		$message .= 'Time: '.array_get_index($this->_response, 'DATE', date('r'))."\n";
		$message .= 'Token: '.array_get_index($this->_response, 'TOKEN', '')."\n";
		$message .= 'Correlation Id: '.array_get_index($this->_response, 'CORRELATIONID', '')."\n";
		$message .= 'Request Status: '.array_get_index($this->_response, 'ACK', '')."\n";
		$message .= 'Amount: '.array_get_index($this->_response, 'AMOUNT', 'n/a')."\n";
		$message .= 'Currency: '.array_get_index($this->_response, 'CURRENCY', 'n/a')."\n";
		$message .= 'Transaction Id: '.array_get_index($this->_response, 'TRANSACTION', 'n/a')."\n";
		$message .= 'Payment Status: '.array_get_index($this->_response, 'PAYMENT_STATUS', 'n/a')."\n";
		$message .= 'Response String: '.array_get_index($this->_response, 'RESPONSE_STR', '')."\n";

		$this->_log($message);

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
		$log_file = PAYPAL_EXPRESS_LOG_FILE.($this->_test_mode ? '_test' : '_live');
		log_write($message, $log_file, $level, $encode);

	}//end log()


}//end class

?>
