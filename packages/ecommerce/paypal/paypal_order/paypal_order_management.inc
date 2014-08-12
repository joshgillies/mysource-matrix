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
* $Id: paypal_order_management.inc,v 1.2 2012/08/30 00:58:34 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Paypal_Order_Management
*
* @see Asset_Management
*
* @author  Anh Ta <ata@squiz.co.uk>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Paypal_Order_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
							'txn_id'					=> Array(
																	'added'			=> '0.1',
																	'type'			=> 'text',
																	'default'		=> '',
															    ),
							'payment_status'			=> Array(
																	'added'			=> '0.1',
																	'type'			=> 'text',
																	'default'		=> '',
															    ),
							'incomplete_reason'			=> Array(
																	'added'			=> '0.1',
																	'type'			=> 'text',
																	'default'		=> '',
															    ),
							'total_amount'				=> Array(
																	'added'			=> '0.1',
																	'type'			=> 'float',
																	'default'		=> 0,
															    ),
							'currency'					=> Array(
																	'added'			=> '0.1',
																	'type'			=> 'text',
																	'default'		=> '',
															    ),
							'products'					=> Array(
																	'added'			=> '0.1',
																	'type'			=> 'serialise',
																	'default'		=> Array(),
															    ),
							'custom_info'				=> Array(
																	'added'			=> '0.1',
																	'type'			=> 'text',
																	'default'		=> '',
															    ),
						   );

	}//end constructor


}//end class
?>