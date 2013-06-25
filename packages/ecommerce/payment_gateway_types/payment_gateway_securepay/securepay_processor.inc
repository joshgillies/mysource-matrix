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
* $Id: securepay_processor.inc,v 1.1 2013/06/25 02:51:33 cupreti Exp $
*
*/


define('SECUREPAY_LOG_FILE', 'securepay_direct_post_transactions');

/**
* Securepay_Processor
*
* SecurePay payment processing object
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Securepay_Processor
{

	/**
	* List of all card types.
	*
	* NOTE: Basically the SecurePay gateway does not accepts the parameter for for the card type.
	* If customer uses the card that merchant is not signed up to accept, SecurePay will decline
	* the transaction. So if any card  type that Merchant is signed with is not listed here, needs
	* be added later, though specifying the card type will have no affect on the transaction outcome
	*
	* @var array
	*/
	private static $_card_types =  Array(
							'AMEX'		=> 'American Express',
							'DC'		=> 'Diners Club',
							'JCB'		=> 'JCB',
							'MC'		=> 'MasterCard',
							'VISA'		=> 'Visa',
						);

	/**
	* Test card info
	*
	* @var array
	*/
	private static $_test_card_info = Array (
											'type' => 'VISA',
											'card_number' => '4444333322221111',
											'ccv' => '123',
										);
	/**
	* SecurePay transaction types
	*
	* @var array
	*/
	private static $_transaction_types = Array(
											0 => 'Payment',
											2 => 'Payment with FraudGuard service',
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
	* SecurePay transaction success return codes
	* All other return codes represnts transaction decline
	*
	* @var array
	*/
	private static $_txn_success_codes = Array('00', '08', '11');


	/**
	* Live and test server for SecurePay where transcation is posted to
	*
	* @var array
	*/
	private static $_action_urls = Array(
						'live' => 'https://api.securepay.com.au/live/directpost/authorise',
						'test' => 'https://api.securepay.com.au/test/directpost/authorise',
					);


	/**
	* Constructor
	*/
	function __construct()
	{

	}//end constructor


	/**
	* Process the transaction
	*
	* @return mixed
	* @access public
	*/
	public function process()
	{
		// Procesing is done by SecurePay Direct Post directly
		// no nothing to do here

	}//end process()


	/**
	* Get the list of the test card numbers
	*
	* @return array
	* @access public
	*/
	public static function getActionUrl($test_mode)
	{
		return self::$_action_urls[$test_mode ? 'test' : 'live'];

	}//end getTestCardNumbers()


	/**
	* Get the list of the test card numbers
	*
	* @return array
	* @access public
	*/
	public static function getTestCardInfo()
	{
		// TODO: set as in the manual
		return self::$_test_card_info;

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
	* Get the list of SecurePay transaction types
	*
	* @return array
	* @access public
	*/
	public static function getTransactionTypes()
	{
		return self::$_transaction_types;

	}//end getCurrencyTypes()


	/**
	* Return transaction sucess codes
	*
	* @return array
	* @access public
	*/
	function getTransactionSucessCodes()
	{
		return self::$_txn_success_codes;

	}//end getTransactionSucessCodes()


	/**
	* Log a message to the log file of the gateway account
	*
	* @param string		$message	the message to log
	* @param boolean 	$test_mode	whether the payment was processed in test mode
	* @param int		$level		the error level of the message
	* @param bolean		$encode		the flag indicates if the message needs to be encoded
	*
	* @return void
	* @access public
	*/
	public static function log($message, $test_mode, $level = E_USER_NOTICE, $encode=FALSE)
	{
		$log_file = SECUREPAY_LOG_FILE.($test_mode ? '_test' : '_live');
		log_write($message, $log_file, $level, $encode);

	}//end log()


}//end class

?>
