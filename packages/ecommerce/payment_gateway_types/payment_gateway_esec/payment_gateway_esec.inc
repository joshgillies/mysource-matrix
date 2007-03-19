<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +--------------------------------------------------------------------+
*
* $Id: payment_gateway_esec.inc,v 1.16 2007/03/19 05:51:18 arailean Exp $
*
*/

check_ssv_licence();
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
* @version $Revision: 1.16 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Payment_Gateway_Esec extends Payment_Gateway
{


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
			$this->returnToCaller('SUCCESS');

		} else if ($card_info) {
			$_SESSION['SQ_ECOM_CARDINFO'] = $card_info;
			if ($this->processPayment()) {
				unset($_SESSION['SQ_ECOM_AMOUNT']);
				$this->returnToCaller('SUCCESS');
			}

		} else {
			echo $this->makeCreditCardForm();

		}

	}//end printBody()


	/**
	* Makes form for entering credit card details
	*
	* @return string
	* @access public
	*/
	function makeCreditCardForm()
	{
		$amount = array_get_index($_SESSION,'SQ_ECOM_AMOUNT', 0);

		$Esec =& new Esec_Processor_Integration_Interface();
		$card_types = $Esec->getCardTypes();

		// format the amount
		$Esec->setAmount($amount);
		$amount = $Esec->getAmount();

		$card_info = array_get_index($_SESSION, 'SQ_ECOM_CARDINFO', FALSE);
		if (!$card_info) {
			$card_info = Array(
							'name'		=> '',
							'number'	=> '',
							'ccv'		=> '',
							'month'		=> '',
							'year'		=> '',
							'type'		=> '',
						 );
		}

		$prefix = $this->getPrefix();
		$card = $prefix.'_card';
		$form_contents = '';

		// determine which card type is selected
		$form_contents_select = '';
		foreach ($card_types as $type) {
			$selected = ($card_info['type'] == $type) ? ' selected="selected"' : '';
			$form_contents_select .= '<option value="'.$type.'"'.$selected.'>'.$type.'</option>';
		}

		// set variables for test mode
		if ($this->attr('test_mode')) {

			$form_contents .= '<h2 style="color: white; background-color: grey; padding: 5px">TEST MODE</h2>';

			$test_card_numbers = $Esec->getTestCardNumbers();
			$card_number = '<select name="'.$card.'[number]">';
			foreach ($test_card_numbers as $number => $label) {
				$selected = ($card_info['number'] == $number) ? ' selected="selected"' : '' ;
				$card_number .= '<option value="'.$number.'"'.$selected.'>'.$number.'</option>';
			}
			$card_number .= '</select>';
		} else {
			$card_number = '<input name="'.$card.'[number]" value="'.$card_info['number'].'" />';
		}

		// check for errors
		if (array_get_index($this->_tmp, 'is_error', FALSE)) {
			$form_contents .= '<h2 style="color: red; ">Error '.$this->_tmp['error_message'].'</h2>';
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
					<td><input name="<?php echo $card; ?>[name]" value="<?php echo $card_info['name'];?>" /></td>
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
					<td><input name="<?php echo $card; ?>[ccv]" value="<?php echo $card_info['ccv'];?>" size='4' /> - from the back of your credit card</td>
				</tr>
				<?php
				}
				?>
				<tr>
					<td align="right">Expiry Date (MM/YY)</td>
					<td><input name="<?php echo $card; ?>[month]" value="<?php echo $card_info['month'];?>" size="2" /> / <input name="<?php echo $card; ?>[year]" value="<?php echo $card_info['year'];?>" size="2" /></td>
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
	* @return boolean
	* @access public
	*/
	function processPayment()
	{
		$ref_no      = $_SESSION['SQ_ECOM_REF_NO'];
		$amount      = $_SESSION['SQ_ECOM_AMOUNT'];
		$card_info   = $_SESSION['SQ_ECOM_CARDINFO'];

		$Esec =& new Esec_Processor_Integration_Interface();

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
		$_SESSION['SQ_ECOM_REF_NO'] = $response['REFERENCE'];

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


}//end class

?>