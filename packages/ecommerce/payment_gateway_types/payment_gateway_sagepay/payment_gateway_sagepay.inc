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
* $Id: payment_gateway_sagepay.inc,v 1.12 2012/11/16 05:02:30 cupreti Exp $
*
*/


require_once dirname(__FILE__).'/../../payment_gateway/payment_gateway.inc';
require_once dirname(__FILE__).'/sagepay_processor.inc';

define('MIN_LINE_IN_ADDRESS', 1);
define('MAX_LINE_IN_ADDRESS', 3);

/**
* Payment_Gateway_SagePay
*
* Interface for online credit card processing using SagePay payment gateway
*
*
* @author  Anh Ta <ata@squiz.co.uk>
* @version $Revision: 1.12 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Payment_Gateway_SagePay extends Payment_Gateway
{


	/**
	 * The response from SagePay with Card Holder Verification redirect information such as Access Control Server (ACS) URL, Payment Authentication Request (PAReq) and MD
	 * @var array
	 */
	private $_redirect_info = NULL;

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
	* SagePay Payment Gateway creates a bodycopy asset when they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	function _createAdditional(&$link)
	{
		if (!parent::_createAdditional($link)) return FALSE;

		return ($this->_createBodycopy('display_format', translate('%%processing_error%%
<div style="background-color: #CCC; padding: 5px"><strong>&pound;%%transaction_amount%%</strong> Pounds Sterling (GBP) are about to be debited from your card</div>
<table cellpadding="5">
	<tr>
		<td align="right">Name on Card</td>
		<td>%%card_name%%</td>
	</tr>
	<tr>
		<td align="right">Card Number</td>
		<td>%%card_number%%</td>
	</tr>
	<tr>
		<td align="right">Expiry Date (MM/YY)</td>
		<td>%%card_expiry_month%% / %%card_expiry_year%%</td>
	</tr>
	<tr>
		<td align="right">Card Type</td>
		<td>%%card_type%%</td>
	</tr>
	<tr>
		<td>&nbsp;</td>
		<td>%%submit_button%% %%reset_button%% %%cancel_button%%</td>
	</tr>
</table>')) && $this->_createBodycopy('card_holder_verification', translate('%%card_holder_verification_form%%')));



	}//end _createAdditional()


	/**
	* Create Bodycopy
	*
	* @param string	$link_value		the link value of the link between this SagePay Payment Gateway asset and the new bodycopy
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

		$keywords['card_name'] = 'Name on Card';
		$keywords['card_number'] = 'Card Number';
		$keywords['card_start_month'] = 'Card Start Month';
		$keywords['card_start_month_select'] = 'Card Start Month Select';
		$keywords['card_start_year'] = 'Card Start Year';
		$keywords['card_start_year_select_XX'] = 'Card Start Year Select';
		$keywords['card_expiry_month'] = 'Card Expiry Month';
		$keywords['card_expiry_month_select'] = 'Card Expiry Month Select';
		$keywords['card_expiry_year'] = 'Card Expiry Year';
		$keywords['card_expiry_year_select_XX'] = 'Card Expiry Year Select';
		$keywords['card_issue_number'] = 'Card Issue Number';
		$keywords['card_cv2'] = 'Card CV2';
		$keywords['card_type'] = 'Card Type';

		$keywords['billing_first_names'] = 'Billing First Names';
		$keywords['billing_surname'] = 'Billing Surname';
		$keywords['billing_address1_X'] = 'Billing Address1 (X: 1 - 3)';
		$keywords['billing_address2_X'] = 'Billing Address2 (X: 1 - 3)';
		$keywords['billing_city'] = 'Billing City';
		$keywords['billing_postcode'] = 'Billing Postcode';
		$keywords['billing_country'] = 'Billing Country';
		$keywords['billing_state'] = 'Billing State';
		$keywords['billing_phone'] = 'Billing Phone';

		$keywords['delivery_first_names'] = 'Delivery First Names';
		$keywords['delivery_surname'] = 'Delivery Surname';
		$keywords['delivery_address1_X'] = 'Delivery Address1 (X: 1 - 3)';
		$keywords['delivery_address2_X'] = 'Delivery Address2 (X: 1 - 3)';
		$keywords['delivery_city'] = 'Delivery City';
		$keywords['delivery_postcode'] = 'Delivery Postcode';
		$keywords['delivery_country'] = 'Delivery Country';
		$keywords['delivery_state'] = 'Delivery State';
		$keywords['delivery_phone'] = 'Delivery Phone';

		$keywords['transaction_amount'] = 'Transaction Amount';
		$keywords['processing_error'] = 'Processing Error (empty if no error)';
		$keywords['submit_button'] = 'Submit Button';
		$keywords['cancel_button'] = 'Cancel Button';
		$keywords['reset_button'] = 'Reset Button';
		$keywords['card_holder_verification_form'] = 'Card Holder Verification Form';

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
	* Get bodycopy keyword replacement
	*
	* If keyword is %card_start_year_select_XX% or %card_expiry_year_select_XX%, call the correct method with XX is
	* $period parameter. Otherwise, call the parent method.
	*
	* @param string	$keyword	Keyword to find a replacement for
	*
	* @return string
	* @access public
	* @see _getKeywordReplacement
	*/
	public function getKeywordReplacement($keyword)
	{
		$replacement = NULL;

		if (0 === strpos($keyword, 'card_start_year_select_')) {
			$period = substr($keyword, 23);
			if (is_numeric($period)) {
				$replacement = $this->getCardStartYearSelectKeywordReplacement($period);
			}

		} else if (0 === strpos($keyword, 'card_expiry_year_select_')) {
			$period = substr($keyword, 24);
			if (is_numeric($period)) {
				$replacement = $this->getCardExpiryYearSelectKeywordReplacement($period);
			}

		} else if (0 === strpos($keyword, 'billing_address1_')) {
			$line = (int)substr($keyword, 17);
			if (($line >= MIN_LINE_IN_ADDRESS) && ($line <= MAX_LINE_IN_ADDRESS)) {
				$replacement = $this->getBillingAddress1KeywordReplacement($line);
			}

		} else if (0 === strpos($keyword, 'billing_address2_')) {
			$line = (int)substr($keyword, 17);
			if (($line >= MIN_LINE_IN_ADDRESS) && ($line <= MAX_LINE_IN_ADDRESS)) {
				$replacement = $this->getBillingAddress2KeywordReplacement($line);
			}

		} else if (0 === strpos($keyword, 'delivery_address1_')) {
			$line = (int)substr($keyword, 18);
			if (($line >= MIN_LINE_IN_ADDRESS) && ($line <= MAX_LINE_IN_ADDRESS)) {
				$replacement = $this->getDeliveryAddress1KeywordReplacement($line);
			}

		} else if (0 === strpos($keyword, 'delivery_address2_')) {
			$line = (int)substr($keyword, 18);
			if (($line >= MIN_LINE_IN_ADDRESS) && ($line <= MAX_LINE_IN_ADDRESS)) {
				$replacement = $this->getDeliveryAddress2KeywordReplacement($line);
			}

		} else {
			$replacement = parent::getKeywordReplacement($keyword);

		}

		return $replacement;

	}//end getKeywordReplacement()


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
					//store the md5 of MD to authenticate the payment after the card holder verify himself
					$_SESSION['SQ_ECOM_MD'] = md5($this->_redirect_info['MD']);
					//print the Card Holder Verification bodycopy
					$this->printBodycopy('card_holder_verification');
					return;
				}
			}

		} else if (!empty($_SESSION['SQ_ECOM_MD']) && (md5($_POST['MD']) == $_SESSION['SQ_ECOM_MD'])) {
			//THIS PAGE IS REDIRECTED TO BY THE ACCESS CONTROL SERVER (ACS) OF ISSUEING BANK AFTER CARD HOLDER VERIFICATION PROCESS
			$is_in_frame = TRUE;
			unset($_SESSION['SQ_ECOM_MD']);
			$success = $this->authorizePayment($_POST['MD'], $_POST['PaRes']);
			if ($success === FALSE) {
				//transaction fails, redirect the parent document of the iframe and display error
				$this->_redirectParent($this->getURL().'?error_message='.urlencode($this->_tmp['error_message'][0]));
			}
		}

		//if transaction is accepted, redirect the user to the success page
		if ($success === 1) {
			unset($_SESSION['SQ_ECOM_AMOUNT']);
			//clear pass through variables
			if (isset($_SESSION['SQ_ECOM_PASS_THROUGH_VARS'])) {
				unset($_SESSION['SQ_ECOM_PASS_THROUGH_VARS']);
			}

			$this->appendPendingOrderAuditLog(translate('Payment is successful'));


			$this->returnToCaller('SUCCESS', $is_in_frame);
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
			$datapath = sq_web_path('data').'/asset_types/payment_gateway_sagepay/files';
			$card = $this->getPrefix().'_card';

			if ($this->attr('run_mode') != 'Live') {
				echo '<h2 class="gateway-test-mode" style="color: white; background-color: #C00; padding: 5px">'.strtoupper($this->attr('run_mode')).' MODE</h2>';
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

		$amount = $_SESSION['SQ_ECOM_AMOUNT'];

		$processor = new SagePay_Processor($this->attr('protocol'), $this->attr('vendor'), $this->attr('timeout'), $this->attr('run_mode'));

		//SET AMOUNT
		if (empty($amount)) {
			$this->_setError(translate('No amount is specified for the payment'));

		} else {
			$processor->setAmount($amount, $this->attr('currency'));
		}

		//SET MERCHANT REFERENCE
		//merchant reference number should be generated here instead of inside default_delivery_method.inc or form_ecommerce.inc
		//because the customer can try to pay many times until his card is accepted. So each time a payment transaction
		//is sent to SagePay, a new merchant reference number should be used because using the old reference number can
		//cause the "duplicate reference" error (VendorTxCode has been used before).
		$processor->setMerchantReference(md5(session_id().time()));
		//get the merchant reference number back, it might be changed because SagePay requires that merchant reference number length must be less than 40 characters
		//it is not changed with the above md5 function which generates 32 character string, but we use it as general way here
		$_SESSION['SQ_ECOM_REF_NO'] = $processor->getMerchantReference();

		//store the merchant reference number to the pending order asset before processing
		$this->setPendingOrderMerchantReference($_SESSION['SQ_ECOM_REF_NO']);

		//SET CARD NAME
		if (empty($card_info['name'])) {
			$this->_setError(translate('No name on card is specified for the payment'));

		} else {
			$processor->setCardName($card_info['name']);
		}

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
			if ($this->_isValidCV2($card_info['cv2'], $card_info['type'])) {
				$processor->setCardCV2($card_info['cv2']);
			} else {
				$this->_setError(translate('The card CV2 is invalid (must be three or four (only American Express Card) digits long)'));

			}
		}

		//SET CARD TYPE
		if (empty($card_info['type'])) {
			$this->_setError(translate('No card type is specified for the payment'));

		} else {
			$processor->setCardType($card_info['type']);
		}

		//SET BILLING FIRST NAMES
		$billing_first_names = array_get_index($card_info, 'billing_first_names', '');
		if ($billing_first_names != '') {
			$processor->setBillingFirstnames($billing_first_names);
		}

		//SET BILLING SURNAME
		$billing_surname = array_get_index($card_info, 'billing_surname', '');
		if ($billing_surname != '') {
			$processor->setBillingSurname($billing_surname);
		}

		$billing_addr = '';
		//SET BILLING ADDRESS 1
		$billing_address1 = trim(array_get_index($card_info, 'billing_address1_'.MIN_LINE_IN_ADDRESS, ''));
		for ($i = MIN_LINE_IN_ADDRESS + 1; $i <= MAX_LINE_IN_ADDRESS; $i++) {
			$line = array_get_index($card_info, "billing_address1_$i", '');
			if (trim($line) != '') {
				$billing_address1 .= ", $line";
			}
		}
		if ($billing_address1 != '') {
			$billing_addr .= $billing_address1."\n";
			$processor->setBillingAddress1($billing_address1);
		}

		//SET BILLING ADDRESS 2
		$billing_address2 = trim(array_get_index($card_info, 'billing_address2_'.MIN_LINE_IN_ADDRESS, ''));
		for ($i = MIN_LINE_IN_ADDRESS + 1; $i <= MAX_LINE_IN_ADDRESS; $i++) {
			$line = array_get_index($card_info, "billing_address2_$i", '');
			if (trim($line) != '') {
				$billing_address2 .= ", $line";
			}
		}
		if ($billing_address2 != '') {
			$billing_addr .= $billing_address2."\n";
			$processor->setBillingAddress2($billing_address2);
		}

		//SET BILLING CITY
		$billing_city = array_get_index($card_info, 'billing_city', '');
		if ($billing_city != '') {
			$billing_addr .= $billing_city."\n";
			$processor->setBillingCity($billing_city);
		}

		//SET BILLING STATE
		$billing_state = array_get_index($card_info, 'billing_state', '');
		if ($billing_state != '') {
			$billing_addr .= $billing_state."\n";
			$processor->setBillingState($billing_state);
		}

		//SET BILLING POSTCODE
		$billing_postcode = array_get_index($card_info, 'billing_postcode', '');
		if ($billing_postcode != '') {
			$billing_addr .= $billing_postcode."\n";
			$processor->setBillingPostcode($billing_postcode);
		}

		//GET THE LIST OF COUNTRY CODE IF NECESSARY
		if ((array_get_index($card_info, 'billing_country', '') != '') || (array_get_index($card_info, 'delivery_country', '') != '')) {
			global $standards_lists_countries;
			require_once SQ_FUDGE_PATH.'/standards_lists/countries.inc';
		}

		//SET BILLING COUNTRY
		$billing_country_code = array_get_index($card_info, 'billing_country', '');
		if ($billing_country_code != '') {
			$billing_country = array_get_index($standards_lists_countries, $billing_country_code, strtoupper($billing_country_code));
			$billing_addr .= $billing_country."\n";
			$processor->setBillingCountry($billing_country_code);
		}

		//SET BILLING PHONE
		$billing_phone = array_get_index($card_info, 'billing_phone', '');
		if ($billing_phone != '') {
			$billing_addr .= 'Phone: '.$billing_phone."\n";
			$processor->setBillingPhone($billing_phone);
		}

		if ($this->attr('delivery_addr_is_billing_addr')) {
			$delivery_str = 'billing_';
		} else {
			$delivery_str = 'delivery_';
		}

		//SET DELIVERY FIRST NAMES
		$delivery_first_names = array_get_index($card_info, $delivery_str.'first_names', '');
		if ($delivery_first_names != '') {
			$processor->setDeliveryFirstnames($delivery_first_names);
		}

		//SET DELIVERY SURNAME
		$delivery_surname = array_get_index($card_info, $delivery_str.'surname', '');
		if ($delivery_surname != '') {
			$processor->setDeliverySurname($delivery_surname);
		}

		$delivery_addr = '';
		//SET DELIVERY ADDRESS 1
		$delivery_address1 = trim(array_get_index($card_info, $delivery_str.'address1_'.MIN_LINE_IN_ADDRESS, ''));
		for ($i = MIN_LINE_IN_ADDRESS + 1; $i <= MAX_LINE_IN_ADDRESS; $i++) {
			$line = array_get_index($card_info, "{$delivery_str}address1_$i", '');
			if (trim($line) != '') {
				$delivery_address1 .= ", $line";
			}
		}
		if ($delivery_address1 != '') {
			$delivery_addr .= $delivery_address1."\n";
			$processor->setDeliveryAddress1($delivery_address1);
		}

		//SET DELIVERY ADDRESS 2
		$delivery_address2 = trim(array_get_index($card_info, $delivery_str.'address2_'.MIN_LINE_IN_ADDRESS, ''));
		for ($i = MIN_LINE_IN_ADDRESS + 1; $i <= MAX_LINE_IN_ADDRESS; $i++) {
			$line = array_get_index($card_info, "{$delivery_str}address2_$i", '');
			if (trim($line) != '') {
				$delivery_address2 .= ", $line";
			}
		}
		if ($delivery_address2 != '') {
			$delivery_addr .= $delivery_address2."\n";
			$processor->setDeliveryAddress2($delivery_address2);
		}

		//SET DELIVERY CITY
		$delivery_city = array_get_index($card_info, $delivery_str.'city', '');
		if ($delivery_city != '') {
			$delivery_addr .= $delivery_city."\n";
			$processor->setDeliveryCity($delivery_city);
		}

		//SET DELIVERY STATE
		$delivery_state = array_get_index($card_info, $delivery_str.'state', '');
		if ($delivery_state != '') {
			$delivery_addr .= $delivery_state."\n";
			$processor->setDeliveryState($delivery_state);
		}

		//SET DELIVERY POSTCODE
		$delivery_postcode = array_get_index($card_info, $delivery_str.'postcode', '');
		if ($delivery_postcode != '') {
			$delivery_addr .= $delivery_postcode."\n";
			$processor->setDeliveryPostcode($delivery_postcode);
		}

		//SET DELIVERY COUNTRY
		$delivery_country_code = array_get_index($card_info, $delivery_str.'country', '');
		if ($delivery_country_code != '') {
			$delivery_country = array_get_index($standards_lists_countries, $delivery_country_code, strtoupper($delivery_country_code));
			$delivery_addr .= $delivery_country."\n";
			$processor->setDeliveryCountry($delivery_country_code);
		}

		//SET DELIVERY PHONE
		$delivery_phone = array_get_index($card_info, $delivery_str.'phone', '');
		if ($delivery_phone != '') {
			$delivery_addr .= 'Phone: '.$delivery_phone."\n";
			$processor->setDeliveryPhone($delivery_phone);
		}


		//SET DESCRIPTION
		$processor->setDescription($this->attr('description'));

		//SET TRANSACTION TYPE AS "PAYMENT"
		$processor->setTransactionTypePayment();

		//SET 3-D SECURE
		if ($this->attr('threeds_enabled')) {
			$processor->setThreeDSecure();
		}

		//SET AVS/CV2
		if ($this->attr('avscv2_enabled')) {
			$processor->setAVSCV2();

			if (empty($card_info['cv2'])) {
				$this->_setError(translate('No CV2 is specified for the card'));

			}

			if ($billing_first_names == '') {
				$this->_setError(translate('No billing first names are specified'));

			}
			if ($billing_surname == '') {
				$this->_setError(translate('No billing surname is specified'));

			}
			if ($billing_address1 == '') {
				$this->_setError(translate('The first line of the billing address is not specified'));

			}
			if ($billing_city == '') {
				$this->_setError(translate('No billing city is specified'));

			}
			if ($billing_postcode == '') {
				$this->_setError(translate('No billing postcode is specified'));

			}
			if ($billing_country_code == '') {
				$this->_setError(translate('No billing country is specified'));

			}
			if (($billing_country_code == 'us') && ($billing_state == '')) {
				$this->_setError(translate('No billing state is specified for the United States address'));

			}

			if (!$this->attr('delivery_addr_is_billing_addr')) {
				if ($delivery_first_names == '') {
					$this->_setError(translate('No delivery first names are specified'));

				}
				if ($delivery_surname == '') {
					$this->_setError(translate('No delivery surname is specified'));

				}
				if ($delivery_address1 == '') {
					$this->_setError(translate('The first line of the delivery address is not specified'));

				}
				if ($delivery_city == '') {
					$this->_setError(translate('No delivery city is specified'));

				}
				if ($delivery_postcode == '') {
					$this->_setError(translate('No delivery postcode is specified'));

				}
				if ($delivery_country_code == '') {
					$this->_setError(translate('No delivery country is specified'));

				}
				if (($delivery_country_code == 'us') && ($delivery_state == '')) {
					$this->_setError(translate('No delivery state is specified for the United States address'));

				}
			}
		}

		//SET GIFT AID PAYMENT
		$var_name = $this->attr('giftaid_var_name');
		if (!empty($var_name)) {
			$gift_aid = $this->_getPassThroughVariable($var_name, FALSE);
			if ($gift_aid !== FALSE){
				$gift_aid = strtolower($gift_aid);
				if (($gift_aid == '1') || ($gift_aid == 'yes') || ($gift_aid == 'true')) {
					$processor->setGiftAidPayment();
				}
			}
		}

		//CHECK IF THERE IS ERROR BEFORE SENDING DATA TO SagePay SERVER, RETURN FALSE
		if ($this->isError()) {
			return FALSE;
		}

		$success = $processor->process();

		//GET RESPONSE
		$response = $processor->getResponse();

		//if success (1 or 2), save the last 4 digits of card number to output later
		if ($success) {
			//store the last 4 digits of card number to display it later in Form Ecommerce or Delivery Method through the Session variable SQ_ECOM_RESPONSE
			$_SESSION['SQ_ECOM_FORMATTED_CARD_NUMBER'] = $this->_getFormattedCardNumber($card_info['number']);
			//store the billing name, billing address, deliver name and deliver address to store in the Order asset of Form Ecommerce
			$_SESSION['SQ_ECOM_ORDER_BILLING_NAME'] = "$billing_first_names $billing_surname";
			$_SESSION['SQ_ECOM_ORDER_BILLING_ADDR'] = $billing_addr;
			$_SESSION['SQ_ECOM_ORDER_DELIVERY_NAME'] = "$delivery_first_names $delivery_surname";
			$_SESSION['SQ_ECOM_ORDER_DELIVERY_ADDR'] = $delivery_addr;
			//build the addresses XML to store in the Ecommerce Order XML
			$billing_state_str = '';
			if ($billing_state != '') {
				$billing_state_str = <<<HEREDOC

			<state><![CDATA[{$this->_getCDATAAllowedString($billing_state)}]]></state>
HEREDOC;
			}

			$delivery_state_str = '';
			if ($delivery_state != '') {
				$delivery_state_str = <<<HEREDOC

			<state><![CDATA[{$this->_getCDATAAllowedString($delivery_state)}]]></state>
HEREDOC;
			}

			$_SESSION['SQ_ECOM_ORDER_PAYMENT_GATEWAY_XML'] = <<<HEREDOC

	<payment_gateway>
		<billing_address>
			<first_names><![CDATA[{$this->_getCDATAAllowedString($billing_first_names)}]]></first_names>
			<surname><![CDATA[{$this->_getCDATAAllowedString($billing_surname)}]]></surname>
			<address1><![CDATA[{$this->_getCDATAAllowedString($billing_address1)}]]></address1>
			<address2><![CDATA[{$this->_getCDATAAllowedString($billing_address2)}]]></address2>
			<city><![CDATA[{$this->_getCDATAAllowedString($billing_city)}]]></city>$billing_state_str
			<postcode><![CDATA[{$this->_getCDATAAllowedString($billing_postcode)}]]></postcode>
			<country><![CDATA[{$this->_getCDATAAllowedString($billing_country)}]]></country>
			<country_code><![CDATA[{$this->_getCDATAAllowedString($billing_country_code)}]]></country_code>
			<phone><![CDATA[{$this->_getCDATAAllowedString($billing_phone)}]]></phone>
		</billing_address>
		<delivery_address>
			<first_names><![CDATA[{$this->_getCDATAAllowedString($delivery_first_names)}]]></first_names>
			<surname><![CDATA[{$this->_getCDATAAllowedString($delivery_surname)}]]></surname>
			<address1><![CDATA[{$this->_getCDATAAllowedString($delivery_address1)}]]></address1>
			<address2><![CDATA[{$this->_getCDATAAllowedString($delivery_address2)}]]></address2>
			<city><![CDATA[{$this->_getCDATAAllowedString($delivery_city)}]]></city>$delivery_state_str
			<postcode><![CDATA[{$this->_getCDATAAllowedString($delivery_postcode)}]]></postcode>
			<country><![CDATA[{$this->_getCDATAAllowedString($delivery_country)}]]></country>
			<country_code><![CDATA[{$this->_getCDATAAllowedString($delivery_country_code)}]]></country_code>
			<phone><![CDATA[{$this->_getCDATAAllowedString($delivery_phone)}]]></phone>
		</delivery_address>
	</payment_gateway>
HEREDOC;

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
	 * @param string $md		the MD sent back by the ACS
	 * @param string $pares		the Payment Authentication Response (PARes) returned by the Access Control Server (ACS)
	 *
	 * @return mixed		return 1 if SUCCESS; otherwise, return FALSE
	 * @access public
	 */
	function authorizePayment($md, $pares)
	{
		$processor = new SagePay_Processor($this->attr('protocol'), $this->attr('vendor'), $this->attr('timeout'), $this->attr('run_mode'));

		$success = $processor->authorize($_SESSION['SQ_ECOM_REF_NO'], $md, $pares);

		//get response
		$response = $processor->getResponse();

		$this->_setTransactionResult($success, $response);

		return $success;

	}//end authorizePayment()


	/**
	 * Set the result of the transaction. If transaction fails, set its error message. If transaction success, set the Session variable SQ_ECOM_RESPONSE
	 *
	 * @param boolean	$success		the reason of the transaction. It is either 1 or FALSE
	 * @param array		$response		the response array from the SagePay Processor
	 *
	 * @return void
	 * @access private
	 */
	private function _setTransactionResult($success, $response)
	{
		if ($success === FALSE) {
			$this->_setError($response['STATUS']);
		} else { //success = 1
			if ($this->attr('run_mode') != 'Live') {
				$response['STATUS'] = '!!!ATTENTION!!! '.strtoupper($this->attr('run_mode')).' MODE (transaction not performed) -- '.$response['STATUS'];
			}

			$card_no = '';
			if (isset($_SESSION['SQ_ECOM_FORMATTED_CARD_NUMBER'])) {
				$card_no = $_SESSION['SQ_ECOM_FORMATTED_CARD_NUMBER'];
				unset($_SESSION['SQ_ECOM_FORMATTED_CARD_NUMBER']);
			}

			$response['CARDNO'] = $card_no;
			$response['AMOUNT'] = $_SESSION['SQ_ECOM_AMOUNT'];

			$billing_name = '';
			if (isset($_SESSION['SQ_ECOM_ORDER_BILLING_NAME'])) {
				$billing_name = $_SESSION['SQ_ECOM_ORDER_BILLING_NAME'];
				unset($_SESSION['SQ_ECOM_ORDER_BILLING_NAME']);
			}
			$response['BILLING_NAME'] = $billing_name;

			$billing_addr = '';
			if (isset($_SESSION['SQ_ECOM_ORDER_BILLING_ADDR'])) {
				$billing_addr = $_SESSION['SQ_ECOM_ORDER_BILLING_ADDR'];
				unset($_SESSION['SQ_ECOM_ORDER_BILLING_ADDR']);
			}
			$response['BILLING_ADDR'] = $billing_addr;

			$delivery_name = '';
			if (isset($_SESSION['SQ_ECOM_ORDER_DELIVERY_NAME'])) {
				$delivery_name = $_SESSION['SQ_ECOM_ORDER_DELIVERY_NAME'];
				unset($_SESSION['SQ_ECOM_ORDER_DELIVERY_NAME']);
			}
			$response['DELIVERY_NAME'] = $delivery_name;

			$delivery_addr = '';
			if (isset($_SESSION['SQ_ECOM_ORDER_DELIVERY_ADDR'])) {
				$delivery_addr = $_SESSION['SQ_ECOM_ORDER_DELIVERY_ADDR'];
				unset($_SESSION['SQ_ECOM_ORDER_DELIVERY_ADDR']);
			}
			$response['DELIVERY_ADDR'] = $delivery_addr;

			$addresses_xml = '';
			if (isset($_SESSION['SQ_ECOM_ORDER_PAYMENT_GATEWAY_XML'])) {
				$addresses_xml = $_SESSION['SQ_ECOM_ORDER_PAYMENT_GATEWAY_XML'];
				unset($_SESSION['SQ_ECOM_ORDER_PAYMENT_GATEWAY_XML']);
			}
			$response['PAYMENT_GATEWAY_XML'] = $addresses_xml;

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
	* Card Number keyword replacement
	*
	* @return string
	* @access public
	*/
	function getCardNumberKeywordReplacement()
	{
		$prefix = $this->getPrefix();
		$card = $prefix.'_card';

		// set variables for simulator and test modes
		if ($this->attr('run_mode') != 'Live') {
			$test_card_numbers = SagePay_Processor::getTestCardNumbers();
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
	* Card Start Month Select keyword replacement
	*
	* @return string
	* @access public
	*/
	function getCardStartMonthSelectKeywordReplacement()
	{
		$card = $this->getPrefix().'_card';

		$string = '<select name="'.$card.'[start_month]">';
		$string .= '<option value="" >--</option>';
		for ($i = 1; $i <= 12; $i++) {
			$month = sprintf('%02s', $i);
			$string .= '<option value="'.$month.'" >'.$month.'</option>';
		}
		$string .= '</select>';

		return $string;

	}//end getCardStartMonthSelectKeywordReplacement()


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
	* Card Start Year Select keyword replacement
	*
	* @return string
	* @access public
	*/
	function getCardStartYearSelectKeywordReplacement($period = 5)
	{
		$card = $this->getPrefix().'_card';

		$string = '<select name="'.$card.'[start_year]">';
		$string .= '<option value="" >----</option>';
		$year = date('Y') - $period;
		for ($i = 0; $i <= $period; $i++) {
			$string .= '<option value="'.substr($year, -2).'" >'.$year.'</option>';
			$year++;
		}
		$string .= '</select>';

		return $string;

	}//end getCardStartYearSelectKeywordReplacement()


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
	function getCardExpiryYearSelectKeywordReplacement($period = 18)
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
	* Card Type keyword replacement
	*
	* @return string
	* @access private
	*/
	function getCardTypeKeywordReplacement()
	{
		$card = $this->getPrefix().'_card';

		$available_card_types = SagePay_Processor::getCardTypes();

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
	function getBillingFirstNamesKeywordReplacement()
	{
		ob_start();
		$card = $this->getPrefix().'_card';
		text_box($card.'[billing_first_names]', $this->_getDefaultInputFromPassThroughAttribute('firstname_var_name'));

		return ob_get_clean();

	}//end getBillingFirstNamesKeywordReplacement()


	/**
	* Billing Surname keyword replacement
	*
	* @return string
	* @access private
	*/
	function getBillingSurnameKeywordReplacement()
	{
		ob_start();
		$card = $this->getPrefix().'_card';
		text_box($card.'[billing_surname]', $this->_getDefaultInputFromPassThroughAttribute('surname_var_name'));

		return ob_get_clean();

	}//end getBillingSurnameKeywordReplacement()


	/**
	* Billing Address 1 keyword replacement
	*
	* @param string		$line	the line number of the billing address 1
	*
	* @return string
	* @access private
	*/
	function getBillingAddress1KeywordReplacement($line = MIN_LINE_IN_ADDRESS)
	{
		ob_start();
		$card = $this->getPrefix().'_card';
		text_box($card."[billing_address1_$line]", $this->_getDefaultInputFromPassThroughAttribute('addr1_var_name', $line));

		return ob_get_clean();

	}//end getBillingAddress1KeywordReplacement()


	/**
	* Billing Address 2 keyword replacement
	*
	* @param string		$line	the line number of the billing address 2
	*
	* @return string
	* @access private
	*/
	function getBillingAddress2KeywordReplacement($line = MIN_LINE_IN_ADDRESS)
	{
		ob_start();
		$card = $this->getPrefix().'_card';
		text_box($card."[billing_address2_$line]", $this->_getDefaultInputFromPassThroughAttribute('addr2_var_name', $line));

		return ob_get_clean();

	}//end getBillingAddress2KeywordReplacement()


	/**
	* Billing City keyword replacement
	*
	* @return string
	* @access private
	*/
	function getBillingCityKeywordReplacement()
	{
		ob_start();
		$card = $this->getPrefix().'_card';
		text_box($card.'[billing_city]', $this->_getDefaultInputFromPassThroughAttribute('city_var_name'));

		return ob_get_clean();

	}//end getBillingCityKeywordReplacement()


	/**
	* Billing Postcode keyword replacement
	*
	* @return string
	* @access private
	*/
	function getBillingPostcodeKeywordReplacement()
	{
		ob_start();
		$card = $this->getPrefix().'_card';
		text_box($card.'[billing_postcode]', $this->_getDefaultInputFromPassThroughAttribute('postcode_var_name'));

		return ob_get_clean();

	}//end getBillingPostcodeKeywordReplacement()


	/**
	* Billing Country keyword replacement
	*
	* @return string
	* @access private
	*/
	function getBillingCountryKeywordReplacement()
	{
		global $standards_lists_countries;
		require_once SQ_FUDGE_PATH.'/standards_lists/countries.inc';

		ob_start();
		$card = $this->getPrefix().'_card';
		combo_box($card.'[billing_country]', $standards_lists_countries, FALSE, $this->_getDefaultInputFromPassThroughAttribute('country_var_name'));

		return ob_get_clean();

	}//end getBillingCountryKeywordReplacement()


	/**
	* Billing State keyword replacement
	*
	* @return string
	* @access private
	*/
	function getBillingStateKeywordReplacement()
	{
		$card = $this->getPrefix().'_card';
		$string = '<input name="'.$card.'[billing_state]" />';
		return $string;

	}//end getBillingStateKeywordReplacement()


	/**
	* Billing Phone keyword replacement
	*
	* @return string
	* @access private
	*/
	function getBillingPhoneKeywordReplacement()
	{
		$card = $this->getPrefix().'_card';
		$string = '<input name="'.$card.'[billing_phone]" autocomplete="off" />';
		return $string;

	}//end getBillingPhoneKeywordReplacement()


	/**
	* Delivery First Names keyword replacement
	*
	* @return string
	* @access private
	*/
	function getDeliveryFirstNamesKeywordReplacement()
	{
		ob_start();
		$card = $this->getPrefix().'_card';
		$this->_delivery_textbox($card.'[delivery_first_names]', $this->_getDefaultInputFromPassThroughAttribute('firstname_var_name'));

		return ob_get_clean();

	}//end getDeliveryFirstNamesKeywordReplacement()


	/**
	* Delivery Surname keyword replacement
	*
	* @return string
	* @access private
	*/
	function getDeliverySurnameKeywordReplacement()
	{
		ob_start();
		$card = $this->getPrefix().'_card';
		$this->_delivery_textbox($card.'[delivery_surname]', $this->_getDefaultInputFromPassThroughAttribute('surname_var_name'));

		return ob_get_clean();

	}//end getDeliverySurnameKeywordReplacement()


	/**
	* Delivery Address 1 keyword replacement
	*
	* @param string		$line	the line number of the delivery address 1
	*
	* @return string
	* @access private
	*/
	function getDeliveryAddress1KeywordReplacement($line = MIN_LINE_IN_ADDRESS)
	{
		ob_start();
		$card = $this->getPrefix().'_card';
		$this->_delivery_textbox($card."[delivery_address1_$line]", $this->_getDefaultInputFromPassThroughAttribute('addr1_var_name', $line));

		return ob_get_clean();

	}//end getDeliveryAddress1KeywordReplacement()


	/**
	* Delivery Address 2 keyword replacement
	*
	* @param string		$line	the line number of the delivery address 2
	*
	* @return string
	* @access private
	*/
	function getDeliveryAddress2KeywordReplacement($line = MIN_LINE_IN_ADDRESS)
	{
		ob_start();
		$card = $this->getPrefix().'_card';
		$this->_delivery_textbox($card."[delivery_address2_$line]", $this->_getDefaultInputFromPassThroughAttribute('addr2_var_name', $line));

		return ob_get_clean();

	}//end getDeliveryAddress2KeywordReplacement()


	/**
	* Delivery City keyword replacement
	*
	* @return string
	* @access private
	*/
	function getDeliveryCityKeywordReplacement()
	{
		ob_start();
		$card = $this->getPrefix().'_card';
		$this->_delivery_textbox($card.'[delivery_city]', $this->_getDefaultInputFromPassThroughAttribute('city_var_name'));

		return ob_get_clean();

	}//end getDeliveryCityKeywordReplacement()


	/**
	* Delivery Postcode keyword replacement
	*
	* @return string
	* @access private
	*/
	function getDeliveryPostcodeKeywordReplacement()
	{
		ob_start();
		$card = $this->getPrefix().'_card';
		$this->_delivery_textbox($card.'[delivery_postcode]', $this->_getDefaultInputFromPassThroughAttribute('postcode_var_name'));

		return ob_get_clean();

	}//end getDeliveryPostcodeKeywordReplacement()


	/**
	* Delivery Country keyword replacement
	*
	* @return string
	* @access private
	*/
	function getDeliveryCountryKeywordReplacement()
	{
		ob_start();
		$card = $this->getPrefix().'_card';
		$name = $card.'[delivery_country]';
		$value = $this->_getDefaultInputFromPassThroughAttribute('country_var_name');
		if ($this->attr('hide_delivery_addr')) {
			hidden_field($name, $value);
		} else {
			global $standards_lists_countries;
			require_once SQ_FUDGE_PATH.'/standards_lists/countries.inc';

			combo_box($name, $standards_lists_countries, FALSE, $value);
		}

		return ob_get_clean();

	}//end getDeliveryCountryKeywordReplacement()


	/**
	* Delivery State keyword replacement
	*
	* @return string
	* @access private
	*/
	function getDeliveryStateKeywordReplacement()
	{
		$card = $this->getPrefix().'_card';
		$string = '<input name="'.$card.'[delivery_state]" />';
		return $string;

	}//end getDeliveryStateKeywordReplacement()


	/**
	* Delivery Phone keyword replacement
	*
	* @return string
	* @access private
	*/
	function getDeliveryPhoneKeywordReplacement()
	{
		$card = $this->getPrefix().'_card';
		$string = '<input name="'.$card.'[delivery_phone]" />';
		return $string;

	}//end getDeliveryPhoneKeywordReplacement()


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
		$string = <<<HEREDOC
<form name="PARequestForm" action="{$this->_redirect_info['ACS_URL']}" method="POST">
	<input type="hidden" name="PaReq" value="{$this->_redirect_info['PAREQ_MESSAGE']}">
	<input type="hidden" name="TermUrl" value="{$this->getURL()}">
	<input type="hidden" name="MD" value="{$this->_redirect_info['MD']}">
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
	 * Set error for the transaction so that the getProcessingErrorKeywordReplacement() method will return the error message
	 *
	 * @param string $err_message	the error message to set
	 * @return void
	 * @access private
	 */
	private function _setError($err_message)
	{
		$this->_tmp['is_error'] = TRUE;
		//we need to split the error message here because SagePay error has the following format:
		//3153 : The Card AddressLine1 is required., 3156 : The Card City field is required., 5022 : The Postcode value is required.
		$errs = explode(',', $err_message);
		foreach ($errs as $err_str) {
			$err_details = explode(':', $err_str, 2);
			if (isset($err_details[1]) && !empty($err_details[1])) {
				$this->_tmp['error_message'][] = trim($err_details[1]);
			} else {
				$this->_tmp['error_message'][] = $err_details[0];
			}
		}

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
	 * @return boolean				return TRUE if the issue number is valid; otherwise, return FALSE
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
	 * @param string $cv2			the input CV2 to test
	 * @param string $card_type		the type of the card to test
	 *
	 * @return boolean				return TRUE if the CV2 is valid; otherwise, return FALSE
	 * @access private
	 */
	private function _isValidCV2($cv2, $card_type)
	{
		$valid = FALSE;

		if ($card_type == 'AMEX') {
			$digit_no = 4;
		} else {
			$digit_no = 3;
		}
		$pattern = '/^\d{'.$digit_no.'}$/';
		//cv2 must be three or four (AMEX card only) digits long
		if (preg_match($pattern, $cv2)) {
			$valid = TRUE;
		}

		return $valid;

	}//end _isValidCV2()


	/**
	 * Get pass-through variable which is stored in a Session variable
	 *
	 * @param string 	$var_name	the name of the pass-through variable
	 * @param mixed 	$default	the default value to get if the $var_name variable is not in the Session
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
	 * @param string	$attr_name		the name of the attribute that stores the variable name
	 * @param int		$line			the line number of the address, this parameter is only passed in if the attribute name is addr1_var_name or addr2_var_name
	 *
	 * @return mixed
	 * @access private
	 */
	private function _getDefaultInputFromPassThroughAttribute($attr_name, $line = '')
	{
		$result = '';
		$var_name = $this->attr($attr_name).$line;
		if (!empty($var_name)) {
			$result = $this->_getPassThroughVariable($var_name, '');
		}

		return $result;

	}//end _getDefaultInputFromPassThroughAttribute()


	/**
	 * Get the string that is allowed in an XML's CDATA section
	 *
	 * @param string $text	the input string to check and get allowed CDATA string
	 * @return string
	 * @access private
	 */
	private function _getCDATAAllowedString($text)
	{
		//CDATA section can not contain a nested CDATA and the closed string ]]>
		$search = Array('<![CDATA[', ']]>');
		$replace = Array('&lt;![CDATA[', ']]&gt;');

		return str_replace($search, $replace, $text);

	}//end _getCDATAAllowedString()


	/**
	 * Paint a textbox or a hidden field for delivery address components based on the hide_delivery_addr attribute
	 *
	 * @param string	$name	Name of the textbox or hidden field
	 * @param string	$value	Value of the textbox or hidden field
	 *
	 * @return void
	 * @access private
	 */
	function _delivery_textbox($name, $value)
	{
		if ($this->attr('hide_delivery_addr')) {
			hidden_field($name, $value);
		} else {
			text_box($name, $value);
		}

	}//end getDeliveryFirstNamesKeywordReplacement()


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
