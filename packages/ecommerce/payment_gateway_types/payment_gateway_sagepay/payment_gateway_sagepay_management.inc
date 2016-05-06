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
* $Id: payment_gateway_sagepay_management.inc,v 1.8 2012/08/30 00:58:34 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Payment_Gateway_SagePay_Management
*
* @see Asset_Management
*
* @author  Anh Ta <ata@squiz.co.uk>
* @version $Revision: 1.8 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Payment_Gateway_SagePay_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Payment_Gateway_SagePay_Management(&$pm)
	{
		$this->Asset_Management($pm);
		
		//include the SagePay_Processor class to get its supported card types. With this, we need only 1 place to change the list of supported card types and do not need to include this file in processing stage
		require_once dirname(__FILE__).'/sagepay_processor.inc';
		$supported_card_types = SagePay_Processor::getCardTypes();
		$default_card_types = implode('|', array_keys($supported_card_types));
		
		$this->vars = Array(
						'protocol'				=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> '2.23',
													'description'	=> 'The Sage Pay Direct Protocol (2.23 is the version when this payment gateway is created)',
												   ),
						'vendor'				=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> '',
													'description'	=> 'Vendor Login Name that is used to authenticate your site',
												   ),
						'timeout'				=> Array(
													'added'			=> '0.1',
													'type'			=> 'int',
													'default'		=> '60',
													'description'	=> 'How long to wait for a response, in seconds, before returning an error',
													'parameters'	=> Array(
																		'allow_negative'	=> FALSE,
																		'allow_empty'		=> FALSE,
																	   ),
												   ),
						'currency'				=> Array(
													'added'			=> '0.1',
													'type'			=> 'selection',
													'default'		=> 'GBP',
													'description'	=> 'The currency used in all the e-commerce transactions',
													'parameters'	=> Array(
																		'multiple'		=> FALSE,
																		'allow_empty'	=> FALSE,
																		'options'		=> Array(
																							'GBP' => 'GBP',
																							'AUD' => 'AUD',
																							'USD' => 'USD',
																							'EUR' => 'EUR',
																							'JPY' => 'JPY',
																							'CAD' => 'CAD',
																						   ),
																	   ),
												   ),
						'description'			=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> 'The total amount',
													'description'	=> 'The free text description of goods or services being purchased',
												   ),
			'delivery_addr_is_billing_addr'		=> Array(
													'added'			=> '0.1',
													'type'			=> 'boolean',
													'default'		=> FALSE,
													'description'	=> 'The flag to indicate if delivery address is billing address',
													'parameters'	=> Array(
																		'allow_empty'	=> FALSE,
																	   ),
												   ),
						'hide_delivery_addr'	=> Array(
													'added'			=> '0.1',
													'type'			=> 'boolean',
													'default'		=> FALSE,
													'description'	=> 'The flag to indicate if delivery address is hidden',
													'parameters'	=> Array(
																		'allow_empty'	=> FALSE,
																	   ),
												   ),
						'supported_card_types'	=> Array(
													'added'			=> '0.1',
													'type'			=> 'selection',
													'default'		=> $default_card_types,
													'description'	=> 'The card types supported by this payment gateway',
													'parameters'	=> Array(
																		'multiple'		=> TRUE,
																		'allow_empty'	=> FALSE,
																		'options'		=> $supported_card_types, 
																	   ),
												   ),
						'threeds_enabled'		=> Array(
													'added'			=> '0.1',
													'type'			=> 'boolean',
													'default'		=> TRUE,
													'description'	=> 'The flag to enable 3-D Secure check for the transactions',
													'parameters'	=> Array(
																		'allow_empty'	=> FALSE,
																	   ),
												   ),
						'avscv2_enabled'		=> Array(
													'added'			=> '0.1',
													'type'			=> 'boolean',
													'default'		=> TRUE,
													'description'	=> 'The flag to enable AVS/CV2 check for the transactions',
													'parameters'	=> Array(
																		'allow_empty'	=> FALSE,
																	   ),
												   ),
						'giftaid_var_name'		=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> '',
													'description'	=> 'The pass through variable name of Gift Aid',
												   ),
						'firstname_var_name'	=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> '',
													'description'	=> 'The pass through variable name of First Name',
												   ),
						'surname_var_name'		=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> '',
													'description'	=> 'The pass through variable name of Surname',
												   ),
						'addr1_var_name'		=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> '',
													'description'	=> 'The pass through variable name of Address 1',
												   ),
						'addr2_var_name'		=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> '',
													'description'	=> 'The pass through variable name of Address 2',
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
						'run_mode'				=> Array(
													'added'			=> '0.1',
													'type'			=> 'selection',
													'default'		=> 'Simulator',
													'description'	=> 'The run mode of this Payment Gateway',
													'parameters'	=> Array(
																		'multiple'		=> FALSE,
																		'allow_empty'	=> FALSE,
																		'options'		=> Array(
																							'Simulator' => 'Simulator',
																							'Test' 		=> 'Test',
																							'Live' 		=> 'Live',
																						   ),
																	   ),
												   ),
					  );

	}//end constructor


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
