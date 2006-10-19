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
* $Id: tool_bulk_file_import_management.inc,v 1.1 2006/10/19 02:23:48 skim Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Tool_Bulk_File_Import_Management
*
* @see Asset_Management
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.1 $
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
