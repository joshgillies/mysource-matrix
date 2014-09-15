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
* $Id: utransact_processor.inc,v 1.4 2012/08/30 00:58:34 ewang Exp $
*
*/


define('UTRANSACT_LOG_FILE', 'utransact_transactions');
define('MAX_TRANSACTION_PROCESS_ATTEMPT', 5);

require_once dirname(__FILE__).'/transaction_dataset.inc';

/**
* Utransact_Processor
*
* UTransact payment processing object
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Utransact_Processor
{

	/**
	* URL of the UTransact bgas webservice
	*
	* @var string
	*/
	private $_bgas_url;


	/**
	* URL of the UTransact transactions webservice
	*
	* @var string
	*/
	private $_transactions_url;


	/**
	* URL of the UTransact services webservice
	*
	* @var string
	*/
	private $_services_url;


	/**
	* UTransact bgas soap client
	*
	* @var string
	*/
	private $_bgas_client;


	/**
	* UTransact transactions soap client
	*
	* @var string
	*/
	private $_transactions_client;


	/**
	* UTransact services soap client
	*
	* @var string
	*/
	private $_services_client;


	/**
	* UTransact global authentication token (used for all soap requests)
	*
	* Token expires after a defined amount of inactivity (not defined via Matrix),
	* hence needs to checked and refreshed occassionally
	*
	* @var string
	*/
	private $_auth_token;


	/**
	* Namespace url for bgas authentication SOAP header
	*
	* @var string
	*/
	private $_auth_bgas_header_ns = "http://www.paymentgateway.co.nz/pgservice/Bgas.asmx";


	/**
	* Namespace url for transactions authentication SOAP header
	*
	* @var string
	*/
	private $_auth_transactions_header_ns = "http://www.paymentgateway.co.nz/pgservice/Transactions.asmx";


	/**
	* Namespace url for services authentication SOAP header
	*
	* @var string
	*/
	private $_auth_services_header_ns = "http://www.paymentgateway.co.nz/pgservice/Services.asmx";


	/**
	* UTransact supported card types (extracted from webservice)
	*
	* @var array
	*/
	private $_card_types;


	/**
	* Test card numbers
	*
	* @var array
	*/
	private $_test_card_numbers = Array (
		"4111111111111111" 	=> "Visa Card Test",
		"5555555555554444" 	=> "MasterCard Card Test",
		"492" 				=> "Invalid Card Test",
        );


	/**
	* Username of a Utransact account
	* @var string
	*/
	private $_username;


	/**
	* Password of a Utransact account
	* @var string
	*/
	private $_password;


	/**
	* Transaction reference number
	* @var string
	*/
	private $_reference_no;


	/**
	* Credit card number
	* @var string
	*/
	private $_credit_card;


	/**
	* Credit card CV2 number
	* @var string
	*/
	private $_credit_card_cv2;


	/**
	* Credit card expiry date: month
	* @var string
	*/
	private $_expiry_month;


	/**
	* Credit card expiry date: year
	* @var string
	*/
	private $_expiry_year;


	/**
	* Card holder's name
	* @var string
	*/
	private $_card_holder_name;


	/**
	 * Merchant id
	 * @var int
	 */
	private $_merchant_id;


	/**
	 * Payment model
	 * @var string
	 */
	private $_payment_model;


	/**
	* Purchaser's first name
	* @var string
	*/
	private $_first_name;

	/**
	* Purchaser's last name
	* @var string
	*/
	private $_surname;


	/**
	* Purchaser's email address
	* @var string
	*/
	private $_email;


	/**
	* Address line 1
	* @var string
	*/
	private $_address_line_1;


	/**
	* Address line 2
	* @var string
	*/
	private $_address_line_2;


	/**
	* Suburb
	* @var string
	*/
	private $_suburb;


	/**
	* City
	* @var string
	*/
	private $_city;


	/**
	* Post code
	* @var int
	*/
	private $_postcode;


	/**
	* Line items from the cart
	* @var array
	*/
	private $_cart_items = Array();


	/**
	* Response
	* @var array
	*/
	private $_response = Array();



	/**
	* Constructor
	*
	* @param string	    $username		    username of the UTransact account
	* @param string	    $password		    password of the UTransact account
	* @param string     $bgas_url    	    url to UTransact Bgas webservice
	* @param string     $transactions_url   url to UTransact Transactions webservice
	* @param string     $services_url       url to UTransact Services webservice
	*
	*/
	function __construct($username, $password, $bgas_url, $transactions_url, $services_url)
	{
     	$this->_username = $username;
       	$this->_password = $password;

       	$this->_bgas_url = $bgas_url;
       	$this->_transactions_url = $transactions_url;
       	$this->_services_url = $services_url;

		$this->_bgas_client = new SoapClient($this->_bgas_url, array('trace' => 1));
		$this->_transactions_client = new SoapClient($this->_transactions_url, array('trace' => 1));
		$this->_services_client = new SoapClient( $this->_services_url, array('trace' => 1));

		$this->_log_file_name = UTRANSACT_LOG_FILE;

		$this->_setSupportedCreditCardTypes();

	}//end constructor


	/**
	* Process the transaction
	*
	* @return mixed
	* @access public
	*/
	public function process()
	{
        $transaction_handle = isset($_SESSION['UTRANSACT_TRANSACTION_HANDLE']) ? $_SESSION['UTRANSACT_TRANSACTION_HANDLE'] : FALSE;

		if (!$transaction_handle) {
            // Cannot proceed without transaction handler
            $this->_response['STATUS'] = "FAIL";
			$this->_response['MESSAGE'] = "Couldn't find transaction handle in session";

			$this->_log("Couldn't find transaction handle in the session for the transaction ref ".$this->_reference_no);
            return FALSE;
        }


        if (empty($this->_cart_items)) {
            // Cart is empty, nothing to process
            $this->_response['STATUS'] = "FAIL";
			$this->_response['MESSAGE'] = "Cart items not found in session";

			$this->_log("Cart items not found in session for transaction ref ".$this->_reference_no);
            return FALSE;
        }

        // First set the UTransact authentication token
		if (!$this->_refreshAuthToken()) {
            $this->_response['STATUS'] = "FAIL";
			$this->_response['MESSAGE'] = sprintf(translate('utransact_problem_while_processing'), $this->_reference_no);

			$this->_log("Cannot acquire Utransact bga authentication token");

            return FALSE;
		}

		// Create a transaction dataset for this tranaction
        $transaction_table = new Transaction_Dataset();

        // Populate transaction row
        $transaction_table->addTransactionRow(
				$this->_reference_no,
				empty($this->_credit_card_cv2) ? $this->_credit_card : $this->_credit_card.','.$this->_credit_card_cv2,
				$this->_expiry_year.'/'.$this->_expiry_month,
				$this->_card_holder_name,
				$this->_merchant_id,
				$this->_payment_model
			);

        // Populate purchaser row
        $transaction_table->addPurchaserRow(
				$this->_first_name,
				$this->_surname,
				$this->_email,
				$this->_address_line_1,
				$this->_address_line_2,
				$this->_suburb,
				$this->_city,
				$this->_postcode
			);

        // Populate line_item row
        foreach($this->_cart_items as $item) {
            $transaction_table->addLineItemRow(
					$item["ApplicationUsername"],
					$item["Name"],
					$item["Description"],
					$item["Quantity"],
					$item["RefundType"],
					$item["RefundExpiryDate"],
					$item["Amount"],
					$item["ProductCode"],
					$item["FinancialCode"],
					$item["FinancialValues"],
					$item["AdditionalInfo"]
				);
        }//end for

        // Once line_item table is populated, empty the cart
        $this->clearCart();

		// New transaction dataset
		$new_transaction_dataset = $transaction_table->getTransactionDatasetXML();

		// Get the authentication header for soap request to payment gateway
		$transaction_auth_header = $this->_getAuthSoapHeader($this->_auth_transactions_header_ns);

		$process_attempt_count = 1;
		$transaction_response = null;

		while (empty($transaction_response)) {

			// Process the transaction
            try {
                // First check if transaction already exists
                $transaction_exists_response = $this->_transactions_client->__soapCall(
                                            'TransactionExists',
                                            Array(
                                                'parameters' => Array(
                                                    'bgaName' 		=> $this->_username,
                                                    'referenceId'	=> $this->_reference_no,
                                                    ),
                                                ),
                                            Array('uri' => ''),
                                            $transaction_auth_header
                                    );


                if (isset($transaction_exists_response->TransactionExistsResult) && $transaction_exists_response->TransactionExistsResult) {
                    $transaction_result = $transaction_exists_response->TransactionExistsResult;
					break;

                } else {
                    // Process the new transaction
                    $transaction_response = $this->_transactions_client->__soapCall(
                                                'ProcessTransaction',
                                                Array(
                                                    'parameters' => Array(
                                                        'transactionHandle' => $transaction_handle,
                                                        'transaction'		=> Array(
																			'any' => $new_transaction_dataset,
														),
                                                    ),
                                                ),
                                                Array('uri' => ''),
                                                $transaction_auth_header
                                            );
					$transaction_result = isset($transaction_response->ProcessTransactionResult->any) ? $transaction_response->ProcessTransactionResult->any : '';
                }

            } catch (SoapFault $fault) {
                $this->_response['STATUS'] = "FAIL";
                $this->_response['MESSAGE'] = $fault->getMessage()."<br>".sprintf(translate('utransact_problem_while_processing'), $this->_reference_no);

                $this->_log($fault->getMessage());
                return FALSE;

            } catch (BusinessRuleException $e) {
                $this->_response['STATUS'] = "FAIL";
                $this->_response['MESSAGE'] = sprintf(translate('utransact_problem_while_processing'), $this->_reference_no);

                $this->_log("ERROR: Transaction expection occured for transaction ref ".$this->_reference_no.": ".$e->getMessage());
                return FALSE;

            } catch (TransactionException $e) {
                $this->_response['STATUS'] = "FAIL";
                $this->_response['MESSAGE'] = sprintf(translate('utransact_problem_while_processing'), $this->_reference_no);

                $this->_log("ERROR: Transaction expection occured for transaction ref ".$this->_reference_no.": ".$e->getMessage());
                return FALSE;

            } catch (CoreUnavailableException $e) {

                // Core is not available
                $this->_log("ERROR: Attempt #$process_attempt_count, Core not available while processing transaction ".$this->_reference_no." :".$e->getMessage());

				// Maximum attempts made, quitting now
                if ($process_attempt_count > MAX_TRANSACTION_PROCESS_ATTEMPT) {
                    $this->_response['STATUS'] = "FAIL";
                    $this->_response['MESSAGE'] = sprintf(translate('utransact_core_not_available_for_processing'), $this->_reference_no);

					return FALSE;
                }

				// Maximum attempt count not reached yet, try again to process the transaction
                $process_attempt_count++;
			}

		}//end while


        // If new transaction was processed then check for error(s) in response, if any
        $transaction_errors = $this->_getTransactionErrors($transaction_result);

		$transaction_successful = FALSE;

        // If there was error processing transaction, then transaction was unsuccessful
        if (!empty($transaction_errors)) {

            $error_str ='';
            foreach($transaction_errors as $error_id => $error_msg) {
                $error_str .= $error_msg."<br>";
            }

            $status = 'FAIL';
            $message = $error_str;

			$this->_log("ERROR: Error occured while processing transaction ref ".$this->_reference_no." : ".$message);

        } else {

            try {
                // Get the Transaction details just processed
                $transaction_details_response = $this->_transactions_client->__soapCall(
                                            'GetDetails',
                                            Array(
                                            'parameters' => Array(
                                                'bgaName' 		=> $this->_username,
                                                'referenceId'	=> $this->_reference_no,
                                                ),
                                            ),
                                            Array('uri' => ''),
                                            $transaction_auth_header
                                        );
            } catch (SoapFault $fault) {
                // Transaction completed, but couldn't obtained the details
                  $this->_log("ERROR: Transaction ref ".$this->_reference_no." completed, but couldn't obtain details: ".$e->getMessage());
            }

            if (isset($transaction_details_response->GetDetailsResult->any)) {

                $transaction_info = $this->_getTransactionDetails($transaction_details_response->GetDetailsResult->any);

                $status = $transaction_info['Status'];
                $message = "Transaction successfull";

                $this->_response['AMOUNT']  = $transaction_info['Amount'];
                $this->_response['GST']  = $transaction_info['GST'];
                $this->_response['INVOICE_NUMBER']  = $transaction_info['InvoiceNumber'];
                $this->_response['UTRANSACT_REFERENCE']  = $transaction_info['uTransactRef'];
                $this->_response['TRANSACTION']  = $transaction_info['uTransactRef'];
                $this->_response['MERCHANT_ID']  = $transaction_info['MerchantId'];
				$this->_response['DATE']  = $transaction_info['Date'];
				$this->_response['PAYMENT_GATEWAY_XML'] = $transaction_details_response->GetDetailsResult->any;

            } else {
                // Transaction completed, but couldn't obtain the details
                $status = "UNKNOWN";
                $message = sprintf(translate('utransact_cannot_obtain_transaction_details'), $this->_reference_no);

				$this->_log("ERROR: Couldn't get transaction ref ".$this->_reference_no." details");
            }

			$transaction_successful = TRUE;
        }

		$this->_response['REFERENCE'] = $this->_reference_no;
		$this->_response['STATUS'] = $status;
		$this->_response['MESSAGE'] = $message;

		// Log this transaction
		$this->_logRequest();
		$this->_logResponse();

		return $transaction_successful;

	}//end process()



	/**
	* Get unique transaction handle for the transation
	*
	* This transaction handle has a very short lifetime (< 2 minutes)
	*
	* @return string|boolean
	* @access public
	*/
	function getTransactionHandle()
	{
		$auth_header = $this->_getAuthSoapHeader($this->_auth_transactions_header_ns);

        if (!$auth_header) {
            $this->_response['STATUS'] = "FAIL";
			$this->_response['MESSAGE'] = "Cannot obtain payment gateway authentication soap header";

			$this->_log("Cannot obtain authentication soap header");
			return FALSE;
        }

		// Get the transaction handle from the webservice
		try {
			$transaction_handle = $this->_transactions_client->__soapCall(
										'RequestTransactionHandle',
										Array(),
                                        Array('uri' => ''),
                                        $auth_header
									);
		} catch(SoapFault $fault){
			$this->_log($fault->getMessage());

			if (isset($_SESSION['UTRANSACT_AUTH_TOKEN'])) unset($_SESSION['UTRANSACT_AUTH_TOKEN']);

			return FALSE;
		}

        return isset($transaction_handle->RequestTransactionHandleResult) ? $transaction_handle->RequestTransactionHandleResult : FALSE;

	}//end getTransactionHandle()


	/**
	* Refreshes the authentication token
	*
	* If existing token doesn't exist or has expired then set a new one
	*
	* @return boolean
	* @access private
	*/
	function _refreshAuthToken()
	{
		// If token is not found, try to get it from session
		if (empty($this->_auth_token)) {
			$this->_auth_token = isset($_SESSION['UTRANSACT_AUTH_TOKEN']) ? $_SESSION['UTRANSACT_AUTH_TOKEN'] : '';
		}

		// If we have token already then check if its still valid
		if (!empty($this->_auth_token)) {

			try {
				$soap_response = $this->_services_client->__soapCall(
											'isValidLogin',
											Array('parameters' =>
												Array(
													'authCookie' => $this->_auth_token,
												),
											)
										);
			} catch(SoapFault $fault){
				$this->_log($fault->getMessage());
				return FALSE;
			}

			// If its valid then we're fine, we don't require new one
			if (isset($soap_response->IsValidLoginResult) && $soap_response->IsValidLoginResult) {
                return TRUE;
            }
		}

		// Token has not been set or has expired
		try{
	    	$auth_token = $this->_bgas_client->__soapCall(
											'LoginBga',
											Array('parameters' => Array(
																'username'	=> $this->_username,
																'password'	=> $this->_password,
													)
											)
										);
		}catch(SoapFault $fault){
			$this->_log($fault->getMessage());
			return FALSE;
		}

		// Extract the bga authenticaitn token from the response soap header
		$response_xml = $this->_bgas_client->__getLastResponse();
        preg_match("|<BgaAuthToken>(.*?)</BgaAuthToken>|i", $response_xml, $bga_token_match);

		$auth_token = isset($bga_token_match[1]) ? $bga_token_match[1] : '';

		if (!$auth_token) {
			$this->_log("BgaAuthToken not found in LoginBga() response");
			return FALSE;
		}

		// Store the authentication token in session so that we don't have to re-login each time for every soap request
		$this->_auth_token = $auth_token;
		$_SESSION['UTRANSACT_AUTH_TOKEN'] = $this->_auth_token;

		return TRUE;

	}//end _refreshAuthToken()


	/**
	* Get PgWebServiceAuthToken (authentication token) SOAP header
	*
	* @parm $namespace  Namespace to set for the SOAP header
	*
	* @return object|boolean
	* @access private
	*/
	function _getAuthSoapHeader($namespace)
	{
	    // Refresh (or set if not already) the UTransact authentication token
		if (!$this->_refreshAuthToken()) {
            $this->_response['STATUS'] = "FAIL";
			$this->_response['MESSAGE'] = "Cannot acquire Utransact bga authentication token";
            return FALSE;
		}

        // Reuse BGA token as User token
        $soap_header_data = "
            <PgWebServiceAuthToken".(empty($namespace) ? ">" : " xmlns='".$namespace."'>")."
                <BgaAuthToken>".$this->_auth_token."</BgaAuthToken>
                <UserAuthToken>".$this->_auth_token."</UserAuthToken>
            </PgWebServiceAuthToken>";


		$soap_header_var = @new SoapVar($soap_header_data, XSD_ANYXML);
        $soap_header = @new SoapHeader($namespace, "PgWebServiceAuthToken", $soap_header_var);

        return $soap_header;

	}//end _getAuthSoapHeader()


	/**
	* Setup the credit card list supported by UTransact webservice
	*
	* @return boolean
	* @access private
	*/
	function _setSupportedCreditCardTypes()
	{
        $auth_header = $this->_getAuthSoapHeader($this->_auth_bgas_header_ns);

        if (!$auth_header) {
            $this->_response['STATUS'] = "FAIL";
			$this->_response['MESSAGE'] = "Cannot obtain list of credit card supported by payement gateway";

			$this->_card_types = Array();

			return FALSE;
        }

		try {
			$cards_dataset = $this->_bgas_client->__soapCall(
										'GetSupportedCreditCards',
										Array(),
                                        Array('uri' => ''),
                                        $auth_header
									);
		} catch(SoapFault $fault){
			$this->_log($fault->getMessage());
			return FALSE;
		}

		preg_match_all('|<Id>(.*?)</Id>.*?<Name>(.*?)</Name>|msi', $cards_dataset->GetSupportedCreditCardsResult->any, $card_matches);

		$card_list = Array();
		foreach((isset($card_matches[1]) ? $card_matches[1] : Array()) as $key => $card_id) {
			$card_list[$card_id] = isset($card_matches[2][$key]) ? $card_matches[2][$key] : $card_id;
		}

		$this->_card_types = $card_list;

        return TRUE;

	}//end _setSupportedCreditCardTypes()


	/**
	* Return the GST rate set from the payment gateway
	*
	* @return string|boolean
	* @access public
	*/
	function getTransactionGstRate()
	{
        $auth_header = $this->_getAuthSoapHeader($this->_auth_transactions_header_ns);

        if (!$auth_header) {
            $this->_log("Cannot obtain authetication header while attempting to get GST rate from payment gateway");
			return FALSE;
        }

		try {
			$gst_rate_response = $this->_transactions_client->__soapCall(
										'GetGstRate',
										Array(),
                                        Array('uri' => ''),
                                        $auth_header
									);
		} catch(SoapFault $fault){
			$this->_log($fault->getMessage());
			return FALSE;
		}

        return isset($gst_rate_response->GetGstRateResult) ? $gst_rate_response->GetGstRateResult : FALSE;

	}//end getTransactionGstRate()


	/**
	* Get the error info from the transaction respose
	*
	* @parm string $result	XML content returned by webservice
	*
	* @return array
	* @access private
	*/
	function _getTransactionErrors($result)
	{
        if (empty($result)) {
            return Array(sprintf(translate('utransact_empty_response_while_processing'), $this->_reference_no));
        }

        preg_match_all('|<ErrorCode>(.*?)</ErrorCode>.*?<DisplayMessage>(.*?)</DisplayMessage>|i',$result, $error_match);

        // No error message in response
        if (!isset($error_match[1][0])) return Array();

        $errors = Array();
        foreach($error_match[1] as $index => $error_code) {
            $errors[$error_code] = isset($error_match[2][$index]) ? $error_match[2][$index] : '';
        }

        return $errors;

	}//end _getTransactionErrors()


	/**
	* Get the invoice number from the transaction respose
	*
	* @parm string $result	XML content returned by webservice
	*
	* @return string
	* @access private
	*/
	function _getTransactionInvoiceNumber($result)
	{
        if (empty($result)) {
            return sprintf(translate('utransact_empty_response_while_processing'), $this->_reference_no);
        }

        preg_match('|<InvoiceNumber>(.*?)</InvoiceNumber>|',$result, $match);

        return isset($match[1]) ? $match[1] : '' ;

	}//end _getTransactionInvoiceNumber()


	/**
	* Get the transaction details from the transaction response
	*
	* @parm string $result	XML content returned by webservice
	*
	* @return Array()
	* @access public
	*/
	function _getTransactionDetails($result)
	{
        if (empty($result)) {
            return Array(sprintf(translate('utransact_empty_response_while_processing'), $this->_reference_no));
        }

        $details = Array();

		// Extrace the following details from the transaction response
        foreach(Array('ReferenceId', 'InvoiceNumber', 'Date', 'Status', 'Amount' , 'GST', 'uTransactRef', 'MerchantId') as $field) {
            $match = Array();

            preg_match("|<Transaction xmlns.*?<$field>(.*?)</$field>.*?</Transaction>|msi", $result, $match);
            $details[$field] = isset($match[1]) ? $match[1] : '' ;
        }

        return $details;

	}//end _getTransactionDetails()


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
	* Get URL for UTransact transactions webservice
	*
	* @return string
	* @access public
	*/
	public function getWebserviceTransactionsURL()
	{
		return $this->_webservice_transactions_url;


	}//end getWebserviceTransactionsURL()


	/**
	* Get URL for UTransact services webservice
	*
	* @return string
	* @access public
	*/
	public function getWebserviceServicesURL()
	{
        return $this->_webservice_services_url;


	}//end getWebserviceServicesURL()


	/**
	* Set credit card number
	*
	* @param string	$credit_card_no
	*
	* @return void
	* @access public
	*/
	public function setCreditCard($credit_card_no)
	{
		$this->_credit_card = $credit_card_no;

	}//end setCreditCard


	/**
	* Set credit card cv2 number
	*
	* @param string	$cv2
	*
	* @return void
	* @access public
	*/
	public function setCreditCardCV2($cv2)
	{
		$this->_credit_card_cv2 = $cv2;

	}//end setCreditCardCV2



	/**
	* Set transaction reference number
	*
	* @param string	$reference_no
	*
	* @return void
	* @access public
	*/
	public function setTransactionReferenceNo($reference_no)
	{
		$this->_reference_no = $reference_no;

	}//end setTransactionReferenceNo


	/**
	* Set expiry date month
	*
	* @param string	$month
	*
	* @return void
	* @access public
	*/
	public function setCardExpiryMonth($month)
	{
		$this->_expiry_month = strtoupper($month);

	}//end setExipryMonth


	/**
	* Set expiry date year
	*
	* @param string	$year
	*
	* @return void
	* @access public
	*/
	public function setCardExpiryYear($year)
	{
		$this->_expiry_year = $year;

	}//end setExipryYear


	/**
	* Set card holder name
	*
	* @param string	$card_holder_name
	*
	* @return void
	* @access public
	*/
	public function setCardHolderName($card_holder_name)
	{
		$this->_card_holder_name = $card_holder_name;

	}//end setCardHolderName


	/**
	* Set merchant id
	*
	* @param int	$merchant id
	*
	* @return void
	* @access public
	*/
	public function setMerchantId($merchant_id)
	{
		$this->_merchant_id = $merchant_id;

	}//end setMerchantId


	/**
	* Set payment model
	*
	* @param string	$model
	*
	* @return void
	* @access public
	*/
	public function setPaymentModel($model)
	{
		$this->_payment_model = $model;

	}//end setPaymentModel


	/**
	* Get transaction reference number
	*
	* @return string
	* @access public
	*/
	public function getTransactionReferenceNo()
	{
		return $this->_reference_no;

	}//end getTransactionReferenceNo()


	/**
	* Get credit card number
	*
	* @return string
	* @access public
	*/
	public function getCreditCard()
	{
		return $this->_credit_card;

	}//end getSurname


	/**
	* Get credit card cv2 number
	*
	* @return string
	* @access public
	*/
	public function getCreditCardCV2($cv2)
	{
		return $this->_credit_card_cv2;

	}//end setCreditCardCV2


	/**
	* Get expiry date
	*
	* @return string
	* @access public
	*/
	public function getExpiryDate()
	{
		return $this->_expiry_date;

	}//end getExipryDate


	/**
	* Get card holder name
	*
	* @return void
	* @access public
	*/
	public function getCardHolderName()
	{
		return $this->_card_holder_name;

	}//end getCardHolderName


	/**
	* Get merchant id
	*
	* @return int
	* @access public
	*/
	public function getMerchantId()
	{
		return $this->_merchant_id;

	}//end getMerchantId


	/**
	* Get payment model
	*
	* @return string
	* @access public
	*/
	public function getPaymentModel()
	{
		return $this->_payment_model;

	}//end getPaymentModel


	/**
	* Set first name
	*
	* @param string	$first_name	    First name
	*
	* @return void
	* @access public
	*/
	public function setFirstName($first_name)
	{
		$this->_first_name = $first_name;

	}//end setFirstName


	/**
	* Set surname
	*
	* @param string	$first_name	    First name
	*
	* @return void
	* @access public
	*/
	public function setSurname($surname)
	{
		$this->_surname = $surname;

	}//end setSurname


	/**
	* Set email
	*
	* @param string	$first_name	    First name
	*
	* @return void
	* @access public
	*/
	public function setEmail($email)
	{
		$this->_email = $email;

	}//end setEmail


	/**
	* Set address line 1
	*
	* @param string	$first_name	    First name
	*
	* @return void
	* @access public
	*/
	public function setAddressLine1($address_1)
	{
		$this->_address_line_1 = $address_1;

	}//end setAddressLine1


	/**
	* Set address line 2
	*
	* @param string	$first_name	    First name
	*
	* @return void
	* @access public
	*/
	public function setAddressLine2($address_2)
	{
		$this->_address_line_2 = $address_2;

	}//end setAddressLine2


	/**
	* Set suburb
	*
	* @param string	$first_name	    First name
	*
	* @return void
	* @access public
	*/
	public function setSuburb($suburb)
	{
		$this->_suburb = $suburb;

	}//end setEmail


	/**
	* Set city
	*
	* @param string	$first_name	    First name
	*
	* @return void
	* @access public
	*/
	public function setCity($suburb)
	{
		$this->_city= $suburb;

	}//end setEmail


	/**
	* Set postcode
	*
	* @param string	$first_name	    First name
	*
	* @return void
	* @access public
	*/
	public function setPostcode($postcode)
	{
		$this->_postcode = $postcode;

	}//end setEmail()


	/**
	* Get first name
	*
	* @return string
	* @access public
	*/
	public function getFirstName()
	{
		return $this->_first_name;

	}//end getFirstName()


	/**
	* Get surname
	*
	* @return string
	* @access public
	*/
	public function getSurname()
	{
		return $this->_surname;

	}//end getSurname()


	/**
	* Get email
	*
	* @return string
	* @access public
	*/
	public function getEmail()
	{
		return $this->_email;

	}//end getEmail()


	/**
	* Get address line 1
    *
	* @return string
	* @access public
	*/
	public function getAddressLine1()
	{
		return $this->_address_line_1;

	}//end getAddressLine1()


	/**
	* Get address line 2
	*
	* @return string
	* @access public
	*/
	public function getAddressLine2()
	{
		return $this->_address_line_2;

	}//end getAddressLine2()


	/**
	* Get suburb
	*
	* @return string
	* @access public
	*/
	public function getSuburb()
	{
		return $this->_suburb;

	}//end getEmail()


	/**
	* Get city
	*
	* @return string
	* @access public
	*/
	public function getCity()
	{
		return $this->_city;

	}//end getEmail()


	/**
	* Get postcode
	*
	* @return int
	* @access public
	*/
	public function getPostcode()
	{
		return $this->_postcode;

	}//end getEmail()


	/**
	* Set reference id
	*
	* @param string	$ref_id
	*
	* @return void
	* @access public
	*/
	public function setReferenceId($ref_id)
	{
		$this->_reference_id = $ref_id;

	}//end setReferenceId


	/**
	* Set application username
	*
	* @param string	$username
	*
	* @return void
	* @access public
	*/
	public function setApplicationUserName($username)
	{
		$this->_application_username = $username;

	}//end setApplicaitonUsername


	/**
	* Set name
	*
	* @param string	$name
	*
	* @return void
	* @access public
	*/
	public function setName($name)
	{
		$this->_name = $name;

	}//end setName


	/**
	* Set description
	*
	* @param string	$description
	*
	* @return void
	* @access public
	*/
	public function setDescription($description)
	{
		$this->_description = $description;

	}//end setDescription


	/**
	* Set quantity
	*
	* @param int	$quantity
	*
	* @return void
	* @access public
	*/
	public function setQuantity($quantity)
	{
		$this->_quantity = $quantity;

	}//end setQuantity


	/**
	* Set refund type
	*
	* @param string	$refund_type
	*
	* @return void
	* @access public
	*/
	public function setRefundType($refund_type)
	{
		$this->_refund_type = $refund_type;

	}//end setRefundType


	/**
	* Set refund expiry date
	*
	* @param string	$refund_expiry_date
	*
	* @return void
	* @access public
	*/
	public function setRefundExpiryDate($refund_expiry_date)
	{
		$this->_refund_expiry_date = $refund_expiry_date;

	}//end setRefundExpiryDate


	/**
	* Set product code
	*
	* @param String	$product_code
	*
	* @return void
	* @access public
	*/
	public function setProductCode($product_code)
	{
		$this->_product_code = $product_code;

	}//end setProductCode


	/**
	* Set financial code
	*
	* @param string	$financial_code
	*
	* @return void
	* @access public
	*/
	public function setFinancialCode($financial_code)
	{
		$this->_financial_code = $financial_code;

	}//end setFinancialCode


	/**
	* Set financial values
	*
	* @param string	$financial_values
	*
	* @return void
	* @access public
	*/
	public function setFinancialValues($financial_values)
	{
		$this->_financial_values = $financial_values;

	}//end setFinancialValues


	/**
	* Set financial values
	*
	* @param string	$additional_info
	*
	* @return void
	* @access public
	*/
	public function setAdditionalInfo($additional_info)
	{
		$this->_additional_info = $additional_info;

	}//end setAdditionalInfo


	/**
	* Add the cart item to the list
	*
	* @param string	$application_username
	* @param string	$name
	* @param string	$description
	* @param int	$quantity
    * @param string	$refund_type
    * @param string	$refund_expiry_date
    * @param float	$amount
    * @param string	$product_code
    * @param string	$financial_code
    * @param string	$financial_values
    * @param string	$additional_info
	*
	* @return boolean
	* @access public
	*/
	function addCartItem($application_username, $name, $description, $quantity, $refund_type, $refund_expiry_date, $amount, $product_code, $financial_code, $financial_values, $additional_info)
	{
        $cart_item = Array(
                "ApplicationUsername"   => $application_username,
                "Name"                  => $name,
                "Description"           => $description,
                "Quantity"              => $quantity,
                "RefundType"            => $refund_type,
                "RefundExpiryDate"      => $refund_expiry_date,
                "Amount"                => $amount,
                "ProductCode"           => $product_code,
                "FinancialCode"         => $financial_code,
                "FinancialValues"       => $financial_values,
                "AdditionalInfo"        => $additional_info,
            );


        $this->_cart_items[] = $cart_item;

        return TRUE;

	}


	/**
    * Clear the shopping cart
	*
	* @return void
	* @access public
	*/
	function clearCart()
	{
        $this->_cart_items = Array();
	}


	/**
    * Returns the cart items
	*
	* @return boolean
	* @access public
	*/
	function getCartItems()
	{
        return $this->_cart_items;
	}


	/**
	* Get expiry date month
	*
	* @return string
	* @access public
	*/
	public function getCardExpiryMonth($month)
	{
		return $this->_expiry_month;

	}//end getExipryMonth


	/**
	* Get expiry date year
	*
	* @return string
	* @access public
	*/
	public function getCardExpiryYear($year)
	{
		return $this->_expiry_year;

	}//end getExipryYear


	/**
	* returns array of possible card types supported by gateway
	*
	* @access public
	* @return array
	*/
	function getCardTypes()
	{
		return $this->_card_types;

	}//end getCardTypes()


	/**
	* Get array of valid card numbers for testing
	*
	* @access public
	* @return array
	*/
	function getTestCardNumbers()
	{
		return $this->_test_card_numbers;

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
		$message .= 'Card holder name: '.$this->_card_holder_name."\n";
		$message .= 'Card number: ****-****-****-'.substr($this->_credit_card, -4)."\n";
		$message .= 'Amount: '.array_get_index($this->_response, 'AMOUNT', '')."\n";
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
		$message .= 'Transaction Ref: '.$this->_reference_no."\n";
		$message .= 'Invoice No.: '.array_get_index($this->_response, 'INVOICE_NUMBER', '')."\n";
		$message .= 'UTransct Ref: '.array_get_index($this->_response, 'UTRANSACT_REFERENCE', '')."\n";
		$message .= 'Merchant Id: '.array_get_index($this->_response, 'MERCHANT_ID', '')."\n";
		$message .= 'Time: '.array_get_index($this->_response, 'DATE', date('r'))."\n";
		$message .= 'Status: '.array_get_index($this->_response, 'STATUS', '')."\n";
		$message .= 'Message: '.array_get_index($this->_response, 'MESSAGE', '')."\n";
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
