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
* $Id: order_line.inc,v 1.1 2005/01/21 00:05:06 lwright Exp $
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
* @version $Revision: 1.1 $
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
	
	/**
	* Create link for this asset
	 *
	 * Called when a form is processed and a link to this order is made
	 * to the ecom form
	 *
	 * @access pulic
	 */
	function create(&$link)
	{
		if (!($linkid = parent::create($link))) {
			return 0;
		}
		
		return $linkid;
		
	}//end create()
	
	
}//end class

?>