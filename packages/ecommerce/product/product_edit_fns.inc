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
 * $Id: product_edit_fns.inc,v 1.5 2005/04/29 05:39:37 gsherwood Exp $
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
* @version $Revision: 1.5 $
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
		$price_obj->paintBackend($o, $prefix.'_'.$asset->vars['price_obj']['attrid'], $wa);
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
		if (!$price_obj->processBackend($o, $prefix.'_'.$asset->vars['price_obj']['attrid'])) return false;
		return $asset->setAttrValue('price_obj', $price_obj);

	}// end processProductPrice()

}//end class


?>