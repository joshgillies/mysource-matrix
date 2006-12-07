<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +--------------------------------------------------------------------+
*
* $Id: order_line_management.inc,v 1.4 2006/12/07 05:26:29 bcaldwell Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Order_Line_Management
*
* @see Asset_Management
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Order_Line_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Order_Line_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
			'product_assetid' => Array(
				'added'       => '0.0.1',
				'type'        => 'assetid',  'default'   => '',
				'description' => 'The assetid of the product being ordered',
				'update_fn'   => '',
			),

			'quantity' => Array(
				'added'       => '0.0.1',
				'type'        => 'int',  'default'   => 1,
				'description' => 'The quantity being ordered',
				'update_fn'   => '',
			),

			'price' => Array(
				'added'       => '0.0.1',
				'type'        => 'float',  'default'   => 0.0,
				'description' => 'The price of the product being ordered',
				'update_fn'   => '',
			),
		);

	}//end Order_Line_Management()


}//end class

?>