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
* $Id: ecommerce_cart_management.inc,v 1.4 2005/04/29 05:39:36 gsherwood Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Ecommerce_Cart_Management
*
* @see Asset_Management
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Ecommerce_Cart_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Ecommerce_Cart_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array('name' => Array(
												'added'       => '0.0.1',
												'type'        => 'text',  'default'   => '',
												'description' => 'The full name of the cart',
												'update_fn'   => '',
												),
							'short_name' => Array(
												'added'       => '0.0.1',
												'type'        => 'text',  'default'   => '',
												'description' => 'The short name of the cart',
												'update_fn'   => '',
												),
							'ecom_form_id' => Array(
												'added'       => '0.0.1',
												'type'        => 'assetid',
												'default'     => 0,
												'description' => 'The assetid of the ecommerce form that we are connected with',			
												),					
							'show_price' => Array(
												'added'       => '0.0.1',
												'type'        => 'boolean',
												'default'     => true,
												'description' => 'Whether to show the price columns',
												'parameters'  => Array(
													'allow_empty' => false,
												),
												'update_fn'   => '',							 
							 					),
							'checkout_text' => Array(
												'added'       => '0.0.1',
												'type'        => 'text',  'default'   => 'Proceed to Checkout',
												'description' => 'The text on the "checkout" button',
												'update_fn'   => '',
												),
							);

	}//end constructor


}//end class

?>