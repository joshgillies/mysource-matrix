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
* $Id: product_management.inc,v 1.9 2006/12/07 05:26:31 bcaldwell Exp $
*
*/


require_once  SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Product_Management
*
* @see Asset_Management
*
* @author  David Heppell <dheppell@squiz.net>
* @version $Revision: 1.9 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Product_Management extends Asset_Management
{

	/**
	* Constructor
	*
	* @param object Package_Manager	&$pm	the package manager to use
	*
	* @return void
	* @access public
	*/
	function Product_Management(&$pm)
	{
		$this->Asset_Management($pm);

		// include price class so unserialize() doesn't barf
		require_once SQ_PACKAGES_PATH.'/ecommerce/price/product_price.inc';

		// images for products will be linked assets so dont include here
		// Also quantity is not an issue at the moment
		// NB: Price is 0 (zero) if free, or -1 if unknown
		$this->vars = array(
						'name' => array(
							'added'			=> '0.1',
							'type'			=> 'text',
							'default'		=> '',
							'description'	=> 'The name of the Product that this asset represents',
						),
						'price' => array(
							'added'			=> '0.1',
							'type'			=> 'float',
							'default'		=> 0.0,
							'description'	=> 'Sale value of this Product',
							'is_admin'		=> false,
						),
						'price_obj' => array(
							'added'			=> '0.1',
							'type'			=> 'serialise',
							'default'		=> new Product_Price(),
							'description'	=> 'Proper object for manipulation of the price',
						),
			/*
						'quantity' => array(
							'added'			=> '0.1',
							'type'			=> 'int',
							'default'		=> 0,
							'description'	=> 'Quantity of the item on hand',
						),
			*/
						'description' => array(
							'added'			=> '0.1',
							'type'			=> 'text',
							'default'		=> '',
							'description'	=> 'Long description of the product',
							'is_admin'		=> false,
						),
						'short_description' => array(
							'added'			=> '0.1',
							'type'			=> 'text',
							'default'		=> '',
							'description'	=> 'Short description of product for small display space',
							'is_admin'		=> false,
						),
						'details' => array(
							'added'			=> '0.1',
							'type'			=> 'text',
							'default'		=> '',
							'description'	=> 'Detailed specifications of the Product',
							'is_admin'		=> false,
						),
		);

	}//end Product_Management()


}//end class
?>
