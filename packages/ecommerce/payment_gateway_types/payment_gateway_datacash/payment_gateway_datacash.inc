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
* $Id: payment_gateway_datacash.inc,v 1.8 2012/11/16 05:00:47 cupreti Exp $
*
*/


require_once dirname(__FILE__).'/../../payment_gateway/payment_gateway.inc';
require_once dirname(__FILE__).'/datacash_processor.inc';
require_once SQ_DATA_PATH.'/private/conf/tools.inc';


/**
* Payment_Gateway_Datacash
*
* Interface for online credit card processing using Datacash payment gateway
*
*
* @author  Anh Ta <ata@squiz.co.uk>
* @version $Revision: 1.8 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Payment_Gateway_Datacash extends Payment_Gateway
{


	/**
	 * The response from Datacash with Card Holder Verification redirect information such as Access Control Server (ACS) URL, Payment Authentication Request (PAReq) and Datacash reference
	 * @var array
	 */
	private $_redirect_info = NULL;


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

		return ($this->_createBodycopy('display_format', translate('%processing_error%
<div style="background-color: #CCC; padding: 5px"><strong>&pound;%transaction_amount%</strong> Pounds Sterling (GBP) are about to be debited from your card</div>
<table cellpadding="5">
	<tr>
		<td align="right">Card Number</td>
		<td>%card_number%</td>
	</tr>
	<tr>
		<td align="right">Expiry Date (MM/YY)</td>
		<td>%card_expiry_month% / %card_expiry_year%</td>
	</tr>
	<tr>
		<td>&nbsp;</td>
		<td>%submit_button% %reset_button% %cancel_button%</td>
	</tr>
</table>')) && $this->_createBodycopy('card_holder_verification', translate('%card_holder_verification_form%')));

	}//end _createAdditional()


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

		$keywords['card_number'] = 'Card Number';
		$keywords['card_start_month'] = 'Card Start Month';
		$keywords['card_start_year'] = 'Card Start Year';
		$keywords['card_expiry_month'] = 'Card Expiry Month';
		$keywords['card_expiry_year'] = 'Card Expiry Year';
		$keywords['card_issue_number'] = 'Card Issue Number';
		$keywords['card_cv2'] = 'Card CV2';

		$keywords['street_address1'] = 'Street Address 1';
		$keywords['street_address2'] = 'Street Address 2';
		$keywords['street_address3'] = 'Street Address 3';
		$keywords['street_address4'] = 'Street Address 4';
		$keywords['postcode'] = 'Postcode';

		$keywords['transaction_amount'] = 'Transaction Amount';
		$keywords['transaction_currency'] = 'Transaction Currency';
		$keywords['processing_error'] = 'Processing Error (empty if no error)';
		$keywords['submit_button'] = 'Submit Button';
		$keywords['cancel_button'] = 'Cancel Button';
		$keywords['reset_button'] = 'Reset Button';
		$keywords['card_holder_verification_form'] = 'Card Holder Verification Form';

		return $keywords;

	}//end getAvailableKeywords()


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

		//this page is redirected to by the child iframe with an error message
		if (array_get_index($_GET, 'error_message', FALSE)) {
			$this->_setError($_GET['error_message']);
		}

		$success = FALSE;
		$is_in_frame = FALSE; //indicates whether this page is in the iframe after it is redirected to by the ACS

		if (empty($_POST['PaRes']) || empty($_POST['MD'])) {
			//THIS PAGE IS REDIRECTED TO BY DELIVERY METHOD OR FORM ECOMMERCE ASSET NOT BY THE ACS
			$amount = array_get_index($_SESSION,'SQ_ECOM_AMOUNT', 0);
			$card_info = array_get_index($_REQUEST, $this->getPrefix().'_card', FALSE);
			$action = array_get_index($_REQUEST, $this->getPrefix().'_action', FALSE);

			if ($action == 'Cancel') {
				$this->returnToCaller('CANCEL');

			} else if (empty($amount)) {
				unset($_SESSION['SQ_ECOM_AMOUNT']);
				$this->_unsetCurrency();
				//clear pass through variables
				if (isset($_SESSION['SQ_ECOM_PASS_THROUGH_VARS'])) {
					unset($_SESSION['SQ_ECOM_PASS_THROUGH_VARS']);
				}

				$this->appendPendingOrderAuditLog(translate('Payment has no amount'));


				$this->returnToCaller('SUCCESS');

			} else if ($card_info) {
				$this->appendPendingOrderAuditLog(translate('Payment Attempted (gateway contacted)'));


				$success = $this->processPayment($card_info);
				//if success = 2, redirect to the Access Control Server (ACS) for Card Holder Verification
				if ($success === 2) {
					//store the Datacash reference number to authorize payment after the card holder verify himself
					$_SESSION['SQ_ECOM_TRANSACTION_REF_NO'] = $this->_redirect_info['TRANSACTION'];
					//print the Card Holder Verification bodycopy
					$this->printBodycopy('card_holder_verification');
					return;
				}
			}

		} else if (!empty($_SESSION['SQ_ECOM_TRANSACTION_REF_NO']) && ($_POST['MD'] == md5($_SESSION['SQ_ECOM_REF_NO']))) {
			//THIS PAGE IS REDIRECTED TO BY THE ACCESS CONTROL SERVER (ACS) OF ISSUEING BANK AFTER CARD HOLDER VERIFICATION PROCESS
			$is_in_frame = TRUE;
			$datacash_ref = $_SESSION['SQ_ECOM_TRANSACTION_REF_NO'];
			unset($_SESSION['SQ_ECOM_TRANSACTION_REF_NO']);
			$success = $this->authorizePayment($datacash_ref, $_POST['PaRes']);
			if ($success === FALSE) {
				//transaction fails, redirect the parent document of the iframe and display error
				$this->_redirectParent($this->getURL().'?error_message='.urlencode($this->_tmp['error_message'][0]));
			}
		}

		//if transaction is accepted, redirect the user to the success page
		if ($success === 1) {
			unset($_SESSION['SQ_ECOM_AMOUNT']);
			$this->_unsetCurrency();
			//clear pass through variables
			if (isset($_SESSION['SQ_ECOM_PASS_THROUGH_VARS'])) {
				unset($_SESSION['SQ_ECOM_PASS_THROUGH_VARS']);
			}
			//broadcast payment completed trigger event before return
			if (isset($_SESSION['SQ_ECOM_PAYMENT_COMPLETED_BROADCASTER_ASSETID'])) {
				$broadcaster = $GLOBALS['SQ_SYSTEM']->am->getAsset($_SESSION['SQ_ECOM_PAYMENT_COMPLETED_BROADCASTER_ASSETID']);
				if (!empty($broadcaster)) {
					//put all returned variables in $_SESSION['SQ_ECOM_RESPONSE'] to $_POST array so that the listening trigger can use them
					$response = array_get_index($_SESSION, 'SQ_ECOM_RESPONSE', FALSE);
					if ($response) {
						$_POST['transaction_reference'] = $response['TRANSACTION'];
						$_POST['transaction_time'] = $response['TIME'];
						$_POST['transaction_status'] = $response['STATUS'];
						$_POST['transaction_card_number'] = $response['CARDNO'];
						$_POST['transaction_amount'] = $response['AMOUNT'];
						$_POST['transaction_currency'] = $response['CURRENCY'];
					}
					$GLOBALS['SQ_SYSTEM']->broadcastTriggerEvent('trigger_event_ecommerce_payment_completed', $broadcaster);
				}
			}

			$this->appendPendingOrderAuditLog(translate('Payment is successful'));


			$this->returnToCaller('SUCCESS', $is_in_frame);
		} else {
			// Notify that the payment was declined
			if ($card_info) {
				$this->appendPendingOrderAuditLog(translate('Payment Declined'));

			}
		}

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

		if ($link_value == 'display_format') {
			//the default body copy with card input
			if ($this->attr('test_mode')) {
				echo '<h2 class="gateway-test-mode" style="color: white; background-color: #C00; padding: 5px">TEST MODE</h2>';
			}

			// Load the JS 'Loader' object to print the overlay when payment form is submitted
			$this->includeLoaderJsObject($this->getPrefix().'_card');
			?>
			<form method="post" action="<?php echo $this->getURL(); ?>" onsubmit="Loader.show();">
			<?php
			$bodycopy->printBody();
			echo '</form>';
		} else {
			//the card holder verification bodycopy
			$bodycopy->printBody();
		}

	}//end printBodycopy()


	/**
	* Process credit card details to gateway
	*
	* @param array	$card_info	Submitted credit card information
	*
	* @return mixed	return FALSE if the card is not valid or transaction fails, return 1 if SUCCESS, return 2 if need to REDIRECT in 3-D Secure check
	* @access public
	*/
	function processPayment($card_info)
	{
		if (!$card_info) return FALSE;

		$amount = $_SESSION['SQ_ECOM_AMOUNT'];

		$processor = new Datacash_Processor($this->attr('client'), $this->attr('password'), SQ_TOOL_DATACASH_API_PATH, SQ_TOOL_DATACASH_CARDINFO_DIR_PATH, $this->attr('timeout'), $this->attr('test_mode'));

		//SET AMOUNT
		if (empty($amount)) {
			$this->_setError(translate('No amount is specified for the payment'));

		} else {
			$processor->setAmount($amount, $this->_getCurrency());
		}

		//SET MERCHANT REFERENCE
		//merchant reference number should be generated here instead of inside default_delivery_method.inc or form_ecommerce.inc
		//because the customer can try to pay many times until his card is accepted. So each time a payment transaction
		//is sent to Datacash, a new merchant reference number should be used because using the old reference number can
		//cause the "duplicate reference" error. We can see that Datacash reference number is different in every response
		$processor->setMerchantReference(md5(session_id().time()));
		//get the merchant reference number back, it was changed because Datacash requires that merchant reference number length must be between 6 and 30 alphanumeric characters
		$_SESSION['SQ_ECOM_REF_NO'] = $processor->getMerchantReference();

		//store the merchant reference number to the pending order asset before processing
		$this->setPendingOrderMerchantReference($_SESSION['SQ_ECOM_REF_NO']);

		//SET CARD NUMBER
		if (empty($card_info['number'])) {
			$this->_setError(translate('No card number is specified for the payment'));

		} else {
			$card_info['number'] = str_replace(Array(' ', "\t"), '', $card_info['number']);
			$processor->setCardNumber($card_info['number']);
		}

		//SET CARD EXPIRY DATE
		if ($this->_isValidDate(array_get_index($card_info, 'expiry_month', ''), array_get_index($card_info, 'expiry_year', ''))) {
			$processor->setCardExpiryDate($card_info['expiry_month'], $card_info['expiry_year']);
		} else {
			$this->_setError(translate('The expiry date (MM/YY) is not valid'));

		}

		//SET CARD START DATE IF SPECIFIED BECAUSE START DATE IS ONLY REQUIRED FOR SOME CARD TYPES, E.G. SOLO
		if (!empty($card_info['start_month']) || !empty($card_info['start_year'])) {
			if ($this->_isValidDate(array_get_index($card_info, 'start_month', ''), array_get_index($card_info, 'start_year', ''))) {
				$processor->setCardStartDate($card_info['start_month'], $card_info['start_year']);
			} else {
				$this->_setError(translate('The start date (MM/YY) is not valid'));

			}
		}

		//SET CARD ISSUE NUMBER, IF SPECIFIED
		if (!empty($card_info['issue_number'])) {
			if ($this->_isValidIssueNumber($card_info['issue_number'])) {
				$processor->setCardIssueNumber($card_info['issue_number']);
			} else {
				$this->_setError(translate('The issue number is invalid (must be one or two digits long)'));

			}
		}

		//SET CARD CV2, IF SPECIFIED
		if (!empty($card_info['cv2'])) {
			if ($this->_isValidCV2($card_info['cv2'])) {
				$processor->setCardCV2($card_info['cv2']);
			} else {
				$this->_setError(translate('The card CV2 is invalid (must be three or four (only American Express Card) digits long)'));

			}
		}

		$addr = '';
		//SET STREET ADDRESS (1 - 4)
		for ($i = 1; $i < 5; $i++) {
			if (!empty($card_info['street_address'.$i])) {
				$addr .= $card_info['street_address'.$i]."\n";
				$processor->setStreetAddress($i, $card_info['street_address'.$i]);
			}
		}

		//SET POSTCODE
		if (!empty($card_info['postcode'])) {
			$addr .= $card_info['postcode']."\n";
			$processor->setPostcode($card_info['postcode']);
		}

		//SET TRANSACTION TYPE AS "AUTH" (ONE STAGE PROCESSING)
		$processor->setTransactionTypeAuth();

		//SET 3-D SECURE PARAMETERS
		$nonthreed_card_types = $this->attr('nonthreed_card_types');
		$processor->setThreeDSecureParams($this->attr('merchant_url'), $this->attr('purchase_desc'), time(), empty($nonthreed_card_types)? Array() : explode('|', $nonthreed_card_types));

		//SET AVS/CV2
		$policy = $this->attr('avscv2_standard_policy');
		if ($policy != 0) {
			$processor->setStandardPolicy($policy);

			if (($policy == 2) || ($policy == 3) || ($policy == 6) || ($policy == 7)) {
				if (empty($card_info['cv2'])) {
					$this->_setError(translate('No CV2 is specified for the card'));

				}
			}

			if (($policy == 1) || ($policy == 3) || ($policy == 5) || ($policy == 7)) {
				if (empty($card_info['street_address1']) && empty($card_info['street_address2']) && empty($card_info['street_address3']) && empty($card_info['street_address4'])) {
					$this->_setError(translate('The street address is not specified'));

				}

				if (empty($card_info['postcode'])) {
					$this->_setError(translate('No postcode is specified'));

				}
			}

		}

		//CHECK IF THERE IS ERROR BEFORE SENDING DATA TO DATACASH SERVER, RETURN FALSE
		if ($this->isError()) {
			return FALSE;
		}

		$success = $processor->process();

		//GET RESPONSE
		$response = $processor->getResponse();

		//if success is 1 (payment is accepted) or 2 (redirect)
		if ($success) {
			//store the last 4 digits of card number to display it later in Form Ecommerce or Delivery Method through the Session variable SQ_ECOM_RESPONSE
			$_SESSION['SQ_ECOM_FORMATTED_CARD_NUMBER'] = $this->_getFormattedCardNumber($card_info['number']);
			//store the billing address to store in the Order asset of Form Ecommerce
			$_SESSION['SQ_ECOM_ORDER_BILLING_ADDR'] = $addr;
		}

		//if $success = 2, redirect user for card holder verification
		if ($success === 2) {
			$this->_redirect_info = $response;
		} else {
			//success = 1 or FALSE
			$this->_setTransactionResult($success, $response);
		}

		return $success;

	}//end processPayment()


	/**
	 * Authorize the payment when the user is redirected from the Access Control Server (ACS) after completing the Card Holder Verification process
	 *
	 * @param string $datacash_ref		the Datacash reference number
	 * @param string $pares				the Payment Authentication Response (PARes) returned by the Access Control Server (ACS)
	 *
	 * @return mixed		return 1 if SUCCESS; otherwise, return FALSE
	 * @access public
	 */
	function authorizePayment($datacash_ref, $pares)
	{
		$processor = new Datacash_Processor($this->attr('client'), $this->attr('password'), SQ_TOOL_DATACASH_API_PATH, SQ_TOOL_DATACASH_CARDINFO_DIR_PATH, $this->attr('timeout'), $this->attr('test_mode'));

		$success = $processor->authorize($datacash_ref, $pares);

		//get response
		$response = $processor->getResponse();

		$this->_setTransactionResult($success, $response);

		return $success;

	}//end authorizePayment()


	/**
	 * Set the result of the transaction. If transaction fails, set its error message. If transaction success, set the Session variable SQ_ECOM_RESPONSE
	 *
	 * @param boolean	$success		the reason of the transaction. It is either 1 or FALSE
	 * @param array		$response		the response array from the Datacash Processor
	 *
	 * @return void
	 * @access private
	 */
	private function _setTransactionResult($success, $response)
	{
		if ($success === FALSE) {
			$this->_setError($response['STATUS']);
		} else { //success = 1
			if ($this->attr('test_mode')) {
				$response['STATUS'] = '!!!ATTENTION!!! TEST MODE (transaction not performed) -- '.$response['STATUS'];
			}

			$card_no = '';
			if (isset($_SESSION['SQ_ECOM_FORMATTED_CARD_NUMBER'])) {
				$card_no = $_SESSION['SQ_ECOM_FORMATTED_CARD_NUMBER'];
				unset($_SESSION['SQ_ECOM_FORMATTED_CARD_NUMBER']);
			}

			$response['CARDNO'] = $card_no;
			$response['AMOUNT'] = $_SESSION['SQ_ECOM_AMOUNT'];
			$response['CURRENCY'] = $this->_getCurrency();

			$billing_addr = '';
			if (isset($_SESSION['SQ_ECOM_ORDER_BILLING_ADDR'])) {
				$billing_addr = $_SESSION['SQ_ECOM_ORDER_BILLING_ADDR'];
				unset($_SESSION['SQ_ECOM_ORDER_BILLING_ADDR']);
			}
			$response['BILLING_ADDR'] = $billing_addr;

			$_SESSION['SQ_ECOM_RESPONSE']   = $response;
		}

	}//end _setTransactionResult()


	/**
	 * Get the formatted last 4 digits of the card, e.g. ****-****-****-1234
	 *
	 * @param string $card_number		the card number to get last 4 digits
	 *
	 * @return string
	 * @access private
	 */
	private function _getFormattedCardNumber($card_number)
	{
		$card_no = $card_number;
		$card_len = strlen($card_no);
		if ($card_len == 16) {
			$card_no = '****-****-****-'.substr($card_no, -4);
		} else {
			$card_no = str_pad(substr($card_no, -4), $card_len, '*', STR_PAD_LEFT);
		}

		return $card_no;

	}//end _getFormattedCardNumber()


	/**
	* Returns back to e-commerce form, which called gateway
	*
	* @param string	$state	state to return
	*
	* @return void
	* @access public
	*/
	function returnToCaller($state='SUCCESS', $is_in_frame = FALSE)
	{
		$back_url = NULL;

		if ($state == 'SUCCESS') {
			$back_url = array_get_index($_SESSION, 'SQ_ECOM_SUCCESS_URL');
		} else if ($state == 'CANCEL') {
			$back_url = array_get_index($_SESSION, 'SQ_ECOM_CANCEL_URL');
		}

		unset($_SESSION['SQ_ECOM_SUCCESS_URL']);
		unset($_SESSION['SQ_ECOM_CANCEL_URL']);

		if (is_null($back_url)) {
			if ($is_in_frame) {
				$this->_redirectParent($this->getURL().'?unknown_referer=1');
			} else {
				trigger_error('Unknown caller reference');
				exit;
			}
		} else {
			if ($is_in_frame) {
				$this->_redirectParent($back_url);
			}else {
				header('Location: '.$back_url);
				exit;
			}
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


	/**
	* Card Number keyword replacement
	*
	* @return string
	* @access public
	*/
	function getCardNumberKeywordReplacement()
	{
		$prefix = $this->getPrefix();
		$card = $prefix.'_card';

		// set variables for test mode
		if ($this->attr('test_mode')) {
			$test_card_numbers = Datacash_Processor::getTestCardNumbers();
			$card_number = '<select name="'.$card.'[number]">';
			foreach ($test_card_numbers as $number => $label) {
				$card_number .= '<option value="'.$number.'" >'.$label.'</option>';
			}
			$card_number .= '</select>';
		} else {
			$card_number = '<input name="'.$card.'[number]" autocomplete="off" />';
		}

		return $card_number;

	}//end getCardNumberKeywordReplacement()


	/**
	* Card Start Month keyword replacement
	*
	* @return string
	* @access public
	*/
	function getCardStartMonthKeywordReplacement()
	{
		$card = $this->getPrefix().'_card';

		$string = '<input name="'.$card.'[start_month]" size="2" autocomplete="off" />';

		return $string;

	}//end getCardStartMonthKeywordReplacement()


	/**
	* Card Start Year keyword replacement
	*
	* @return string
	* @access public
	*/
	function getCardStartYearKeywordReplacement()
	{
		$card = $this->getPrefix().'_card';

		$string = '<input name="'.$card.'[start_year]" size="2" autocomplete="off" />';

		return $string;

	}//end getCardStartYearKeywordReplacement()


	/**
	* Card Expiry Month keyword replacement
	*
	* @return string
	* @access public
	*/
	function getCardExpiryMonthKeywordReplacement()
	{
		$card = $this->getPrefix().'_card';

		$string = '<input name="'.$card.'[expiry_month]" size="2" autocomplete="off" />';

		return $string;

	}//end getCardExpiryMonthKeywordReplacement()


	/**
	* Card Expiry Year keyword replacement
	*
	* @return string
	* @access public
	*/
	function getCardExpiryYearKeywordReplacement()
	{
		$card = $this->getPrefix().'_card';

		$string = '<input name="'.$card.'[expiry_year]" size="2" autocomplete="off" />';

		return $string;

	}//end getCardExpiryYearKeywordReplacement()


	/**
	* Card Issue Number keyword replacement
	*
	* @return string
	* @access public
	*/
	function getCardIssueNumberKeywordReplacement()
	{
		$card = $this->getPrefix().'_card';

		$string = '<input name="'.$card.'[issue_number]" size="4" autocomplete="off" />';

		return $string;

	}//end getCardIssueNumberKeywordReplacement()


	/**
	* Card CV2 keyword replacement
	*
	* @return string
	* @access public
	*/
	function getCardCv2KeywordReplacement()
	{
		$card = $this->getPrefix().'_card';

		$string = '<input name="'.$card.'[cv2]" size="4" autocomplete="off" />';

		return $string;

	}//end getCardCv2KeywordReplacement()


	/**
	* Street Address 1 keyword replacement
	*
	* @return string
	* @access private
	*/
	function getStreetAddress1KeywordReplacement()
	{
		ob_start();
		$card = $this->getPrefix().'_card';
		text_box($card.'[street_address1]', $this->_getDefaultInputFromPassThroughAttribute('addr1_var_name'));

		return ob_get_clean();

	}//end getStreetAddress1KeywordReplacement()


	/**
	* Street Address 2 keyword replacement
	*
	* @return string
	* @access private
	*/
	function getStreetAddress2KeywordReplacement()
	{
		ob_start();
		$card = $this->getPrefix().'_card';
		text_box($card.'[street_address2]', $this->_getDefaultInputFromPassThroughAttribute('addr2_var_name'));

		return ob_get_clean();

	}//end getStreetAddress2KeywordReplacement()


	/**
	* Street Address 3 keyword replacement
	*
	* @return string
	* @access private
	*/
	function getStreetAddress3KeywordReplacement()
	{
		ob_start();
		$card = $this->getPrefix().'_card';
		text_box($card.'[street_address3]', $this->_getDefaultInputFromPassThroughAttribute('addr3_var_name'));

		return ob_get_clean();

	}//end getStreetAddress3KeywordReplacement()


	/**
	* Street Address 4 keyword replacement
	*
	* @return string
	* @access private
	*/
	function getStreetAddress4KeywordReplacement()
	{
		ob_start();
		$card = $this->getPrefix().'_card';
		text_box($card.'[street_address4]', $this->_getDefaultInputFromPassThroughAttribute('addr4_var_name'));

		return ob_get_clean();

	}//end getStreetAddress4KeywordReplacement()


	/**
	* Postcode keyword replacement
	*
	* @return string
	* @access private
	*/
	function getPostcodeKeywordReplacement()
	{
		ob_start();
		$card = $this->getPrefix().'_card';
		text_box($card.'[postcode]', $this->_getDefaultInputFromPassThroughAttribute('postcode_var_name'));

		return ob_get_clean();

	}//end getPostcodeKeywordReplacement()


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
		return $this->_getCurrency();

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
		$button_text = $this->attr('cancel_text');
		if (empty($button_text)) $button_text = 'Cancel';

		return '<input type="submit" name="'.$this->getPrefix().'_action" value="'.$button_text.'" />';

	}//end getCancelButtonKeywordReplacement()


	/**
	* Card Holder Verification Form keyword replacement
	*
	* @return string
	* @access public
	*/
	function getCardHolderVerificationFormKeywordReplacement()
	{
		$md = md5($_SESSION['SQ_ECOM_REF_NO']);

		$string = <<<HEREDOC
<form name="PARequestForm" action="{$this->_redirect_info['ACS_URL']}" method="POST">
	<input type="hidden" name="PaReq" value="{$this->_redirect_info['PAREQ_MESSAGE']}">
	<input type="hidden" name="TermUrl" value="{$this->getURL()}">
	<input type="hidden" name="MD" value="{$md}">
</form>
<iframe name="ACSframe" width="390" height="400" frameborder="0">
	<b>Your browser does not support iframes</b>
</iframe>
<script type="text/javascript">
	window.onload = function()
	{
		document.PARequestForm.target = "ACSframe";
		document.PARequestForm.submit();
	}
</script>
HEREDOC;

		return $string;

	}//end getCardHolderVerificationFormKeywordReplacement()


	/**
	 * Check if the input date is valid
	 *
	 * @param string $month		the month of the input date
	 * @param string $year		the year of the input date
	 * @return boolean			return TRUE if the date is valid; otherwise, return FALSE
	 * @access private
	 */
	private function _isValidDate($month, $year)
	{
		$valid = FALSE;

		$two_digits_pattern = '/^\d{2}$/';
		//month and year must have 2 digit pattern mm/yy
		if (preg_match($two_digits_pattern, $month) && preg_match($two_digits_pattern, $year)) {
			//month must be in 1 and 12
			if ((0 < $month) && ($month < 13)) {
				$valid = TRUE;
			}
		}

		return $valid;

	}//end _isValidDate()


	/**
	 * Check if the issue number is valid
	 *
	 * @param string $issue_no		the input issue number to test
	 *
	 * @return boolean				return TRUE if the date is valid; otherwise, return FALSE
	 * @access private
	 */
	private function _isValidIssueNumber($issue_no)
	{
		$valid = FALSE;

		$pattern = '/^\d{1,2}$/';
		//issue number must be one or two digits long
		if (preg_match($pattern, $issue_no)) {
			$valid = TRUE;
		}

		return $valid;

	}//end _isValidIssueNumber()


	/**
	 * Check if the CV2 is valid
	 *
	 * @param string $cv2		the input CV2 to test
	 *
	 * @return boolean				return TRUE if the CV2 is valid; otherwise, return FALSE
	 * @access private
	 */
	private function _isValidCV2($cv2)
	{
		$valid = FALSE;

		$pattern = '/^\d{3,4}$/';
		//cv2 must be three or four (AMEX card only) digits long
		if (preg_match($pattern, $cv2)) {
			$valid = TRUE;
		}

		return $valid;

	}//end _isValidCV2()


	/**
	 * Get the currency of this transaction
	 *
	 * @return string
	 * @access private
	 */
	private function _getCurrency()
	{
		return isset($_SESSION['SQ_ECOM_CURRENCY'])? $_SESSION['SQ_ECOM_CURRENCY'] : $this->attr('currency');

	}//end _getCurrency()


	/**
	 * Unset the currency in the Session variable if it exists
	 *
	 * @return void
	 * @access private
	 */
	private function _unsetCurrency()
	{
		if (isset($_SESSION['SQ_ECOM_CURRENCY'])) {
			unset($_SESSION['SQ_ECOM_CURRENCY']);
		}

	}//end _unsetCurrency()


}//end class

?>
