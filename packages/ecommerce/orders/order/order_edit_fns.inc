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
* $Id: order_edit_fns.inc,v 1.2 2005/04/29 05:39:36 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_PACKAGES_PATH.'/cms/form/form_submission/form_submission_edit_fns.inc';

/**
* Order_Edit_Fns
*
* Purpose
*
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.2 $
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