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
* $Id: order_line_edit_fns.inc,v 1.3 2005/07/27 11:09:45 brobertson Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Order_Line_Edit_Fns
*
* Purpose
*
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Order_Line_Edit_Fns extends Asset_Edit_Fns
{
	
	
	/**
	* Constructor
	*
	* @return void
	* @access public
	*/
	function Order_Line_Edit_Fns()
	{
		$this->Asset_Edit_Fns();
		
	}//end Order_Line_Edit_Fns() 
	

}//end class

?>