<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: tool_structured_file_import_edit_fns.inc,v 1.5 2012/08/30 00:58:50 ewang Exp $
*
*/


require_once SQ_SYSTEM_ROOT.'/core/assets/system/tool/tool_edit_fns.inc';

/**
* Tool_Structured_File_Import_Edit_Fns
*
* @author  Darren McKee <dmckee@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage search
*/
class Tool_Structured_File_Import_Edit_Fns extends Tool_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Tool_Structured_File_Import_Edit_Fns()
	{
		$this->Tool_Edit_Fns();

	}//end constructor


}//end class

?>
