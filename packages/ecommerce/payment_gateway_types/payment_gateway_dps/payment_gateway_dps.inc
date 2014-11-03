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
* $Id: payment_gateway_dps.inc,v 1.5 2012/11/16 05:01:17 cupreti Exp $
*
*/


require_once dirname(__FILE__).'/../../payment_gateway/payment_gateway.inc';
require_once dirname(__FILE__).'/dps_processor.inc';


/**
* Payment_Gateway_Dps
*
* @see Asset_Management
*
* @author  Edison Wang <ewang@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Payment_Gateway_Dps extends Payment_Gateway
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
</table>')));


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

		$keywords['card_number'] = 'Card Number';
		$keywords['card_name'] = 'Name on Card';
		$keywords['card_start_month'] = 'Card Start Month';
		$keywords['card_start_year'] = 'Card Start Year';
		$keywords['card_expiry_month'] = 'Card Expiry Month';
		$keywords['card_expiry_year'] = 'Card Expiry Year';
		$keywords['card_issue_number'] = 'Card Issue Number';
		$keywords['card_cv2'] = 'Card CV2';

		$keywords['street_address1'] = 'Address Line 1';
		$keywords['street_address2'] = 'Address Line 2';
		$keywords['street_address3'] = 'Address Line 3';
		$keywords['street_address4'] = 'Address Line 4';
		$keywords['postcode'] = 'Postcode';

		$keywords['transaction_amount'] = 'Transaction Amount';
		$keywords['transaction_currency'] = 'Transaction Currency';
		$keywords['processing_error'] = 'Processing Error (empty if no error)';
		$keywords['submit_button'] = 'Submit Button';
		$keywords['cancel_button'] = 'Cancel Button';
		$keywords['reset_button'] = 'Reset Button';


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

		//OUTPUT HERE
		if ($link_value == 'display_format') {
			//the default body copy with card input
			$datapath = sq_web_path('data').'/asset_types/payment_gateway_datacash/files';
			$card = $this->getPrefix().'_card';

			if ($this->attr('test_mode')) {
				echo '<h2 class="gateway-test-mode" style="color: white; background-color: #C00; padding: 5px">TEST MODE</h2>';
			}

			?>
			<script src="<?php echo $datapath; ?>/loader.js"></script>
			<script>
				var Loader = new Loader('<?php echo $card; ?>','#FFFFFF','Processing Transaction...','<?php echo $datapath; ?>/loader.gif');
				Loader.print();
			</script>
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

		$processor = new Dps_Processor($this->attr('client'), $this->attr('password'), $this->attr('test_mode'));

		//SET AMOUNT
		if (empty($amount)) {
			$this->_setError($this->attr('amount_not_specified_error'));
		} else {
			$processor->setAmount($amount);
			$processor->setCurrency($this->_getCurrency());
		}


		//SET CARD NUMBER
		if (empty($card_info['number'])) {
			$this->_setError($this->attr('card_number_not_specified_error'));
		} else {
			$card_info['number'] = str_replace(Array(' ', "\t"), '', $card_info['number']);
			$processor->setCardNumber($card_info['number']);
		}

		//SET CARD NAME
		if (!empty($card_info['name'])) {
			$processor->setCardName($card_info['name']);
		}

		//SET CARD EXPIRY DATE
		if ($this->_isValidDate(array_get_index($card_info, 'expiry_month', ''), array_get_index($card_info, 'expiry_year', ''))) {
			$processor->setCardExpiryDate($card_info['expiry_month'], $card_info['expiry_year']);
		} else {
			$this->_setError($this->attr('expiry_date_invalid_error'));
		}

		//SET CARD START DATE IF SPECIFIED BECAUSE START DATE IS ONLY REQUIRED FOR SOME CARD TYPES, E.G. SOLO
		if (!empty($card_info['start_month']) || !empty($card_info['start_year'])) {
			if ($this->_isValidDate(array_get_index($card_info, 'start_month', ''), array_get_index($card_info, 'start_year', ''))) {
				$processor->setCardStartDate($card_info['start_month'], $card_info['start_year']);
			} else {
				$this->_setError($this->attr('start_date_invalid_error'));
			}
		}

		//SET CARD ISSUE NUMBER, IF SPECIFIED
		if (!empty($card_info['issue_number'])) {
			if ($this->_isValidIssueNumber($card_info['issue_number'])) {
				$processor->setCardIssueNumber($card_info['issue_number']);
			} else {
				$this->_setError($this->attr('issue_number_invalid_error'));
			}
		}

		//SET CARD CV2, IF SPECIFIED
		if($this->attr('cv2_standard_policy') != 0){
			if (!empty($card_info['cv2']) && $this->_isValidCV2($card_info['cv2'])) {
				$processor->setCardCV2($card_info['cv2']);
			} else {
				$this->_setError($this->attr('card_cv2_invalid_error'));
			}
		}

		$addr = '';
		//SET STREET ADDRESS (1 - 4)
		for ($i = 1; $i < 5; $i++) {
			if (!empty($card_info['street_address'.$i])) {
				$addr .= $card_info['street_address'.$i]."\n";
			}
		}
		if(!empty($addr))
			$processor->setStreetAddress($addr);


		//SET POSTCODE
		if (!empty($card_info['postcode'])) {
			$addr .= $card_info['postcode']." ";
			$processor->setPostcode($card_info['postcode']);
		}


		//SET AVS
		$policy = $this->attr('avs_standard_policy');
		$processor->setStandardPolicy($policy);
		if ($policy != 0) {
			if (empty($card_info['street_address1']) && empty($card_info['street_address2']) && empty($card_info['street_address3']) && empty($card_info['street_address4'])) {
				$this->_setError($this->attr('street_address_not_specified_error'));
			}

			if (empty($card_info['postcode'])) {
				$this->_setError($this->attr('postcode_not_specified_error'));
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

		$this->_setTransactionResult($success, $response);


		return $success;

	}//end processPayment()


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
			$test_card_numbers = Dps_Processor::getTestCardNumbers();
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
	* Card Name keyword replacement
	*
	* @return string
	* @access private
	*/
	function getCardNameKeywordReplacement()
	{

		$card = $this->getPrefix().'_card';
		$string = '<input name="'.$card.'[name]" autocomplete="off" />';
		return $string;

	}//end getCardNameKeywordReplacement()


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

		$pattern = '/^\d{1,4}$/';
		//issue number must be one to four digits long
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


	private function _getPassThroughVariable($var_name, $default)
	{
		if (is_null($this->_pass_through_vars)) {
			$this->_pass_through_vars = array_get_index($_SESSION, 'SQ_ECOM_PASS_THROUGH_VARS', Array());
		}

		return array_get_index($this->_pass_through_vars, $var_name, $default);

	}//end _getPassThroughVariable()


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


}//end class

?>
