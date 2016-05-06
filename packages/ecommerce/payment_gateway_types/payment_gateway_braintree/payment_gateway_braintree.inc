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
*/


require_once dirname(__FILE__).'/../../payment_gateway/payment_gateway.inc';
require_once dirname(__FILE__).'/braintree_processor.inc';


/**
* Payment_Gateway_Braintree
*
* @see Payment_Gateway
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Payment_Gateway_Braintree extends Payment_Gateway
{

	/**
	* Braintree process object
	* @object
	*/
	private $_processor;


	/**
	* Constructor
	*
	* @param int    $assetid    the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

		$this->_processor = new Braintree_Processor(
														$this->attr('test_mode'),
														$this->attr('merchant_id'),
														$this->attr('public_key'),
														$this->attr('private_key'),
														$this->attr('merchant_account_id')
													);

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

		return ($this->_createBodycopy('display_format', translate('%processing_error%
<div style="background-color: #CCC; padding: 5px"><strong>$%transaction_amount%</strong> dollars are about to be debited from your card</div>
<table cellpadding="5">
	<tr>
		<td align="right">Card Number</td>
		<td>%card_number%  %card_cvv%</td>
	</tr>
	<tr>
		<td align="right">Expiry Date (MM/YY)</td>
		<td>%card_expiry_month% / %card_expiry_year%</td>
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
		$keywords['braintree_form'] = 'Braintree Drop-in payment form';

		$keywords['card_number'] = 'Card number';
		$keywords['card_expiry_month'] = 'Card expiry month (text box)';
		$keywords['card_expiry_month_select'] = 'Card expiry month (dropdown box)';
		$keywords['card_expiry_year'] = 'Card expiry year (text box)';
		$keywords['card_expiry_year_select_X'] = 'Card expiry year (dropdown box)';
		$keywords['card_cvv'] = 'Card Verification Value';

		$keywords['billing_address_first_name'] = 'Card billing address first name';
		$keywords['billing_address_last_name'] = 'Card billing address last name';
		$keywords['billing_address_street'] = 'Card billing address street';
		$keywords['billing_address_postal_code'] = 'Card billing address postal code';

		$keywords['paypal_button'] = 'PayPal button';

		$keywords['submit_button'] = 'Submit button';
		$keywords['cancel_button'] = 'Cancel button';
		$keywords['reset_button'] = 'Reset button';

		// Transaction keywords
		$keywords['transaction_amount'] = 'Transaction amount';
		$keywords['processing_error'] = 'Processing error (empty if no error)';

		return $keywords;

	}//end getAvailableKeywords()


	/**
	* Prints the output of the payment gateway
	*
	* @return void
	* @access public
	*/
	function printBody()
	{
		// This page is redirected to by the child iframe with unknown reference error
		if (array_get_index($_GET, 'unknown_referer', FALSE)) {
			trigger_error('Unknown caller reference');
			exit;
		}
		// This page is redirected to by the child iframe with an error message
		if (array_get_index($_GET, 'error_message', FALSE)) {
			$this->_setError($_GET['error_message']);
		}

		// Payment transation "success" status
		$success = FALSE;

		$amount = array_get_index($_SESSION,'SQ_ECOM_AMOUNT', 0);
		$cancelled = array_get_index($_REQUEST, $this->getPrefix().'_cancel', '');
		$payment_nonce = Braintree_Processor::getPaymentNonce();
		$billing_address = array_get_index($_REQUEST, $this->getPrefix().'_billing_address', Array());

		if (!empty($cancelled)) {
			// Payment form was cancelled
			$this->returnToCaller('CANCEL');

		} else {
			// Payment form was submitted
			if (empty($amount)) {
				unset($_SESSION['SQ_ECOM_AMOUNT']);
				//clear pass through variables
				if (isset($_SESSION['SQ_ECOM_PASS_THROUGH_VARS'])) {
					unset($_SESSION['SQ_ECOM_PASS_THROUGH_VARS']);
				}

				$this->appendPendingOrderAuditLog(translate('Payment has no amount'));
				$this->returnToCaller('SUCCESS');
			} else if (!empty($payment_nonce)) {
				$this->appendPendingOrderAuditLog(translate('Payment Attempted (gateway contacted)'));
				$success = $this->processPayment($payment_nonce, $billing_address);
			}
		}

		//if transaction is accepted, redirect the user to the success page
		if ($success) {
			unset($_SESSION['SQ_ECOM_AMOUNT']);
			$this->_processor->resetPaymentParameters();

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
			if ($payment_nonce) {
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

		if ($this->attr('test_mode')) {
			echo '<h2 class="gateway-test-mode" style="color: white; background-color: #C00; padding: 5px">TEST MODE</h2>';
		}

		// Load the JS 'Loader' object to print the overlay when payment form is submitted
		// Not applicable when using Braintree Drop-in integration
		$loader_js = '';
		if (empty($this->_tmp['dropin_integration'])) {
			$this->includeLoaderJsObject($this->getPrefix().'_card');
			$loader_js = ' onsubmit="Loader.show();"';
		}
		?>
		<form id="<?php echo $this->_getPaymentFormId(); ?>" method="post" action="<?php echo $this->getURL(); ?>" <?php echo $loader_js; ?>>
			<?php
			$bodycopy->printBody();
			?>
		</form>
		<?php
		// Print Braintree payment form JS
		$this->_processor->printClinetSdkJs(
										$this->_getPaymentFormId(),
										$this->_getErrorMessageContainerId(),
										!empty($this->_tmp['dropin_integration']) ? $this->_getDropinFormContainerId() : '',
										!empty($this->_tmp['paypal_integration']) ? $this->_getPaypalButtonContainerId() : ''
									);

	}//end printBodycopy()


	/**
	* Returns the defined payment form element's ID
	*
	* @return string
	* @access private
	*/
	private function _getPaymentFormId()
	{
		return $this->getPrefix().'_payment_form';

	}//end _getPaymentFormId()


	/**
	* Returns the defined ID of the container for the Drop-in UI form
	*
	* @return string
	* @access private
	*/
	private function _getDropinFormContainerId()
	{
		return $this->getPrefix().'_payment_form_container';

	}//end _getPaypalButtonContainerId()


	/**
	* Returns the defined ID of the container for the PayPal button
	*
	* @return string
	* @access private
	*/
	private function _getPaypalButtonContainerId()
	{
		return $this->getPrefix().'_paypal_container';

	}//end _getPaypalButtonContainerId()


	/**
	* Returns the defined ID of the container for displaying the processing error message(s)
	*
	* @return string
	* @access private
	*/
	private function _getErrorMessageContainerId()
	{
		return $this->getPrefix().'_error_message';

	}//end _getErrorMessageContainerId()


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
	* Process the payment based on the nonce token returned by client
	*
	* @param array	$payment_nonce	  Payment method nonce token returned by Braintree client SDK
	* @param array	$billing_address  Payment card billing address
	*
	* @return boolean
	* @access public
	*/
	function processPayment($payment_nonce, $billing_address)
	{
		$amount = $_SESSION['SQ_ECOM_AMOUNT'];

		// Store the Payment Nonce token as the transaction reference
		$_SESSION['SQ_ECOM_REF_NO'] = $payment_nonce;

		// Set amount
		if (!$this->_processor->setTransactionAmount($amount)) {
			$this->_setError(translate('Transaction amount is invalid.'));
		}

		// Set AVS check option
		// AVS check is disabled with using 'Drop-in' integration
		$verify_card = $this->attr('verify_card') && !isset($_REQUEST[$this->getPrefix().'_dropin']);
		$this->_processor->setVerifyCard($verify_card);

		if ($verify_card) {
			$first_name = isset($billing_address['first_name']) ? trim($billing_address['first_name']) : '';
			if (!$this->_processor->setBillingAddressFirstName(array_get_index($billing_address, 'first_name', ''))) {
				$this->_setError(translate('Billing address first name needs to be specified.'));
			}
			if (!$this->_processor->setBillingAddressLastName(array_get_index($billing_address, 'last_name', ''))) {
				$this->_setError(translate('Billing address last name needs to be specified.'));
			}
			if (!$this->_processor->setBillingAddressPostalCode(array_get_index($billing_address, 'postal_code', ''))) {
				$this->_setError(translate('Billing address postal code needs to be specified.'));
			}
			if (!$this->_processor->setBillingAddressStreet(array_get_index($billing_address, 'street', ''))) {
				$this->_setError(translate('Billing address street needs to be specified.'));
			}
			if (!$this->_processor->setBillingAddressCountry(array_get_index($billing_address, 'country', ''))) {
				$this->_setError(translate('Valid billing address country needs to be specified.'));
			}
		}

		// Ensure that Braintree parametes are set
		if (!$this->_processor->parametersSet()) {
			$this->_setError(translate('Required Braintree parameters are not set.'));
		}

		// Set the payment nonce
		$this->_processor->setPaymentNonce($payment_nonce);

		// Check for any error before sending request to the Braintree
		if ($this->isError()) {
			return FALSE;
		}

		// Do the processing
		$success = $this->_processor->process($payment_nonce);

		// Get the response
		$response = $this->_processor->getResponse();

		// Set the results
		$this->_setTransactionResult($success, $response);

		return $success;

	}//end processPayment()


	/**
	* Set the result of the transaction. If transaction fails, set its error message.
	* If transaction is successful, set the Session variable SQ_ECOM_RESPONSE
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
			$this->_setError($response['message']);
		} else {
			if ($this->attr('test_mode')) {
				$response['status'] = sprintf(translate('!!!ATTENTION!!! TEST MODE (transaction not performed) -- %s'), $response['status']);
			}
			$response['BILLING_NAME'] = array_get_index($response, 'billing_name', '');
			$response['BILLING_NAME'] = array_get_index($response, 'billing_address', '');
			$response['TRANSACTION'] = array_get_index($response, 'braintree_ref', '');
			unset( $response['billing_name']);
			unset( $response['billing_address']);

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


	//--- Braintree form fields ---//
	// Values submitted to these form fields never makes it to the Matrix server

	/**
	* Braintree Drop-in payment form keyword replacement
	*
	* NOTE: This will just output the container for the Drop-in Payment UI
	* Braintree SDK will be replace it by the payment form
	*
	* @return string
	* @access public
	*/
	function getBraintreeFormKeywordReplacement()
	{
		$this->_tmp['dropin_integration'] = TRUE;

		$string = '<div id="'.$this->_getDropinFormContainerId().'" />';
		$string .= '<input name="'.$this->getPrefix().'_dropin" type="hidden" />';

		return $string;

	}//end getPaypalButtonKeywordReplacement()


	/**
	* Card Number keyword replacement
	*
	* @return string
	* @access public
	*/
	function getCardNumberKeywordReplacement()
	{
		$prefix = $this->getPrefix();
		// Set the test cards for test mode
		if ($this->attr('test_mode')) {
			$test_card_numbers = Braintree_Processor::getTestCardNumbers();
			$string = '<select onChange="this.nextSibling.value=this.value">';
			$string .= '<option value=""></option>';
			foreach ($test_card_numbers as $number => $label) {
				$string .= '<option value="'.$number.'" >'.$label.'</option>';
			}
			$string .= '</select>';
			$string .= '<input data-braintree-name="number" type="hidden" />';
		} else {
			$string = '<input data-braintree-name="number" autocomplete="off" />';
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
		return '<input data-braintree-name="expiration_year" size="4" autocomplete="off" />';

	}//end getCardExpiryYearKeywordReplacement()


	/**
	* Card Expiry Year Select keyword replacement
	*
	* @return string
	* @access public
	*/
	function getCardExpiryYearSelectKeywordReplacement($period=18)
	{
		$string = '<select onChange="this.nextSibling.value=this.value">';
		$string .= '<option value="" >----</option>';
		$year = date('Y');
		for ($i = 0; $i <= $period; $i++) {
			$string .= '<option value="'.$year.'" >'.$year.'</option>';
			$year++;
		}
		$string .= '</select>';
		$string .= '<input data-braintree-name="expiration_year" type="hidden" />';

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
		return '<input data-braintree-name="expiration_month" size="2" autocomplete="off" />';

	}//end getCardExpiryMonthKeywordReplacement()


	/**
	* Card Expiry Month Select keyword replacement
	*
	* @return string
	* @access public
	*/
	function getCardExpiryMonthSelectKeywordReplacement()
	{
		$string = '<select onChange="this.nextSibling.value=this.value">';
		$string .= '<option value="">--</option>';
		for($i = 1; $i <= 12; $i++) {
			$month = sprintf('%02s', $i);
			$string .= '<option value="'.$month.'" >'.$month.'</option>';
		}
		$string .= '</select>';
		$string .= '<input data-braintree-name="expiration_month" type="hidden" />';

		return $string;

	}//end getCardExpiryMonthSelectKeywordReplacement()


	/**
	* Card CCV keyword replacement
	*
	* @return string
	* @access public
	*/
	function getCardCvvKeywordReplacement()
	{
		return '<input data-braintree-name="cvv" size="4" autocomplete="off" />';

	}//end getCardCvvKeywordReplacement()


	//--- Matrix form fields ---//

	/**
	* Billing address first name keyword input replacement
	*
	* @return string
	* @access private
	*/
	function getBillingAddressFirstNameKeywordReplacement()
	{
		$value = trim($this->_getDefaultInputFromPassThroughAttribute('first_name_var_name'));

		return '<input name="'.$this->getPrefix().'_billing_address[first_name]" value="'.trim($value).' "autocomplete="off" />';

	}//end getBillingAddressFirstNameKeywordReplacement()


	/**
	* Billing address last name keyword input replacement
	*
	* @return string
	* @access private
	*/
	function getBillingAddressLastNameKeywordReplacement()
	{
		$value = trim($this->_getDefaultInputFromPassThroughAttribute('last_name_var_name'));

		return '<input name="'.$this->getPrefix().'_billing_address[last_name]" value="'.trim($value).' "autocomplete="off" />';

	}//end getBillingAddressLastNameKeywordReplacement()


	/**
	* Billing address street keyword input replacement
	*
	* @return string
	* @access private
	*/
	function getBillingAddressStreetKeywordReplacement()
	{
		$value = trim($this->_getDefaultInputFromPassThroughAttribute('street_var_name'));

		return '<input name="'.$this->getPrefix().'_billing_address[street]" value="'.trim($value).' "autocomplete="off" />';

	}//end getBillingAddressStreetKeywordReplacement()


	/**
	* Card billing address postal code input keyword replacement
	*
	* @return string
	* @access private
	*/
	function getBillingAddressPostalCodeKeywordReplacement()
	{
		$value =  $this->_getDefaultInputFromPassThroughAttribute('postcode_var_name');

		return '<input name="'.$this->getPrefix().'_billing_address[postal_code]" value ="'.trim($value).' "autocomplete="off" />';

	}//end getBillingAddressPostalCodeKeywordReplacement()


	/**
	* Card billing address country input keyword replacement
	*
	* @return string
	* @access private
	*/
	function getBillingAddressCountryKeywordReplacement()
	{
		$value =  $this->_getDefaultInputFromPassThroughAttribute('country_var_name');
		require SQ_FUDGE_PATH.'/standards_lists/countries.inc';
		array_unshift($standards_lists_countries, '');
		ob_start();
			combo_box($this->getPrefix().'_billing_address[country]', $standards_lists_countries, FALSE, $value);
			$output = ob_get_contents();
		ob_end_clean();

		return $output;

	}//end getBillingAddressCountryKeywordReplacement()


	/**
	* PayPal button keyword replacement
	*
	* NOTE: This will just output the container for the PayPal button
	* Braintree SDK will be replace it by PayPal button in the output
	*
	* @return string
	* @access public
	*/
	function getPaypalButtonKeywordReplacement()
	{
		$this->_tmp['paypal_integration'] = TRUE;

		return '<div id="'.$this->_getPaypalButtonContainerId().'" />';

	}//end getPaypalButtonKeywordReplacement()


	/**
	* Transaction Amount keyword replacement
	*
	* @return string
	* @access public
	*/
	function getTransactionAmountKeywordReplacement()
	{
		return array_get_index($_SESSION,'SQ_ECOM_AMOUNT', 0);

	}//end getTransactionAmountKeywordReplacement()


	/**
	* Processing Error keyword replacement
	*
	* @return string
	* @access public
	*/
	function getProcessingErrorKeywordReplacement()
	{
		$replacement = '<div id="'.$this->_getErrorMessageContainerId().'" class="payment-gateway-transaction-error">';
		if ($this->isError()) {
			$replacement .= '<p>'.$this->getErrorMessage().'</p>';
		}

		$replacement .= '</div>';

		return $replacement;

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

		return '<input type="submit" name="'.$this->getPrefix().'_submit"  value="'.$button_text.'" />';

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

		$string = '';
		if (empty($this->_tmp['dropin_integration'])) {
			$string =  '<input type="reset" value="'.$button_text.'" />';
		}

		return $string;

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

		// Can't use submit button to 'reset' as it will trigger payment request to Braintree JS API
		$replacement = 	'<input type="hidden" name="'.$this->getPrefix().'_cancel" value="">'.
						'<input type="button" value="'.$button_text.'" onclick="this.previousSibling.value=\''.$button_text.'\'; document.getElementById(\''.$this->_getPaymentFormId().'\').submit();" />';

		return $replacement;

	}//end getCancelButtonKeywordReplacement()


}//end class

?>
