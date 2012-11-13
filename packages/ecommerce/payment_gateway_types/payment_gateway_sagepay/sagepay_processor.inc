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
* $Id: sagepay_processor.inc,v 1.5 2012/11/13 00:09:48 ewang Exp $
*
*/


define('SAGEPAY_LOG_FILE_PREFIX', 'sagepay_transactions_account_');


/**
* SagePay_Processor
*
* Sage Pay payment gateway processing object
*
* @author  Anh Ta <ata@squiz.co.uk>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class SagePay_Processor
{
	
	
	/**
	 * This array stores the URLs of the transaction destinations
	 * 
	 * @var array
	 */
	private static $_URLS = Array(
									'Live' 		=> Array(
															'payment_registration'	=> 'https://live.sagepay.com/gateway/service/vspdirect-register.vsp',
															'threeds_callback'		=> 'https://live.sagepay.com/gateway/service/direct3dcallback.vsp',
														),
									'Test' 		=> Array(
															'payment_registration'	=> 'https://test.sagepay.com/gateway/service/vspdirect-register.vsp',
															'threeds_callback'		=> 'https://test.sagepay.com/gateway/service/direct3dcallback.vsp',
														),
									'Simulator' => Array(
															'payment_registration'	=> 'https://test.sagepay.com/simulator/VSPDirectGateway.asp',
															'threeds_callback'		=> 'https://test.sagepay.com/simulator/VSPDirectCallback.asp',
														),
								 );
	
	/**
	 * The urlencoded string to send to SagePay Direct Payment URL in a HTTPS POST request 
	 * @var string
	 */
	private $_post_string;
	
	/**
	 * An array that points to the list of URLs to send the POST request to. It is either Live, Test or Simulator sub-array
	 * of the above $_URLS array
	 * @var array
	 */
	private $_post_urls;
	
	/**
	 * The merchant reference number to identify a transaction
	 * @var string
	 */
	private $_merchant_ref;
	
	/**
	 * The last 4 digits of the card number used in a transaction
	 * @var string
	 */
	private $_card_number;
	
	/**
	 * The card type of the card used in a transaction
	 * @var string
	 */
	private $_card_type;
	
	/**
	 * A transaction type that can be PAYMENT, AUTHENTICATE, CANCEL (SagePay types) or 3-D Secure Authorization Callback (just for logging)
	 * @var string
	 */
	private $_tx_type;
	
	/**
	 * The amount of money used in a transaction
	 * @var string
	 */
	private $_amount;
	
	/**
	 * The currency that used in a transaction
	 * @var string
	 */
	private $_currency;
	
	/**
	 * The gift aid payment of this transaction
	 * @var string
	 */
	private $_gift_aid = 'no';
	
	/**
	 * The time to wait for a SagePay response
	 * @var int
	 */
	private $_timeout;
	
	/**
	 * The name of the log file that stores transaction information. This name is built from the prefix SAGEPAY_LOG_FILE_PREFIX and the Vendor name
	 * @var string
	 */
	private $_log_file_name;
	
	/**
	 * The Response that is received from SagePay Payment Gateway
	 * @var Array
	 */
	private $_response = NULL;
	
	/**
	 * The flag indicates whether a transaction succeeds or fails
	 * @var boolean
	 */
	private $_success = FALSE;
	
	/**
	 * The list of SagePay magic test card numbers to use in the Test Server
	 * @var array
	 */
	private static $_TEST_CARD_NUMBERS = Array(
						'4929000000006'			=> 'Visa',
						'4462000000000003'		=> 'Visa Delta',
						'4917300000000008'		=> 'Visa Electron',
						'5404000000000001'		=> 'MasterCard',
						'300000000000000004'	=> 'Maestro',
						'5641820000000005'		=> 'Switch (UK Maestro)',
						'6334900000000005'		=> 'Solo',
						'374200000000004'		=> 'American Express',
						'36000000000008'		=> 'Diners Club',
						'3569990000000009'		=> 'JCB',
						'6304990000000000044'	=> 'Laser',
					);
					
	/**
	 * The list of card types that SagePay supports
	 * @var array
	 */
	private static $_CARD_TYPES = Array(
						'VISA'		=> 'Visa',
						'DELTA'		=> 'Visa Delta',
						'UKE'		=> 'Visa Electron',
						'MC'		=> 'MasterCard',
						'MAESTRO'	=> 'Maestro',
						'SOLO'		=> 'Solo',
						'AMEX'		=> 'American Express',
						'DC'		=> 'Diners Club',
						'JCB'		=> 'JCB',
						'LASER'		=> 'Laser',
					);
	
	
	/**
	 * Constructor
	 * 
	 * @param string	$protocol		the protocol version of SagePay Direct Protocol
	 * @param string	$vendor			the vendor name of a SagePay account
	 * @param int		$timeout		the time to wait for a response from SagePay Payment Gateway
	 * @param string	$run_mode		specify if this processor is in live,test or simulator mode; default is simulator mode
	 * 
	 */
	function __construct($protocol, $vendor, $timeout = 60, $run_mode = 'Simulator')
	{
		$this->_post_string = "VPSProtocol=$protocol&Vendor=$vendor";
		$this->_timeout = $timeout;
		$this->_post_urls = self::$_URLS[$run_mode];
		$this->_log_file_name = SAGEPAY_LOG_FILE_PREFIX.$vendor.'_'.strtolower($run_mode);
		
	}//end constructor
	
	
	/**
	* Set card name
	*
	* @param string	$card_name	the card name
	*
	* @return void
	* @access public
	*/
	public function setCardName($card_name)
	{
		$this->_post_string .= '&CardHolder='.urlencode($card_name);
		
	}//end setCardName()
	
	
	/**
	* Set card number
	*
	* @param string	$card_no	the card number
	*
	* @return void
	* @access public
	*/
	public function setCardNumber($card_no)
	{
		//get the last 4 digits of card numbers for logging later
		$this->_card_number = substr($card_no, -4);
		$this->_post_string .= "&CardNumber=$card_no";
		
	}//end setCardNumber()
	
	
	/**
	* Set expiry date
	*
	* @param string	$month	the expiry month
	* @param string	$year	the expiry year
	*
	* @return void
	* @access public
	*/
	public function setCardExpiryDate($month, $year)
	{
		$this->_post_string .= "&ExpiryDate=$month$year";
		
	}//end setCardExpiryDate()
	
	
	/**
	* Set start date
	*
	* @param string	$month	the start month
	* @param string	$year	the start year
	*
	* @return void
	* @access public
	*/
	public function setCardStartDate($month, $year)
	{
		$this->_post_string .= "&StartDate=$month$year";
		
	}//end setCardStartDate()
	
	
	/**
	* Set card issue number
	*
	* @param string	$issue_no	the card issue number
	*
	* @return void
	* @access public
	*/
	public function setCardIssueNumber($issue_no)
	{
		$this->_post_string .= "&IssueNumber=$issue_no";
		
	}//end setCardIssueNumber()
	
	
	/**
	* Set card CV2
	*
	* @param string	$cv2	the card CV2
	*
	* @return void
	* @access public
	*/
	public function setCardCV2($cv2)
	{
		$this->_post_string .= "&CV2=$cv2";
		
	}//end setCardCV2()
	
	
	/**
	* Set transaction type to PAYMENT
	*
	* @return void
	* @access public
	*/
	public function setTransactionTypePayment()
	{
		$this->_tx_type = 'PAYMENT';
		$this->_post_string .= "&TxType=PAYMENT";
		
	}//end setTransactionTypePayment()
	
	
	/**
	* Set merchant reference
	*
	* @param string	$merchant_ref	the merchant reference number
	*
	* @return void
	* @access public
	*/
	public function setMerchantReference($merchant_ref)
	{
		$this->_merchant_ref = $this->_getUniqueReferenceNumber($merchant_ref);
		$this->_post_string .= '&VendorTxCode='.$this->_merchant_ref;
		
	}//end setMerchantReference()
	
	
	/**
	* Set amount of transaction
	*
	* @param string	$amount		the amount of the transaction
	* @param string	$currency	the currency of the amount
	*
	* @return void
	* @access public
	*/
	public function setAmount($amount, $currency)
	{
		$this->_amount = sprintf('%01.2f',$amount);
		$this->_currency = $currency;
		$this->_post_string .= '&Amount='.$this->_amount."&Currency=$currency";
		
	}//end setAmount()
	

	/**
	* Set description of transaction
	*
	* @param string	$desc	the description of transaction
	*
	* @return void
	* @access public
	*/
	public function setDescription($desc)
	{
		$this->_post_string .= '&Description='.urlencode($desc);
		
	}//end setDescription()
	
	
	/**
	* Set card type
	*
	* @param string	$card_type	the card type
	*
	* @return void
	* @access public
	*/
	public function setCardType($card_type)
	{
		$this->_card_type = $card_type;
		$this->_post_string .= "&CardType=$card_type";
		
	}//end setCardType()
	
	
	/**
	* Set billing first names
	*
	* @param string	$first_names	the customer's first names
	*
	* @return void
	* @access public
	*/
	public function setBillingFirstnames($first_names)
	{
		$this->_post_string .= '&BillingFirstnames='.urlencode($first_names);
		
	}//end setBillingFirstnames()
	

	/**
	* Set billing surname
	*
	* @param string	$first_names	the customer's surname
	*
	* @return void
	* @access public
	*/
	public function setBillingSurname($surname)
	{
		$this->_post_string .= '&BillingSurname='.urlencode($surname);
		
	}//end setBillingSurname()
	

	/**
	* Set billing address 1
	*
	* @param string	$address	the first line of the billing address
	*
	* @return void
	* @access public
	*/
	public function setBillingAddress1($address)
	{
		$this->_post_string .= '&BillingAddress1='.urlencode($address);
		
	}//end setBillingAddress1()

	
	/**
	* Set billing address 2
	*
	* @param string	$address	the second line of the billing address
	*
	* @return void
	* @access public
	*/
	public function setBillingAddress2($address)
	{
		$this->_post_string .= '&BillingAddress2='.urlencode($address);
		
	}//end setBillingAddress2()

	
	/**
	* Set billing city
	*
	* @param string	$city	the city component of the billing address
	*
	* @return void
	* @access public
	*/
	public function setBillingCity($city)
	{
		$this->_post_string .= '&BillingCity='.urlencode($city);
		
	}//end setBillingCity()
	

	/**
	* Set billing postcode
	*
	* @param string	$postcode	the postcode component of the billing address
	*
	* @return void
	* @access public
	*/
	public function setBillingPostcode($postcode)
	{
		$this->_post_string .= '&BillingPostCode='.urlencode($postcode);
		
	}//end setBillingPostcode()

	
	/**
	* Set billing country
	*
	* @param string	$address	the country component of the billing address
	*
	* @return void
	* @access public
	*/
	public function setBillingCountry($country)
	{
		$this->_post_string .= '&BillingCountry='.urlencode($country);
		
	}//end setBillingCountry()

	
	/**
	* Set billing state
	*
	* @param string	$state	the state component of the billing address if the country is the United States
	*
	* @return void
	* @access public
	*/
	public function setBillingState($state)
	{
		$this->_post_string .= '&BillingState='.urlencode($state);
		
	}//end setBillingState()
	
	
	/**
	* Set billing phone
	*
	* @param string	$phone_no	the phone number at the billing address
	*
	* @return void
	* @access public
	*/
	public function setBillingPhone($phone_no)
	{
		$this->_post_string .= '&BillingPhone='.urlencode($phone_no);
		
	}//end setBillingPhone()

	
	/**
	* Set delivery first names
	*
	* @param string	$first_names	the customer's first names
	*
	* @return void
	* @access public
	*/
	public function setDeliveryFirstnames($first_names)
	{
		$this->_post_string .= '&DeliveryFirstnames='.urlencode($first_names);
		
	}//end setDeliveryFirstnames()
	

	/**
	* Set delivery surname
	*
	* @param string	$first_names	the customer's surname
	*
	* @return void
	* @access public
	*/
	public function setDeliverySurname($surname)
	{
		$this->_post_string .= '&DeliverySurname='.urlencode($surname);
		
	}//end setDeliverySurname()
	

	/**
	* Set delivery address 1
	*
	* @param string	$address	the first line of the delivery address
	*
	* @return void
	* @access public
	*/
	public function setDeliveryAddress1($address)
	{
		$this->_post_string .= '&DeliveryAddress1='.urlencode($address);
		
	}//end setDeliveryAddress1()

	
	/**
	* Set delivery address 2
	*
	* @param string	$address	the second line of the delivery address
	*
	* @return void
	* @access public
	*/
	public function setDeliveryAddress2($address)
	{
		$this->_post_string .= '&DeliveryAddress2='.urlencode($address);
		
	}//end setDeliveryAddress2()

	
	/**
	* Set delivery city
	*
	* @param string	$city	the city component of the delivery address
	*
	* @return void
	* @access public
	*/
	public function setDeliveryCity($city)
	{
		$this->_post_string .= '&DeliveryCity='.urlencode($city);
		
	}//end setDeliveryCity()
	

	/**
	* Set delivery postcode
	*
	* @param string	$postcode	the postcode component of the delivery address
	*
	* @return void
	* @access public
	*/
	public function setDeliveryPostcode($postcode)
	{
		$this->_post_string .= '&DeliveryPostCode='.urlencode($postcode);
		
	}//end setDeliveryPostcode()

	
	/**
	* Set delivery country
	*
	* @param string	$address	the country component of the delivery address
	*
	* @return void
	* @access public
	*/
	public function setDeliveryCountry($country)
	{
		$this->_post_string .= '&DeliveryCountry='.urlencode($country);
		
	}//end setDeliveryCountry()


	/**
	* Set delivery state
	*
	* @param string	$state	the state component of the delivery address if the country is the United States
	*
	* @return void
	* @access public
	*/
	public function setDeliveryState($state)
	{
		$this->_post_string .= '&DeliveryState='.urlencode($state);
		
	}//end setDeliveryState()
	
	
	/**
	* Set delivery phone
	*
	* @param string	$phone_no	the phone number at the delivery address
	*
	* @return void
	* @access public
	*/
	public function setDeliveryPhone($phone_no)
	{
		$this->_post_string .= '&DeliveryPhone='.urlencode($phone_no);
		
	}//end setDeliveryPhone()
	
	
	/**
	* Set 3-D Secure for the transaction
	*
	* @return void
	* @access public
	*/
	public function setThreeDSecure()
	{
		$this->_post_string .= '&Apply3DSecure=0';
		
	}//end setThreeDSecure()
	

	/**
	* Set AVS/CV2 for the transaction
	*
	* @return void
	* @access public
	*/
	public function setAVSCV2()
	{
		$this->_post_string .= '&ApplyAVSCV2=0';
		
	}//end setAVSCV2()
	
	
	/**
	* Set Gift Aid Payment for the transaction
	*
	* @return void
	* @access public
	*/
	public function setGiftAidPayment()
	{
		$this->_gift_aid = 'yes';
		$this->_post_string .= '&GiftAidPayment=1';
		
	}//end setGiftAidPayment()
	
	
	/**
	* Process the transaction
	*
	* @return mixed	return FALSE if the card is not valid or transaction fails, return 1 if SUCCESS, return 2 if need to REDIRECT in 3-D Secure check
	* @access public
	*/
	public function process()
	{
		//send POST request to the Payment Registration URL
		$this->_success = $this->_sendRequest($this->_post_urls['payment_registration']);
		
		if ($this->_success) {
			switch ($this->_response['Status']) {
				case 'OK':
					$this->_success = 1;
					break;
				case '3DAUTH':
					$this->_success = 2;
					break;
				default: //the transaction is denied
					$this->_success = FALSE;
			}
		}
		
		return $this->_success;
		
	}//end process()
	
	
	/**
	* Authorize a 3-D Secure transaction
	*
	* @param string	$merchant_ref	the merchant reference number for logging
	* @param string	$md				the MD that is sent back from the ACS
	* @param string	$pares			the Payment Authentication Response (PARes) returned by the Access Control Server (ACS)
	*
	* @return mixed		return 1 if SUCCESS; otherwise, return FALSE
	* @access public
	*/
	public function authorize($merchant_ref, $md, $pares)
	{
		//set up parameters for logging
		$this->_merchant_ref = $merchant_ref;
		$this->_tx_type = '3-D Secure Authorization Callback';
		
		//set authorization parameters
		$this->_post_string = "MD=$md&PARes=".urlencode($pares);
		
		//send POST request to the 3-D Secure Callback URL
		$this->_success = $this->_sendRequest($this->_post_urls['threeds_callback']);
		
		if ($this->_success) {
			$this->_success = $this->_response['Status'] == 'OK'? 1 : FALSE;
		}
		
		return $this->_success;
		
	}//end authorize()
	
	
	/**
	* Send a HTTPS POST request to the SagePay Payment Gateway and receive the Response back
	*
	* @param string	$url	the URL to send the POST request to. It can be the Payment Registration URL or 3-D Secure Callback URL
	* 
	* @return boolean	return TRUE if success; otherwise, return FALSE
	* @access public
	*/
	private function _sendRequest($url)
	{
		$success = TRUE;
		
		//log the request of the transaction
		$this->_logRequest();

		$options = array(
				 'POST'           => 1,
				 'POSTFIELDS'     => $this->_post_string,
				 'RETURNTRANSFER' => 1,
				 'TIMEOUT'        => $this->_timeout,
				 'SSL_VERIFYPEER' => 1,
				 'SSL_VERIFYHOST' => 2,
				);
		$details  = fetch_url($url, $options);
		$response = $details['response'];
		
		if ($response === FALSE) {
			$success = FALSE;
			$response = Array('CURL_ERROR' => TRUE, 'Status' => 'CURL_ERROR_'.$details['errornumber'], 'StatusDetail' => $details['errorstring']);
		} else {
			$lines = explode("\r\n", $response);
			$response = Array();
			foreach ($lines as $line) {
				if (!empty($line)) {
					$key_val_pairs = explode('=', $line, 2);
					$response[$key_val_pairs[0]] = $key_val_pairs[1];
				}
			}
		}
		
		$this->_response = $response;
		
		//log the transaction response
		$this->_logResponse();
			
		return $success;
		
	}//end _sendRequest()
	
	
	/**
	* Returns the gateway response array in standard format
	*
	* This should be the format other gateways use to return their response
	*
	* @access public
	* @return array
	*/
	function getResponse()
	{
		$response = Array();
		
		switch ($this->_success) {
			case 1: //payment is accepted
				$response['TRANSACTION']	= $this->_response['VPSTxId'];
				$response['TIME']			= time();
			case FALSE: //error
				$response['STATUS']			= $this->_response['StatusDetail'];
				break;
			case 2: //redirect user to the Access Control Server (ACS) for card holder verification
				$response['ACS_URL']		= $this->_response['ACSURL'];
				$response['PAREQ_MESSAGE']	= $this->_response['PAReq'];
				$response['MD']				= $this->_response['MD'];
				break;
		}
		
		return $response;

	}//end getResponse()
	
	
	/**
	* Get merchant reference
	*
	* @return string
	* @access public
	*/
	public function getMerchantReference()
	{
		return $this->_merchant_ref;
		
	}//end getMerchantReference()
	
	
	/**
	* Get the list of the magic test card numbers
	*
	* @return array
	* @access public
	*/
	public static function getTestCardNumbers()
	{
		return self::$_TEST_CARD_NUMBERS;
		
	}//end getTestCardNumbers()
	

	/**
	* Get the list of the card types supported by SagePay
	*
	* @return array
	* @access public
	*/
	public static function getCardTypes()
	{
		return self::$_CARD_TYPES;
		
	}//end getCardTypes()
	
	
	/**
	* Log the Request's information
	*
	* @return void
	* @access private
	*/
	private function _logRequest()
	{
		$message = "Request\n";
		if (empty($this->_card_number)) { //a historic (3-D Secure Callback) request
			$message .= "Historic reference: {$this->getMerchantReference()}\n";
			
		} else { //a non-historic (PAYMENT) request
			//log 4 last digits of card number
			$message .= "Card number: {$this->_card_number}\n";
			
			//log card type
			$message .= "Card type: {$this->_card_type}\n";
			
			//log merchant reference
			$message .= "Merchant reference: {$this->getMerchantReference()}\n";
			
			//log amount
			$message .= "Amount: {$this->_amount} {$this->_currency}\n";
			
			//gift aid
			$message .= "Gift Aid Payment: {$this->_gift_aid}\n";
			
		}
			
		//log trasaction type 
		$message .= "Transaction type: {$this->_tx_type}\n";
		
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
		$message = "Response\n";
		if (array_get_index($this->_response, 'CURL_ERROR', FALSE)) {
			$message .= 'CURL Error: '.$this->_response['StatusDetail'].' (status = '.$this->_response['Status'].")\n";
			
		} else {
			//log the status
			$message .= 'Transaction result: '.$this->_response['Status']."\n";
			
			//log the status details
			if (isset($this->_response['StatusDetail'])) {
				$message .= 'Status details: '.$this->_response['StatusDetail']."\n";
			}
			
			//log the merchant reference so that we can match this response with its request in the log file
			$message .= "Merchant reference: {$this->getMerchantReference()}\n";
			
			//log Sage Pay transaction ID
			if (isset($this->_response['VPSTxId'])) {
				$message .= 'Transaction ID: '.$this->_response['VPSTxId']."\n";
			}
			
			//log security key
			if (isset($this->_response['SecurityKey'])) {
				$message .= 'Security Key: '.$this->_response['SecurityKey']."\n";
			}
			
			//log authorization code
			if (isset($this->_response['TxAuthNo'])) {
				$message .= 'Authorization code: '.$this->_response['TxAuthNo']."\n";
			}
			
			//log 3DS Status
			if (isset($this->_response['3DSecureStatus'])) {
				$message .= '3-D Secure Status: '.$this->_response['3DSecureStatus']."\n";
			}
			
			//log CAVV result code
			if (isset($this->_response['CAVV'])) {
				$message .= '3-D Secure Result Code: '.$this->_response['CAVV']."\n";
			}
			
		}
		
		$this->_log($message);
		
	}//end _logResponse()
	
	
	/**
	* Log a message to the log file of the vendor SagePay account
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
		log_write($message, $this->_log_file_name, $level, $encode);
		
	}//end log()
	
	
	/**
	 * Get the unique merchant reference number whose length is less than 40 characters 
	 * 
	 * @param 	string $ref_no	The unique reference number passed in by ecommerce form or delivery method asset. The default value is md5(session_id().time())
	 * 
	 * @return	string			The unique reference number has the length that SagePay accepts (less than 40 characters)
	 * @access	private
	 */
	private function _getUniqueReferenceNumber($ref_no)
	{
		if (strlen($ref_no) > 40) {
			$ref_no = md5($ref_no);
		}
		
		return $ref_no;
		
	}//end _getUniqueReferenceNumber()
	
	
}//end class

?>
