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
* $Id: payment_gateway_dps_management.inc,v 1.2 2012/08/30 00:58:34 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Payment_Gateway_Dps_Management
*
* @see Asset_Management
*
* @author  Edison Wang <ewang@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Payment_Gateway_Dps_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Payment_Gateway_Dps_Management(&$pm)
	{
		$this->Asset_Management($pm);
		$this->vars = Array(
						'client'				=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> '',
													'description'	=> 'The client ID of the DPS Account used for the e-commerce transaction',
												   ),
						'password'				=> Array(
													'added'			=> '0.1',
													'type'			=> 'password',
													'default'		=> '',
													'description'	=> 'The password of the DPS Account used for the e-commerce transaction',
												   ),
						
						'currency'				=> Array(
													'added'			=> '0.1',
													'type'			=> 'selection',
													'default'		=> 'GBP',
													'description'	=> 'The currency used in all the e-commerce transactions. If blank, currency will be determined by the bank account used which is selected using the Username/Password details',
													'parameters'	=> Array(
																		'multiple'		=> FALSE,
																		'allow_empty'	=> TRUE,
																		'options'		=> Array(
												   											'CAD' => 'CAD',
												   											'CHF' => 'CHF',
																							'EUR' => 'EUR',
																							'FRF' => 'FRF',
																							'GBP' => 'GBP',
																							'HKD' => 'HKD',
																							'JPY' => 'JPY',
												   											'NZD' => 'NZD',
												   											'SGD' => 'SGD',
																							'USD' => 'USD',
																							'ZAR' => 'ZAR',
																							'AUD' => 'AUD',
																							'WST' => 'WST',
																							'VUV' => 'VUV',
												   											'TOP' => 'TOP',
																							'SBD' => 'SBD',
																							'PGK' => 'PGK',
																							'MYR' => 'MYR',
																							'WST' => 'WST',
																							'KWD' => 'KWD',
												   											'FJD' => 'FJD',
																						   ),
																	   ),
												   ),
						
						'purchase_desc'			=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> 'The total amount',
													'description'	=> 'A short description of what has been purchased. This is shown to the card holder by the ACS',
												   ),
					
						'avs_standard_policy'	=> Array(
													'added'			=> '0.1',
													'type'			=> 'selection',
													'default'		=> '0',
													'description'	=> 'Address Verification System property',
													'parameters'	=> Array(
																		'multiple'		=> FALSE,
																		'allow_empty'	=> FALSE,
																		'options'		=> Array(
																							'0'	=> 'No',
																							'1'	=> 'Attempt AVS check',
																							'2'	=> 'The transactions needs to be checked by AVS',
																						   ),
																	   ),
												   ),
						'cv2_standard_policy'	=> Array(
													'added'			=> '0.1',
													'type'			=> 'selection',
													'default'		=> '0',
													'description'	=> 'Card Verification Code 2 check',
													'parameters'	=> Array(
																		'multiple'		=> FALSE,
																		'allow_empty'	=> FALSE,
																		'options'		=> Array(
																							'0'	=> 'No',
																							'1'	=> 'Attempt CV2 check',
																						   ),
																	   ),
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
						'addr3_var_name'		=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> '',
													'description'	=> 'The pass through variable name of Address 3',
												   ),
						'addr4_var_name'		=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> '',
													'description'	=> 'The pass through variable name of Address 4',
												   ),
						'postcode_var_name'	=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> '',
													'description'	=> 'The pass through variable name of Postcode',
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
						'amount_not_specified_error'	=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> 'No amount is specified for the payment',
													'description'	=> 'Amount Not Specified Error',
													'is_contextable'	=> TRUE,
												   ),
						'card_number_not_specified_error'	=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> 'No card number is specified for the payment',
													'description'	=> 'Card Number Not Specified Error',
													'is_contextable'	=> TRUE,
												   ),
						'expiry_date_invalid_error'	=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> 'The expiry date (MM/YY) is not valid',
													'description'	=> 'Expiry Date Invalid Error',
													'is_contextable'	=> TRUE,
												   ),
						'start_date_invalid_error'	=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> 'The start date (MM/YY) is not valid',
													'description'	=> 'Start Date Invalid Error',
													'is_contextable'	=> TRUE,
												   ),						
						'issue_number_invalid_error'	=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> 'The issue number is invalid (must be one or two digits long)',
													'description'	=> 'Issue Number Invalid Error',
													'is_contextable'	=> TRUE,
												   ),
						'card_cv2_not_specified_error'	=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> 'No CV2 is specified for the card',
													'description'	=> '>Card Cv2 Not Specified Error',
													'is_contextable'	=> TRUE,
												   ),						
						'card_cv2_invalid_error'	=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> 'The card CV2 is invalid (must be three or four (only American Express Card) digits long)',
													'description'	=> 'Card Cv2 Invalid Error',
													'is_contextable'	=> TRUE,
												   ),
						'street_address_not_specified_error'	=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> 'The address is not specified',
													'description'	=> 'Street Address Not Specified Error',
													'is_contextable'	=> TRUE,
												   ),
						'postcode_not_specified_error'	=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> 'No postcode is specified',
													'description'	=> 'Postcode Not Specified Error',
													'is_contextable'	=> TRUE,
												   ),
						'test_mode'				=> Array(
													'added'			=> '0.1',
													'type'			=> 'boolean',
													'default'		=> TRUE,
													'description'	=> 'Turn this ON for test mode',
													'parameters'	=> Array(
																		'allow_empty'	=> FALSE,
																	   ),
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
