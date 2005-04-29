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
* $Id: ecommerce_cart_edit_fns.inc,v 1.4 2005/04/29 05:39:36 gsherwood Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page_edit_fns.inc';

/**
* Ecommerce_Cart_Edit_Fns
*
* Purpose
*
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Ecommerce_Cart_Edit_Fns extends Page_Edit_Fns
{
	
	
	/**
	* Constructor
	*
	* @return void
	* @access public
	*/
	function Ecommerce_Cart_Edit_Fns()
	{
		$this->Page_Edit_Fns();
		
	}//end Ecommerce_Cart_Edit_Fns() 
	

}//end class

?>