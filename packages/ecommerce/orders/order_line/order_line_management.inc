<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: order_line_management.inc,v 1.2 2005/04/29 05:39:36 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Order_Line_Management
*
* @see Asset_Management
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.2 $
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