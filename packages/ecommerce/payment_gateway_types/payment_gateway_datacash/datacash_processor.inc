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
* $Id: datacash_processor.inc,v 1.3 2012/08/30 00:58:34 ewang Exp $
*
*/


define('DATACASH_LOG_FILE_PREFIX', 'datacash_transactions_account_');


/**
* Datacash_Processor
*
* Datacash payment gateway processing object. Use Datacash PHP API to build the Request XML and parse the Response XML
*
* @author  Anh Ta <ata@squiz.co.uk>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Datacash_Processor
{
	
	
	/**
	 * This array stores the URLs of the transaction destinations
	 * 
	 * @var array
	 */
	private static $_URLS = Array(
									'live' 	=> 'https://mars.transaction.datacash.com/Transaction',
									'test'	=> 'https://testserver.datacash.com/Transaction',
								 );

	/**
	 * ID of a Datacash account
	 * @var string
	 */
	private $_client;
	
	/**
	 * Password of a Datacash account 
	 * @var string
	 */
	private $_password;
	
	/**
	 * The time to wait for a Datacash response
	 * @var int
	 */
	private $_timeout;
	
	/**
	 * The flag indicates whether the processor is in live or test mode
	 * @var boolean
	 */
	private $_test_mode = TRUE;
	
	/**
	 * The name of the log file that stores transaction information. This name is built from the prefix DATACASH_LOG_FILE_PREFIX and the Datacash client ID
	 * @var string
	 */
	private $_log_file_name;
	
	/**
	 * The configuration of the Request sent to Datacash. This configuration will be passed as parameter to the constructor of DataCash_SSLAgent class
	 * @var DataCash_Document
	 */
	private $_config = NULL;
	
	/**
	 * The Request that will be sent to Datacash Payment Gateway
	 * @var DataCash_Document
	 */
	private $_request = NULL;
	
	/**
	 * The Response that is received from Datacash Payment Gateway
	 * @var DataCash_Document
	 */
	private $_response = NULL;
	
	/**
	 * The flag indicates whether a transaction succeeds or fails
	 * @var boolean
	 */
	private $_success = FALSE;
	
	/**
	 * The URL of the website on which the payment is made
	 * This variable is used in 3-D Secure transaction
	 * @var string
	 */
	private $_merchant_url;
	
	/**
	 * A short description of what has been purchased. This is shown to the card holder by the Access Control Server (ACS) in the Card Holder verification stage
	 * This variable is used in 3-D Secure transaction
	 * @var string
	 */
	private $_purchase_desc;

	/**
	 * The Unix timestamp of the transaction
	 * This variable is used in 3-D Secure transaction
	 * @var int
	 */
	private $_purchase_time;
	
	/**
	 * The path to the directory that keeps the Datacash CardInfo files
	 * @var string
	 */
	private $_cardinfo_dir_path;
	
	/**
	 * The card type of the card used in the transaction
	 * @var string
	 */
	private $_card_type;
	
	/**
	 * The list of non 3-D Secure card types
	 * @var array
	 */
	private $_nonthreed_card_types = Array('American Express', 'Diners Club', 'Duet', 'GE Capital', 'JCB', 'Laser');
	
	/**
	 * The list of the card types that have to go through the 3-D Secure check. If a card of those types is not enrolled, the transaction will fail.
	 * This list is static because the merchant might be fined if those card types' transactions are not validated with 3-D Secure
	 * @var array
	 */
	private static $_MANDATE_THREED_CARD_TYPES = Array('Maestro', 'Switch');
	
	/**
	 * The list of Datacash magic test card numbers
	 * @var array
	 */
	private static $_TEST_CARD_NUMBERS = Array(
						'1000070000000001'		=> 'VISA - SUCCESS (Status: 1)',
						'100063000000007'		=> 'VISA - DECLINED (Status: 7)',
						'10006500000000'		=> 'VISA Delta - REFERRED (Status: 7)',
						'1000010000000007'		=> 'MASTER - SUCCESS (Status: 1)',
						'1000350000000106'		=> 'MASTER - DECLINED (Status: 7)',
						'1000350000000122'		=> 'MASTER - REFERRED (Status: 7)',
						'1000350000000098'		=> 'MASTER - No tids (Status: 53)',
						'1000350000000460'		=> 'MASTER - Timeout (Sleep 600s)',
						'1000350000000395'		=> 'MASTER - Missing start (Status: 28)',
						'1000130000000003'		=> 'Maestro - SUCCESS (Status: 1)',
						'1000310000000019'		=> 'Maestro - DECLINED (Status: 7)',
						'1000060000000002'		=> 'Switch - SUCCESS (Status: 1)',
						'4936000000000000019'	=> 'Switch - DECLINED (Status: 7)',
						'633499100000000004'	=> 'Solo - SUCCESS (Status: 1)',
						'633499999999999997'	=> 'Solo - DECLINED (Status: 7)',
						'1000681548753263'		=> 'Laser - SUCCESS (Status: 1)',
						'1000681548753461'		=> 'Laser - DECLINED (Status: 7)',
						'3528000000000007'		=> 'JCB - SUCCESS (Status: 1)',
						'3528000000000023'		=> 'JCB - DECLINED (Status: 7)',
						'1000350000000007'		=> 'MASTER - AVS/CV2 TEST (Amount: 1010.00 - 1010.14)',
					);
	
	
	/**
	 * Constructor
	 * 
	 * @param string	$client				ID of the Datacash account
	 * @param string	$password			password of the Datacash account
	 * @param string	$datacash_api_path	the path to the Datacash PHP API's DataCash.php file
	 * @param string	$cardinfo_dir_path	the path to the directory that holds the Datacash CardInfo files
	 * @param int		$timeout			the time to wait for a response from Datacash Payment Gateway
	 * @param boolean	$test_mode			specify if this processor is in live or test mode; default is test mode
	 * 
	 */
	function __construct($client, $password, $datacash_api_path, $cardinfo_dir_path, $timeout = 60, $test_mode = TRUE)
	{
		$this->_client = $client;
		$this->_password = $password;
		require_once $datacash_api_path;
		$this->_cardinfo_dir_path = $cardinfo_dir_path;
		$this->_timeout = $timeout;
		$this->_test_mode = $test_mode;
		$this->_log_file_name = DATACASH_LOG_FILE_PREFIX.$this->_client.($test_mode? '_test' : '_live');
		
		$this->_createConfigurationDocument();
		$this->_createRequestDocument();
		
	}//end constructor
	
	
	/**
	* Create the configuration Datacash Document object
	*
	* @return void
	* @access private
	*/
	private function _createConfigurationDocument()
	{
		$url = $this->_test_mode? self::$_URLS['test'] : self::$_URLS['live'];
		
		$config_string = <<<HEREDOC
<Configuration>
	<host>$url</host>
	<timeout>{$this->_timeout}</timeout>
	<logging>0</logging>
	<logfile></logfile>
</Configuration>
HEREDOC;
		
		$config = new DataCash_Document();
		$config->readDocumentFromString($config_string);
		$this->_config = $config;
		
		//create the global variable $config because the DataCash_Logger class of the Datacash PHP API use it in dc_log()
		//method and that method is called in the set() method of the Datacash_Document class.
		if (!isset($GLOBALS['config'])) {
			$GLOBALS['config'] = $config;
		}
		
	}//end createConfigurationDocument()
	
	
	/**
	* Create the default Request Datacash Document object with client ID and password
	*
	* @return void
	* @access private
	*/
	private function _createRequestDocument()
	{
		$request_string = <<<HEREDOC
<Request>
<Authentication>
	<client>{$this->_client}</client>
	<password>{$this->_password}</password>
</Authentication>
</Request>		
HEREDOC;
		
		$request = new DataCash_Document();
		$request->readDocumentFromString($request_string);
		$this->_request = $request;
		
	}//end createRequestDocument()
	
	
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
		@$this->_request->set('Request.Transaction.CardTxn.Card.pan', $card_no);
		
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
		@$this->_request->set('Request.Transaction.CardTxn.Card.expirydate', "$month/$year");
		
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
		@$this->_request->set('Request.Transaction.CardTxn.Card.startdate', "$month/$year");
		
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
		@$this->_request->set('Request.Transaction.CardTxn.Card.issuenumber', $issue_no);
		
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
		@$this->_request->set('Request.Transaction.CardTxn.Card.Cv2Avs.cv2', $cv2);
		
	}//end setCardCV2()

	
	/**
	* Set Street Address
	*
	* @param int	$number		the order of line of the address
	* @param string	$address	the $number-th line of the address
	*
	* @return void
	* @access public
	*/
	public function setStreetAddress($number, $address)
	{
		@$this->_request->set('Request.Transaction.CardTxn.Card.Cv2Avs.street_address'.$number, $address);
		
	}//end setStreetAddress()
	

	/**
	* Set Postcode
	*
	* @param string	$postcode	the postcode of the address
	*
	* @return void
	* @access public
	*/
	public function setPostcode($postcode)
	{
		@$this->_request->set('Request.Transaction.CardTxn.Card.Cv2Avs.postcode', $postcode);
		
	}//end setPostcode()
	
	
	/**
	* Set transaction type to "auth"
	*
	* @return void
	* @access public
	*/
	public function setTransactionTypeAuth()
	{
		@$this->_request->set('Request.Transaction.CardTxn.method', 'auth');
		
	}//end setTransactionTypeAuth()
	
	
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
		@$this->_request->set('Request.Transaction.TxnDetails.merchantreference', $this->_getUniqueReferenceNumber($merchant_ref));
		
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
		@$this->_request->set('Request.Transaction.TxnDetails.amount', sprintf('%01.2f',$amount), array('currency' => $currency));
		
	}//end setAmount()
	
	
	/**
	* Set 3-D Secure Parameters
	*
	* @param string	$merchant_url					the URL of the website on which the payment is made
	* @param string	$purchase_desc					the short description of what has been purchased
	* @param int	$purchase_time					the Unix timestamp of the transaction
	* @param array	$extra_nonthreed_card_types		the list of card types that do not need to go through 3-D Secure check in addition to the list $this->_nonthreed_card_types
	*
	* @return void
	* @access public
	*/
	public function setThreeDSecureParams($merchant_url, $purchase_desc, $purchase_time, $extra_nonthreed_card_types)
	{
		$this->_merchant_url = $merchant_url;
		//purchase_desc must be less than 125 characters so it needs to be cut off if longer than 125 characters
		if (strlen($purchase_desc) > 125) {
			$purchase_desc = substr($purchase_desc, 0, 125);
		}
		$this->_purchase_desc = $purchase_desc;
		$this->_purchase_time = $purchase_time;
		$this->_nonthreed_card_types = array_merge($this->_nonthreed_card_types, $extra_nonthreed_card_types);
		
	}//end setThreeDSecureParams()
	
	
	/**
	* Set 3-D Secure for the transaction
	*
	* @return void
	* @access public
	*/
	public function setThreeDSecure()
	{
		@$this->_request->set('Request.Transaction.TxnDetails.ThreeDSecure.verify', 'yes');
		@$this->_request->set('Request.Transaction.TxnDetails.ThreeDSecure.merchant_url', $this->_merchant_url);
		@$this->_request->set('Request.Transaction.TxnDetails.ThreeDSecure.purchase_desc', $this->_purchase_desc);
		@$this->_request->set('Request.Transaction.TxnDetails.ThreeDSecure.purchase_datetime', date('Ymd H:i:s', $this->_purchase_time));
		//set browser
		//we assume that all browsers run on PC, we do not support mobile device at the moment
		@$this->_request->set('Request.Transaction.TxnDetails.ThreeDSecure.Browser.device_category', 0);
		@$this->_request->set('Request.Transaction.TxnDetails.ThreeDSecure.Browser.accept_headers', array_get_index($_SERVER, 'HTTP_ACCEPT', '*/*'));
		@$this->_request->set('Request.Transaction.TxnDetails.ThreeDSecure.Browser.user_agent', array_get_index($_SERVER, 'HTTP_USER_AGENT', 'Mozilla/5.0'));
		
	}//end setThreeDSecure()
	

	/**
	* Set Standard Policy
	*
	* @param string	$policy		the policy value (1, 2, 3, 5, 6, 7) for standard policy
	*
	* @return void
	* @access public
	*/
	public function setStandardPolicy($policy)
	{
		@$this->_request->set('Request.Transaction.CardTxn.Card.Cv2Avs.policy', $policy);
		
	}//end setStandardPolicy()
	
	
	/**
	* Set Parameters for Authorization transaction
	*
	* @param string	$reference		the Datacash reference number
	* @param string	$pares			the Payment Authentication Response (PARes) returned by the Access Control Server (ACS)
	* @param int	$method			the transaction type that can be threedsecure_authorization_request, threedsecure_authorize_referral_request or fulfill
	*
	* @return void
	* @access public
	*/
	public function setAuthorization($reference, $pares = NULL, $method = 'threedsecure_authorization_request')
	{
		@$this->_request->set('Request.Transaction.HistoricTxn.reference', $reference);
		@$this->_request->set('Request.Transaction.HistoricTxn.method', $method);
		if (!is_null($pares)) {
			@$this->_request->set('Request.Transaction.HistoricTxn.pares_message', $pares);
		}
		
	}//end setAuthorization()
	
	
	/**
	* Process the transaction
	*
	* @return mixed	return FALSE if the card is not valid or transaction fails, return 1 if SUCCESS, return 2 if need to REDIRECT in 3-D Secure check
	* @access public
	*/
	public function process()
	{
		$cardinfo = @new DataCash_CardInfo(Array('datadir' => $this->_cardinfo_dir_path, 'pan' => $this->_request->get('Request.Transaction.CardTxn.Card.pan')));
		$invalid = @$cardinfo->validation($this->_request);
		if (!is_null($invalid)) {
			$this->_response = $invalid;
			return FALSE;
		}
		
		//the card is valid, get its scheme
		$this->_card_type  = $cardinfo->scheme();
		
		//the card is not in the non 3DS card type list, set 3DS parameters for the request
		if (!in_array($this->_card_type, $this->_nonthreed_card_types)) {
			$this->setThreeDSecure();
		}
		
		//send the request
		$this->_success = $this->_sendRequest();
		
		if ($this->_success) {
			switch (@$this->_response->get('Response.status')) {
				case 1: //the transaction is accepted
					$this->_success = 1;
					break;
				case 150: //the card is enrolled, redirect it
					$this->_success = 2;
					break;
				case 162: //the card is not enrolled
				case 159: //no VERes from the directory server
				case 160: //invalid VERes from the directory server
				case 186: //invalid VEReq
				case 187: //the directory server is unable to verify
				case 158: //3DS is not supported
				case 163: //3DS Merchant not enabled for scheme acquirer
				case 164: //3DS Acquirer not supported
				case 165: //3DS Merchant not enabled for acquirer
				case 173: //3DS Card not enrolled in Cache
					$card_type = @$this->_response->get('Response.CardTxn.card_scheme');
					//if the card type is in the madate 3DS card list, return FALSE; otherwise, send an authorization request without a PARes (for non-enrolled card or card scheme which is not supported by 3-D Secure
					if (in_array($card_type, self::$_MANDATE_THREED_CARD_TYPES)) {
						$this->_success = FALSE;
					} else {
						//recreate the request
						$this->_createRequestDocument();
						//authorize payment
						$this->authorize(@$this->_response->get('Response.datacash_reference'));
					}
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
	* @param string	$datacash_ref	the Datacash reference number
	* @param string	$pares			the Payment Authentication Response (PARes) returned by the Access Control Server (ACS)
	*
	* @return mixed		return 1 if SUCCESS; otherwise, return FALSE
	* @access public
	*/
	public function authorize($datacash_ref, $pares = NULL)
	{
		//set authorization parameters
		$this->setAuthorization($datacash_ref, $pares);
		//send request
		$this->_success = $this->_sendRequest();
		if ($this->_success) {
			$this->_success = @$this->_response->get('Response.status') == 1? 1 : FALSE;
		}
		
		return $this->_success;
		
	}//end authorize()
	
	
	/**
	* Send the Request Datacash Document object to Datacash Payment Gateway and receive the Response back
	*
	* @return boolean	return TRUE if success; otherwise, return FALSE
	* @access public
	*/
	private function _sendRequest()
	{
		//log the request of the transaction
		$this->_logRequest();
		
		$agent = new DataCash_SSLAgent($this->_config);
		$success = @$agent->send($this->_request);
		
		$response = NULL;
		
		if ($success) {
			$response = $agent->getResponseDocument();
		} else {
			$response = new DataCash_Document("Response");
			@$response->set("Response.status", $agent->errno);
			@$response->set("Response.reason", $agent->err_str);
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
				$response['TRANSACTION']	= @$this->_response->get('Response.datacash_reference');
				$response['TIME']			= @$this->_response->get('Response.time');
			case FALSE: //error
				$response['STATUS']			= @$this->_response->get('Response.reason');
				break;
			case 2: //redirect user to the Access Control Server (ACS) for card holder verification
				$response['ACS_URL']		= @$this->_response->get('Response.CardTxn.ThreeDSecure.acs_url');
				$response['PAREQ_MESSAGE']	= @$this->_response->get('Response.CardTxn.ThreeDSecure.pareq_message');
				$response['TRANSACTION']	= @$this->_response->get('Response.datacash_reference');
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
		return @$this->_request->get('Request.Transaction.TxnDetails.merchantreference');
		
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
	* Log the Request's information
	*
	* @return void
	* @access private
	*/
	private function _logRequest()
	{
		$message = "Request\n";
		$datacash_ref = @$this->_request->get('Request.Transaction.HistoricTxn.reference');
		if ($datacash_ref === FALSE) { //a non-historic (auth, pre) request
			//log 4 last digits of card number
			$card_no = @$this->_request->get('Request.Transaction.CardTxn.Card.pan');
			$message .= 'Card number: '.substr($card_no, -4)."\n";
			
			//log card type if known
			if (!empty($this->_card_type)) {
				$message .= 'Card type: '.$this->_card_type."\n";
			}
			
			//log trasaction type 
			$message .= 'Transaction type: '.@$this->_request->get('Request.Transaction.CardTxn.method')."\n";
			
			//log merchant reference
			$message .= 'Merchant reference: '.$this->getMerchantReference()."\n";
			
			//log amount
			$amount_ele = @$this->_request->get('Request.Transaction.TxnDetails.amount', FALSE);//this second parameter is different from the phpdoc of the Datacash_Document class
			if ($amount_ele !== FALSE) {
				$attr_arr = $amount_ele->getAllAttributes();
				$currency = '';
				foreach ($attr_arr as $attr) {
					if ($attr->getName() == 'currency') {
						$currency = $attr->getValue();
						break;
					}
				}
				$message .= 'Amount: '.$amount_ele->getText().' '.$currency."\n";
			}
			
			//3-D Secure check
			$verify = @$this->_request->get('Request.Transaction.TxnDetails.ThreeDSecure.verify');
			if ($verify === FALSE) {
				$verify = 'no';
			}
			$message .= '3-D Secure check: '.$verify."\n";
			
		} else { //a historic request
			$message .= "Historic reference: $datacash_ref\n";
			$message .= 'Transaction type: '.@$this->_request->get('Request.Transaction.HistoricTxn.method')."\n";
		}
		
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
		$datacash_ref = @$this->_response->get('Response.datacash_reference');
		if ($datacash_ref !== FALSE) { //a transaction result returned by Datacash
			//log the reason message and status
			$message .= 'Transaction result: '.@$this->_response->get('Response.reason').' (status = '.@$this->_response->get('Response.status').")\n";
			
			//if this response is received by a non-historic (auth, pre) request, log the merchant reference so that we can match this response with its request in the log file
			if (@$this->_request->get('Request.Transaction.HistoricTxn.reference') === FALSE) {
				$message .= 'Merchant reference: '.$this->getMerchantReference()."\n";
			}
			
			//log Datacash reference
			$message .= 'Datacash reference: '.$datacash_ref."\n";
			
			//log card type if it exists
			$card_type = @$this->_response->get('Response.CardTxn.card_scheme');
			if ($card_type !== FALSE) {
				$message .= "Card type by Datacash: $card_type\n";
			}
			
			//log AVS/CV2 check status
			$cv2avs_ele = @$this->_response->get('Response.CardTxn.Cv2Avs.cv2avs_status', FALSE);//this second parameter is different from the phpdoc of the Datacash_Document class
			if ($cv2avs_ele !== FALSE) {
				$message .= 'AVS/CV2 check: '.$cv2avs_ele->getText()."\n";
				$attr_arr = $cv2avs_ele->getAllAttributes();
				foreach ($attr_arr as $attr) {
					if ($attr->getName() == 'reversal') {
						$message .= 'Reversal: '.($attr->getValue() == 0? 'failed' : 'successful')."\n";
						break;
					}
				}
			}
			
		} else { //an error occurs
			$message .= 'HTTP Error: '.@$this->_response->get('Response.reason').' (status = '.@$this->_response->get('Response.status').")\n";
		}
		
		$this->_log($message);
		
	}//end _logResponse()
	
	
	/**
	* Log a message to the log file of the $this->_client Datacash account
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
	 * Get the unique merchant reference number whose length is between 6 and 30 alphanumeric characters 
	 * 
	 * @param 	string $ref_no	The unique reference number passed in by ecommerce form or delivery method asset. The default value is md5(session_id().time())
	 * 
	 * @return	string			The unique reference number has the length that Datacash accepts (between 6 and 30 characters)
	 * @access	private
	 */
	private function _getUniqueReferenceNumber($ref_no)
	{
		$ref_length = strlen($ref_no);
		//if the reference number is not a hexadecimal number and its length is not between 6 and 30 alphanumeric characters,
		//convert it to a 32 character hexadecimal number using md5() function so that we can convert its base later
		if (!is_numeric('0x'.$ref_no) && (($ref_length > 30) || ($ref_length < 6))) {
			$ref_no = md5($ref_no);
			$ref_length = strlen($ref_no);
		} else if (($ref_length > 32) || ($ref_length < 6)) {
			//if the reference number is a hexadecimal number and (too short (<6) or too long (>32))
			$ref_no = md5($ref_no);
			$ref_length = strlen($ref_no);
		}
		
		//ref_length should be between 6 and 32 when it comes here
		//if ref_length > 30, convert base from 16 (hexadecimal) to 36. We are sure to have the number of base 36 with the
		//length between 23 and 26 characters because the smallest and biggest number from md5() are 16^29 and 16^33
		if ($ref_length > 30) {
			$ref_no = base_convert($ref_no, 16, 36);
		}
		
		return $ref_no;
		
	}//end _getUniqueReferenceNumber()
	
	
}//end class

?>