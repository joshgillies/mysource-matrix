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
* $Id: paypal_business_account_management.inc,v 1.1 2009/03/30 17:58:51 ata Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Paypal_Business_Account_Management
*
* @see Asset_Management
*
* @author  Anh Ta <ata@squiz.co.uk>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Paypal_Business_Account_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
							'name'					=> Array(
																'added'		=> '0.1',
																'type'		=> 'text',
																'default'	=> '',
														    ),
							'account_type'			=> Array(
																'added'			=> '0.1',
																'type'			=> 'selection',
																'default'		=> 'sandbox',
																'parameters'	=> Array(
																							'multiple'		=> FALSE,
																							'allow_empty'	=> FALSE,
																							'options'		=> Array(
																														'live'			=> 'Live',
																														'sandbox'	=> 'Sandbox',
																											   		),
																					   ),
															),
							'account_id'			=> Array(
																'added'		=> '0.1',
																'type'		=> 'text',
																'default'	=> '',
														    ),
							'public_cert_path'		=> Array(
																'added'		=> '0.1',
																'type'		=> 'text',
																'default'	=> '',
														    ),
							'public_cert_id'		=> Array(
																'added'		=> '0.1',
																'type'		=> 'text',
																'default'	=> '',
														    ),
							'private_key_path'		=> Array(
																'added'		=> '0.1',
																'type'		=> 'text',
																'default'	=> '',
														    ),
							'paypal_cert_path'		=> Array(
																'added'		=> '0.1',
																'type'		=> 'text',
																'default'	=> '',
														    ),
							'error_email'			=> Array(
																'added'		=> '0.1',
																'type'		=> 'email',
																'default'	=> '',
																'is_admin'	=> 0,
															),
							'send_email_on_error'	=> Array(
																'added'			=> '0.1',
																'type'			=> 'boolean',
																'default'		=> TRUE,
																'parameters'	=> Array(
																							'allow_empty'	=> FALSE,
																						),
															),
						   );

	}//end constructor


}//end class
?>