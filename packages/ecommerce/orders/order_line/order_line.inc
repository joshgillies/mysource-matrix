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
* $Id: order_line.inc,v 1.5 2006/12/07 05:26:29 bcaldwell Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Order_Line
*
* Purpose
*
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Order_Line extends Asset
{


	/**
	* Constructor
	*
	* @param string	$assetid	the assetid of the asset to load into this object
	*
	* @return void
	* @access public
	*/
	function Order_Line($assetid=0)
	{
		$this->Asset($assetid);

	}//end Order_Line()


	/**
	* Calculate a simple line total
	*
	* @return double
	* @access public
	*/
	function lineTotal()
	{
		return $this->attr('price') * $this->attr('quantity');

	}//end lineTotal()


	/**
	* Returns a reference to the product asset in this line item
	*
	* @return object Product
	* @access public
	*/
	function &getProductAsset()
	{
		return $GLOBALS['SQ_SYSTEM']->am->getAsset($this->attr('product_assetid'));

	}//end getProductAsset()


}//end class

?>