<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: order_line_management.inc,v 1.1 2005/01/21 00:05:06 lwright Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Order_Line_Management
*
* @see Asset_Management
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.1 $
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