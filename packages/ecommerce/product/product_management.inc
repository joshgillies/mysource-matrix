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
* $Id: product_management.inc,v 1.1 2005/01/12 23:57:01 lwright Exp $
* $Name: not supported by cvs2svn $
*/


require_once  SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Product_Management
*
* @see Asset_Management
*
* @author  David Heppell <dheppell@squiz.net>
* @version $Revision: 1.1 $
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
		
		//images for products will be linked assets so dont include her
		//Also quantity is not an issue at the moment
		$this->vars = array(
			'name' => array(
									'added'			=> '0.1',
									'type'			=> 'text',
									'default'		=> '',
									'description'   => 'The name of the Product that this asset represents'
							),
			
			'price' => array(
							 'added'		=> '0.1',
							 'type'			=> 'float',
							 'default'		=> 0.0,
							 'description'  => 'Sale value of this Product'
							 ),
			//keep note on the usefulness of this
			//'id' => array(),
			
			/*				
			'quantity' => array(
								'added'			=> '0.1',
								'type'			=> 'int',
								'default'		=> 0,
								'description'	=> 'Quantity of the item on hand'
								),
			*/
			'description' => array(
								   'added'			=> '0.1',
								   'type'			=> 'text',
								   'default'		=> '',
								   'description'	=> 'Contains the long description of the product'
								   ),
							
			'short_description' => array(
										 'added'		=> '0.1',
										 'type'			=> 'text',
										 'default'		=> '',
										 'description'  => 'Short description of Procudt for small display space'
										 ),
			'details' => array(
							   'added'		=> '0.1',
							   'type'		=> 'text',
							   'default'	=> '',
							   'description'=> 'Detailed specifications of the Product'
							   ),
			
			'display_format' => array(
									  'added'		=> '0.1',
									  'type'		=> 'wysiwyg',
									  'default'		=> ''
									  )
		);
			
	}//end Product_Management()
	
	
}//end class
?>
