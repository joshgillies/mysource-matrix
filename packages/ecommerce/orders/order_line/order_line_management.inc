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
* $Id: order_line_management.inc,v 1.5 2008/11/25 00:48:05 mbrydon Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Order_Line_Management
*
* @see Asset_Management
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.5 $
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