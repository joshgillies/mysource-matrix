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
* $Id: payflow_processor.inc,v 1.1 2013/03/18 04:13:10 cupreti Exp $
*
*/


define('PAYFLOW_LOG_FILE', 'payflow_transactions');

/**
* Payflow_Processor
*
* UTransact payment processing object
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Payflow_Processor
{

	/**
	* List of all card types.
	*
	* NOTE: Basically the PayPal Payflow gateway does not checks for the card type,
	* and if customer uses the card that merchant is not signed up to accept,
	* Payflow will return value 23, "Invalid accound number" error. So if any card
	* type that Merchant is signed with is not listed here, needs be added later,
	* though, specifying the card type will have no affect on the transaction outcome
	*
	* @var array
	*/
	private static $_card_types =  Array(
							'AMEX'		=> 'American Express',
							'AMEXC'		=> 'American Express Corporate',
							'DC'		=> 'Diners Club',
							'DELTA'		=> 'Visa Delta',
							'DISCOVER'	=> 'Discover',
							'JCB'		=> 'JCB',
							'LASER'		=> 'Laser',
							'MAESTRO'	=> 'Maestro',
							'MC'		=> 'MasterCard',
							'SOLO'		=> 'Solo',
							'UKE'		=> 'Visa Electron',
							'VISA'		=> 'Visa',
						);


	/**
	* Test card numbers
	*
	* @var array
	*/
	private static $_test_card_numbers = Array (
											'378282246310005' 	=> 'American Express Test',
											'378734493671000' 	=> 'American Express Corporate Test',
											'30569309025904' 	=> 'Diners Club Test',
											'6011111111111117'	=> 'Discover Test',
											'5555555555554444' 	=> 'MasterCard',
											'4111111111111111'	=> 'Visa',
											'333' 				=> 'Invalid Card Test',
										);

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
	* Live and test host address for Payflow
	*
	* @var array
	*/
	private $_hosts = Array(
						'live' => 'https://payflowpro.paypal.com',
						'test' => 'https://pilot-payflowpro.paypal.com',
					);


	/**
	* Response details
	*
	* @param array
	*/
	private $_response;


	/**
	* Transaction reference number
	*
	* @var string
	*/
	private $_reference_no;


	/**
	* Transaction amount
	*
	* @var string
	*/
	private $_transaction_amount;


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
	* Constructor
	* @param boolean $test_mode
	*/
	function __construct($test_mode)
	{
		// Set three required parameters supported by the Matrix Payflow payment gateway
		//
		// TRXTYPE=S (Type of transaction to perform is "Sale transaction"
		// TENDER=C (Method of payment is "Credit card"
     	$this->_parm_list = 'TRXTYPE[1]=S&TENDER[1]=C';

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
		$payflow_url = $this->_test_mode ? $this->_hosts['test'] : $this->_hosts['live'];

		// Set curl headers and options
		$headers = array();
		$headers[] = "Content-Type: text/namevalue";
		$headers[] = "Content-Length: ".strlen($this->_parm_list);
		$headers[] = "X-VPS-Timeout: 60";
		$headers[] = "X-VPS-VIT-Client-Type: PHP/cURL";
		$headers[] = "X-VPS-Request-ID: ".md5($this->_reference_no);
		$curl_options = Array(
						'TIMEOUT'			=> $this->_timeout,
						'FOLLOWLOCATION' 	=> 0,
						'RETURNTRANSFER'	=> 1,
						'SSL_VERIFYPEER' 	=> 0,
						'PORT'				=> 443,
						'POST'				=> 1,
						'POSTFIELDS'		=> $this->_parm_list,
						'SSL_VERIFYHOST'	=> 2,
						'FORBID_REUSE'		=> 1,
						'HTTPHEADER'		=> $headers,
						'USERAGENT'			=> $_SERVER['HTTP_USER_AGENT'],
					);
		$result = fetch_url($payflow_url, $curl_options);

		if (!empty($result['response'])) {
			$this->_response['RESPONSE_STR'] = $result['response'];
			$name_value = explode('&', $result['response']);
			foreach($name_value as $pair) {
				if (strpos($pair, '=') === FALSE) continue;
				list($key, $val) = explode('=', $pair);
				$this->_response[$key] = $val;
			}//end foreach
		}

		$payflow_ref = array_get_index($this->_response, 'PNREF', '');
		$message = array_get_index($this->_response, 'RESPMSG', 'No response returned from gateway');
		$status = array_get_index($this->_response, 'RESULT', '');

		$this->_response['MERCHANT_REFERENCE'] = $this->_reference_no;
		$this->_response['PAYFLOW_REFERENCE'] = $payflow_ref;
		$this->_response['STATUS'] = $status;
		$this->_response['MESSAGE'] = $message;
		$this->_response['TIME'] = date('r');

		$this->_logRequest();
		$this->_logResponse();

		// '0' means transaction approved
		return $status == 0;

	}//end process()


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
	* Set the merchant reference value
	*
	* @param string $val
	*
	* @return string
	* @access public
	*/
	public function setMerchantRef($val)
	{
		$this->_reference_no = $val;
		$this->_parm_list .= '&COMMENT1['.strlen($val).']='.$val;

	}//end setMerchantRef()


	/**
	* Set an extra optional merchant reference value
	*
	* @param string $val
	*
	* @return string
	* @access public
	*/
	public function setOptionalMerchantRef($val)
	{
		if (empty($val)) {
			return $val;
		}

		$order_assetid = isset($_SESSION['sq_local_cart_contents']['pending_order_id']) ? $_SESSION['sq_local_cart_contents']['pending_order_id'] : '';
		// Replace the order asset keywords in the reference text if any
		if (extract_keywords($val) && $order_assetid && $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($order_assetid)) {
			$order = $GLOBALS['SQ_SYSTEM']->am->getAsset($order_assetid);
			$val = $order->replaceKeywordsInString($val);
			replace_global_keywords($val);
		}

		$this->_parm_list .= '&COMMENT2['.strlen($val).']='.$val;

	}//end setOptionalMerchantRef()


	/**
	* Set the account user
	*
	* @param string $val
	*
	* @return string
	* @access public
	*/
	public function setAccountUser($val)
	{
		$this->_parm_list .= '&USER['.strlen($val).']='.$val;

	}//end setAccountUser()


	/**
	* Set the account vendor
	*
	* @param string $val
	*
	* @return string
	* @access public
	*/
	public function setAccountVendor($val)
	{
		$this->_parm_list .= '&VENDOR['.strlen($val).']='.$val;

	}//end setAccountVendor()


	/**
	* Set the account vendor
	*
	* @param string $val
	*
	* @return string
	* @access public
	*/
	public function setAccountPartner($val)
	{
		$this->_parm_list .= '&PARTNER['.strlen($val).']='.$val;

	}//end setAccountPartner()


	/**
	* Set the account password
	*
	* @param string $val
	*
	* @return string
	* @access public
	*/
	public function setAccountPassword($val)
	{
		$this->_parm_list .= '&PWD['.strlen($val).']='.$val;

	}//end setAccountPassword()


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

		$this->_parm_list .= '&AMT['.strlen($val).']='.$val;

	}//end setTransactionAmount()


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

		$this->_parm_list .= '&CURRENCY['.strlen($val).']='.$val;

	}//end setTransactionCurrency()


	/**
	* Set the card number
	*
	* @param string $val
	*
	* @return string
	* @access public
	*/
	public function setCardNumber($val)
	{
		$this->_parm_list .= '&ACCT['.strlen($val).']='.$val;

	}//end setCardNumber()


	/**
	* Set the card expiry date
	*
	* @param string $month
	* @param string $year
	*
	* @return string
	* @access public
	*/
	public function setCardExpiryDate($month, $year)
	{
		// Card expiry date needs to be in MMYY format
		$month = str_pad($month, 2, '0', STR_PAD_LEFT);
		$this->_parm_list .= '&EXPDATE['.strlen($month.$year).']='.$month.$year;

	}//end setCardExpiryDate()


	/**
	* Set the card CVV2 value
	*
	* @param string $val
	*
	* @return string
	* @access public
	*/
	public function setCardCV2($val)
	{
		$this->_parm_list .= '&CV2['.strlen($val).']='.$val;

	}//end setCardCV2()


	/**
	* Set the card's billing address first name
	*
	* @param string $val
	*
	* @return string
	* @access public
	*/
	public function setCardFirstName($val)
	{
		$this->_parm_list .= '&FIRSTNAME['.strlen($val).']='.$val;

	}//end setCardFirstName()


	/**
	* Set the card's billing address last name
	*
	* @param string $val
	*
	* @return string
	* @access public
	*/
	public function setCardLastName($val)
	{
		$this->_parm_list .= '&LASTNAME['.strlen($val).']='.$val;

	}//end setCardLastName()


	/**
	* Set the card's billing address street
	*
	* @param string $val
	*
	* @return string
	* @access public
	*/
	public function setStreet($val)
	{
		$this->_parm_list .= '&STREET['.strlen($val).']='.$val;

	}//end setStreet()


	/**
	* Set the card's billing address postcode
	*
	* @param string $val
	*
	* @return string
	* @access public
	*/
	public function setPostcode($val)
	{
		$this->_parm_list .= '&ZIP['.strlen($val).']='.$val;

	}//end setPostcode()


	/**
	* Set the card's billing address city
	*
	* @param string $val
	*
	* @return string
	* @access public
	*/
	public function setCity($val)
	{
		$this->_parm_list .= '&CITY['.strlen($val).']='.$val;

	}//end setCity()


	/**
	* Set the card's billing address state
	*
	* @param string $val
	*
	* @return string
	* @access public
	*/
	public function setState($val)
	{
		$this->_parm_list .= '&STATE['.strlen($val).']='.$val;

	}//end setState()


	/**
	* Set the card's billing address country
	*
	* @param string $val
	*
	* @return string
	* @access public
	*/
	public function setCountry($val)
	{
		$this->_parm_list .= '&BILLTOCOUNTRY['.strlen($val).']='.strtoupper($val);

	}//end setCountry()


	/**
	* Get the list of the test card numbers
	*
	* @return array
	* @access public
	*/
	public static function getTestCardNumbers()
	{
		return self::$_test_card_numbers;

	}//end getTestCardNumbers()


	/**
	* Get the list of the card types
	*
	* @return array
	* @access public
	*/
	public static function getCardTypes()
	{
		return self::$_card_types;

	}//end getCardTypes()


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
	* @return void
	* @access private
	*/
	private function _logRequest()
	{
		$message = "\nRequest:\n";
		$message .= 'Time: '.date('r')."\n";
		$message .= 'Transaction Ref: '.$this->_reference_no."\n";
		 if (preg_match('/ACCT\[[0-9]+\]=([0-9]+)/', $this->_parm_list, $match)) {
			$message .= 'Card number: '.str_repeat('*', strlen($match[1])-4).substr($match[1], -4)."\n";
		} else {
			$message .= 'Card number: Not supplied'."\n";
		}
		$message .= 'Amount: '.$this->_transaction_amount."\n";
		$message .= 'Currency: '.array_get_index($this->_response, 'CURRENCY', '')."\n";
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
		$message = "\nResponse:\n";
		$message .= 'Transaction Ref: '.array_get_index($this->_response, 'MERCHANT_REFERENCE', '')."\n";
		$message .= 'Payflow Ref: '.array_get_index($this->_response, 'PAYFLOW_REFERENCE', '')."\n";
		$message .= 'Time: '.array_get_index($this->_response, 'DATE', date('r'))."\n";
		$message .= 'Status: '.array_get_index($this->_response, 'STATUS', '')."\n";
		$message .= 'Message: '.array_get_index($this->_response, 'MESSAGE', '')."\n";
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
		$log_file = PAYFLOW_LOG_FILE.($this->_test_mode ? '_test' : '_live');
		log_write($message, $log_file, $level, $encode);

	}//end log()


}//end class

?>
