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
* $Id: payment_gateway_paypal_express_management.inc,v 1.1.2.2 2013/06/25 06:52:22 cupreti Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Payment_Gateway_Paypal_Express_Management
*
* @see Asset_Management
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: 1.1.2.2 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Payment_Gateway_Paypal_Express_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Payment_Gateway_Paypal_Express_Management(&$pm)
	{
		$this->Asset_Management($pm);

		// Include the Paypal_Express_Processor class to get the supported currency
		require_once dirname(__FILE__).'/paypal_express_processor.inc';
		$currency_types = Paypal_Express_Processor::getCurrencyTypes();

		$this->vars = Array(
						'username'				=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> '',
													'description'	=> 'API Username authorized to process transactions',
												   ),
						'password'			=> Array(
													'added'			=> '0.1',
													'type'			=> 'password',
													'default'		=> '',
													'description'	=> 'API Password',
												   ),
						'signature'			=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> '',
													'description'	=> 'API Signature',
												   ),
						'button_image'		=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> 'https://www.paypal.com/en_US/i/btn/btn_xpressCheckout.gif',
													'description'	=> 'Paypal checkout button image url',
												   ),
						'timeout'			=> Array(
													'added'			=> '0.1',
													'type'			=> 'int',
													'default'		=> '60',
													'description'	=> 'How long to wait for a response, in seconds, before returning an error',
													'parameters'	=> Array(
																		'allow_negative'	=> FALSE,
																		'allow_empty'		=> FALSE,
																	   ),
												   ),
						'currency'			=> Array(
													'added'			=> '0.1',
													'type'			=> 'selection',
													'default'		=> 'AUD',
													'description'	=> 'The currency used in all the e-commerce transactions',
													'parameters'	=> Array(
																		'multiple'		=> FALSE,
																		'allow_empty'	=> FALSE,
																		'options'		=> $currency_types,
																	   ),
														),
						'test_mode'			=> Array(
													'added'			=> '0.1',
													'type'			=> 'boolean',
													'default'		=> TRUE,
													'description'	=> 'Live or testing mode',
													'parameters'	=> Array(
																		'allow_empty'	=> FALSE,
																	   ),
												  ),

						'first_name_var_name'	=> Array(
														'added'			=> '0.1',
														'type'			=> 'text',
														'default'		=> '',
														'description'	=> 'The pass through variable name of First Name',
													   ),
						'last_name_var_name'	=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> '',
													'description'	=> 'The pass through variable name of Last Name',
												   ),
						'addr_street_var_name'	=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> '',
													'description'	=> 'The pass through variable name of Street',
												   ),
						'city_var_name'			=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> '',
													'description'	=> 'The pass through variable name of City',
												   ),
						'postcode_var_name'		=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> '',
													'description'	=> 'The pass through variable name of Postcode',
												   ),
						'state_var_name'		=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> '',
													'description'	=> 'The pass through variable name of State',
												   ),
						'country_var_name'		=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> '',
													'description'	=> 'The pass through variable name of Country',
												   ),
						'submit_text'			=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> '   Pay   ',
													'description'	=> 'Submit button text',
													'is_contextable'	=> TRUE,
												   ),
						'reset_text'			=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> 'Reset',
													'description'	=> 'Reset button text',
													'is_contextable'	=> TRUE,
												   ),
						'cancel_text'			=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> 'Cancel',
													'description'	=> 'Cancel button text',
													'is_contextable'	=> TRUE,
												   ),

					  );

	}//end constructor


	/**
	* Gets a list of all files
	*
	* @return array
	* @access private
	*/
	function _getFileList()
	{
		return Array(
				'files/loader.js',
				'files/loader.gif',
			   );

	}//end _getFileList()


	/**
	* Return an array with information about the events that this page is listening for
	*
	* @return boolean
	* @access public
	*/
	function getEventList()
	{
		return Array(
				Array(
					'event_name'					=> 'requestKeywords',
					'broadcast_type_code'			=> 'content_type',
					'broadcast_strict_type_code'	=> FALSE,
					'options'						=> Array(
														'side_of_link'	=> 'major',
														'indirect'		=> TRUE,
														'is_exclusive'	=> NULL,
														'is_dependant'	=> 1,
														'value'			=> '',
														'link_type'		=> SQ_LINK_TYPE_2,
													   ),
				),
			   );

	}//end getEventList()


}//end class

?>
