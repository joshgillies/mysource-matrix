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
* $Id: tool_structured_file_import_management.inc,v 1.5 2012/08/30 00:58:50 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Tool_Structured_File_Import_Management
*
* @see Asset_Management
*
* @author  Darren McKee <dmckee@squiz.net>
* @version $Revision: 1.5 $
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
