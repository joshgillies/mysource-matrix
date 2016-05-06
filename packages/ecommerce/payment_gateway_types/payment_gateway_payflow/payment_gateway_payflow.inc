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
* $Id: payment_gateway_payflow.inc,v 1.1 2013/03/18 04:13:10 cupreti Exp $
*
*/


require_once dirname(__FILE__).'/../../payment_gateway/payment_gateway.inc';
require_once dirname(__FILE__).'/payflow_processor.inc';


/**
* Payment_Gateway_Payflow
*
* @see Payment_Gateway
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Payment_Gateway_Payflow extends Payment_Gateway
{

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
<div style="background-color: #CCC; padding: 5px"><strong>$%transaction_amount%</strong> dollars are about to be debited from your card</div>
<table cellpadding="5">
	<tr>
		<td align="right">Card Number</td>
		<td>%card_number%  %card_cv2%</td>
	</tr>
	<tr>
		<td align="right">Expiry Date (MM/YY)</td>
		<td>%card_expiry_month% / %card_expiry_year%</td>
	</tr>
	<tr>
		<td align="right">Card Type</td>
		<td>%card_type%</td>
	</tr>
	<tr>
		<td>&nbsp;</td>
		<td>%submit_button% %reset_button% %cancel_button%</td>
	</tr>
</table>')));


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

		// Form fields keywords
		$keywords['card_number'] = 'Card Number';
		$keywords['card_expiry_month'] = 'Card Expiry Month (text box)';
		$keywords['card_expiry_month_select'] = 'Card Expiry Month (dropdown box)';
		$keywords['card_expiry_year'] = 'Card Expiry Year (text box)';
		$keywords['card_expiry_year_select_X'] = 'Card Expiry Year (dropdown box)';
		$keywords['card_cv2'] = 'Card CV2';

		$keywords['billing_address_first_name'] = 'Card holder\'s first name';
		$keywords['billing_address_last_name'] = 'Card holder\'s last name';
		$keywords['billing_address_street'] = 'Billing address street (street number, name and type)';
		$keywords['billing_address_city'] = 'Billing address city';
		$keywords['billing_address_state'] = 'Billing address state';
		$keywords['billing_address_postcode'] = 'Billing address postcode';
		$keywords['billing_address_country'] = 'Billing address country';

		$keywords['submit_button'] = 'Submit Button';
		$keywords['cancel_button'] = 'Cancel Button';
		$keywords['reset_button'] = 'Reset Button';

		// Transaction keywords
		$keywords['transaction_amount'] = 'Transaction Amount';
		$keywords['transaction_currency'] = 'Transaction Currency';
		$keywords['processing_error'] = 'Processing Error (empty if no error)';

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

		$amount = array_get_index($_SESSION,'SQ_ECOM_AMOUNT', 0);
		$card_info = array_get_index($_REQUEST, $this->getPrefix().'_card', FALSE);
		$action = array_get_index($_REQUEST, $this->getPrefix().'_action', FALSE);


		if ($action == 'Cancel') {
			$this->returnToCaller('CANCEL');

		} else if (empty($amount)) {
			unset($_SESSION['SQ_ECOM_AMOUNT']);
			//clear pass through variables
			if (isset($_SESSION['SQ_ECOM_PASS_THROUGH_VARS'])) {
				unset($_SESSION['SQ_ECOM_PASS_THROUGH_VARS']);
			}

			$this->appendPendingOrderAuditLog(translate('Payment has no amount'));


			$this->returnToCaller('SUCCESS');

		} else if ($card_info) {
			$this->appendPendingOrderAuditLog(translate('Payment Attempted (gateway contacted)'));

			$success = $this->processPayment($card_info);
		}

		//if transaction is accepted, redirect the user to the success page
		if ($success) {
			unset($_SESSION['SQ_ECOM_AMOUNT']);
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
						$_POST['transaction_reference'] = $response['MERCHANT_REFERENCE'];
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


			$this->returnToCaller('SUCCESS');
		} else {
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

		//clean input
		foreach($card_info as $key => $value){
			$card_info[$key] = htmlentities($value);
		}

		$amount = $_SESSION['SQ_ECOM_AMOUNT'];

		$processor = new Payflow_Processor($this->attr('test_mode'));

		// Merchand reference should be generated here instead of default_delivery_method.inc or elsewhere because
		// Payflow requires a unqiue transaction reference id for each transaction attempt
		// For example, user can make multiple submission attempt when submitting the card details,
		// and not sending unique reference for each attempt will casue "Duplicate transaction" error
		$ref_no = md5(session_id().time());
		$processor->setMerchantRef($ref_no);
		$_SESSION['SQ_ECOM_REF_NO'] = $ref_no;

		// Set the Payflow account paramaters
		$acc_user = $this->attr('user');
		if (!empty($acc_user)) {
			$processor->setAccountUser($acc_user);
		} else {
			$this->_setError(translate('Payflow User ID not specified'));

		}

		$acc_vendor = $this->attr('vendor');
		if (!empty($acc_vendor)) {
			$processor->setAccountVendor($acc_vendor);
		} else {
			$this->_setError(translate('Payflow Vendor ID not specified'));

		}

		$acc_partner = $this->attr('partner');
		if (!empty($acc_partner)) {
			$processor->setAccountPartner($acc_partner);
		} else {
			$this->_setError(translate('Payflow Partner ID not specified'));

		}

		$acc_password = $this->attr('password');
		if (!empty($acc_password)) {
			$processor->setAccountPassword($acc_password);
		} else {
			$this->_setError(translate('Payflow Password not specified'));

		}
		$currency = $this->attr('currency');
		if (!empty($currency)) {
			$processor->setTransactionCurrency($currency);
		}
		$timeout = $this->attr('timeout');
		if (!empty($timeout)) {
			$processor->setRequestTimeout($timeout);
		}

		// Set amount
		if (!empty($amount) && is_numeric($amount) && $amount > 0) {
			$processor->setTransactionAmount($amount);
		} else {
			$this->_setError(translate('Transaction amount is invalid'));

		}

		// Set card number
		if (!empty($card_info['number'])) {
			$card_info['number'] = preg_replace('|[^0-9]|', '', $card_info['number']);
			$processor->setCardNumber($card_info['number']);
		} else {
			$this->_setError(translate('Card number is not specified'));

		}

		// Set card holder's first name
		if (!empty($card_info['billing_address_first_name'])) {
			$processor->setCardFirstName($card_info['billing_address_first_name']);
		}

		// Set card holder's last name
		if (!empty($card_info['billing_address_last_name'])) {
			$processor->setCardLastName($card_info['billing_address_last_name']);
		}

		// Set card's expiry date
		if ($this->_isValidDate(array_get_index($card_info, 'expiry_month', ''), array_get_index($card_info, 'expiry_year', ''))) {
			$processor->setCardExpiryDate($card_info['expiry_month'], $card_info['expiry_year']);
		} else {
			$this->_setError(translate('Card expiry date is invalid'));

		}

		// Set card CV2 if required
		if($this->attr('cv2_enabled')){
			if (!empty($card_info['cv2']) && $this->_isValidCV2($card_info['cv2'])) {
				$processor->setCardCV2($card_info['cv2']);
			} else {
				$this->_setError(translate('Card CVV2 value is invalid'));

			}
		}

		// Set billing street address if required
		if ($this->attr('avs_enabled')) {
			// Set street (required for AVS)
			if (!empty($card_info['billing_address_street'])) {
				$processor->setStreet($card_info['billing_address_street']);
			} else {
				$this->_setError(translate('Card billing address\'s street value is required'));

			}

			// Set city
			if (!empty($card_info['billing_address_city'])) {
				$processor->setCity($card_info['billing_address_city']);
			}

			// Set postcode (required for AVS)
			if (!empty($card_info['billing_address_postcode'])) {
				$processor->setPostcode($card_info['billing_address_postcode']);
				$billing_addr = $card_info['billing_address_postcode']." ";
			} else {
				$this->_setError(translate('Card billing address\'s postcode value is required'));

			}

			// Set state
			if (!empty($card_info['billing_address_state'])) {
				$processor->setState($card_info['billing_address_state']);
				$billing_addr = $card_info['billing_address_state']."\n";
			}

			// Set country
			if (!empty($card_info['billing_address_country'])) {
				$processor->setCountry($card_info['billing_address_country']);
				$billing_addr = $card_info['billing_address_country'];
			}
		}

		// Check for any error before sending request to the Payflow
		if ($this->isError()) {
			return FALSE;
		}

		// Do the processing
		$success = $processor->process();

		// Get the response
		$response = $processor->getResponse();

		// Set the results
		$this->_setTransactionResult($success, $response, $card_info);

		return $success;

	}//end processPayment()


	/**
	 * Set the result of the transaction. If transaction fails, set its error message.
	 * If transaction success, set the Session variable SQ_ECOM_RESPONSE
	 *
	 * @param boolean	$success		the reason of the transaction. It is either 1 or FALSE
	 * @param array		$response		the response array from the Datacash Processor
	 * @param array     $card_info      submitted credit card information
	 *
	 * @return void
	 * @access private
	 */
	private function _setTransactionResult($success, $response, $card_info)
	{
		if (!$success) {
			$this->_setError($response['MESSAGE']);
		} else {
			if ($this->attr('test_mode')) {
				$response['STATUS'] = '!!!ATTENTION!!! TEST MODE (transaction not performed) -- '.$response['STATUS'];
			}

			// Card number keep only last four digits
			$cardno = str_repeat('*', strlen($card_info['number'])-4).substr($card_info['number'], -4)."\n";

			// Billing address
			$addr = '';
			$addr .= !empty($card_info['billing_address_street']) ? ($card_info['billing_address_street'].'\n') : '';
			$addr .= !empty($card_info['billing_address_city']) ? ($card_info['billing_address_city'].'\n') : '';
			$addr .= !empty($card_info['billing_address_postcode']) ? ($card_info['billing_address_postcode'].'') : '';
			$addr .= !empty($card_info['billing_address_state']) ? ($card_info['billing_address_state'].'') : '';

			$response['CARDNO'] = $cardno;
			$response['AMOUNT'] = $_SESSION['SQ_ECOM_AMOUNT'];
			$response['CURRENCY'] = $this->attr('currency');
			$response['BILLING_ADDR'] = $addr;

			$_SESSION['SQ_ECOM_RESPONSE']   = $response;
		}

	}//end _setTransactionResult()


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
		$prefix = $this->getPrefix();
		$card = $prefix.'_card';

		// Set the test cards for test mode
		if ($this->attr('test_mode')) {
			$test_card_numbers = Payflow_Processor::getTestCardNumbers();
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
	* Card Expiry Year Select keyword replacement
	*
	* @return string
	* @access public
	*/
	function getCardExpiryYearSelectKeywordReplacement($period=18)
	{
		$card = $this->getPrefix().'_card';

		$string = '<select name="'.$card.'[expiry_year]">';
		$string .= '<option value="" >----</option>';
		$year = date('Y');
		for ($i = 0; $i <= $period; $i++) {
			$string .= '<option value="'.substr($year, -2).'" >'.$year.'</option>';
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
		$card = $this->getPrefix().'_card';

		$string = '<input name="'.$card.'[expiry_month]" size="2" autocomplete="off" />';

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
		$card = $this->getPrefix().'_card';

		$string = '<select name="'.$card.'[expiry_month]">';
		$string .= '<option value="" >--</option>';
		for ($i = 1; $i <= 12; $i++) {
			$month = sprintf('%02s', $i);
			$string .= '<option value="'.$month.'" >'.$month.'</option>';
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
		$card = $this->getPrefix().'_card';

		$string = '<input name="'.$card.'[cv2]" size="4" autocomplete="off" />';

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
		$card = $this->getPrefix().'_card';

		$available_card_types = Payflow_Processor::getCardTypes();
		$selected_card_types = explode('|', $this->attr('supported_card_types'));

		ob_start();
		echo '<select name="'.$card.'[type]">';
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
		ob_start();
		$card = $this->getPrefix().'_card';
		text_box($card.'[billing_address_first_name]', $this->_getDefaultInputFromPassThroughAttribute('first_name_var_name'));

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
		$card = $this->getPrefix().'_card';
		text_box($card.'[billing_address_last_name]', $this->_getDefaultInputFromPassThroughAttribute('last_name_var_name'));

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
		ob_start();
		$card = $this->getPrefix().'_card';
		text_box($card."[billing_address_street]", $this->_getDefaultInputFromPassThroughAttribute('addr_street_var_name'));

		return ob_get_clean();

	}//end getBillingAddressAddressLine1KeywordReplacement()


	/**
	* Billing City keyword replacement
	*
	* @return string
	* @access private
	*/
	function getBillingAddressCityKeywordReplacement()
	{
		ob_start();
		$card = $this->getPrefix().'_card';
		text_box($card.'[billing_address_city]', $this->_getDefaultInputFromPassThroughAttribute('city_var_name'));

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
		ob_start();
		$card = $this->getPrefix().'_card';
		text_box($card.'[billing_address_postcode]', $this->_getDefaultInputFromPassThroughAttribute('postcode_var_name'));

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

		ob_start();
		$card = $this->getPrefix().'_card';
		combo_box($card.'[billing_address_country]', $standards_lists_countries, FALSE, $this->_getDefaultInputFromPassThroughAttribute('country_var_name'));

		return ob_get_clean();

	}//end getBillingAddressCountryKeywordReplacement()


	/**
	* Billing State keyword replacement
	*
	* @return string
	* @access private
	*/
	function getBillingAddressStateKeywordReplacement()
	{
		ob_start();
		$card = $this->getPrefix().'_card';
		text_box($card.'[billing_address_state]', $this->_getDefaultInputFromPassThroughAttribute('state_var_name'));

		return ob_get_clean();

	}//end getBillingAddressStateKeywordReplacement()


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
		$button_text = $this->attr('cancel_text');
		if (empty($button_text)) $button_text = 'Cancel';

		return '<input type="submit" name="'.$this->getPrefix().'_action" value="'.$button_text.'" />';

	}//end getCancelButtonKeywordReplacement()



	/**
	 * Check if the input date is valid
	 *
	 * @param string $month		the month of the input date
	 * @param string $year		the year of the input date
	 *
	 * @return boolean			return TRUE if the date is valid; otherwise, return FALSE
	 * @access private
	 */
	private function _isValidDate($month, $year)
	{
		$valid = FALSE;

		// Check month
		if (is_numeric($month) && $month >= 1 && $month <= 12) {
			// Check year
			if (is_numeric($year) && strlen($year) == 2) {
				$valid = TRUE;
			}
		}

		return $valid;

	}//end _isValidDate()


	/**
	 * Check if the cv2 is valid
	 *
	 * @param string $cv2		the input CV2 to test
	 *
	 * @return boolean				return TRUE if the CV2 is valid; otherwise, return FALSE
	 * @access private
	 */
	private function _isValidCv2($cv2)
	{
		$valid = FALSE;

		$pattern = '/^\d{3,4}$/';
		//cv2 must be three or four (AMEX card only) digits long
		if (preg_match($pattern, $cv2)) {
			$valid = TRUE;
		}

		return $valid;

	}//end _isValidCv2()


}//end class

?>
