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
* $Id: tool_delete_bmail_users_edit_fns.inc,v 1.2.2.2 2008/06/13 01:58:40 mbrydon Exp $
*
*/


require_once SQ_SYSTEM_ROOT.'/core/assets/system/tool/tool_edit_fns.inc';

/**
* Delete Bulkmail Users Tool - Asset Edit Functions
*
* @author Mark Brydon <mbrydon@squiz.net>
* @version $Revision: 1.2.2.2 $
* @package MySource_Matrix_Packages
* @subpackage bulkmail
*/
class Tool_Delete_Bmail_Users_Edit_Fns extends Tool_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Tool_Delete_Bmail_Users_Edit_Fns()
	{
		$this->Tool_Edit_Fns();

	}//end constructor


}//end class
