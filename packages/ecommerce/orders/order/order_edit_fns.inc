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
* $Id: order_edit_fns.inc,v 1.4 2006/12/07 05:26:29 bcaldwell Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/cms/form/form_submission/form_submission_edit_fns.inc';

/**
* Order_Edit_Fns
*
* Purpose
*
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Order_Edit_Fns extends Form_Submission_Edit_Fns
{


	/**
	* Constructor
	*
	* @return void
	* @access public
	*/
	function Order_Edit_Fns()
	{
		$this->Asset_Edit_Fns();

	}//end Order_Edit_Fns()


}//end class

?>