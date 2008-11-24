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
* $Id: tool_import_bmail_user_edit_fns.inc,v 1.3 2008/11/24 22:28:21 mbrydon Exp $
*
*/


require_once SQ_SYSTEM_ROOT.'/core/assets/system/tool/tool_edit_fns.inc';

/**
* Tool_Import_Bmail_User_Edit_Fns
*
* @author Nathan de Vries <ndvries@squiz.net>
* @author Rayn Ong <rong@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage bulkmail
*/
class Tool_Import_Bmail_User_Edit_Fns extends Tool_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Tool_Import_Bmail_User_Edit_Fns()
	{
		$this->Tool_Edit_Fns();

	}//end constructor


}//end class
