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
* $Id: payment_gateway_securepay.inc,v 1.3 2013/07/10 00:50:28 cupreti Exp $
*
*/


require_once dirname(__FILE__).'/../../payment_gateway/payment_gateway.inc';
require_once dirname(__FILE__).'/securepay_processor.inc';


/**
* Payment_Gateway_Securepay
*
* @see Payment_Gateway
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Payment_Gateway_Securepay extends Payment_Gateway
{

	/**
	* The pass through variable array
	* @var array
	*/
	private $_pass_through_vars = NULL;


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


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

		return ($this->_createBodycopy('display_format', translate('default_bodycopy_content_pg_securepay')));

	}//end _createAdditional()


	/**
	* Create Bodycopy
	*
	* @param string	$link_value		the link value of the link between this Datacash Payment Gateway asset and the new bodycopy
	* @param string	$content		the content to put into the bodycopy
	*
	* @return string
	* @access private
	*/
	function _createBodycopy($link_value, $content)
	{
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');
		$bodycopy = new Bodycopy();
		$copy_link = Array('asset' => &$this, 'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1, 'value' => $link_value);

		$bodycopy->setAttrValue('name', ucwords(str_replace('_',' ', $link_value)));
		$args = Array('content' => $content);
		if ($bodycopy->create($copy_link, $args)) {
			$child_bodycopy = $GLOBALS['SQ_SYSTEM']->am->getLink($bodycopy->id, SQ_LINK_TYPE_2, 'bodycopy_div');
			$GLOBALS['SQ_SYSTEM']->am->releaseLock($child_bodycopy['minorid'], 'attributes');
			return TRUE;
		}
		return FALSE;

	}//end _createBodycopy()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowedLinks()
	*/
	function _getAllowedLinks()
	{
		$page_links = parent::_getAllowedLinks();
		$page_links[SQ_LINK_TYPE_2]['bodycopy'] = Array('card' => 2,   'exclusive' => TRUE);
		return $page_links;

	}//end _getAllowedLinks()


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
		$keywords['card_number'] = 'Card Number';
		$keywords['card_expiry_month'] = 'Card Expiry Month (text box)';
		$keywords['card_expiry_month_select'] = 'Card Expiry Month (dropdown box)';
		$keywords['card_expiry_year'] = 'Card Expiry Year (text box)';
		$keywords['card_expiry_year_select_X'] = 'Card Expiry Year (dropdown box)';
		$keywords['card_cv2'] = 'Card CV2';
		$keywords['card_type'] = 'Card Type';

		$keywords['billing_address_first_name'] = 'Card holder\'s first name';
		$keywords['billing_address_last_name'] = 'Card holder\'s last name';
		$keywords['billing_address_street'] = 'Billing address street (street number, name and type)';
		$keywords['billing_address_postcode'] = 'Billing address postcode';
		$keywords['billing_address_city'] = 'Billing address city';
		$keywords['billing_address_state'] = 'Billing address state';
		$keywords['billing_address_country'] = 'Billing address country';
		$keywords['customer_email_address'] = 'Customer\'s email address';

		$keywords['submit_button'] = 'Submit Button';
		$keywords['cancel_button'] = 'Cancel Button';
		$keywords['cancel_url'] = 'Cancel URL';
		$keywords['reset_button'] = 'Reset Button';

		// Transaction keywords
		$keywords['transaction_amount'] = 'Transaction Amount';
		$keywords['transaction_currency'] = 'Transaction Currency';
		$keywords['processing_error'] = 'Processing Error (empty if no error)';

		return $keywords;

	}//end getAvailableKeywords()


	/**
	* Add valid keywords for this asset to an array of keywords when asked
	*
	* @param object	&$broadcaster	the asset that triggered the event
	* @param array	$vars			the vars that get submitted by the broadcaster
	*								we add keywords to the $vars['keywords'] array
	*
	* @return boolean
	* @access private
	*/
	function onRequestKeywords(&$broadcaster, $vars=Array())
	{
		$keywords = $this->getAvailableKeywords();
		$vars['keywords'] = array_merge($vars['keywords'], $keywords);

	}//end onRequestKeywords()


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

		// Make sure all the account parameters are set first
		if (!$this->checkAccountParameters()) {
			$this->_setError(translate('securepay_account_not_set'));
		}

		//this page is redirected to by the child iframe with an error message
		if (array_get_index($_GET, 'error_message', FALSE)) {
			$this->_setError($_GET['error_message']);
		}

		$success = FALSE;

		$amount = array_get_index($_SESSION,'SQ_ECOM_AMOUNT', 0);
		// TODO: how to action cancel
		$action = array_get_index($_REQUEST, $this->getPrefix().'_action', FALSE);

		// This is send back from SecurePay
		$signature = array_get_index($_REQUEST, 'fingerprint', '');

		if ($action == 'Cancel') {
			$this->returnToCaller('CANCEL');

		} else if (empty($amount)) {
			unset($_SESSION['SQ_ECOM_AMOUNT']);
			//clear pass through variables
			if (isset($_SESSION['SQ_ECOM_PASS_THROUGH_VARS'])) {
				unset($_SESSION['SQ_ECOM_PASS_THROUGH_VARS']);
			}
			$this->appendPendingOrderAuditLog(translate('ecom_payment_no_amount'));
			$this->returnToCaller('SUCCESS');

		} else if ($signature) {
			$this->appendPendingOrderAuditLog(translate('ecom_payment_attempted_gateway_contacted'));

			// Gateway has already processed the transaction at their end,
			// now verify the transaction and log it at our end
			$success = $this->processPayment();
		}

		//if transaction is complete, redirect the user to the success page
		if ($success) {
			unset($_SESSION['SQ_ECOM_AMOUNT']);
			//clear pass through variables
			if (isset($_SESSION['SQ_ECOM_PASS_THROUGH_VARS'])) {
				unset($_SESSION['SQ_ECOM_PASS_THROUGH_VARS']);
			}
			if (isset($_SESSION['SECUREPAY_REQUEST_TIMESTAMP'])) {
				unset($_SESSION['SECUREPAY_REQUEST_TIMESTAMP']);
			}
			//broadcast payment completed trigger event before return
			if (isset($_SESSION['SQ_ECOM_PAYMENT_COMPLETED_BROADCASTER_ASSETID'])) {
				$broadcaster = $GLOBALS['SQ_SYSTEM']->am->getAsset($_SESSION['SQ_ECOM_PAYMENT_COMPLETED_BROADCASTER_ASSETID']);
				if (!empty($broadcaster)) {
					//put all returned variables in $_SESSION['SQ_ECOM_RESPONSE'] to $_POST array so that the listening trigger can use them
					$response = array_get_index($_SESSION, 'SQ_ECOM_RESPONSE', FALSE);
					if ($response) {
						$_POST['transaction_reference'] = $response['MERCHANT_REFERENCE'];
						$_POST['transaction_time'] = $response['TRANSACTION_DATE'];
						$_POST['transaction_status'] = $response['STATUS'];
						$_POST['transaction_card_number'] = $response['CARDNO'];
						$_POST['transaction_amount'] = $response['AMOUNT'];
						$_POST['transaction_currency'] = $response['CURRENCY'];
					}
					$GLOBALS['SQ_SYSTEM']->broadcastTriggerEvent('trigger_event_ecommerce_payment_completed', $broadcaster);
				}
			}

			$this->appendPendingOrderAuditLog(translate('ecom_payment_succeeded'));

			$this->returnToCaller('SUCCESS');
		} else {
			if ($signature) {
				if (isset($_SESSION['SECUREPAY_REQUEST_TIMESTAMP'])) {
					unset($_SESSION['SECUREPAY_REQUEST_TIMESTAMP']);
				}
				$this->appendPendingOrderAuditLog(translate('ecom_payment_declined'));
			}
		}

		// Set the request timestamp in GMT/UTC
		$_SESSION['SECUREPAY_REQUEST_TIMESTAMP'] = gmdate('YmdHis');

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

		//OUTPUT HERE
		if ($link_value == 'display_format') {

			//the default body copy with card input
			$datapath = sq_web_path('data').'/asset_types/payment_gateway_securepay/files';
			$card = $this->getPrefix().'_card';

			if ($this->attr('test_mode')) {
				echo '<h2 class="gateway-test-mode" style="color: white; background-color: #C00; padding: 5px">TEST MODE</h2>';
				echo '<h3>In test mode, only amount ending with 00 and 08 will be approved by the gateway. All other amounts will return different unsuccessful codes</h3>';
			}

			?>
			<script src="<?php echo $datapath; ?>/loader.js"></script>
			<script>
				var Loader = new Loader('<?php echo $card; ?>','#FFFFFF','Processing Transaction...','<?php echo $datapath; ?>/loader.gif');
				Loader.print();
			</script>
			<form id="<?php echo $this->getPrefix().'_main'; ?>" method="post" action="<?php echo Securepay_Processor::getActionUrl($this->attr('test_mode')); ?>" onsubmit="Loader.show();">
			<?php
			$this->_printSecurePayFields();
			$bodycopy->printBody();
			echo '</form>';
		} else {
			//the card holder verification bodycopy
			$bodycopy->printBody();
		}

	}//end printBodycopy()


	/**
	* Return the bodycopy linked to this page
	*
	* @return object
	* @access public
	*/
	function getBodycopy($link_value)
	{
		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, $link_value);
		if (empty($link)) return NULL;
		$bodycopy = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
		return $bodycopy;

	}//end getBodycopy()


	/**
	* Prints the required SecurePay hidden input fields
	*
	*
	* @return void
	* @access private
	*/
	private function _printSecurePayFields()
	{
		$amount = sprintf('%0.2f', array_get_index($_SESSION, 'SQ_ECOM_AMOUNT', 0));

		// These are mandatory fields required by SecurePay
		ob_start();
			hidden_field('EPS_MERCHANT', $this->attr('merchant'));
			hidden_field('EPS_TXNTYPE', $this->attr('transaction_type'));
			hidden_field('EPS_REFERENCEID', array_get_index($_SESSION, 'SQ_ECOM_REF_NO', ''));
			hidden_field('EPS_AMOUNT', $amount);
			hidden_field('EPS_TIMESTAMP', array_get_index($_SESSION, 'SECUREPAY_REQUEST_TIMESTAMP', ''));
			hidden_field('EPS_FINGERPRINT', $this->_generateSecurePayFingerPrint());
			hidden_field('EPS_RESULTURL', $this->getUrl());
			hidden_field('EPS_REDIRECT', 'TRUE');

			// If sending FraudGuard parameters
			if ($this->attr('transaction_type') == 2) {
				$ip_address = array_get_index($_SERVER, 'REMOTE_ADDR', '');
				hidden_field('EPS_IP', $ip_address);
			}
			$output = ob_get_contents();
		ob_end_clean();

		echo $output;

	}//end _printSecurePayFields()


	/**
	* Logs and verify the response from the gateway
	* The transaction is processed by the gateway directly
	*
	* @return boolean	return TRUE if the transaction succeeds, otherwise return FALSE
	* @access public
	*/
	function processPayment()
	{
		// Get the response from returned by SecurePay
		$ref_id = array_get_index($_REQUEST, 'refid', '');
		$txn_id = array_get_index($_REQUEST, 'txnid', '');
		$summary_code = array_get_index($_REQUEST, 'summarycode', '');
		$res_code = array_get_index($_REQUEST, 'rescode', '');
		$res_text = array_get_index($_REQUEST, 'restext', '');
		$txn_date = array_get_index($_REQUEST, 'settdate', '');
		$response_ts = array_get_index($_REQUEST, 'timestamp', '');
		$response_sig = array_get_index($_REQUEST, 'fingerprint', '');
		$response_str = '';
		foreach($_REQUEST as $key => $val) {
			$response_str .= $key.'='.$val.'&';
		}
		$response_str = rtrim($response_str, '&');

		if (empty($summary_code) || empty($response_sig) || empty($txn_id) || empty($txn_date) || empty($response_sig)) {
			$this->_setError(translate('securepay_expected_parameter_not_returned'));
			return FALSE;
		}

		$success_codes = SecurePay_Processor::getTransactionSucessCodes();
		$success =  in_array($res_code, $success_codes);

		// Being here means transaction was processed. Now log and verify the result

		// Below should always be set, otherwise "signature" verification will definitely fail
		$transaction_ref = array_get_index($_SESSION, 'SQ_ECOM_REF_NO', '');
		$amount = array_get_index($_SESSION, 'SQ_ECOM_AMOUNT', 0);
		$timestamp = array_get_index($_SESSION, 'SECUREPAY_REQUEST_TIMESTAMP', '');

		$request_sig = $this->_generateSecurePayFingerPrint();

		// Validate the returned response by comparing the "fingerprint"
		$fingerprint = $this->_generateResponseSecurePayFingerPrint($response_ts, $summary_code);

		$request = Array(
						'AMOUNT' => $amount,
						'REQUEST_DATE' => $timestamp,
						'MERCHANT_REFERENCE' => $transaction_ref,
						'SIGNATURE' => $request_sig,
					);

		$response = Array(
						'STATUS' => $success ? '1' : '0',
						'TRANSACTION' => $txn_id,
						'MERCHANT_REFERENCE' => $ref_id,
						'RESPONSE_CODE' => $res_code,
						'RESPONSE_TEXT' => $res_text,
						'RESPONSE_DATE' => $response_ts,
						'SIGNATURE' => $response_sig,
						'VERIFICATION' => $response_sig == $fingerprint ? 'PASSED' : 'FAILED',
						'RESPONSE_STR' => $response_str,
					);

		// Set the results
		$this->_setTransactionResult($success, $response);

		// Log request and response
		$this->_logRequest($request);
		$this->_logResponse($response);

		// If signature verification fails, the transaction is not legit
		if ($response_sig != $fingerprint) {
			$this->_setError(translate('securepay_fingerprint_verification_failed'));
			$success = FALSE;
		}

		return $success;

	}//end processPayment()


	/**
	* Generate the "signature" hash for SecurePay
	*
	* @return string|boolean
	* @access private
	*/
	function _generateSecurePayFingerPrint()
	{
		$merchant = $this->attr('merchant');
		$password = $this->attr('password');
		$transaction_type = $this->attr('transaction_type');

		$transaction_ref = array_get_index($_SESSION, 'SQ_ECOM_REF_NO', '');
		$amount = array_get_index($_SESSION, 'SQ_ECOM_AMOUNT', '');
		$timestamp = array_get_index($_SESSION, 'SECUREPAY_REQUEST_TIMESTAMP', '');

		// Can't generate signature without these parameters
		if (!$merchant || !$password || !$transaction_ref || !$amount || !$timestamp) {
			return FALSE;
		}

		// Ordered array of elements required to generate the "fingerprint"
		$sig_parms = Array(
						$merchant,
						$password,
						$transaction_type,
						$transaction_ref,
						sprintf('%0.2f', $amount),
						$timestamp,
					);

		return sha1(implode('|', $sig_parms));

	}//end _generateSecurePayFingerPrint()


	/**
	* Generate the fingerprint from the response
	*
	* @param string $timestamp
	* @param string $summary_code
	*
	* @return string|boolean
	* @access private
	*/
	function _generateResponseSecurePayFingerPrint($timestamp, $summary_code)
	{
		$merchant = $this->attr('merchant');
		$password = $this->attr('password');
		$transaction_ref = array_get_index($_SESSION, 'SQ_ECOM_REF_NO', '');
		$amount = sprintf('%0.2f', array_get_index($_SESSION, 'SQ_ECOM_AMOUNT', '0'));

		return sha1($merchant.'|'.$password.'|'.$transaction_ref.'|'.$amount.'|'.$timestamp.'|'.$summary_code);

	}//end _generateResponseSecurePayFingerPrint()


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
			$this->_setError($response['RESPONSE_CODE'].': '.$response['RESPONSE_TEXT']);
		} else {
			if ($this->attr('test_mode')) {
				$response['STATUS'] = '!!!ATTENTION!!! TEST MODE (transaction not performed) -- '.$response['STATUS'];
			}

			$response['CARDNO'] = 'n/a';
			$response['AMOUNT'] = $_SESSION['SQ_ECOM_AMOUNT'];
			$response['CURRENCY'] = $this->attr('currency');
			$response['BILLING_ADDR'] = 'n/a';

			$_SESSION['SQ_ECOM_RESPONSE']   = $response;
		}

	}//end _setTransactionResult()


	/**
	* Get the gateway asset's "https" url
	*
	* @access public
	* @return string
	*/
	function getGatewaySecureUrl()
	{
		preg_match('!(https?)://(.*)!', $this->getUrl(), $matches);
		$protocol = array_get_index($matches, '1', '');
		$url = array_get_index($matches, '2', '');

		if ($protocol == 'https') {
			return 'https://'.$url;
		}


		$secure_url = '';
		$urls = $GLOBALS['SQ_SYSTEM']->am->getUrls($this->id);
		foreach($urls as $url_info) {
			if ($url_info['url'] == $url && $url_info['https']) {
				$secure_url = 'https://'.$url;
			}
		}

		return $secure_url;

	}//end getGatewaySecureUrl()


	/**
	* Returns TRUE if all the necessary account parameters are set
	*
	* @access public
	* @return boolean
	*/
	function checkAccountParameters()
	{
		$merchant = $this->attr('merchant');
		$password = $this->attr('password');
		$transaction_ref = array_get_index($_SESSION, 'SQ_ECOM_REF_NO', '');

		// Can't generate signature without these parameters
		if (!$merchant || !$password || !$transaction_ref) {
			return FALSE;
		}

		return TRUE;

	}//end checkAccountParameters()


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

		if (isset($_SESSION['SECUREPAY_REQUEST_TIMESTAMP'])) {
			unset($_SESSION['SECUREPAY_REQUEST_TIMESTAMP']);
		}

		if (is_null($back_url)) {
			trigger_error('Unknown caller reference');
		} else {
			header('Location: '.$back_url);
			exit;
		}

	}//end returnToCaller()


	/**
	 * Redirect the parent of the iframe that this page is loaded in
	 *
	 * @param string $url	the URL to redirect the parent to using Javascript
	 *
	 * @return void
	 * @access private
	 */
	private function _redirectParent($url)
	{
		$string = <<<HEREDOC
<script type="text/javascript">
	if (parent != self) {
		parent.location = "{$url}";
	}
</script>
HEREDOC;

		echo $string;
		exit;

	}//end _redirectParent()


	// Keyword Replacements /////////////////////

	/**
	* Get bodycopy keyword replacement
	*
	* If keyword is %card_start_year_select_XX% or %card_expiry_year_select_XX%, call the correct method with XX is
	* $period parameter. Otherwise, call the parent method.
	*
	* @param string $keyword    Keyword to find a replacement for
	*
	* @return string
	* @access public
	* @see _getKeywordReplacement
	*/
	public function getKeywordReplacement($keyword)
	{
		$replacement = NULL;

        if (0 === strpos($keyword, 'card_expiry_year_select_')) {
            $period = substr($keyword, 24);
            if (is_numeric($period)) {
                $replacement = $this->getCardExpiryYearSelectKeywordReplacement($period);
            } else {
                $replacement = $this->getCardExpiryYearSelectKeywordReplacement();
			}
		} else {
			$replacement = parent::getKeywordReplacement($keyword);
		}

		return $replacement;

	}//end getKeywordReplacement()


	/**
	* Card Number keyword replacement
	*
	* @return string
	* @access public
	*/
	function getCardNumberKeywordReplacement()
	{
		$field_name = 'EPS_CARDNUMBER';

		// Set the test cards for test mode
		if ($this->attr('test_mode')) {
			$test_card = Securepay_Processor::getTestCardInfo();
			$string = '<select name="'.$field_name.'">'.
						'<option value="'.$test_card['card_number'].'">Valid Number</option>'.
						'<option value="4444333322221112">Invalid Number</option>'.
					  '</select>';
		} else {
			$string = '<input name="'.$field_name.'" autocomplete="off" />';
		}

		return $string;

	}//end getCardNumberKeywordReplacement()


	/**
	* Card Expiry Year keyword replacement
	*
	* @return string
	* @access public
	*/
	function getCardExpiryYearKeywordReplacement()
	{
		$field_name = 'EPS_EXPIRYYEAR';

		$string = '<input name="'.$field_name.'" size="2" autocomplete="off" />';

		return $string;

	}//end getCardExpiryYearKeywordReplacement()


	/**
	* Card Expiry Year Select keyword replacement
	*
	* @return string
	* @access public
	*/
	function getCardExpiryYearSelectKeywordReplacement($period=18)
	{
		$field_name = 'EPS_EXPIRYYEAR';

		$string = '<select name="'.$field_name.'">';
		$year = date('Y');
		for ($i = 0; $i <= $period; $i++) {
			$string .= '<option value="'.substr($year, -2).'" '.($i==0 ? 'selected' : '').'>'.$year.'</option>';
			$year++;
		}
		$string .= '</select>';

	return $string;

	}//end getCardExpiryYearSelectKeywordReplacement()


	/**
	* Card Expiry Month keyword replacement
	*
	* @return string
	* @access public
	*/
	function getCardExpiryMonthKeywordReplacement()
	{
		$field_name = 'EPS_EXPIRYMONTH';

		$string = '<input name="'.$field_name.'" size="2" autocomplete="off" />';

		return $string;

	}//end getCardExpiryMonthKeywordReplacement()


	/**
	* Card Expiry Month Select keyword replacement
	*
	* @return string
	* @access public
	*/
	function getCardExpiryMonthSelectKeywordReplacement()
	{
		$field_name = 'EPS_EXPIRYMONTH';

		$string = '<select name="'.$field_name.'">';
		for ($i = 1; $i <= 12; $i++) {
			$month = sprintf('%02s', $i);
			$string .= '<option value="'.$month.'" '.($i==1 ? 'selected' : '').'>'.$month.'</option>';
		}
		$string .= '</select>';

		return $string;

	}//end getCardExpiryMonthSelectKeywordReplacement()


	/**
	* Card cv2 keyword replacement
	*
	* @return string
	* @access public
	*/
	function getCardcv2KeywordReplacement()
	{
		$field_name = 'EPS_CCV';

		// Set the test card ccv for test mode
		if ($this->attr('test_mode')) {
			$test_card = Securepay_Processor::getTestCardInfo();
			$string = '<input type="hidden" name="'.$field_name.'" value="'.$test_card['ccv'].'">';
		} else {
			$string = '<input name="'.$field_name.'" size="4" autocomplete="off" />';
		}

		return $string;

	}//end getCardCv2KeywordReplacement()

	/**
	* Card Type keyword replacement
	*
	* @return string
	* @access private
	*/
	function getCardTypeKeywordReplacement()
	{
		// NOTE: This field is not processed by gateway

		$field_name = $this->getPrefix().'_card_type';

		$available_card_types = Securepay_Processor::getCardTypes();
		$selected_card_types = explode('|', $this->attr('supported_card_types'));

		ob_start();
		echo '<select name="'.$field_name.'">';
		foreach ($selected_card_types as $type) {
			echo '<option value="'.$type.'">'.$available_card_types[$type].'</option>';
		}

		echo '</select>';

		return ob_get_clean();

	}//end getCardTypeKeywordReplacement()

	/**
	* Billing First Names keyword replacement
	*
	* @return string
	* @access private
	*/
	function getBillingAddressFirstNameKeywordReplacement()
	{
		$field_name = 'EPS_FIRSTNAME';
		ob_start();
		text_box($field_name, $this->_getDefaultInputFromPassThroughAttribute('first_name_var_name'));

		return ob_get_clean();

	}//end getBillingAddressFirstNamesKeywordReplacement()


	/**
	* Billing Surname keyword replacement
	*
	* @return string
	* @access private
	*/
	function getBillingAddressLastNameKeywordReplacement()
	{
		ob_start();
		$field_name = 'EPS_LASTNAME';
		text_box($field_name, $this->_getDefaultInputFromPassThroughAttribute('last_name_var_name'));

		return ob_get_clean();

	}//end getBillingAddressLastNameKeywordReplacement()


	/**
	* Billing Street address keyword replacement
	*
	* @return string
	* @access private
	*/
	function getBillingAddressStreetKeywordReplacement()
	{
		// NOTE: This field is not processed by gateway
		// Its here just to make look the Billing Address form look normal

		ob_start();
		$field_name = $this->getPrefix().'_billing_address_street';
		text_box($field_name, $this->_getDefaultInputFromPassThroughAttribute('addr_street_var_name'));

		return ob_get_clean();

	}//end getBillingAddressAddressLine1KeywordReplacement()


	/**
	* Billing State keyword replacement
	*
	* @return string
	* @access private
	*/
	function getBillingAddressStateKeywordReplacement()
	{
		// NOTE: This field is not processed by gateway
		// Its here just to make look the Billing Address form look normal
		$field_name = $this->getPrefix().'_billing_address_state';

		ob_start();
		text_box($field_name, $this->_getDefaultInputFromPassThroughAttribute('state_var_name'));

		return ob_get_clean();

	}//end getBillingAddressStateKeywordReplacement()


	/**
	* Billing City keyword replacement
	*
	* @return string
	* @access private
	*/
	function getBillingAddressCityKeywordReplacement()
	{
		$field_name = 'EPS_TOWN';
		ob_start();
		text_box($field_name, $this->_getDefaultInputFromPassThroughAttribute('city_var_name'));

		return ob_get_clean();

	}//end getBillingAddressCityKeywordReplacement()


	/**
	* Buyer's email address
	*
	* @return string
	* @access private
	*/
	function getCustomerEmailAddressKeywordReplacement()
	{
		$field_name = 'EPS_EMAILADDRESS';
		ob_start();
		text_box($field_name, $this->_getDefaultInputFromPassThroughAttribute('email_var_name'));

		return ob_get_clean();

	}//end getBillingAddressCityKeywordReplacement()



	/**
	* Billing Postcode keyword replacement
	*
	* @return string
	* @access private
	*/
	function getBillingAddressPostcodeKeywordReplacement()
	{
		$field_name = 'EPS_ZIPCODE';

		ob_start();
		text_box($field_name, $this->_getDefaultInputFromPassThroughAttribute('postcode_var_name'));

		return ob_get_clean();

	}//end getBillingAddressPostcodeKeywordReplacement()


	/**
	* Billing Country keyword replacement
	*
	* @return string
	* @access private
	*/
	function getBillingAddressCountryKeywordReplacement()
	{
		global $standards_lists_countries;
		require_once SQ_FUDGE_PATH.'/standards_lists/countries.inc';
		$fiels_name = 'EPS_BILLINGCOUNTRY';

		ob_start();
		combo_box($fiels_name, $standards_lists_countries, FALSE, $this->_getDefaultInputFromPassThroughAttribute('country_var_name'));

		return ob_get_clean();

	}//end getBillingAddressCountryKeywordReplacement()


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

		return '<input type="submit" value="'.$button_text.'" />';

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
		$onclick = "document.getElementById('".$this->getPrefix()."_main').action='".array_get_index($_SESSION, 'SQ_ECOM_CANCEL_URL', '')."'";
		$button_text = $this->attr('cancel_text');
		if (empty($button_text)) $button_text = 'Cancel';

		return '<input type="submit" name="'.$this->getPrefix().'_action" value="'.$button_text.'" onClick="'.$onclick.'" />';

	}//end getCancelButtonKeywordReplacement()



	/**
	* Cancel url keyword replacement
	*
	* @return string
	* @access public
	*/
	function getCancelUrlKeywordReplacement()
	{
		return array_get_index($_SESSION, 'SQ_ECOM_CANCEL_URL', '');

	}//end getCancelUrlKeywordReplacement()


	/**
	 * Set error for the transaction so that the getProcessingErrorKeywordReplacement() method will return the error message
	 *
	 * @param string $err_message	the error message to set
	 * @return void
	 * @access private
	 */
	private function _setError($err_message)
	{
		$this->_tmp['is_error'] = TRUE;
		$this->_tmp['error_message'][] = $err_message;

	}//end _setError()


	/**
	* Get pass-through variable which is stored in a Session variable
	*
	* @param string    $var_name   the name of the pass-through variable
	* @param mixed     $default    the default value to get if the $var_name variable is not in the Session
	*
	* @return mixed
	* @access private
	*/
	private function _getPassThroughVariable($var_name, $default)
	{
		if (is_null($this->_pass_through_vars)) {
			$this->_pass_through_vars = array_get_index($_SESSION, 'SQ_ECOM_PASS_THROUGH_VARS', Array());
		}

		return array_get_index($this->_pass_through_vars, $var_name, $default);

	}//end _getPassThroughVariable()


	/**
	* Get pass-through variable with the variable names from attributes
	*
	* @param string    $attr_name      the name of the attribute that stores the variable name
	*
	* @return mixed
	* @access private
	*/
	private function _getDefaultInputFromPassThroughAttribute($attr_name)
	{
		$result = '';
		$var_name = $this->attr($attr_name);
		if (!empty($var_name)) {
			$result = $this->_getPassThroughVariable($var_name, '');
		}

		return $result;

	}//end _getDefaultInputFromPassThroughAttribute()


	/**
	 * Check if the transaction has error
	 *
	 * @return boolean
	 * @access public
	 */
	public function isError()
	{
		return array_get_index($this->_tmp, 'is_error', FALSE);

	}//end isError()


	/**
	 * Get the error message under HTML format
	 *
	 * @return string		the HTML error message
	 * @access public
	 */
	public function getErrorMessage()
	{
		$err_message = '';
		$errors = array_get_index($this->_tmp, 'error_message', Array());
		foreach ($errors as $error) {
			$err_message .= "<li>".htmlspecialchars($error, ENT_NOQUOTES, SQ_CONF_DEFAULT_CHARACTER_SET)." </li>";
		}

		return '<ul>'.$err_message.'</ul>';

	}//end getErrorMessage()


	/**
	* Log the Request's information
	*
	* @return void
	* @access private
	*/
	private function _logRequest($request)
	{
		$message = "\nRequest:\n";
		$message .= 'Time: '.$request['REQUEST_DATE']."\n";
		$message .= 'Transaction Ref: '.$request['MERCHANT_REFERENCE']."\n";
		$message .= 'Card number: n/a'."\n";
		$message .= 'Amount: '.$request['AMOUNT']."\n";
		$message .= 'Currency: '.$this->attr('currency')."\n";
		$message .= 'Signature: '.$request['SIGNATURE']."\n";


		SecurePay_Processor::log($message, $this->attr('test_mode'));

	}//end _logRequest()


	/**
	* Log the Response's information
	*
	* @return void
	* @access private
	*/
	private function _logResponse($response)
	{
		$message = "\nResponse:\n";
		$message .= 'Transaction Ref: '.$response['TRANSACTION']."\n";
		$message .= 'Merchant Ref: '.$response['MERCHANT_REFERENCE']."\n";
		$message .= 'Signature: '.$response['SIGNATURE']."\n";
		$message .= 'Status: '.$response['STATUS']."\n";
		$message .= 'Verification: '.$response['VERIFICATION']."\n";
		$message .= 'Response Code: '.$response['RESPONSE_CODE']."\n";
		$message .= 'Response Text: '.$response['RESPONSE_TEXT']."\n";
		$message .= 'Time: '.$response['RESPONSE_DATE']."\n";
		$message .= 'Response String: '.$response['RESPONSE_STR']."\n";

		SecurePay_Processor::log($message, $this->attr('test_mode'));

    }//end _logResponse()


}//end class

?>
