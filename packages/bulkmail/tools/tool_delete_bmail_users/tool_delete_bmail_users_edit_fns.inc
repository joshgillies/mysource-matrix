<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: tool_delete_bmail_users_edit_fns.inc,v 1.4 2012/08/30 00:57:13 ewang Exp $
*
*/


require_once SQ_SYSTEM_ROOT.'/core/assets/system/tool/tool_edit_fns.inc';

/**
* Delete Bulkmail Users Tool - Asset Edit Functions
*
* @author Mark Brydon <mbrydon@squiz.net>
* @version $Revision: 1.4 $
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
