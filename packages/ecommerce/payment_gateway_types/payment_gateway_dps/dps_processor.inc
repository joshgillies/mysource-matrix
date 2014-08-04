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
* $Id: dps_processor.inc,v 1.9 2013/03/21 07:54:27 ewang Exp $
*
*/


define('DPS_LOG_FILE_PREFIX', 'dps_transactions_account_');


/**
* Dps_Processor
*
* @see Asset_Management
*
* @author  Edison Wang <ewang@squiz.net>
* @version $Revision: 1.9 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Dps_Processor
{
	
	
	/**
	 * This array stores the URL of the transaction destinations
	 * 
	 * @var array
	 */
	private static $_URL = 'https://sec.paymentexpress.com/pxpost.aspx';


	/**
	 * This array stores the test DPS account
	 * 
	 * @var array
	 */
	private static $_test_account = Array(
									'username' 	=> 'SquizLabsDev',
									'password'	=> 'd25wdfs6',
								 );
	/**
	 * ID of a DPS account
	 * @var string
	 */
	private $_client;
	
	/**
	 * Password of a DPS account 
	 * @var string
	 */
	private $_password;
	
	
	/**
	 * amount
	 * @var int
	 */
	private $_amount;
	
	/**
	 * currency
	 * @var string
	 */
	private $_currency;
	
	
	/**
	 * Card number
	 * @var int
	 */
	private $_card_number;
	
	/**
	 * Card holder name
	 * @var int
	 */
	private $_card_name;
	
	
	/**
	 * Card Expiry Date
	 * @var string
	 */
	private $_card_expiry_date;
	
	/**
	 * Card Start Date
	 * @var int
	 */
	private $_card_start_date;
	
	/**
	 * Card Issue Number
	 * @var int
	 */
	private $_card_issue_number;
	
	
	/**
	 * Card CV2
	 * @var int
	 */
	private $_card_cv2;
	
	/**
	 * Card holder street address
	 * @var string
	 */
	private $_street_address;
	
	
	/**
	 * Card holder postcode
	 * @var int
	 */
	private $_postcode;
	
	/**
	 * AVS standard policy
	 * @var int
	 */
	private $_standard_policy = 0;
	
	/**
	 * The flag indicates whether the processor is in live or test mode
	 * @var boolean
	 */
	private $_test_mode = TRUE;
	
	/**
	 * The name of the log file that stores transaction information. This name is built from the prefix DATACASH_LOG_FILE_PREFIX and the DPS client ID
	 * @var string
	 */
	private $_log_file_name;
	

	/**
	 *  
	 * @var array
	 */
	private $_response;
	
	
	/**
	 * The flag indicates whether a transaction succeeds or fails
	 * @var boolean
	 */
	private $_success = FALSE;
	

	
	/**
	 * The list of DPS magic test card numbers
	 * @var array
	 */
	private static $_TEST_CARD_NUMBERS = Array(
						'4111111111111119'		=> 'VISA',
						'5431111111111111'		=> 'MasterCard',
						'371111111111114'		=> 'Amex',
						'36000000000008'		=> 'Diners',
						'4999999999999202'		=> 'VISA(Declined)',
					);
	
	
	/**
	 * Constructor
	 * 
	 * @param string	$client				ID of the DPS account
	 * @param string	$password			password of the DPS account
	 * @param string	$datacash_api_path	the path to the DPS PHP API's DataCash.php file
	 * @param string	$cardinfo_dir_path	the path to the directory that holds the DPS CardInfo files
	 * @param int		$timeout			the time to wait for a response from DPS Payment Gateway
	 * @param boolean	$test_mode			specify if this processor is in live or test mode; default is test mode
	 * 
	 */
	function __construct($client, $password, $test_mode = TRUE)
	{
		$this->_client = $client;
		$this->_password = $password;
		$this->_test_mode = $test_mode;
		$this->_log_file_name = DPS_LOG_FILE_PREFIX.$this->_client.($test_mode? '_test' : '_live');
		
	}//end constructor
	
	
	

	
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
		$this->_card_number = $card_no;
		
	}//end setCardNumber()
	
	
	/**
	* Set card holder name
	*
	* @param string	$name	the card holder name
	*
	* @return void
	* @access public
	*/
	public function setCardName($name)
	{
		$this->_card_name = $name;
		
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
		$this->_card_expiry_date = $month.$year;
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
		$this->_card_start_date = $month.$year;
		
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
		$this->_card_issue_number = $issue_no;
		
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
		$this->_card_cv2 = $cv2;
		
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
	public function setStreetAddress($address)
	{
		$this->_street_address = $address;
		
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
		$this->_postcode = $postcode;
		
	}//end setPostcode()
	
	
	
	
	
	
	/**
	* Set amount of transaction
	*
	* @param string	$amount		the amount of the transaction
	*
	* @return void
	* @access public
	*/
	public function setAmount($amount)
	{
		$this->_amount = sprintf('%01.2f',$amount);
		
	}//end setAmount()
	
	
	/**
	 * Set currency of transaction
	 * @param string	$currency	the currency of the amount
	 * 
	* @return void
	* @access public
	 */
	public function setCurrency($currency)
	{
		$this->_currency = $currency;
		
	}//end setCurrency()
	

	/**
	* Set Standard Policy
	*
	* @param string	$policy		the policy value (0, 1, 2) for standard policy
	*
	* @return void
	* @access public
	*/
	public function setStandardPolicy($policy)
	{
		$this->_standard_policy = $policy;
		
	}//end setStandardPolicy()
	
	
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
		return $this->_response;

	}//end getResponse()
	
	
	
	/**
	* Process the transaction
	*
	* @return mixed	return FALSE if the card is not valid or transaction fails, return 1 if SUCCESS
	* @access public
	*/
	public function process()
	{
		$this->_sendRequest();
		return $this->_success;
		
	}//end process()
	
	
	
	/**
	* Send the Request DPS Document object to DPS Payment Gateway and receive the Response back
	*
	* @return boolean	return TRUE if success; otherwise, return FALSE
	* @access public
	*/
	private function _sendRequest()
	{
		//log the request of the transaction
		$this->_logRequest();
		
		$success = TRUE;
		$client = $this->_test_mode ? self::$_test_account['username']: $this->_client;
		$password = $this->_test_mode ? self::$_test_account['password']: $this->_password;
		$enableAvs = ($this->_standard_policy != 0) ? 1 : 0;
		$cmdDoTxnTransaction = '';
		$cmdDoTxnTransaction .= "<Txn>";
		$cmdDoTxnTransaction .= "<PostUsername>$client</PostUsername>";
		$cmdDoTxnTransaction .= "<PostPassword>$password</PostPassword>"; #Insert your DPS Password here
		$cmdDoTxnTransaction .= "<Amount>$this->_amount</Amount>";
		$cmdDoTxnTransaction .= "<InputCurrency>$this->_currency</InputCurrency>";
		$cmdDoTxnTransaction .= "<CardHolderName>$this->_card_name</CardHolderName>";
		$cmdDoTxnTransaction .= "<CardNumber>$this->_card_number</CardNumber>";
		$cmdDoTxnTransaction .= "<DateExpiry>$this->_card_expiry_date</DateExpiry>";
		$cmdDoTxnTransaction .= "<TxnType>Purchase</TxnType>";
		$cmdDoTxnTransaction .= "<MerchantReference></MerchantReference>";
		$cmdDoTxnTransaction .= "<Cvc2>$this->_card_cv2</Cvc2>";
		$cmdDoTxnTransaction .= "<EnableAvsData>".$enableAvs."</EnableAvsData>";
		$cmdDoTxnTransaction .= "<AvsAction>$this->_standard_policy</AvsAction>";
		$cmdDoTxnTransaction .= "</Txn>";
		
		$options = array(
				 'POST'           => 1,
				 'POSTFIELDS'     => $cmdDoTxnTransaction,
				 'RETURNTRANSFER' => 1,
				 'SSL_VERIFYPEER' => 1,//Needs to be included if no *.crt is available to verify SSL certificates
				 'SSL_VERIFYHOST' => 2,
				 'SSLVERSION'     => 3,
				 'TIMEOUT'			=> 30,
				 'CONNECTTIMEOUT'	=> 10,
				);

		$details = fetch_url(self::$_URL, $options);

		$this->_parse_xml($details['response']);

		//log the transaction response
		$this->_logResponse();
		
	}//end _sendRequest()
	
	
	function _parse_xml($data)
	{
		$xml_parser = xml_parser_create();
		xml_parse_into_struct($xml_parser, $data, $vals, $index);
		xml_parser_free($xml_parser);
			
		$params = array();
		$level = array();
		foreach ($vals as $xml_elem) {
			if ($xml_elem['type'] == 'open') {
				if (array_key_exists('attributes',$xml_elem)) {
					list($level[$xml_elem['level']],$extra) = array_values($xml_elem['attributes']);
				} 
				else {
					$level[$xml_elem['level']] = $xml_elem['tag'];
				}
			}
			if ($xml_elem['type'] == 'complete') {
				$start_level = 1;
				$php_stmt = '$params';
							
				while($start_level < $xml_elem['level']) {
					$php_stmt .= '[$level['.$start_level.']]';
					$start_level++;
				}
				$php_stmt .= '[$xml_elem[\'tag\']] = isset($xml_elem[\'value\']) ? $xml_elem[\'value\'] : \'\';';
				eval($php_stmt);
			}
		}

			
		$success = isset($params['TXN']['SUCCESS']) ? $params['TXN']['SUCCESS'] : FALSE;
		$responseTxt = isset($params['TXN']['RESPONSETEXT']) ? $params['TXN']['RESPONSETEXT'] : 'DECLINED';
		$DPSTxnRef = isset($params['TXN']['DPSTXNREF']) ? $params['TXN']['DPSTXNREF'] : '';			
		$this->_success = $success == 1 ? 1 : FALSE;
		
		$this->_response['TRANSACTION'] = $DPSTxnRef;
		$this->_response['TIME'] = time();
		$this->_response['STATUS'] = $responseTxt;
		
	}


	
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
		$message = "\nRequest\n";
		$message .= 'Card holder name: '.$this->_card_name."\n";
		$message .= 'Card number: '.substr($this->_card_number, -4)."\n";
		$message .= 'Amount: '.$this->_amount.' '.$this->_currency."\n";
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
		$message = "\nResponse\n";
		$message .= 'Transaction: '.$this->_response['TRANSACTION']."\n";
		$message .= 'Time: '.$this->_response['TIME']."\n";
		$message .= 'Status: '.$this->_response['STATUS']."\n";
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
		log_write($message, $this->_log_file_name, $level, $encode);
		
	}//end log()
	
	

	
}//end class

?>
