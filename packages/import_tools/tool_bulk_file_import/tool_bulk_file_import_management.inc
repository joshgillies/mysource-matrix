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
* $Id: tool_bulk_file_import_management.inc,v 1.3 2006/12/06 22:21:22 emcdonald Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Tool_Bulk_File_Import_Management
*
* @see Asset_Management
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage import_tools
*/
class Tool_Bulk_File_Import_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Tool_Bulk_File_Import_Management(&$pm)
	{
		$this->Asset_Management($pm);

	}//end constructor


}//end class

?>
