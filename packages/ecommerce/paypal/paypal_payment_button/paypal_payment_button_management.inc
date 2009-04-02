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
* $Id: paypal_payment_button_management.inc,v 1.3 2009/04/02 23:45:24 mbrydon Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Paypal_Payment_Button_Management
*
* @see Asset_Management
*
* @author  Anh Ta <ata@squiz.co.uk>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Paypal_Payment_Button_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function __construct(&$pm)
	{
		$this->Asset_Management($pm);
		$this->vars = Array(
							'button_type'			=> Array(
																'added'			=> '0.1',
																'type'			=> 'selection',
																'default'		=> '_xclick',
																'parameters'	=> Array(
																							'multiple'		=> FALSE,
																							'allow_empty'	=> FALSE,
																							'options'		=> Array(
																														'_xclick'															=> 'Buy Now',
																											   		),
																					   ),
															),
							'item_name'				=> Array(
																'added'		=> '0.1',
																'type'		=> 'text',
																'default'	=> '',
																'is_admin'	=> FALSE,
															),
							'item_id'				=> Array(
																'added'		=> '0.1',
																'type'		=> 'text',
																'default'	=> '',
															),
							'price'					=> Array(
																'added'		=> '0.1',
																'type'		=> 'text',
																'default'	=> '',
																'is_admin'	=> FALSE,
															),
							'currency'				=> Array(
																'added'			=> '0.1',
																'type'			=> 'selection',
																'default'		=> 'GBP',
																'parameters'	=> Array(
																							'multiple'		=> FALSE,
																							'allow_empty'	=> FALSE,
																							'options'		=> Array(
																														'GBP' => 'GBP',
																														'AUD' => 'AUD',
																														'USD' => 'USD',
																														'EUR' => 'EUR',
																														'JPY' => 'JPY',
																												   ),
																					   ),
																'is_admin'	=> FALSE,
															),
							'undefined_quantity'	=> Array(
																'added'			=> '0.1',
																'type'			=> 'boolean',
																'default'		=> TRUE,
																'parameters'	=> Array(
																							'allow_empty'	=> FALSE,
																						),
															),
							'paypal_account_id'		=> Array(
																'added'		=> '0.1',
																'type'		=> 'assetid',
																'default'	=> '',
															),
							'paypal_config_id'		=> Array(
																'added'		=> '0.1',
																'type'		=> 'assetid',
																'default'	=> '',
															),
							'custom_var'			=> Array(
																'added'		=> '0.1',
																'type'		=> 'text',
																'default'	=> '',
															),
							'notify_url'			=> Array(
																'added'		=> '0.1',
																'type'		=> 'text',
																'default'	=> '',
															),
							'return_url'			=> Array(
																'added'		=> '0.1',
																'type'		=> 'text',
																'default'	=> '',
															),
							'cancel_return_url'		=> Array(
																'added'		=> '0.1',
																'type'		=> 'text',
																'default'	=> '',
															),
							'image_url'				=> Array(
																'added'		=> '0.1',
																'type'		=> 'text',
																'default'	=> 'https://www.paypal.com/en_GB/i/btn/btn_buynowCC_LG.gif',
															),
							'extra_vars'			=> Array(
																'added'		=> '0.1',
																'type'		=> 'text',
																'default'	=> '',
															),
						);

	}//end constructor


}//end class
?>
