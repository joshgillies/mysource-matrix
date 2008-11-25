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
* $Id: order_edit_fns.inc,v 1.5 2008/11/25 00:48:05 mbrydon Exp $
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
* @version $Revision: 1.5 $
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