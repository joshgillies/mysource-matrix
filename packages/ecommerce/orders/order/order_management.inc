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
* $Id: order_management.inc,v 1.6 2009/07/13 11:14:55 ata Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Order_Management
*
* @see Asset_Management
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.6 $
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
												   ),
						'billing_name'		=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> '',
													'description'	=> 'The billing name returned by the Payment Gateway',
												   ),
						'billing_addr'		=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> '',
													'description'	=> 'The billing address returned by the Payment Gateway',
												   ),
						'delivery_name'		=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> '',
													'description'	=> 'The delivery name returned by the Payment Gateway',
												   ),
						'delivery_addr'		=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> '',
													'description'	=> 'The delivery address returned by the Payment Gateway',
												   ),
		);

	}//end Order_Management()


}//end class

?>