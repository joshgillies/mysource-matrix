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
 * $Id: product_edit_fns.inc,v 1.9 2008/11/25 00:48:06 mbrydon Exp $
 *
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
* @version $Revision: 1.9 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Product_Edit_Fns extends Page_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

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