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
* $Id: paypal_configuration_management.inc,v 1.4 2012/08/30 00:58:34 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Paypal_Configuration_Management
*
* @see Asset_Management
*
* @author  Anh Ta <ata@squiz.co.uk>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Paypal_Configuration_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
							'name'						=> Array(
																	'added'			=> '0.1',
																	'type'			=> 'text',
																	'default'		=> '',
																	'is_admin'		=> FALSE,
																	'is_contextable'=> TRUE,
															    ),
							'custom_param_secret_str'	=> Array(
																	'added'			=> '0.1',
																	'type'			=> 'text',
																	'default'		=> '8f93f16844b964a162ab436ddb82c273',
															    	'description'	=> 'The secret string that is used to calculate the checksum of custom variable so that no malicious users can modify it',
															    ),
						   );

	}//end constructor


}//end class
?>
