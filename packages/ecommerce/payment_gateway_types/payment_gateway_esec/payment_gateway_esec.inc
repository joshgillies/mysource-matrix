<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: payment_gateway_esec.inc,v 1.3.2.1 2004/10/18 22:50:21 arailean Exp $
* $Name: not supported by cvs2svn $
*/


require_once dirname(__FILE__).'/../../payment_gateway/payment_gateway.inc';
require_once dirname(__FILE__).'/esec_processor.inc';

/**
* Payment_Gateway_Esec
*
* Interface for online credit card processing using E-sec payment gateway
*
*
* @author  Andrei Railean		<arailean@squiz.net>
* @author  Dmitry Baranovskiy	<dbaranovskiy@squiz.net>
* @version $Revision: 1.3.2.1 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Payment_Gateway_Esec extends Payment_Gateway
{


	/**
	* basic output function. root of output process
	*
	* @access public
	* @return void
	*/
	function printBody()
	{

		$amount      = array_get_index($_SESSION,'SQ_ECOM_AMOUNT', 0);

		$card_info = array_get_index($_REQUEST, $this->getPrefix().'_card', false);
		$action    = array_get_index($_REQUEST, $this->getPrefix().'_action', false);
		if ($action == 'Cancel' || empty($amount)) {
			$this->goBackToCaller();
		}

		if ($card_info) {
			$_SESSION['SQ_ECOM_CARDINFO'] = $card_info;
			if ($this->processPayment()) {
				unset($_SESSION['SQ_ECOM_AMOUNT']);
				$this->goBackToCaller();
			}
		}

		echo $this->makeCreditCardForm();

	}//end printBody()


	/**
	* makes form for entering credit card details
	*
	* @access public
	* @return string
	*/
	function makeCreditCardForm()
	{
		$amount = array_get_index($_SESSION,'SQ_ECOM_AMOUNT', 0);

		$Esec = new Esec_Processor();
		$card_types = $Esec->getCardTypes();

		// format the amount
		$Esec->setAmount($amount);
		$amount = $Esec->getAmount();

		$card_info = array_get_index($_SESSION, 'SQ_ECOM_CARDINFO', false);
		if (!$card_info) {
			$card_info = Array(
					'name' => '',
					'number' => '',
					'month' => '',
					'year' => '',
					'type' => '',
			);
		}

		$prefix = $this->getPrefix();
		$card = $prefix.'_card';
		$form_contents = '';

		// determine which card type is selected
		$form_contents_select = '';
		foreach ($card_types as $type) {
			$selected = ($card_info['type'] == $type)?' selected="selected"':'';
			$form_contents_select .= '<option value="'.$type.'"'.$selected.'>'.$type.'</option>';
		}

		// set variables for test mode
		if ($this->attr('test_mode')){

			$form_contents .= '<h2 style="color: white; background-color: grey; padding: 5px">TEST MODE</h2>';

			$test_card_numbers = $Esec->getTestCardNumbers();
			$card_number = '<select name="'.$card.'[number]">';
			foreach ($test_card_numbers as $number) {
				$selected = ($card_info['number'] == $number)?' selected="selected"':'';
				$card_number .= '<option value="'.$number.'"'.$selected.'>'.$number.'</option>';
			}
			$card_number .= '</select>';
		} else {
			$card_number = '<input name="'.$card.'[number]" value="'.$card_info['number'].'" />';
		}

		// check for errors
		if (array_get_index($this->_tmp, 'is_error', false)) {
			$form_contents .= '<h2 style="color: red; ">Error '.$this->_tmp['error_message'].'</h2>';
		}

		$message = $this->attr('message');

		$datapath = sq_web_path('data').'/asset_types/payment_gateway_esec/files';

		$form_contents .= <<<FORMBODY
		<script src="{$datapath}/loader.js"></script>
		<script>
			var Loader = new Loader('{$card}','#FFFFFF','Processing Transaction...','{$datapath}/loader.gif');
			Loader.print();
		</script>
		<form method="post" action="" onsubmit="Loader.show();">
			<div>$message</div>
			<div style="background-color: #CCC; padding: 5px"><strong>\$$amount</strong> Australian Dollars are about to be debited from your credit card</div>
			<table>
				<tr>
					<td>Name on Card</td>
					<td><input name="{$card}[name]" value="{$card_info['name']}" /></td>
				</tr>
				<tr>
					<td>Number</td>
					<td>$card_number</td>
				</tr>
				<tr>
					<td>Expiry Date (MM/YY)</td>
					<td><input name="{$card}[month]" value="{$card_info['month']}" size="2" /> / <input name="{$card}[year]" value="{$card_info['year']}" size="2" /></td>
				</tr>
				<tr>
					<td>Type</td>
					<td>
						<select name="{$card}[type]">
							$form_contents_select
						</select>
					</td>
				</tr>
				<tr>
					<td>&nbsp;</td>
					<td><input type="submit" value="                  Pay                  " /><input type="reset" value="Reset" /><input type="submit" name="{$prefix}_action" value="Cancel" /></td>
				</tr>
			</table>
		</form>
FORMBODY;

		return $form_contents;

	}//end makeCreditCardForm()


	/**
	* process credit card details to gateway
	*
	* @access public
	* @return boolean
	*/
	function processPayment()
	{
		$ref_no      = $_SESSION['SQ_ECOM_REF_NO'];
		$amount      = $_SESSION['SQ_ECOM_AMOUNT'];
		$card_info   = $_SESSION['SQ_ECOM_CARDINFO'];

		$Esec = new Esec_Processor();

		$Esec->setAmount($amount);
		$Esec->setReference($ref_no);
		$Esec->setMerchant($this->attr('merchant_id'));
		$Esec->setCardNumber($card_info['number']);
		$Esec->setCardType($card_info['type']);
		$Esec->setCardExpiryMonth($card_info['month']);
		$Esec->setCardExpiryYear($card_info['year']);
		$Esec->setCardName($card_info['name']);

		if ($this->attr('test_mode')){
			$Esec->setTest();
		}

		$success = $Esec->process();

		// process response
		$response = $Esec->getResponse();
		$_SESSION['SQ_ECOM_REF_NO']   = $response['REFERENCE'];

		if (!$success) {
			$this->_tmp['is_error']         = true;
			$this->_tmp['error_message']    = $response['MESSAGE'];
		} else {
			$status = $Esec->explainStatus();
			if ($this->attr('test_mode')){
				$status = '*** TEST MODE (do not honour) *** : '.$status;
			}

			$response['STATUS'] = $status;
			$_SESSION['SQ_ECOM_RESPONSE']   = $response;
		}

		return $success;

	}//end processPayment()


	/**
	* redirects back to e-commerce form that called gateway
	*
	* @access public
	* @return void
	*/
	function goBackToCaller()
	{
		$back_url = array_get_index($_SESSION,'SQ_ECOM_BACK_URL', 'error');
		echo('<script type="text/javascript">document.location.href = "'.$back_url.'";</script>');
		exit;

	}//end goBackToCaller()

}//end class

?>
