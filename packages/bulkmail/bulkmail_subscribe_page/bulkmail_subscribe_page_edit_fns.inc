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
* $Id: bulkmail_subscribe_page_edit_fns.inc,v 1.1.1.1 2005/10/25 04:53:43 ndvries Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/cms/page_templates/page_user_group_manager/page_user_group_manager_edit_fns.inc';

/**
* Bulkmail_Subscribe_Page_Edit_Fns
*
* Purpose
*
*
* @author  Nathan de Vries <ndvries@squiz.net>
* @author  Rayne Ong <rong@squiz.net>
* @version $Revision: 1.1.1.1 $
* @package MySource_Matrix_Packages
* @subpackage bulkmail
*/
class Bulkmail_Subscribe_Page_Edit_Fns extends Page_User_Group_Manager_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Bulkmail_Subscribe_Page_Edit_Fns()
	{
		$this->Page_User_Group_Manager_Edit_Fns();

	}//end constructor


}//end class

?>
