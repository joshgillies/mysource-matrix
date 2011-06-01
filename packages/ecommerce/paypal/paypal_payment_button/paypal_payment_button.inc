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
* $Id: paypal_payment_button.inc,v 1.2 2009/03/30 18:11:23 ata Exp $
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
* @version $Revision: 1.2 $
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
		require_once $paypal_config->attr('encryption_lib_path');
		
		$paypal_account_id = $this->attr('paypal_account_id');
		if (empty($paypal_account_id)) {
			trigger_error('Paypal Business Account is not specified for this asset.', E_USER_WARNING);
			return;
		}
		
		$paypal_account = $GLOBALS['SQ_SYSTEM']->am->getAsset($paypal_account_id);
		
		//gather button's data
		$data = $this->_concatButtonData($paypal_config, $paypal_account);
		
		$encryptedButton = PPCrypto::signAndEncrypt($data, $paypal_account->attr('public_cert_path'), $paypal_account->attr('private_key_path'), '', $paypal_account->attr('paypal_cert_path'));
		
		if (!$encryptedButton['status']) {
			trigger_error('FAIL TO ENCRYPT BUTTON. ERROR: '.$encryptedButton['error_msg'], E_USER_WARNING);
			return;
		}
		
		$post_url = self::$_post_url[$paypal_account->attr('account_type')];
		
		$content = <<<HEREDOC
<form action="{$post_url}" method="post">
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
		$data = 'cmd='.$this->attr('button_type')."\n";
		
		$data .= 'business='.$paypal_business_account->attr('account_id')."\n";
		$data .= 'cert_id='.$paypal_business_account->attr('public_cert_id')."\n";
		
		$item_name = $this->attr('item_name');
		//Replace keywords for item name
		replace_global_keywords($item_name);
		$data .= 'item_name='.$item_name."\n";
		
		if ($this->attr('item_id') != '') {
			$data .= 'item_number='.$this->attr('item_id')."\n";
		}

		$price = $this->attr('price');
		//Replace keywords for price
		replace_global_keywords($price);
		$data .= 'amount='.$price."\n";
		
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
		
		$data .= $this->attr('extra_vars');
		
		return $data;
		
	}//end _concatButtonData()
	
	
}//end class

?>