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
* $Id: paypal_payment_button.inc,v 1.1 2009/03/05 18:33:12 ata Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';
require_once dirname(__FILE__).'/../include/paypal_config.inc';

/**
* Paypal_Payment_Button
*
* Purpose
*
*
* @author  Anh Ta <ata@squiz.co.uk>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Paypal_Payment_Button extends Page
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
	* Called by the design to print the body of this asset
	*
	* Print the encrypted HTML of the button
	*
	* @return void
	* @access public
	*/
	public function printBody()
	{
		require_once SQ_PAYPAL_LIB_PATH.'/PPCrypto.php';
		
		$data = $this->_concatButtonData();
		
		if ($this->attr('test_mode')) {
			//test mode
			$encryptedButton = PPCrypto::signAndEncrypt($data, TEST_SQ_PUBLIC_CERT_PATH, TEST_SQ_PRIVATE_KEY_PATH, TEST_SQ_PRIVATE_KEY_PASSWORD, TEST_SQ_PAYPAL_CERT_PATH);
		} else {
			//real mode
			$encryptedButton = PPCrypto::signAndEncrypt($data, SQ_PUBLIC_CERT_PATH, SQ_PRIVATE_KEY_PATH, SQ_PRIVATE_KEY_PASSWORD, SQ_PAYPAL_CERT_PATH);
		}
		
		if (!$encryptedButton['status']) {
			trigger_error($encryptedButton['error_msg'], E_USER_WARNING);
			return;
		}
		
		$content = <<<HEREDOC
<form action="{$this->attr('post_url')}" method="post">
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
	private function _concatButtonData() {
		//concat the content of this payment button
		$data = 'cmd='.$this->attr('button_type')."\n";
		$data .= 'business='.$this->attr('merchant_id')."\n";
		
		if ($this->attr('test_mode')) {
			//test mode
			$data .= 'cert_id='.TEST_SQ_PUBLIC_CERT_ID."\n";
		} else {
			//real mode
			$data .= 'cert_id='.SQ_PUBLIC_CERT_ID."\n";
		}
		
		$data .= 'item_name='.$this->attr('item_name')."\n";
		
		if ($this->attr('item_id') != '') {
			$data .= 'item_number='.$this->attr('item_id')."\n";
		}
		
		$data .= 'amount='.$this->attr('price')."\n";
		$data .= 'currency_code='.$this->attr('currency')."\n";
		
		if ($this->attr('undefined_quantity')) {
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
		$custom_var = $this->id.SQ_PAYPAL_CUSTOM_SPLIT_CHARS.$custom;
		
		//Get the query string of Notify URL
		$notify_url_query = parse_url($this->attr('notify_url'), PHP_URL_QUERY);
		if (is_null($notify_url_query)) {
			$notify_url_query = '';
		}
		//Hash the custom variable string and notify URL's query with our secret string to prevent malicious changes to 
		//the custom parameter and notify URL in Replay attack
		$custom_hash = md5(SQ_PAYPAL_CUSTOM_SECRET_STRING.$custom_var.SQ_PAYPAL_CUSTOM_SECRET_STRING.$notify_url_query);
		
		$data .= 'custom='.$custom_hash.SQ_PAYPAL_CUSTOM_SPLIT_CHARS.$custom_var."\n";
		
		if ($this->attr('notify_url') != '') {
			$data .= 'notify_url='.$this->attr('notify_url')."\n";
		}
		
		if ($this->attr('return_url') != '') {
			$data .= 'return='.$this->attr('return_url')."\n";
		}
		
		if ($this->attr('cancel_return_url') != '') {
			$data .= 'cancel_return='.$this->attr('cancel_return_url')."\n";
		}
		
		$data .= $this->attr('extra_vars');
		
		return $data;
		
	}//end _concatButtonData()
	
	
}//end class

?>