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
* $Id: form_ecommerce.inc,v 1.5 2004/10/15 03:30:19 arailean Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_PACKAGES_PATH.'/cms/form/form_email/form_email.inc';

require_once SQ_FUDGE_PATH.'/general/www.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';

/**
* Form_Ecommerce
* Form that can have e-commerce dollar value rules assigned to it and later
* processed using an attached payment gateway. Uses Form-email's emailing
* mechanisms. Relies on session variables for communication with the
* payment gateway asset
*
*
* @author	Andrei Railean		<arailean@squiz.net>
* @author	Dmitry Baranovskiy	<dbaranovskiy@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Form_Ecommerce extends Form_Email
{
	/**
	* confirm view request index ($_REQUEST['view_name'])
	* @var string
	*/
	var $confirm_view = 'confirm';

	/**
	* process view index
	* @var string
	*/
	var $process_view = 'process';

	/**
	* receipt view index
	* @var string
	*/
	var $receipt_view = 'receipt';


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Form_Ecommerce($assetid=0)
	{
		$this->Form_Email($assetid);

	}//end Form_Ecommerce()


	/**
	* Check if question type can be used for e-commerce
	*
	* @param string $type   question type code
	*
	* @access public
	* @return true
	*/
	function _isAllowedQuestionType($type = '')
	{

		switch($type){
			case('form_question_type_numeric'):
			case('form_question_type_select'):
			case('form_question_type_option_list'):
			case('form_question_type_tickbox_list'):
			case('form_question_type_tickbox_table'):
				return true;

			default:
				return false;

		}

	}//end _isAllowedQuestionType()


	/**
	* Return question rule for e-commerce form
	*
	* @param string $name   question type code
	*
	* @access public
	* @return boolean
	*/
	function _getEcommerceRule($name = '')
	{

		// get e-commerce rules
		$e_questions = $this->attr('rules');
		$result = array_get_index($e_questions, $name, false);
		return $result;

	}//end _isEcommerceRule()


	/**
	* Save one rule
	*
	* @param string $name   rule name - question id
	* @param mixed $value   rule contents
	*
	* @access private
	* @return boolean
	*/
	function _setEcommerceRule($name, $value)
	{
		$rules = $this->attr('rules');

		if (!isset($rules[$name]))
			return false;

		$rules[$name] = $value;
		$this->setAttrValue('rules', $rules);

		return true;

	}//end _setEcommerceRule()


	/**
	* Main controller for this asset
	*
	* This function determines what action to take next, based on the various
	* request parameters and statuses of various actions
	*
	* @access public
	* @return boolean
	*/
	function printStandard()
	{
		$prefix = $this->getPrefix();

		// check to see if the form has been submitted or not
		if (isset($_REQUEST[$prefix.'_submit'])) {
			$action = array_get_index($_REQUEST, $prefix.'_action', $this->confirm_view);

			// determine which view to display
			switch ($action) {

				// receipt view: after the gateway has processed payment and redirected back to us
				// here we need to display the receipt and do the logging, etc
				case ($this->receipt_view):
					// check for response from the payment gateway
					$response = array_get_index($_SESSION, 'SQ_ECOM_RESPONSE', false);
					if (!$response) {
						// no response: we've either been here already - user has seen the receipt
						// or someone has used an invalid url
						break;
					}

					unset($_SESSION['SQ_ECOM_RESPONSE']);
					$this->_tmp['GATEWAY_RESPONSE'] = $response;

					// this function is reponsible for doing business logic related to finalizing the
					// e-commerce transaction: e-mailing, logging and display
					$this->_processReceipt();

					$this->finishForm();
					return true;

				break;

				// confirm view: after the form has been filled out it needs
				// to be processed and confirmation displayed
				case ($this->confirm_view):
				default:

					// check if there were errors in the input
					if (!$this->processForm()) {
						break; // re-display the form
					} else {

						// this function handles the business logic for preparing the order,
						// displaying the confirmation and calling the gateway
						$this->_processOrder();

						$this->finishForm();
						return true;
					}
			}

		}

		// print the initial form
		$this->printForm();
		return true;

	}//end printStandard()


	/**
	* prints current form
	*
	* @access public
	* @return void
	*/
	function printForm()
	{
		$order = $this->_readOrder();
		if (!empty($order)) {
			$this->_deleteOrderFile();
			$this->active_submission = $order['submission'];
		}

		parent::printForm();

	}//end printForm()


	/**
	* Generates and prints final receipt after payment processing
	*
	* @access private
	* @return void
	*/
	function _processReceipt()
	{
		// get gateway's response
		$response = array_get_index($this->_tmp, 'GATEWAY_RESPONSE', false);
		if (!$response) {
			return;
		}

		$order = $this->_readOrder($response['REFERENCE']);
		$this->_deleteOrderFile($response['REFERENCE']);

		$this->active_submission = $order['submission'];

		$this->emailAndLog();

	}//end processReceipt()


	/**
	* Makes Receipt Header
	*
	* @access public
	* @return string
	*/
	function _getReceiptHeader($asHTML = true)
	{
		// start out with empty header
		$header = '';

		// get response from the gateway
		$response = array_get_index($this->_tmp, 'GATEWAY_RESPONSE', false);
		if (!$response) {
			return $header;
		}

		// today
		$date = date("F j, Y, g:i a");

		if ($asHTML) {
			$header =<<<HEREDOC
			<table class="ecom_header_table">
				<tr>
					<td colspan="2"><h2>Receipt</h2></td>
				</tr>
				<tr>
					<td>Transaction Number:</td>
					<td>{$response['TRANSACTION']}</td>
				</tr>
				<tr>
					<td>Transaction Statis:</td>
					<td>{$response['STATUS']}</td>
				</tr>
				<tr>
					<td>Date:</td>
					<td>$date</td>
				</tr>
			</table>
HEREDOC;

		} else {
			$header .= "Transaction Number: {$response['TRANSACTION']}\n";
			$header .= "Transaction Status: {$response['STATUS']}\n";
			$header .= "Date: $date\n";
		}

		return $header;

	}


	/**
	* Wrapper for parent's emailing an logging functions
	*
	* @access public
	* @return void
	*/
	function emailAndLog()
	{
		parent::mailRecipients();
		parent::mailReceipt();
		parent::sendToFriends();
		parent::runSelectiveEmails();
		parent::logSubmission();

	}//end emailAndLog()


	/**
	* Generates receipt and invites user to make a payment
	*
	* Prints to screen the result
	*
	* @access private
	* @return void
	*/
	function _processOrder()
	{
		$gateway = $this->attr('gateway_assetid');

		// check if there is anything to e-process
		// and that gateway is specified
		if ($this->_makeOrder() && !empty($gateway)) {
			$order = $this->_tmp['order'];
			$this->_saveOrder();

			$action_name = $this->getPrefix().'_action';

			// get own url. because the form cannot print itself, we have
			// to determine the url of the parent page
			$parents         = $GLOBALS['SQ_SYSTEM']->am->getParents($this->id, 'page_custom_form_ecommerce', false);
			$parent_page_id  = array_pop($parents);
			$parent_page     = $GLOBALS['SQ_SYSTEM']->am->getAsset($parent_page_id);
			$parent_page_url = $parent_page->getURL();

			// save some data to session for the gateway asset
			$_SESSION['SQ_ECOM_BACK_URL']   = $parent_page_url.'?'.$action_name.'='.$this->receipt_view.'&'.$this->getPrefix().'_submit=true';
			$_SESSION['SQ_ECOM_REF_NO']     = $GLOBALS['SQ_SYSTEM']->getUniqueSessionKey();
			$_SESSION['SQ_ECOM_AMOUNT']     = $order['total'];

			$gateway_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($gateway);
			$gateway_asset_url = $gateway_asset->getURL();

			$confirm_button = $this->attr('confirm_button_name');
			$cancel_button  = $this->attr('cancel_button_name');

			echo '<br /><input type="button" value="'.$confirm_button.'" onclick="document.location.href=\''.$gateway_asset_url.'\'" /><input type="button" value="'.$cancel_button.'" onclick="document.location.href=\''.$_SERVER['PHP_SELF'].'\'">';

		} else {
			// if there's nothing to process, treat input as a normal form
			unset($this->_tmp['order']);
			$this->emailAndLog();
		}

	}//end _processOrder()


	/**
	* Print Summary of the submission
	*
	* Overridden parent function to include the receipt information
	*
	* @access public
	* @return void
	*/
	function printSummary($asHTML = false)
	{
		echo $this->_getReceiptHeader($asHTML);
		echo $this->_getReceipt($asHTML);
		if ($asHTML) {
			echo "<br /><br />";
		} else {
			echo "\n\n";
		}
		parent::printSummary($asHTML);
	}


	/**
	* format reciept values into table for nice output
	*
	* @access private
	* @return string
	*/
	function _getReceipt($asHTML = true)
	{
		$order = array_get_index($this->_tmp,'order');
		if (empty($order)) {
			return false;
		}

		if ($asHTML) {
			$receipt_fmt   = '<hr /><table class="sq_ecomTable">%s%s</table><hr />';
			$order_row_fmt = '<tr class="sq_ecomTableItemRow"><td class="sq_ecomTableItemName">%s</td><td class="sq_ecomTableItemValue" align="right">%01.2f</td></tr>';
			$total_row_fmt = '<tr class="sq_ecomTableTotalRow"><td class="sq_ecomTableTotalName" align="right">%s</td><td class="sq_ecomTableTotalValue" align="right">$%01.2f</td></tr>';
		} else {
			$receipt_fmt   = "RECEIPT:\n############################\n%s\n%s\n############################";
			$order_row_fmt = "%s: %01.2f\n";
			$total_row_fmt = "*** %s: $%01.2f ***\n";
		}

		$rows = '';
		foreach ($order['items'] as $item) {
			$rows .= sprintf($order_row_fmt,$item['name'],$item['price']);
		}

		$total   = sprintf($total_row_fmt,'Total:',$order['total']);
		$receipt = sprintf($receipt_fmt, $rows, $total);

		return $receipt;

	}//end _getReceipt()


	/**
	* Generates order array, which contains all necessary information for payment processing
	*
	* @access private
	* @return array
	*/
	function _makeOrder() {

		$total = 0;

		$questions  = $this->getAllQuestionAssets();
		$ecom_items = Array();

		foreach ($questions as $id => $question)
		{
			if (($rule = $this->_getEcommerceRule($id)) !== false) {
				$question_title = $question->attr('name');
				// if it is a selection question, we ask the question to
				// tell us if an option was selected
				if ($question->isSelection()) {
					foreach ($rule as $option_id => $price) {
						if ($question->isSelected($option_id)) {
							$ecom_item['name']  = $question_title.': '.$question->getOptionName($option_id);
							$ecom_item['price'] = $price;

							$ecom_items[] = $ecom_item;
							$total += $ecom_item['price'];
						}
					}
				// if it is not a question with options, we get the submission directly
				// we assume that it is a numeric question
				} else {
					$ecom_item['name']  = $question_title;
					$ecom_item['price'] = $this->active_submission[$id];

					$ecom_items[] = $ecom_item;
					$total += $ecom_item['price'];
				}

			}
		}

		// proceed only if there is e-commerce value generated by this form
		if (empty($ecom_items) || $total <= 0){
			return false;
		}

		$order['items'] = $ecom_items;
		$order['total'] = $total;
		$order['submission'] = $this->active_submission;

		$this->_tmp['order'] = $order;

		return true;

	}//end _makeOrder()


	/**
	* saves order into file, to prevent losing information in case of session dying
	*
	* @access private
	* @return boolean
	*/
	function _saveOrder()
	{
		$filename = $this->_getOrderFileName();
		$order_contents = serialize($this->_tmp['order']);
		return string_to_file($order_contents, $filename);

	}//end _saveOrder()


	/**
	* reads order from the file
	*
	* @param	$order_no	id of the saved file
	*
	* @access private
	* @return array
	*/
	function _readOrder($order_no = false)
	{
		$filename = $this->_getOrderFileName($order_no);
		$order = file_to_string($filename);
		if (!$order) return false;
		$this->_tmp['order'] = unserialize($order);
		return $this->_tmp['order'];

	}//end _readOrder()


	/**
	* delete file with order.
	* @see _readOrder()
	*
	* @param	$order_no	id of the saved file
	*
	* @access private
	* @return boolean
	*/
	function _deleteOrderFile($order_no = false)
	{
		$filename = $this->_getOrderFileName($order_no);
		unlink($filename);
		return true;

	}//end _deleteOrderFile()

	/**
	* returns file name for order file
	* @see _saveOrder()
	* @see _readOrder()
	* @see _deleteOrderFile()
	*
	* @param	$order_no	id of the saved file
	*
	* @access private
	* @return string
	*/
	function _getOrderFileName($order_no = false)
	{
		$order_path = $this->data_path.'/tmp_orders';

		if (!is_dir($order_path)) {	// doesn't exist = create it
			mkdir($order_path);
		}

		if (!$order_no){
			$order_no = $GLOBALS['SQ_SYSTEM']->getUniqueSessionKey();
		}

		return $order_path.'/'.$order_no.'.tmp';

	}//end _getOrderFileName()


	/////// Overwritten parent functions to do nothing ///////
	function mailRecipients()
	{
		return true;

	}//end mailRecipients

	function mailReceipt()
	{
		return true;

	}//end mailReceipt

	function sendToFriends()
	{
		return true;

	}//end sendToFriends

	function runSelectiveEmails()
	{
		return true;

	}//end runSelectiveEmails

	function logSubmission()
	{
		return true;

	}//end logSubmission

}//end class

?>
