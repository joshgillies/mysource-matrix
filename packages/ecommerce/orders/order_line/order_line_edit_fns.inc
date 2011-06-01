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
* $Id: order_line_edit_fns.inc,v 1.5 2008/11/25 00:48:05 mbrydon Exp $
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
* @version $Revision: 1.5 $
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