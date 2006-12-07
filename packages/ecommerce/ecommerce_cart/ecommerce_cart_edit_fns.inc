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
* $Id: ecommerce_cart_edit_fns.inc,v 1.9 2006/12/07 05:26:28 bcaldwell Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/cms/listing_engine/listing_engine_edit_fns.inc';

/**
* Ecommerce_Cart_Edit_Fns
*
* Edit Functions for the shopping cart
*
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.9 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Ecommerce_Cart_Edit_Fns extends Listing_Engine_Edit_Fns
{


	/**
	* Constructor
	*
	* @return void
	* @access public
	*/
	function Ecommerce_Cart_Edit_Fns()
	{
		$this->Listing_Engine_Edit_Fns();

	}//end constructor


}//end class

?>