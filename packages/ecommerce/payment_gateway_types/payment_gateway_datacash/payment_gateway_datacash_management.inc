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
* $Id: payment_gateway_datacash_management.inc,v 1.1 2009/06/22 09:27:23 ata Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Payment_Gateway_Datacash_Management
*
* @see Asset_Management
*
* @author  Anh Ta <ata@squiz.co.uk>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Payment_Gateway_Datacash_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Payment_Gateway_Datacash_Management(&$pm)
	{
		$this->Asset_Management($pm);
		$this->vars = Array(
						'client'				=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> '',
													'description'	=> 'The client ID of the Datacash Account used for the e-commerce transaction',
												   ),
						'password'				=> Array(
													'added'			=> '0.1',
													'type'			=> 'password',
													'default'		=> '',
													'description'	=> 'The password of the Datacash Account used for the e-commerce transaction',
												   ),
						'datacash_api_path'		=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> '/usr/bin/DataCash.php',
											    	'description'	=> 'The path to the Datacash PHP API',
												   ),
						'cardinfo_dir_path'		=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> '/usr/bin/CardInfoBins',
											    	'description'	=> 'The path to the directory of CardInfo files',
												   ),
						'timeout'				=> Array(
													'added'			=> '0.1',
													'type'			=> 'int',
													'default'		=> '60',
													'description'	=> 'How long to wait for a response, in seconds, before returning an error',
													'parameters'	=> Array(
																		'allow_negative'	=> FALSE,
																		'allow_empty'		=> TRUE,
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
																						   ),
																	   ),
												   ),
						'merchant_url'			=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> '',
													'description'	=> 'The URL of the website on which the payment is being made',
												   ),
						'purchase_desc'			=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> 'The total amount',
													'description'	=> 'A short description of what has been purchased. This is shown to the card holder by the ACS',
												   ),
						'nonthreed_card_types'	=> Array(
													'added'			=> '0.1',
													'type'			=> 'selection',
													'default'		=> 'Solo',
													'description'	=> 'The currency used in all the e-commerce transactions',
													'parameters'	=> Array(
																		'multiple'		=> TRUE,
																		'allow_empty'	=> TRUE,
																		'options'		=> Array(
																							'Debit Mastercard'	=> 'Debit Mastercard',
																							'Mastercard' 		=> 'Mastercard',
																							'VISA'				=> 'VISA',
																							'VISA Delta'		=> 'VISA Delta',
																							'VISA Electron'		=> 'VISA Electron',
																							'VISA Purchasing'	=> 'VISA Purchasing',
																							'Solo'				=> 'Solo',
																						   ),
																	   ),
												   ),
						'submit_text'			=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> '   Pay   ',
													'description'	=> 'Submit button text',
												   ),
						'reset_text'			=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> 'Reset',
													'description'	=> 'Reset button text',
												   ),
						'cancel_text'			=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> 'Cancel',
													'description'	=> 'Cancel button text',
												   ),
						'test_mode'				=> Array(
													'added'			=> '0.1',
													'type'			=> 'boolean',
													'default'		=> FALSE,
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