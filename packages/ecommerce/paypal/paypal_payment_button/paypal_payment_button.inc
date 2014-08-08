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
* $Id: paypal_payment_button.inc,v 1.5 2012/08/30 00:58:34 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';

/**
* Paypal_Payment_Button
*
* Purpose
*
*
* @author  Anh Ta <ata@squiz.co.uk>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Paypal_Payment_Button extends Page
{

	/**
	 * The string that is used to split the custom variables of this button asset
	 * 
	 * @var string
	 */
	public static $CUSTOM_PARAM_SPLIT_STR = '::';
	
	
	/**
	 * This array stores the POST URLs of the Paypal Buttons
	 * 
	 * @var array
	 */
	private static $_post_url = Array(
										'live' 		=> 'https://www.paypal.com/cgi-bin/webscr',
										'sandbox'	=> 'https://www.sandbox.paypal.com/cgi-bin/webscr',
									 );
	
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
	* Called by the design to print the body of this asset
	*
	* Print the encrypted HTML of the button
	*
	* @return void
	* @access public
	*/
	public function printBody()
	{
		$paypal_config_id = $this->attr('paypal_config_id');
		if (empty($paypal_config_id)) {
			trigger_error('Paypal Configuration is not specified for this asset.', E_USER_WARNING);
			return;
		}
		
		$paypal_config = $GLOBALS['SQ_SYSTEM']->am->getAsset($paypal_config_id);
		
		$paypal_account_id = $this->attr('paypal_account_id');
		if (empty($paypal_account_id)) {
			trigger_error('Paypal Business Account is not specified for this asset.', E_USER_WARNING);
			return;
		}
		
		$paypal_account = $GLOBALS['SQ_SYSTEM']->am->getAsset($paypal_account_id);
		
		//gather button's data
		$data = $this->_concatButtonData($paypal_config, $paypal_account);
		
		$encryptedButton = $this->_signAndEncrypt($data, $paypal_account->attr('public_cert_path'), $paypal_account->attr('private_key_path'), '', $paypal_account->attr('paypal_cert_path'));
		
		if (!$encryptedButton['status']) {
			trigger_error('FAIL TO ENCRYPT BUTTON. ERROR: '.$encryptedButton['error_msg'], E_USER_WARNING);
			return;
		}
		
		$post_url = self::$_post_url[$paypal_account->attr('account_type')];
		
		$target = '';
		if (($this->attr('button_type') == '_cart') || ($this->attr('button_type') == '_view_cart')) {
			$target = 'target="'.$this->attr('cart_target_window').'"';
		}
		
		$content = <<<HEREDOC
<form $target action="{$post_url}" method="post">
	<input type="hidden" name="cmd" value="_s-xclick">
	<input type="hidden" name="encrypted" value="-----BEGIN PKCS7-----{$encryptedButton['encryptedData']}-----END PKCS7-----">
	<input type="image" src="{$this->attr('image_url')}" border="0" name="submit" alt="Paypal payment button">
</form>
HEREDOC;

		echo $content;
		
	}//end printBody()
	
	
	/*
	 * Concatenate the payment button data that need to be encrypted
	 * 
	 * @return string	The concatenated string of all the button data
	 * @access private
	 */
	private function _concatButtonData($paypal_config, $paypal_business_account)
	{
		//concat the content of this payment button
		$cmd = $this->attr('button_type');
		if ($cmd == '_view_cart') {
			$cmd = '_cart';
		}
		
		$data = 'cmd='.$cmd."\n";
		
		if ($cmd == '_cart') {
			if ($this->attr('button_type') == '_cart') {
				$data .= 'add=1'."\n";
			} else {
				$data .= 'display=1'."\n";
			}
			//add shopping URL if it exists
			$shopping_url = trim($this->attr('shopping_url'));
			if ($shopping_url != '') {
				$data .= 'shopping_url='.$shopping_url."\n";
			}
		}
		
		$data .= 'business='.$paypal_business_account->attr('account_id')."\n";
		$data .= 'cert_id='.$paypal_business_account->attr('public_cert_id')."\n";
		
		if ($this->attr('button_type') != '_view_cart') {
			$item_name = $this->attr('item_name');
			//Replace keywords for item name
			replace_global_keywords($item_name);
			$data .= 'item_name='.$item_name."\n";
			
			if ($this->attr('item_id') != '') {
				$data .= 'item_number='.$this->attr('item_id')."\n";
			}
	
			$price = trim($this->attr('price'));
			if ($price != '') {
				//Replace keywords for price
				replace_global_keywords($price);
				$data .= 'amount='.$price."\n";
			}
			
			$data .= 'currency_code='.$this->attr('currency')."\n";
			
			if (($this->attr('button_type') == '_xclick') && $this->attr('undefined_quantity')) {
				$data .= 'undefined_quantity=1'."\n";
			}
			
			$data .= 'charset='.SQ_CONF_DEFAULT_CHARACTER_SET."\n";
			
			//Replace keyword and encode custom variable
			$custom = $this->attr('custom_var');
			if ($custom != '') {
				replace_global_keywords($custom);
				$custom = urlencode($custom);
			}
			
			//Add this button id to custom variable so that IPN receiver asset will know which button triggered the payment
			$custom_var = $this->id.self::$CUSTOM_PARAM_SPLIT_STR.$custom;
			
			//Get the query string of Notify URL
			$notify_url_query = parse_url($this->attr('notify_url'), PHP_URL_QUERY);
			if (is_null($notify_url_query)) {
				$notify_url_query = '';
			}
			
			//Hash the custom variable string and notify URL's query with our secret string to prevent malicious changes to 
			//the custom parameter and notify URL in Replay attack.
			//Actually, the notify URL query should not be checked or used here but we might sometimes want to use the trigger
			//condition URL matches to trigger an action and we do not want the query string to be changed.
			$custom_hash = md5($paypal_config->attr('custom_param_secret_str').$custom_var.$paypal_config->attr('custom_param_secret_str').$notify_url_query);
			
			$data .= 'custom='.$custom_hash.self::$CUSTOM_PARAM_SPLIT_STR.$custom_var."\n";
			
			if ($this->attr('notify_url') != '') {
				$data .= 'notify_url='.$this->attr('notify_url')."\n";
			}
			
			if ($this->attr('return_url') != '') {
				$data .= 'return='.$this->attr('return_url')."\n";
			}
			
			if ($this->attr('cancel_return_url') != '') {
				$data .= 'cancel_return='.$this->attr('cancel_return_url')."\n";
			}
		}
		
		$data .= $this->attr('extra_vars');
		
		return $data;
		
	}//end _concatButtonData()
	

	/**
	 * Sign and Envelope the passed data string, returning a PKCS7 blob that can be posted to PayPal.
	 * Make sure the passed data string is seperated by UNIX linefeeds (ASCII 10, '\n').
	 *
	 * @param	string	The candidate for signature and encryption
	 * @param	string	The file path to the EWP(merchant) certificate
	 * @param	string	The file path to the EWP(merchant) private key
	 * @param	string	The EWP(merchant) private key password
	 * @param	string	The file path to the PayPal Certificate
	 * @return	array	Contains a bool status, error_msg, error_no, and an encrypted string: encryptedData if successfull
	 *
	 * @access	public
	 * @static
	 */
	function _signAndEncrypt($dataStr_, $ewpCertPath_, $ewpPrivateKeyPath_, $ewpPrivateKeyPwd_, $paypalCertPath_)
	{
		$dataStrFile  = realpath(tempnam('/tmp', 'pp_'));
        $fd = fopen($dataStrFile, 'w');
		if(!$fd) {
			$error = "Could not open temporary file $dataStrFile.";
			return array("status" => false, "error_msg" => $error, "error_no" => 0);
		}
		fwrite($fd, $dataStr_);
		fclose($fd);

		$signedDataFile = realpath(tempnam('/tmp', 'pp_'));
		if(!@openssl_pkcs7_sign(	$dataStrFile,
									$signedDataFile,
									"file://$ewpCertPath_",
									array("file://$ewpPrivateKeyPath_", $ewpPrivateKeyPwd_),
									array(),
									PKCS7_BINARY)) {
			unlink($dataStrFile);
			unlink($signedDataFile);
			$error = "Could not sign data: ".openssl_error_string();
			return array("status" => false, "error_msg" => $error, "error_no" => 0);
		}

		unlink($dataStrFile);

		$signedData = file_get_contents($signedDataFile);
		$signedDataArray = explode("\n\n", $signedData);
		$signedData = $signedDataArray[1];
		$signedData = base64_decode($signedData);

		unlink($signedDataFile);

		$decodedSignedDataFile = realpath(tempnam('/tmp', 'pp_'));
		$fd = fopen($decodedSignedDataFile, 'w');
		if(!$fd) {
			$error = "Could not open temporary file $decodedSignedDataFile.";
			return array("status" => false, "error_msg" => $error, "error_no" => 0);
		}
		fwrite($fd, $signedData);
		fclose($fd);

		$encryptedDataFile = realpath(tempnam('/tmp', 'pp_'));
		if(!@openssl_pkcs7_encrypt(	$decodedSignedDataFile,
									$encryptedDataFile,
									file_get_contents($paypalCertPath_),
									array(),
									PKCS7_BINARY)) {
			unlink($decodedSignedDataFile);
			unlink($encryptedDataFile);
			$error = "Could not encrypt data: ".openssl_error_string();
			return array("status" => false, "error_msg" => $error, "error_no" => 0);
		}

		unlink($decodedSignedDataFile);

		$encryptedData = file_get_contents($encryptedDataFile);
		if(!$encryptedData) {
			$error = "Encryption and signature of data failed.";
			return array("status" => false, "error_msg" => $error, "error_no" => 0);
		}

		unlink($encryptedDataFile);

		$encryptedDataArray = explode("\n\n", $encryptedData);
		$encryptedData = trim(str_replace("\n", '', $encryptedDataArray[1]));

        return array("status" => true, "encryptedData" => $encryptedData);
	} // _signAndEncrypt
	
}//end class

?>