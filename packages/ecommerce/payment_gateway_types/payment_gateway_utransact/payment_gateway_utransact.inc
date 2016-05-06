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
* $Id: payment_gateway_utransact.inc,v 1.6 2012/11/16 05:02:56 cupreti Exp $
*
*/

require_once dirname(__FILE__).'/../../payment_gateway/payment_gateway.inc';
require_once dirname(__FILE__).'/utransact_processor.inc';

/**
* Payment_Gateway_Utransact
*
* Interface for online credit card processing using Utransact payment gateway
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: 1.6 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Payment_Gateway_UTransact extends Payment_Gateway
{


	/**
	* UTransact processor object
	*
	* @var Object
	*/
	private $_processor = null;


	/**
	* Weather the asset is deprecated or not.
	* this will be used in asset_map.inc:_cloneAsset() to see if the asset type
	* can be created (cloned) on the system
	*/
	public $deprecated = TRUE;


	/**
	* Marking the asset deprecated since so that we can
	* use it when warning the users on backend
	*/
	public $deprecated_since = '5.1';


    /**
    * Constructor
    *
    */
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* Standard pages create a bodycopy asset when they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	function _createAdditional(&$link)
	{
		if (!parent::_createAdditional($link)) return FALSE;

		return $this->_createBodycopy();

	}//end _createAdditional()


	/**
	 * Create the Utransact processor object
	 *
	 * @return boolean	    return TRUE processor was created successfully
	 * @access private
	 */
	private function _loadProcessor()
	{
		// Create the UTransact processor for the gateway
		$this->_processor = new UTransact_Processor(
                                    $this->attr('username'),
                                    $this->attr('password'),
                                    $this->attr('bgas_webservice_url'),
                                    $this->attr('transactions_webservice_url'),
                                    $this->attr('services_webservice_url')
                            );

		return TRUE;
	}//end _loadProcessor()


	/**
	* Create Bodycopy
	*
	* @param string	$default_content	default content to put into the bodycopy
	*
	* @return string
	* @access private
	*/
	function _createBodycopy($default_content=NULL)
	{
		if (is_null($default_content)) {
			$default_content = translate('%processing_error%
<div style="background-color: #CCC; padding: 5px"><strong>$%transaction_amount%</strong> (%transaction_gst% percent GST)  dollars are about to be debited from your card</div>
<table cellpadding="5">
	<tr>
		<td align="right">Name on Card</td>
		<td>%credit_card_name%</td>
	</tr>
	<tr>
		<td align="right">Card Number</td>
		<td>%credit_card_number%  %credit_card_ccv%</td>
	</tr>
	<tr>
		<td align="right">Expiry Date (MMM/YYYY)</td>
		<td>%credit_card_expiry_month% / %credit_card_expiry_year%</td>
	</tr>
	<tr>
		<td align="right">Card Type</td>
		<td>%credit_card_type_selector%</td>
	</tr>
	<tr>
		<td>&nbsp;</td>
		<td>%submit_button% %reset_button% %cancel_button%</td>
	</tr>
</table>');

		}

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');
		$bodycopy = new Bodycopy();
		$copy_link = Array('asset' => &$this, 'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);

		$bodycopy->setAttrValue('name', 'Display Format');
		$args = Array('content' => $default_content);
		if ($bodycopy->create($copy_link, $args)) {
			$child_bodycopy = $GLOBALS['SQ_SYSTEM']->am->getLink($bodycopy->id, SQ_LINK_TYPE_2, 'bodycopy_div');
			$GLOBALS['SQ_SYSTEM']->am->releaseLock($child_bodycopy['minorid'], 'attributes');
			return TRUE;
		}
		return FALSE;

	}//end _createBodycopy()


	/**
	* Return the available keywords from the parents
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

		$keywords['credit_card_number'] = 'Credit Card Number';
		$keywords['credit_card_expiry_year'] = 'Credit Card Expiry Year';
		$keywords['credit_card_expiry_month'] = 'Credit Card Expiry Month';
		$keywords['credit_card_name'] = 'Name on card';
		$keywords['credit_card_ccv'] = 'Credit Card CCV field';
		$keywords['credit_card_type_selector'] = 'Credit Card selector';

		$keywords['transaction_amount'] = 'Total transaction amount';
		$keywords['transaction_gst'] = 'GST percent set at gateway (Not set for transaction)';

		$keywords['submit_button'] = 'Submit Button';
		$keywords['cancel_button'] = 'Cancel Button';
		$keywords['reset_button'] = 'Reset Button';

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
		$amount = array_get_index($_SESSION,'SQ_ECOM_AMOUNT', 0);

		$card_info = array_get_index($_REQUEST, $this->getPrefix().'_card', FALSE);
		$action = array_get_index($_REQUEST, $this->getPrefix().'_action', FALSE);

		if ($action == 'Cancel') {
			unset($_SESSION['UTRANSACT_TRANSACTION_HANDLE']);
			$this->returnToCaller('CANCEL');

		} else if (empty($amount)) {
			unset($_SESSION['UTRANSACT_TRANSACTION_HANDLE']);
			unset($_SESSION['SQ_ECOM_AMOUNT']);
			unset($_SESSION['SQ_ECOM_GST']);

			$this->appendPendingOrderAuditLog(translate('Payment has no amount'));


			$this->returnToCaller('SUCCESS');

		} else if ($card_info) {
			$this->appendPendingOrderAuditLog(translate('UTransact payment gateway contacted, payment attempted'));


			if ($this->processPayment($card_info)) {

				unset($_SESSION['UTRANSACT_TRANSACTION_HANDLE']);
				unset($_SESSION['SQ_ECOM_AMOUNT']);
				unset($_SESSION['SQ_ECOM_GST']);

				// Broadcast payment completed trigger event before return
				if (isset($_SESSION['SQ_ECOM_PAYMENT_COMPLETED_BROADCASTER_ASSETID'])) {

					$broadcaster = $GLOBALS['SQ_SYSTEM']->am->getAsset($_SESSION['SQ_ECOM_PAYMENT_COMPLETED_BROADCASTER_ASSETID']);
					if (!empty($broadcaster)) {

						//Put all returned variables in $_SESSION['SQ_ECOM_RESPONSE'] to $_POST array so that the listening trigger can use them
						$response = array_get_index($_SESSION, 'SQ_ECOM_RESPONSE', FALSE);
						if ($response) {
							$_POST['transaction_reference'] = array_get_index($response, 'REFERENCE', '');
							$_POST['transaction_time'] = array_get_index($response, 'TIME', '');
							$_POST['transaction_status'] = array_get_index($response, 'STATUS', '');
							$_POST['transaction_card_number'] = array_get_index($response, 'CARDNO', '');
							$_POST['transaction_amount'] = array_get_index($response, 'AMOUNT', 0) + array_get_index($response, 'GST', 0);
							$_POST['transaction_invoice'] = array_get_index($response, 'INVOICE_NUMBER', '');

						}
						$GLOBALS['SQ_SYSTEM']->broadcastTriggerEvent('trigger_event_ecommerce_payment_completed', $broadcaster);
					}
				}

				$this->appendPendingOrderAuditLog(translate('Payment is successful'));


				$this->returnToCaller('SUCCESS');

			} else {
				$this->appendPendingOrderAuditLog(translate('Payment declined by gateway'));

			}
		}

		if (empty($this->_processor)) {
			$this->_loadProcessor();
		}

		if ($this->_processor) {

            // Get the transaction handle before getting the payment details
            $transaction_handle = $this->_processor->getTransactionHandle();

            if (!$transaction_handle) {
                $this->_setError('Cannot obtain the transaction handle from the payment gateway. Please submit the payment form again or contact the administrator');

            }

            // Save the handler in the session
            $_SESSION['UTRANSACT_TRANSACTION_HANDLE'] = $transaction_handle;

        } else {
            $this->_setError("Cannot load payment processor. Please contact the administrator.");
        }

		$this->printBodycopy();


	}//end printBody()


	/**
	* Prints the front-end interface for this asset
	* @return void
	* @access public
	*/
	function printBodycopy()
	{
        $bodycopy = $this->getBodycopy();
		if (is_null($bodycopy)) return;

		$keywords = $bodycopy->getKeywords();
		$replacements = Array();
		foreach ($keywords as $word) {
			$replacements[$word] = $this->getKeywordReplacement($word);
		}
		$bodycopy->setKeywordReplacements($replacements);

		// Load the JS 'Loader' object to print the overlay when payment form is submitted
		$this->includeLoaderJsObject($this->getPrefix().'_card');
		?>
		<form method="post" action="<?php echo $this->getURL(); ?>" onsubmit="Loader.show();">
		<?php
		$bodycopy->printBody();
		echo '</form>';

	}//end printBodycopy()


	/**
	* Process credit card details to UTransact gateway

	* @param array	$card_info	Submitted credit card information
	*
	* @return boolean
	* @access public
	*/
	function processPayment($card_info)
	{
		if (!$card_info) return FALSE;

		$amount = $_SESSION['SQ_ECOM_AMOUNT'];
		if (empty($amount)) {
			$this->_setError(translate('No amount is specified for the payment'));

			return FALSE;
		}

        if (!$this->attr('username')) {
        	$this->_setError(translate("utransact_account_username_not_specified"));
        }
        if (!$this->attr('password')) {
        	$this->_setError(translate("utransact_account_password_not_specified"));
        }
		if (!$this->attr('bgas_webservice_url')) {
        	$this->_setError(translate("url_to_bgas_webservice_not_specified"));
        }
		if (!$this->attr('transactions_webservice_url')) {
        	$this->_setError(translate("url_to_transactions_webservice_not_specified"));
        }
		if (!$this->attr('services_webservice_url')) {
        	$this->_setError(translate("url_to_services_webservice_not_specified"));
      	}

        // If UTransact account details are not complete then we cannot proceed further
       	if ($this->isError()) {
        	return FALSE;
        }

		if (empty($this->_processor)) {
			$this->_loadProcessor();

			if (empty($this->_processor)) {
            	$this->_setError("Cannot load processor");
            	return FALSE;
			}
		}

        // Set unique reference number for this transaction
		// NOTE:
		// Merchand reference should be generated here instead of default_delivery_method.inc or elsewhere because
		// UTransact gateway requires a unqiue transaction reference id for each transaction attempt
		// For example, user can make multiple payment submission attempt untill card details finallygets accepted,
		// and not sending unique reference for each attempt will casue "Duplicate transaction reference" exception error
        $reference_no = md5(session_id().time());
        $this->_processor->setTransactionReferenceNo($reference_no);

       	$_SESSION['SQ_ECOM_REF_NO'] = $this->_processor->getTransactionReferenceNo();

		$pending_order_id = array_get_index($_SESSION['sq_local_cart_contents'], 'pending_order_id', NULL);
		if (!$pending_order_id) {
        	$this->_setError("Order asset not found in session.");
            return FALSE;
		}
		$pending_order = $GLOBALS['SQ_SYSTEM']->am->getAsset($pending_order_id);
		$pending_order->setAttrValue('ecom_ref_no', $_SESSION['SQ_ECOM_REF_NO']);
		$pending_order->appendAuditLog(translate('Order reference number recorded before processing'));


        // Set credit card number
		if (!empty($card_info['number'])) {
			$card_info['number'] = str_replace(Array(' ', "\t"), '', $card_info['number']);
            $this->_processor->setCreditCard($card_info['number']);
		} else {
            $this->_setError(translate("utransact_card_number_not_specified"));
        }

        // Set credit card holder's name
		if (!empty($card_info['name'])) {
			$this->_processor->setCardHolderName($card_info['name']);
		} else {
            $this->_setError(translate("utransact_card_name_not_specified"));
        }

        // Set credit card expiry month and year
        if ($this->_isValidMonth(array_get_index($card_info, 'month', '')) && $this->_isValidYear(array_get_index($card_info, 'year', ''))) {
            $this->_processor->setCardExpiryMonth($card_info['month']);
            $this->_processor->setCardExpiryYear($card_info['year']);
        } else {
            $this->_setError(translate("utransact_expiry_date_invalid"));
        }

   		// Set credit card CV2, if required
		if($this->attr('display_ccv')){
			if (!empty($card_info['ccv']) && $this->_isValidCV2($card_info['ccv'])) {
				$this->_processor->setCreditCardCV2($card_info['ccv']);
			} else {
				$this->_setError(translate("utransact_card_cv2_invalid"));
			}
		}

		// Set merchant id, if specified
        if ($this->attr('merchant_id') != '') {
            $this->_processor->setMerchantId($this->attr('merchant_id'));
        }

        // Set payment model, if specified
        if ($this->attr('payment_model') != '') {
            $this->_processor->setPaymentModel($this->attr('payment_model'));
        }

        // Get the purchaser info
        $purchaser_info = $this->_getPurchaserInfo();

        $this->_processor->setFirstName(isset($purchaser_info['firstname']) ? $purchaser_info['firstname'] : '');
        $this->_processor->setSurname(isset($purchaser_info['surname']) ? $purchaser_info['surname'] : '');
        $this->_processor->setEmail(isset($purchaser_info['email']) ? $purchaser_info['email'] : '');
        $this->_processor->setAddressLine1(isset($purchaser_info['address1']) ? $purchaser_info['address1'] : '');
        $this->_processor->setAddressLine2(isset($purchaser_info['address2']) ? $purchaser_info['address2'] : '');
        $this->_processor->setSuburb(isset($purchaser_info['suburb']) ? $purchaser_info['suburb'] : '');
        $this->_processor->setCity(isset($purchaser_info['city']) ? $purchaser_info['city'] : '');
        $this->_processor->setPostcode(isset($purchaser_info['postcode']) ? $purchaser_info['postcode'] : '');


        // Populate the line items from the cart in the session
        $session_cart = isset($_SESSION['sq_local_cart_contents']['cart_contents']) ? $_SESSION['sq_local_cart_contents']['cart_contents'] : Array();

        if (empty($session_cart)) {
            $this->_setError(translate("utransact_shopping_cart_empty"));
        }

        foreach($session_cart as $product_assetid => $item) {
            $item['additional_info'] = "IS DONATION: ".($item['is_donation'] ? 'YES' : 'NO');

            $item["application_username"] = isset($item["application_username"]) ? $item["application_username"] : '';
            $item["description"] = isset($item["description"]) ? $item["description"] : '';
            $item["financial_values"] = isset($item["financial_values"]) ? $item["financial_values"] : '';

			// If product refund type is set to 'Expiry', then set the expiry date to one week from now
			$refund_expiry_date = ($item["refund_type"] === 'Expires') ? date("Y-m-d\TH:i:s", strtotime("+7 days")) : '';

			$this->_processor->addCartItem(
					$item["application_username"],
					$item["name"],
					$item["description"],
					$item["quantity"],
					$item["refund_type"],
					$refund_expiry_date,
					$item["price"],
					$item["product_code"],
					$item["financial_code"],
					$item["financial_values"],
					$item["additional_info"]
				);

        }

		// If theres some error in input, we cannot proceed with this transaction
		if ($this->isError()) {
            return FALSE;
        }

		// Finally process transaction
		$success = $this->_processor->process();

		// Get response
		$response = $this->_processor->getResponse();

		$this->_setTransactionResult($success, $response, $card_info);

		return $success;

	}//end processPayment()


	/**
	 * Set the result of the transaction.
	 * If transaction fails, set its error message. If transaction success, set the Session variable SQ_ECOM_RESPONSE
	 *
	 * @param boolean	$success		the reason of the transaction. It is either 1 or FALSE
	 * @param array		$response		the response array from the Datacash Processor
     * @param array	    $card_info	    submitted credit card information
	 *
	 * @return void
	 * @access private
	 */
	private function _setTransactionResult($success, $response, $card_info)
	{
		if (!$success) {
			$this->_tmp['is_error']  = TRUE;
			$this->_tmp['error_message']  = Array($response['MESSAGE']);
		} else {
         	$status = $response['STATUS'];

			// preparing card number; should contain only 4 last digits
			$cardno = '';
			$cardlength = strlen($card_info['number']);
			if ($cardlength < 16) {
				for ($i = 0; $i < $cardlength - 4; $i++) {
					$cardno .= '*';
				}
				$cardno .= substr($card_info['number'], $i);
			} else {
				$cardno = '****-****-****-'.substr($card_info['number'],12);
			}

			$response['CARDNO'] = $cardno;
			$response['TIME'] = array_get_index($response, 'DATE', date('r'));
			$response['TRANSACTION'] = array_get_index($response, 'INVOICE_NUMBER', '');

			$_SESSION['SQ_ECOM_RESPONSE']   = $response;
		}

	}//end _setTransactionResult()


	/**
	 * Map the checkout form quesionids and get the purchaser info from the session
	 *
	 * @return array
	 * @access private
	 */
	private function _getPurchaserInfo()
	{
		$purchaser_data = isset($_SESSION['order_extra_information']) ? $_SESSION['order_extra_information'] : '';
		if (empty($purchaser_data)) {
			$this->_setError(translate("utransact_purchaser_info_not_in_session"));
			return FALSE;
		}

		$purchaser_data = unserialize($purchaser_data);

		// Fields in the checkout form ('1' means required and '0' means optional)
        $checkout_fields = Array(
                        'firstname'	=> 1,
                        'surname' 	=> 1,
                        'email'		=> 0,
                        'address1'	=> 1,
                        'address2'	=> 0,
                        'suburb'	=> 0,
                        'city'		=> 1,
                        'postcode'	=> 0,
			);

		$purchaser_info = Array();
		foreach($checkout_fields as $field => $required) {
			$form_qid = trim($this->attr('map_'.$field.'_assetid'));

			// Extract the shadow part of the form question assetid
			if (!preg_match('/^[0-9]+?:(.*)$/',$form_qid, $match) && $required) {
				$this->_setError(translate("utransact_form_question_id_not_set", ucfirst($field)));
				return FALSE;
			}

			$shadow_qid = isset($match[1]) ? trim($match[1]) : 0;
			$purchaser_info[$field] = isset($purchaser_data[$shadow_qid]) ? $purchaser_data[$shadow_qid] : '';

			if ($required && !$purchaser_info[$field]) {
				$this->_setError(translate("utransact_form_question_id_not_set", ucfirst($field)));
				return FALSE;
			}

		}

		return $purchaser_info;

	}//end _getPurchaserInfo()


	/**
	 * Check if the credit card expiry month is valid
	 *
	 * @param string $month		the month of the input date

	 * @return boolean			return TRUE if the date is valid; otherwise, return FALSE
	 * @access private
	 */
	private function _isValidMonth($month)
	{
		$month_of_year = Array('JAN', 'FEB', 'MAR', 'APR', 'MAY', 'JUN', 'JUL', 'AUG', 'SEP', 'OCT', 'NOV', 'DEC');
		return (in_array(strtoupper($month), $month_of_year));

	}//end _isValidMonth()


	/**
	 * Check if the credit card expiry year is valid
	 *
	 * @param string $month		the year of the input date

	 * @return boolean			return TRUE if the date is valid; otherwise, return FALSE
	 * @access private
	 */
	private function _isValidYear($year)
	{
		return preg_match('/^\d{4}$/', $year);

	}//end _isValidYear()


	/**
	 * Check if the CV2 is valid
	 *
	 * @param string $cv2		the input CV2 to test
	 *
	 * @return boolean
	 * @access private
	 */
	private function _isValidCV2($cv2)
	{
		return preg_match('/^\d{3,4}$/', $cv2);

	}//end _isValidCV2()


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
			unset($_SESSION['SQ_ECOM_SUCCESS_URL']);

		} else if ($state == 'CANCEL') {
			$back_url = array_get_index($_SESSION, 'SQ_ECOM_CANCEL_URL');
			unset($_SESSION['SQ_ECOM_CANCEL_URL']);
		}

		if (is_null($back_url)) {
			trigger_error('Unknown caller reference');
		} else {
			header('Location: '.$back_url);
			exit;
		}

	}//end returnToCaller()


	/**
	 * Get transaction GST from the payment gateway
	 *
	 * @parm boolean    $from_gateway    If true get the GST from gateway
	 *
	 * @return string|boolean
	 * @access public
	 */
	public function getTransactionGstRate($from_gateway=FALSE)
	{
		if (!$from_gateway && isset($_SESSION['SQ_ECOM_GST'])) {
            return $_SESSION['SQ_ECOM_GST'];
        }

		if (empty($this->_processor)) {
			$this->_loadProcessor();

            if (empty($this->_processor)) {
                $this->_setError("Cannot load processor");
                return FALSE;
            }
        }

        // Get the gst tax rate value from the payment gateway and put it into the session
        $_SESSION['SQ_ECOM_GST'] = $this->_processor->getTransactionGstRate();

        return $_SESSION['SQ_ECOM_GST'];
	}


	/**
	* Credit Card Number keyword replacement
	*
	* @return string
	* @access private
	*/
	function getCreditCardNumberKeywordReplacement()
	{

		$prefix = $this->getPrefix();
		$card = $prefix.'_card';

		$card_number = '<input name="'.$card.'[number]" autocomplete="off" />';

		return $card_number;

	}//end getCreditCardNumberKeywordReplacement()


	/**
	* Credit Card Number keyword replacement
	*
	* @return string
	* @access private
	*/
	function getCreditCardNameKeywordReplacement()
	{
		$card = $this->getPrefix().'_card';
		$string = '<input name="'.$card.'[name]" autocomplete="off" />';
		return $string;

	}//end getCreditCardNameKeywordReplacement()


	/**
	* Credit Card Number keyword replacement
	*
	* @return string
	* @access private
	*/
	function getCreditCardCcvKeywordReplacement()
	{
		if (!$this->attr('display_ccv')) return '';

		$card = $this->getPrefix().'_card';
		$string = 'CV2 <input name="'.$card.'[ccv]" size="4" autocomplete="off" />';
		return $string;

	}//end getCreditCardCcvKeywordReplacement()


	/**
	* Credit Card Number keyword replacement
	*
	* @return string
	* @access private
	*/
	function getCreditCardTypeSelectorKeywordReplacement()
	{
		if (empty($this->_processor)) {
			$this->_loadProcessor();

			if (empty($this->_processor)) {
            	$this->_setError("Cannot load processor");
            	return FALSE;
			}
		}

		$card = $this->getPrefix().'_card';

		$card_types = $this->_processor->getCardTypes();

		ob_start();
            echo '<select name="'.$card.'[type]">';
            foreach ($card_types as $type) {
                echo '<option value="'.$type.'">'.$type.'</option>';
            }

            echo '</select>';
		return ob_get_clean();

	}//end getCreditCardTypeSelectorKeywordReplacement()


	/**
	* Transaction Amount (without GST) keyword replacement
	*
	* @return string
	* @access private
	*/
	function getTransactionAmountKeywordReplacement()
	{
		// Total amount from session shouldn't include tax
		// GST is calculated and added by gateway when processing the transaction

		return array_get_index($_SESSION,'SQ_ECOM_AMOUNT', 0);

	}//end getTransactionAmountKeywordReplacement()


	/**
	* Transaction GST keyword replacement
	*
	* @return string
	* @access private
	*/
	function getTransactionGSTKeywordReplacement()
	{
		$gst_rate = $this->getTransactionGstRate();
		return $gst_rate ? ($gst_rate*100) : 'GST NOT SET!';

	}//end getTransactionGSTKeywordReplacement()


	/**
	* Credit Card Number keyword replacement
	*
	* @return string
	* @access private
	*/
	function getCreditCardExpiryYearKeywordReplacement()
	{

		$card = $this->getPrefix().'_card';

		$string = '<input name="'.$card.'[year]" size="2" autocomplete="off" />';
		return $string;

	}//end getCreditCardExpiryYearKeywordReplacement()


	/**
	* Credit Card Number keyword replacement
	*
	* @return string
	* @access private
	*/
	function getCreditCardExpiryMonthKeywordReplacement()
	{

		$card = $this->getPrefix().'_card';

		$string = '<input name="'.$card.'[month]" size="2" autocomplete="off" />';
		return $string;

	}//end getCreditCardExpiryMonthKeywordReplacement()


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
	* Credit Card Number keyword replacement
	*
	* @return string
	* @access private
	*/
	function getSubmitButtonKeywordReplacement()
	{
		$button_text = $this->attr('submit_text');
		if (empty($button_text)) $button_text = 'Submit';
		return '<input type="submit" value="'.$button_text.'" />';

	}//end getSubmitButtonKeywordReplacement()


	/**
	* Credit Card Number keyword replacement
	*
	* @return string
	* @access private
	*/
	function getResetButtonKeywordReplacement()
	{
		$button_text = $this->attr('reset_text');
		if (empty($button_text)) $button_text = 'Reset';
		return '<input type="reset" value="'.$button_text.'" />';

	}//end getResetButtonKeywordReplacement()


	/**
	* Credit Card Number keyword replacement
	*
	* @return string
	* @access private
	*/
	function getCancelButtonKeywordReplacement()
	{
		$button_text = $this->attr('cancel_text');
		if (empty($button_text)) $button_text = 'Cancel';
		return '<input type="submit" name="'.$this->getPrefix().'_action" value="'.$button_text.'" />';

	}//end getCancelButtonKeywordReplacement()


	/**
	* Returns a new transaction handle
	*
	* @return string
	* @access public
	*/
	function getNewTransactionHandle()
	{
		if (empty($this->_processor)) {
			$this->_loadProcessor();
		}

		if ($this->_processor) {
            return $this->_processor->getTransactionHandle();
        } else {
            return FALSE;
        }

	}//end getNewTransactionHandle()

}//end class

?>
