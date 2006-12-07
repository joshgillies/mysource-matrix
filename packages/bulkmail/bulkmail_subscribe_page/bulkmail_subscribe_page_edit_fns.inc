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
* $Id: bulkmail_subscribe_page_edit_fns.inc,v 1.2 2006/12/07 05:55:57 bcaldwell Exp $
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
* @version $Revision: 1.2 $
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
