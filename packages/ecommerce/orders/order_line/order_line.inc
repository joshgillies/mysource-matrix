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
* $Id: order_line.inc,v 1.3 2005/04/29 05:39:36 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Order_Line
*
* Purpose
*
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.3 $
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