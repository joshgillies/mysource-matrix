<?php 
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: paypal_ipn_receiver.inc,v 1.1 2009/03/05 18:29:55 ata Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';
require_once dirname(__FILE__).'/../include/paypal_config.inc';

/**
* Paypal_IPN_Receiver
*
* Purpose
*
*
* @author  Anh Ta <ata@squiz.co.uk>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Paypal_IPN_Receiver extends Page
{

	/**
	* Constructor
	*
	* @param string	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor

	
	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	public function _getAllowedLinks()
	{
		$allowed = parent::_getAllowedLinks();

		$allowed[SQ_LINK_NOTICE]['asset'] = Array('card' => 'M', 'exclusive' => FALSE);

		return $allowed;

	}//end _getAllowedLinks()
	

	/**
	* Prints out the Frontend for this asset
	*
	* @return void
	* @access public
	*/
	public function printFrontend()
	{
		$log_message = "There is a {$_SERVER['REQUEST_METHOD']} Request from {$_SERVER['REMOTE_ADDR']}\n";
		
		//only process if the request method is POST
		if ($_SERVER['REQUEST_METHOD'] == 'POST') {
			$log_message .= "payment_status={$this->_getPOSTLogString('payment_status')}\n";
			$log_message .= "txn_id={$this->_getPOSTLogString('txn_id')}\n";
			
			//do not proceed if payment_status is not Completed or there is no transaction ID
			if (!isset($_POST['payment_status']) || ($_POST['payment_status'] != 'Completed') || empty($_POST['txn_id'])) {
				$this->_returnPageNotFound($log_message);
			}
			
			//if there is no custom parameter, it must not be a Paypal POST request (our payment button always sends custom variable)
			if (!isset($_POST['custom'])) {
				$log_message .= "custom=[Not set]\n";
				$this->_returnPageNotFound($log_message);;
			}
			
			list($custom_hash, $button_id, $custom) = explode(SQ_PAYPAL_CUSTOM_SPLIT_CHARS, $_POST['custom']);
			
			//check if there is any malicious changes to our custom parameter
			$custom_var = $button_id.SQ_PAYPAL_CUSTOM_SPLIT_CHARS.$custom;
			
			if (!empty($custom)) {
				$custom = urldecode($custom);
			}
			
			$notify_url_query = array_get_index($_SERVER, 'QUERY_STRING', '');
			
			$log_message .= "button_id=$button_id\n";
			$log_message .= "custom=$custom\n";
			$log_message .= "notify_url_query=$notify_url_query\n";
			
			if ($custom_hash != md5(SQ_PAYPAL_CUSTOM_SECRET_STRING.$custom_var.SQ_PAYPAL_CUSTOM_SECRET_STRING.$notify_url_query)) {
				//someone tampered with the custom paramater
				$log_message .= "WRONG CHECKSUM\n";
				$this->_returnPageNotFound($log_message);;
			}
			
			//get other information of this purchase
			$merchant_id = array_get_index($_POST, 'business', '');
			$item_name = array_get_index($_POST, 'item_name', '');
			$item_id = array_get_index($_POST, 'item_number', '');
			$currency = array_get_index($_POST, 'mc_currency', '');
			$mc_gross = array_get_index($_POST, 'mc_gross', 0);
			$quantity = array_get_index($_POST, 'quantity', 1);
			$price = $mc_gross/$quantity;
			
			$log_message .= "business=$merchant_id\n";
			$log_message .= "item_name=$item_name\n";
			$log_message .= "item_number=$item_id\n";
			$log_message .= "mc_currency=$currency\n";
			$log_message .= "mc_gross=$mc_gross\n";
			$log_message .= "quantity=$quantity\n";
			$log_message .= "price=$price\n";
			
			$button = $GLOBALS['SQ_SYSTEM']->am->getAsset($button_id);
			
			//if there is no payment button or its attributes are different from Paypal parameters, there is something wrong
			if (is_null($button) || ($merchant_id != $button->attr('merchant_id')) || ($item_name != $button->attr('item_name')) ||
			($item_id != $button->attr('item_id')) || $currency != $button->attr('currency') || $price != $button->attr('price')) {
				$log_message .= "INFORMATION DOES NOT MATCH\n";
				$this->_returnPageNotFound($log_message);;
			}
			
			//THE PURCHASE IS VALIDATED, get assetid from custom parameter to broadcast IPN Completed event
			
			//split custom parameters
			$params = Array();
			$custom_splits = explode(',', $custom);
			foreach ($custom_splits as $name_value) {
				list($name, $value) = explode('=', $name_value);
				$params[trim($name)] = trim($value);
			}
			
			//set all custom parameters to $_POST array so that triggers can use them later
			foreach ($params as $key => $value) {
				$_POST[$key] = $value;
			}
			
			//get target asset
			$assetid = '';
			$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, '', FALSE, 'target_asset');
			if (!empty($link)) {
				$assetid = $link['minorid'];
			}
			
			if (empty($assetid)) {
				$assetid = array_get_index($_POST, $this->attr('assetid_param_name'), '');
			}
			
			if (empty($assetid)) {
				$log_message .= "No target asset ID specified.\n";
			} else {
				$target_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				
				$GLOBALS['SQ_SYSTEM']->broadcastTriggerEvent('trigger_event_paypal_ipn_completed', $target_asset);
				
				if (empty($target_asset)) {
					$log_message .= "There is no asset with ID #$assetid.\n";
				} else {
					$log_message .= "An IPN Completed event was broadcasted to the asset #$assetid.\n";
				}
				
			}
			
			//log message
			$this->_logIncomingRequestInfo($log_message);
			
		} else {
			//trick that there is no asset at this URL because the URL of IPN Receiver should not be revealed
			$this->_returnPageNotFound($log_message);;
		}

	}//end printFrontend()
	
	
	/**
	 * This function sends an HTTP status code 404 (Page Not Found) to the client to trick it that there is no page at
	 * this requested URL
	 * 
	 * @return void
	 * @access private
	 */
	private function _returnPageNotFound($log_message) {
		$protocol = current_protocol();
		$url = strip_url(current_url(FALSE, TRUE));
		if (!headers_sent()) {
			header('HTTP/1.0 404 Not Found');
		}

		//log message before triggering an error
		$log_message .= "A 404 status code was sent back to the client.\n";
		$this->_logIncomingRequestInfo($log_message); 
		
		trigger_localised_error('SYS0218', E_USER_NOTICE, $protocol.'://'.$url);
		
		exit(1);
		
	}//end _returnPageNotFound()
	
	
	/**
	 * 
	 * @return unknown_type
	 */
	private function _getPOSTLogString($var_name) {
		if (empty($_POST[$var_name])) {
			return '[Empty]';
		}
		
		return $_POST[$var_name];
		
	}//end _getPOSTLogString()
	
	
	/**
	 * 
	 * @return unknown_type
	 */
	private function _logIncomingRequestInfo($data) {
		log_write($data, SQ_PAYPAL_LOG_FILE_NAME, E_USER_NOTICE, FALSE);
		
	}//end _logIncomingRequest()
	
	
}//end class

?>