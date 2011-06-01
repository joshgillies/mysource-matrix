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
* $Id: tool_structured_file_import_management.inc,v 1.3 2006/12/07 01:09:33 emcdonald Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Tool_Structured_File_Import_Management
*
* @see Asset_Management
*
* @author  Darren McKee <dmckee@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage import_tools
*/
class Tool_Structured_File_Import_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Tool_Structured_File_Import_Management(&$pm)
	{
		$this->Asset_Management($pm);

	}//end constructor


}//end class

?>
