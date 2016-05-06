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
* $Id: esec_processor.inc,v 1.17 2012/08/30 00:58:34 ewang Exp $
*
*/

/**
* Note:
*
* The Esec Processor is used in matrix and mysource. Please maintain these files in both repositories
* Matrix:	packages/ecommerce/payment_gateway_types/payment_gateway_esec
* Mysource: xtras/sites/extensions/ecommerce_store/xtras/payment_gateway/esec
*/

/**
* Esec_Processor
*
* Esec payment gateway processing object. Relies on HTTP_Rrequest PEAR module
* Implements a 'Direct Transaction' scenario
*
* Esec specific comments are at the bottom of this class
*
* @author  Andrei Railean		<arailean@squiz.net>
* @author  Dmitry Baranovskiy	<dbaranovskiy@squiz.net>
* @version $Revision: 1.17 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Esec_Processor
{

	var $parameters;

	var $response;

	// returned response details
	var $eps_v3plus_responses = Array(
		'ref-id',			//= referenceID
		'auth-id',			//= authorisationID
		'message',			//= message
		'signature',		//= signature
		'eft-response',		//= eftResponse
		'txn-id',			//= bank transaction ID
		'settlement-date'	//= bank settlement date
	);

	var $card_types = Array(
		"visa"      => "visa",
		"mastercard"=> "mastercard",
		"amex"      => "amex",
		"dinersclub"=> "dinersclub",
		"jcb"       => "jcb"
	);

	var $_status_codes = Array (
		'00' => 'approved',
		'01' => 'refer to issuer',
		'02' => 'refer to issuer\'s special conditions',
		'03' => 'invalid merchant',
		'04' => 'pickup card',
		'05' => 'do not honour',
		'06' => 'error',
		'07' => 'pickup card, special conditions',
		'08' => 'approved',
		'09' => 'request in progress',
		'10' => 'approved for partial amount',
		'11' => 'approved VIP',
		'12' => 'invalid transaction',
		'13' => 'invalid amount',
		'14' => 'invalid card number',
		'15' => 'no such issuer',
		'16' => 'approved, update track 3',
		'17' => 'customer cancellation',
		'18' => 'customer dispute',
		'19' => 're-enter transaction',
		'20' => 'invalid response',
		'21' => 'no action taken',
		'22' => 'suspected malfunction',
		'23' => 'unacceptable transaction fee',
		'24' => 'file date not supported',
		'25' => 'unable to locate record on file',
		'26' => 'duplicate file update record, old record replaced',
		'27' => 'file update field error',
		'28' => 'file update file locked out',
		'29' => 'file update not successful, contact acquirer',
		'30' => 'format error',
		'31' => 'bank not supported by switch',
		'32' => 'completed partially',
		'33' => 'expired card',
		'34' => 'suspected fraud',
		'35' => 'contact acquirer',
		'36' => 'restricted card',
		'37' => 'contact acquirer security',
		'38' => 'allowable PIN retries exceeded',
		'39' => 'no credit account',
		'40' => 'request function not supported',
		'41' => 'lost card',
		'42' => 'no universal account',
		'43' => 'stolen card',
		'44' => 'no investment account',
		'51' => 'insufficient funds',
		'52' => 'no cheque account',
		'53' => 'no savings account',
		'54' => 'expired card',
		'55' => 'incorrect PIN',
		'56' => 'no card record',
		'57' => 'transaction not permitted to cardholder',
		'58' => 'transaction not permitted to terminal',
		'59' => 'suspected fraud',
		'60' => 'contact acquirer',
		'61' => 'exceeds withdrawal amount limit',
		'62' => 'restricted card',
		'63' => 'security violation',
		'64' => 'original amount incorrect',
		'65' => 'exceeds withdrawal frequency limit',
		'66' => 'contact acquirer security',
		'67' => 'hard capture',
		'68' => 'response received too late',
		'75' => 'allowable number of PIN retries exceeded',
		'90' => 'cutoff in progress',
		'91' => 'issuer inoperative',
		'92' => 'financial institution cannot be found',
		'93' => 'transaction cannot be completed, violation of law',
		'94' => 'duplicate transmission',
		'95' => 'reconcile error',
		'96' => 'system malfunction',
		'97' => 'reconciliation totals have been reset',
		'98' => 'MAC error',
		'99' => 'reserved, will not be returned'
	);

	/**
	* Constructor
	*
	*/
	function Esec_Processor()
	{
		$this->parameters = Array();
		$this->response = Array();

		$this->set('EPS_CCV',       '');
		$this->set('EPS_VERSION',   3);
		$this->set('EPS_TEST',      'false');
		$this->set('EPS_3DSECURE',  'false');

	}//end constructor


	/**
	* sets parameter value by name
	*
	* @param	$name	name of the parameter
	* @param	$value	new value of the parameter
	*
	* @access private
	* @return void
	*/
	function set($name, $value)
	{
		$this->parameters[$name] = $value;
	}//end set()


	/**
	* sets parameter value by name as an array
	*
	* @param	$name	name of the parameter
	* @param	$value	new value of the parameter
	*
	* @access private
	* @return void
	*/
	function setArray($name, $value)
	{
		if (!is_array($this->parameters[$name])) {
			$this->parameters[$name] = Array();
		}
		$this->parameters[$name][] = $value;
	}//end setArray()


	/**
	* returns parameter value by name
	*
	* @param	$name	name of the parameter
	*
	* @access private
	* @return mixed
	*/
	function get($name)
	{
		return $this->parameters[$name];

	}//end get()


	/**
	* sets amount for transaction
	*
	* @param	$amount	amount
	* @param	$cents	if true, then amount in cents otherwise in dollars.
	*
	* @access public
	* @return void
	*/
	function setAmount($amount, $cents=false)
	{
		if ($cents) {
			$amount = floor($amount);
		} else {
			$amount = sprintf('%01.2f',$amount);
		}
		$this->set('EPS_AMOUNT', $amount);

	}//end setAmount()


	/**
	* Get Charge Amount
	*
	* @access public
	* @return string
	*/
	function getAmount()
	{
		return $this->get('EPS_AMOUNT');

	}//end getAmount()


	/**
	* returns array of possible card types
	*
	* @access public
	* @return array
	*/
	function getCardTypes()
	{
		return $this->card_types;

	}//end getCardTypes()


	/**
	* sets card type
	*
	* @param	$type	type of the card
	*
	* @access public
	* @return void
	*/
	function setCardType($type)
	{
		$this->set('EPS_CARDTYPE', $type);

	}//end setCardType()

	/**
	* sets URL for gateway
	*
	* @param	$new_url	new URL of the gateway
	*
	* @access public
	* @return void
	*/
	function setURL($new_url)
	{
		$this->_url = $new_url;

	}//end setURL()


	/**
	* Process the given Transaction
	*
	* @access public
	* @return boolean
	*/
	function process()
	{

	}//end process()


	/**
	* Returns the gateway response array in standard format
	*
	* This should be the format other gateways use to return their response
	*
	* @access public
	* @return Array
	*/
	function getResponse()
	{
		$response['REFERENCE']      = isset($this->response['ref-id']) ? $this->response['ref-id'] : '';
		$response['AUTHORISATION']  = isset($this->response['auth-id']) ? $this->response['auth-id'] : '';
		$response['MESSAGE_CODE']   = isset($this->response['message']) ? substr($this->response['message'], 0, 3) : '';
		$response['MESSAGE']        = isset($this->response['message']) ? substr($this->response['message'], 4) : '';
		$response['STATUS']         = isset($this->response['eft-response']) ? $this->response['eft-response'] : '';
		$response['TRANSACTION']    = isset($this->response['txn-id']) ? $this->response['txn-id'] : '';
		$response['DATE']           = isset($this->response['settlement-date']) ? $this->response['settlement-date'] : '';

		return $response;

	}//end getResponse()


	/**
	* returns message according to result status code
	*
	* @access public
	* @return string
	*/
	function explainStatus()
	{

		$code = $this->response['eft-response'];
		if (isset($this->_status_codes[$code])) {
			$result = $this->_status_codes[$code];
		} else {
			$result = "Unknown";
		}

		return $result;

	}//end explainStatus()


	/**
	* sets merchant
	*
	* @param	$merchant_no	merchant number
	*
	* @access public
	* @return void
	*/
	function setMerchant($merchant_no)
	{
		$this->set('EPS_MERCHANT', $merchant_no);

	}//end setMerchant()


	/**
	* sets reference number
	*
	* @param	$ref_no	reference number
	*
	* @access public
	* @return void
	*/
	function setReference($ref_no)
	{
		$this->set('EPS_REFERENCEID', $ref_no);

	}//end setReference()


	/**
	* set CCV number
	*
	* @param	$ccv_no	CCV - credit card verification number
	*
	* @access public
	* @return void
	*/
	function setCCV($ccv_no)
	{
		$this->set('EPS_CCV', $ccv_no);

	}//end setCCV()


	/**
	* sets card number
	*
	* @param	$card_no	card number
	*
	* @access public
	* @return void
	*/
	function setCardNumber($card_no)
	{
		$this->set('EPS_CARDNUMBER', $card_no);

	}//end setCardNumber()


	/**
	* sets card expiry month
	*
	* @param	$month	card expiry month
	*
	* @access public
	* @return void
	*/
	function setCardExpiryMonth($month)
	{
		$this->set('EPS_EXPIRYMONTH', $month);

	}//end setCardExpiryMonth()


	/**
	* sets card expiry year
	*
	* @param	$year	card expiry year
	*
	* @access public
	* @return void
	*/
	function setCardExpiryYear($year)
	{
		$this->set('EPS_EXPIRYYEAR', $year);

	}//end setCardExpiryYear()

	/**
	* sets card name
	*
	* @param	$name	card name
	*
	* @access public
	* @return void
	*/
	function setCardName($name)
	{
		$this->set('EPS_NAMEONCARD', $name);

	}//end setCardName()

	/**
	* sets card type
	*
	* @param	string $email	the email address
	*
	* @access public
	* @return void
	*/
	function setInfoEmail($email)
	{
		$this->set('EPS_INFOEMAIL', 'mailto: '.$email);

	}//end setInfoEmail()

	/**
	* sets the redirect URL
	*
	* @param	string $value	the value to set
	* @param	bool $email		flag to specify it is an email address
	*
	* @access public
	* @return void
	*/
	function setRedirectURL($value, $email=false)
	{
		if ($email && is_array($this->get('EPS_RESULTURL'))) {
			// mailto is only valid if we already have an email address
			$value = 'mailto:'.$value;
		}
		$this->setArray('EPS_RESULTURL', $value);

	}//end setRedirectURL()

}//end class

