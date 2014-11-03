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
* $Id: payment_gateway_esec.inc,v 1.27 2012/11/16 05:01:54 cupreti Exp $
*
*/

require_once dirname(__FILE__).'/../../payment_gateway/payment_gateway.inc';
require_once dirname(__FILE__).'/esec_processor_integration_interface.inc';

/**
* Payment_Gateway_Esec
*
* Interface for online credit card processing using E-sec payment gateway
*
*
* @author  Andrei Railean		<arailean@squiz.net>
* @author  Dmitry Baranovskiy	<dbaranovskiy@squiz.net>
* @version $Revision: 1.27 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Payment_Gateway_Esec extends Payment_Gateway
{


	/**
	* Esec integration interface instance
	*
	* @var object
	*/
	var $esec = NULL;


	function __construct($assetid=0)
	{
		$this->_esec = new Esec_Processor_Integration_Interface();
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
<div style="background-color: #CCC; padding: 5px"><strong>$%transaction_amount%</strong> Australian Dollars are about to be debited from your credit card</div>
<table cellpadding="5">
	<tr>
		<td align="right">Name on Card</td>
		<td>%credit_card_name%</td>
	</tr>
	<tr>
		<td align="right">Number</td>
		<td>%credit_card_number%</td>
	</tr>
	<tr>
		<td  align="right">CCV</td>
		<td>%credit_card_ccv% - from the back of your credit card</td>
	</tr>
	<tr>
		<td align="right">Expiry Date (MM/YY)</td>
		<td>%credit_card_expiry_month% / %credit_card_expiry_year%</td>
	</tr>
	<tr>
		<td align="right">Type</td>
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
		$keywords['credit_card_type_selector'] = 'Credit Card Type Selector';
		$keywords['submit_button'] = 'Submit Button';
		$keywords['cancel_button'] = 'Cancel Button';
		$keywords['reset_button'] = 'Reset Button';
		$keywords['transaction_amount'] = 'Transaction Amount';
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
		$amount    = array_get_index($_SESSION,'SQ_ECOM_AMOUNT', 0);

		$card_info = array_get_index($_REQUEST, $this->getPrefix().'_card', FALSE);
		$action    = array_get_index($_REQUEST, $this->getPrefix().'_action', FALSE);
		if ($action == 'Cancel') {
			$this->returnToCaller('CANCEL');

		} else if (empty($amount)) {
			unset($_SESSION['SQ_ECOM_AMOUNT']);
			$this->appendPendingOrderAuditLog(translate('Payment has no amount'));

			$this->returnToCaller('SUCCESS');

		} else if ($card_info) {
			$this->appendPendingOrderAuditLog(translate('Payment Attempted (gateway contacted)'));


			if ($this->processPayment($card_info)) {
				unset($_SESSION['SQ_ECOM_AMOUNT']);
				$this->appendPendingOrderAuditLog(translate('Payment is successful'));

				$this->returnToCaller('SUCCESS');
			} else {
				$this->appendPendingOrderAuditLog(translate('Payment Declined'));

			}
		}

		$this->printBodycopy();


	}//end printBody()


	/**
	* Prints the front-end interface for this asset
	*
	* @return void
	* @access public
	*/
	function printBodycopy()
	{
		$bodycopy =& $this->getBodycopy();
		if (is_null($bodycopy)) return;

		$keywords = $bodycopy->getKeywords();
		$replacements = Array();
		foreach ($keywords as $word) {
			$replacements[$word] = $this->getKeywordReplacement($word);
		}
		$bodycopy->setKeywordReplacements($replacements);

		// OUTPUT HERE
		$datapath = sq_web_path('data').'/asset_types/payment_gateway_esec/files';
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

	}//end printBodycopy()


	/**
	* Return the bodycopy linked to this page
	*
	* @return object
	* @access public
	*/
	function &getBodycopy()
	{
		$null = NULL;
		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, '', 'major');
		if (empty($link)) return $null;
		$bodycopy = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
		return $bodycopy;

	}//end getBodycopy()


	/**
	* Makes form for entering credit card details
	*
	* @return string
	* @access public
	*/
	function makeCreditCardForm()
	{
		$amount = array_get_index($_SESSION,'SQ_ECOM_AMOUNT', 0);

		$Esec = new Esec_Processor_Integration_Interface();
		$card_types = $Esec->getCardTypes();

		// format the amount
		$Esec->setAmount($amount);
		$amount = $Esec->getAmount();

		$prefix = $this->getPrefix();
		$card = $prefix.'_card';
		$form_contents = '';

		// determine which card type is selected
		$form_contents_select = '';
		foreach ($card_types as $type) {
			$form_contents_select .= '<option value="'.$type.'>'.$type.'</option>';
		}

		// set variables for test mode
		if ($this->attr('test_mode')) {

			$form_contents .= '<h2 style="color: white; background-color: grey; padding: 5px">TEST MODE</h2>';

			$test_card_numbers = $Esec->getTestCardNumbers();
			$card_number = '<select name="'.$card.'[number]">';
			foreach ($test_card_numbers as $number => $label) {
				$card_number .= '<option value="'.$number.'>'.$number.'</option>';
			}
			$card_number .= '</select>';
		} else {
			$card_number = '<input name="'.$card.'[number]" autocomplete="off" />';
		}

		// check for errors
		if (array_get_index($this->_tmp, 'is_error', FALSE)) {
			$form_contents .= '<h2 style="color: red; ">Error '.htmlspecialchars($this->_tmp['error_message'], ENT_NOQUOTES, SQ_CONF_DEFAULT_CHARACTER_SET).'</h2>';
		}

		$message = $this->attr('message');

		$datapath = sq_web_path('data').'/asset_types/payment_gateway_esec/files';

		$own_url = $this->getURL();

		ob_start();
		?>
		<script src="<?php echo $datapath; ?>/loader.js"></script>
		<script>
			var Loader = new Loader('<?php echo $card; ?>','#FFFFFF','Processing Transaction...','<?php echo $datapath; ?>/loader.gif');
			Loader.print();
		</script>
		<form method="post" action="<?php echo $own_url; ?>" onsubmit="Loader.show();">
			<div><?php echo $message; ?></div>
			<div style="background-color: #CCC; padding: 5px"><strong>$<?php echo $amount; ?></strong> Australian Dollars are about to be debited from your credit card</div>
			<table cellpadding="5">
				<tr>
					<td align="right">Name on Card</td>
					<td><input name="<?php echo $card; ?>[name]" autocomplete="off" /></td>
				</tr>
				<tr>
					<td align="right">Number</td>
					<td><?php echo $card_number; ?></td>
				</tr>
				<?php
				if ($this->attr('display_ccv')) {
				?>
				<tr>
					<td  align="right">CCV</td>
					<td><input name="<?php echo $card; ?>[ccv]" size='4' autocomplete="off" /> - from the back of your credit card</td>
				</tr>
				<?php
				}
				?>
				<tr>
					<td align="right">Expiry Date (MM/YY)</td>
					<td><input name="<?php echo $card; ?>[month]" size="2" autocomplete="off" /> / <input name="<?php echo $card; ?>[year]" size="2" autocomplete="off" /></td>
				</tr>
				<tr>
					<td align="right">Type</td>
					<td>
						<select name="<?php echo $card; ?>[type]">
							<?php echo $form_contents_select; ?>
						</select>
					</td>
				</tr>
				<tr>
					<td>&nbsp;</td>
					<td><input type="submit" value="                  Pay                  " /><input type="reset" value="Reset" /><input type="submit" name="<?php echo $prefix; ?>_action" value="Cancel" /></td>
				</tr>
			</table>
		</form>
		<?php
		$form_contents .= ob_get_clean();


		return $form_contents;

	}//end makeCreditCardForm()


	/**
	* Process credit card details to gateway
	*
	* @param array	$card_info	Submitted credit card information
	*
	* @return boolean
	* @access public
	*/
	function processPayment($card_info)
	{
		if (!$card_info) return FALSE;

		$ref_no      = $_SESSION['SQ_ECOM_REF_NO'];
		$amount      = $_SESSION['SQ_ECOM_AMOUNT'];

		$Esec = new Esec_Processor_Integration_Interface();

		$Esec->setAmount($amount);
		$Esec->setReference($ref_no);
		$Esec->setMerchant($this->attr('merchant_id'));
		$Esec->setCardNumber($card_info['number']);
		$Esec->setCCV(array_get_index($card_info, 'ccv', ''));
		$Esec->setCardType($card_info['type']);
		$Esec->setCardExpiryMonth($card_info['month']);
		$Esec->setCardExpiryYear($card_info['year']);
		$Esec->setCardName($card_info['name']);

		if ($this->attr('test_mode')) $Esec->setTest();

		$success = $Esec->process();

		// process response
		$response = $Esec->getResponse();
		if (!empty($response['REFERENCE'])) {
			$_SESSION['SQ_ECOM_REF_NO'] = $response['REFERENCE'];
		}

		if (!$success) {
			$this->_tmp['is_error']         = TRUE;
			$this->_tmp['error_message']    = $response['MESSAGE_CODE'].' '.$response['MESSAGE'];
		} else {
			$status = $Esec->explainStatus();
			if ($this->attr('test_mode')) {
				$status = '!!!ATTENTION!!! TEST MODE (transaction not performed) -- '.$status;
			}

			$response['STATUS'] = $status;
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
			$response['TIME'] = date('r');
			$response['AMOUNT'] = $amount;

			$_SESSION['SQ_ECOM_RESPONSE']   = $response;
		}

		return $success;

	}//end processPayment()


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
	* Credit Card Number keyword replacement
	*
	* @return string
	* @access private
	*/
	function getCreditCardNumberKeywordReplacement()
	{

		$prefix = $this->getPrefix();
		$card = $prefix.'_card';

		// set variables for test mode
		if ($this->attr('test_mode')) {
			$test_card_numbers = $this->_esec->getTestCardNumbers();
			$card_number = '<select name="'.$card.'[number]">';
			foreach ($test_card_numbers as $number => $label) {
				$card_number .= '<option value="'.$number.'" >'.$number.'</option>';
			}
			$card_number .= '</select>';
		} else {
			$card_number = '<input name="'.$card.'[number]" autocomplete="off" />';
		}

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

		$card = $this->getPrefix().'_card';
		$string = '<input name="'.$card.'[ccv]" size="4" autocomplete="off" />';
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

		$card = $this->getPrefix().'_card';

		$card_types = $this->_esec->getCardTypes();

		ob_start();
		echo '<select name="'.$card.'[type]">';
		foreach ($card_types as $type) {
			echo '<option value="'.$type.'">'.$type.'</option>';
		}

		echo '</select>';

		return ob_get_clean();

	}//end getCreditCardTypeSelectorKeywordReplacement()


	/**
	* Transaction Amount keyword replacement
	*
	* @return string
	* @access private
	*/
	function getTransactionAmountKeywordReplacement()
	{
		$amount = array_get_index($_SESSION,'SQ_ECOM_AMOUNT', 0);
		$this->_esec->setAmount($amount);
		$amount = $this->_esec->getAmount();

		return $amount;

	}//end getTransactionAmountKeywordReplacement()


	/**
	* Processing Error keyword replacement
	*
	* @return string
	* @access private
	*/
	function getProcessingErrorKeywordReplacement()
	{
		if (array_get_index($this->_tmp, 'is_error', FALSE)) {
			return '<span class="payment-gateway-transaction-error">'.htmlspecialchars($this->_tmp['error_message'], ENT_NOQUOTES, SQ_CONF_DEFAULT_CHARACTER_SET).'</span>';
		}

		return '';

	}//end getProcessingErrorKeywordReplacement()


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


}//end class

?>
