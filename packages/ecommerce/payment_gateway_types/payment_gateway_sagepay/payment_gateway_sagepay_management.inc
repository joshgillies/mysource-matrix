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
* $Id: payment_gateway_sagepay_management.inc,v 1.1 2009/06/22 09:32:12 ata Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Payment_Gateway_SagePay_Management
*
* @see Asset_Management
*
* @author  Anh Ta <ata@squiz.co.uk>
* @version $Revision: 1.1 $
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
																						   ),
																	   ),
												   ),
						'description'			=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> 'The total amount',
													'description'	=> 'The free text description of goods or services being purchased',
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