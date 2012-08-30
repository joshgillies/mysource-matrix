<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: order_line.inc,v 1.8 2012/08/30 00:58:34 ewang Exp $
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
* @version $Revision: 1.8 $
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