/*
explanation of request parameters expected by the gateway
taken from
http://www.esecpayments.com.au/index.jsp?id=integdevel
8/10/2004

	EPS_MERCHANT
		An unique identifier for the merchant within the Payment Gateway. This merchant identifier value is an alphanumeric string allocated to the merchant by eSec, although for testing purposes an identifier value of "test" may be used. This merchant identifier value is not the same as the merchant agreement number given to the merchant by an acquiring financial institution.
		Examples:
		<input type="HIDDEN" name="EPS_MERCHANT" value="test">
		<input type="HIDDEN" name="EPS_MERCHANT" value="widgetsrus">

	EPS_REFERENCEID
		An alphanumeric string that allows the merchant's processing system to identify an individual transaction. The format of this string is of no importance to the Payment Gateway, since the value is simply stored by the Payment Gateway as part of the transaction record and returned to the merchant's processing system in the transaction result.
		Examples:
		<INPUT TYPE="HIDDEN" NAME="EPS_REFERENCEID" VALUE="1234567890">
		<INPUT TYPE="HIDDEN" NAME="EPS_REFERENCEID" VALUE="20010410-123456">

	EPS_CARDNUMBER
		The number from the credit card to be used for the purchase. This number must be greater than 12 digits, less than 19 digits and must conform to the credit card checkdigit scheme. Spaces and hyphens included in the card number value will be removed before processing.
		When sending transactions to the Payment Gateway test facility, and only to the test facility, the following special card "numbers" may be submitted:
		testsuccess - Always successfully processed and authorised
		testfailure - Always successfully processed and refused
		testtimeout - Never responds and the transaction will time out
		The test facility does not normally accept real credit card numbers, however additional test facilities may be made available under certain specific exceptional circumstances. The Payment Gateway live facility will not accept the test card numbers listed above under any circumstances.
		Examples:
		<INPUT TYPE="HIDDEN" NAME="EPS_CARDNUMBER" VALUE="testsuccess">
		<INPUT TYPE="HIDDEN" NAME="EPS_CARDNUMBER" VALUE="1234567890123456">

	EPS_CARDTYPE
		A string containing the name of the credit card issuer that provided the credit card. This may currently be one of the strings "visa", "mastercard", "bankcard", "amex", "dinersclub" or "jcb" in any mixture of case. If this parameter is not correctly set to one of the values listed above, the transaction will be rejected.
		When sending transactions to the Payment Gateway test facility, and only to the test facility, the special card type "testcard" should be used to identify the special test card numbers listed above. Failure to set the card type field to "testcard" when sending test card numbers will cause the Payment Gateway to reject the transaction.
		Examples:
		<INPUT TYPE="HIDDEN" NAME="EPS_CARDTYPE" VALUE="testcard">
		<INPUT TYPE="HIDDEN" NAME="EPS_CARDTYPE" VALUE="visa">

	EPS_EXPIRYMONTH
		The month in which the credit card expires. This may only contain an integer value between 1 and 12, inclusive, corresponding to the month of the year.
		The expiry month and expiry year together must form a date that is at least the current month. Transactions that contain an expiry date in the past will be rejected.
		Examples:
		<INPUT TYPE="HIDDEN" NAME="EPS_EXPIRYMONTH" VALUE="1">
		<INPUT TYPE="HIDDEN" NAME="EPS_EXPIRYMONTH" VALUE="12">

	EPS_EXPIRYYEAR
		The year in which the credit card expires. This should ideally be a full 4 digit year value to remove any possible ambiguity, however if a two digit year is provided, the Payment Gateway will assume that a value of "99" refers to 1999 and that any other value refers to a value of "20XX", where XX is the 2 digit value provided. The expiry month and expiry year together must form a date that is at least the current month. Transactions that contain an expiry date in the past will be rejected. It is strongly recommended that a full 4 digit year be specified as the value of this parameter since support for two digit years will be withdrawn in a future release of this interface.
		Examples:
		<input type="HIDDEN" name="EPS_EXPIRYYEAR" value="2001">
		<input type="HIDDEN" name="EPS_EXPIRYYEAR" value="01">

	EPS_NAMEONCARD
		The card holder's name as specified on the credit card. This parameter must contain a non-null alphabetic string of up to 50 characters.
		Examples:
		<input type="HIDDEN" name="EPS_NAMEONCARD" value="John A. Citizen">
		<input type="HIDDEN" name="EPS_NAMEONCARD" value="Jane M. Person">

	EPS_AMOUNT
		The total amount of the purchase transaction. This value may be specified either as a decimal dollar amount (in which case there MUST be two digits after the decimal place) or as a value in cents. If a decimal point is not included in the amount value, then the amount is assumed to be a value in cents. Please be careful to correctly specify the amount as the Payment Gateway has no way of determining whether an amount has been correctly specified. As an example, assume a transaction is being submitted for an amount of AUD$107.95. This amount may be submitted to the Payment Gateway in either of the following forms:
		<input type="HIDDEN" name="EPS_AMOUNT" value="10795">
		<input type="HIDDEN" name="EPS_AMOUNT" value="107.95">
		The first form above indicates a value of 10795 cents, which is the preferred method for submitting amounts. Using the minor currency unit (i.e. cents) allows for easier expansion into future multi-currency services. Be very careful to ensure that amounts in whole dollars, i.e. with only zeros after the decimal point, are submitted *with the trailing zeros intact*. If an amount such as AUD$107.00 is simply submitted as 107, the Payment Gateway will assume that this is 107 *cents* and will process the transactions as such. The correct form for an amount such as this one is either of "10700" or "107.00".
		Null or zero and negative amounts are not acceptable and transactions containing such amount values will be rejected.

	EPS_CCV
		The Card Check Value (CCV) field should contain a three or four digit value that is printed on the back of the credit card itself. If the CCV value is not available, either because it is not present on the card (some cards still do not have CCV values printed on them), or because the card holder does not wish to enter it, this field should be left empty.
		When sending transactions to the Payment Gateway test facility, and only to the test facility, CCV values of "1234" and "999" are accepted as valid fields. The CCV field may also be left empty for testing.
		This field may be referred to elsewhere as a Card Verification Value (CVV) or a Card Verification Code (CVC), most notably in information provided by banks or credit card providers.
		Examples:
		<input type="HIDDEN" name="EPS_CCV" value="1234">
		<input type="HIDDEN" name="EPS_CCV" value="999">

	EPS_VERSION
		An integer that specifies the SSL Web Interface Specification response version that the merchant's processing system requires. This may take a value of 1, 2, 3, or 4, for a response in the required format. The differences between the response versions is described later in this document. If not set to any value, the Payment Gateway assumes a Version 1 response.
		Examples:
		<input type="HIDDEN" name="EPS_VERSION" value="1">
		<input type="HIDDEN" name="EPS_VERSION" value="2">

	EPS_TEST
		A boolean field that specifies whether the transaction should be sent to the Payment Gateway's test facility. If this element exists and contains a value of "true", the transaction will be sent to the test service, and if it contains a value of "false", the transaction will be sent to the live service. If this element does not exist, the default service will be utilised (usually the live service).
		Examples:
		<input type="HIDDEN" name="EPS_TEST" value="true">
		<input type="HIDDEN" name="EPS_TEST" value="false">
*/


?>
