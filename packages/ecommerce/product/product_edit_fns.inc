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
 * $Id: product_edit_fns.inc,v 1.3 2005/01/26 09:49:21 brobertson Exp $
 * $Name: not supported by cvs2svn $
 */


require_once SQ_CORE_PACKAGE_PATH.'/page/page_edit_fns.inc';

/**
* Product_Edit_Fns
*
* Purpose
*
*
*
* @author  David Heppell <dheppell@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Product_Edit_Fns extends Page_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function Product_Edit_Fns()
	{
		$this->Page_Edit_Fns();
		
	}//end constructor

	
	/**
	* Paint the interface for editing the product price
	*
	* @param object Asset				$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintProductPrice(&$asset, &$o, $prefix)
	{
		$price_obj = $asset->attr('price_obj');
		$wa = $asset->writeAccess('attributes');
		$price_obj->paintBackend($o, $prefix, $wa);
		return $wa;

	}//end paintProductPrice()


	/**
	* Process the interface for editing the product price
	*
	* @param object Asset				$asset	the asset to which we belong
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processProductPrice(&$asset, &$o, $prefix)
	{
		$price_obj = $asset->attr('price_obj');
		if (!$price_obj->processBackend($o, $prefix)) return false;
		return $asset->setAttrValue('price_obj', $price_obj);

	}// end processProductPrice()

}//end class


?>