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
* $Id: tool_bulk_file_import_edit_fns.inc,v 1.3 2006/12/06 22:21:21 emcdonald Exp $
*
*/


require_once SQ_SYSTEM_ROOT.'/core/assets/system/tool/tool_edit_fns.inc';

/**
* Tool_Bulk_File_Import_Edit_Fns
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage search
*/
class Tool_Bulk_File_Import_Edit_Fns extends Tool_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Tool_Bulk_File_Import_Edit_Fns()
	{
		$this->Tool_Edit_Fns();

	}//end constructor


}//end class

?>
