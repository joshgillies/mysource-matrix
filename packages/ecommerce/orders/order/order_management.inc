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
* $Id: order_management.inc,v 1.8 2012/08/30 00:58:34 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Order_Management
*
* @see Asset_Management
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.8 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Order_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Order_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'transaction_id'		=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> '',
													'description'	=> 'The ID of the payment transaction returned by the Payment Gateway',
													'is_admin'		=> FALSE,
												   ),
						'billing_name'		=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> '',
													'description'	=> 'The billing name returned by the Payment Gateway',
													'is_admin'		=> FALSE,
												   ),
						'billing_addr'		=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> '',
													'description'	=> 'The billing address returned by the Payment Gateway',
													'is_admin'		=> FALSE,
												   ),
						'delivery_name'		=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> '',
													'description'	=> 'The delivery name returned by the Payment Gateway',
													'is_admin'		=> FALSE,
												   ),
						'delivery_addr'		=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> '',
													'description'	=> 'The delivery address returned by the Payment Gateway',
													'is_admin'		=> FALSE,
												   ),
						'transaction_response'			=> Array(
													'added'			=> '0.2',
													'type'			=> 'text',
													'default'		=> '',
													'description'	=> 'The response raw data from payment transaction',
													'is_admin'		=> FALSE,
												   ),
		);

	}//end Order_Management()


}//end class

?>