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
* $Id: design_area_ecommerce_cart_management.inc,v 1.2 2005/04/29 05:39:36 gsherwood Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Area_Ecommerce_Cart_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Ecommerce_Cart_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Design_Area_Ecommerce_Cart_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
							'cart' => Array(
								'added'			=> '0.1',
								'type'			=> 'assetid',
								'default'		=> 0,
								'description'	=> 'The assetid of the cart that we are connected with',
							),
							'ecom_form' => Array(
								'added'			=> '0.1',
								'type'			=> 'assetid',
								'default'		=> 0,
								'description'	=> 'The assetid of the ecommerce form that we are connected with',
							),
							'show_price' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'boolean',
								'default'		=> true,
								'description'	=> 'Whether to show the total cost in the design area',
								'parameters'	=> Array(
									'allow_empty' => false,
								),
								'update_fn'		=> '',
			 					),
							);

	}//end constructor


}//end class
?>
