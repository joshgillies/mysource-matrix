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
* $Id: tool_import_assets_from_xml_management.inc,v 1.2 2012/08/30 00:58:50 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Tool_Import_Assets_From_Xml_Management
*
* @see Asset_Management
*
* @author  Ash Karelia <akarelia@squiz.com.au>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage import_tools
*/
class Tool_Import_Assets_From_Xml_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Tool_Import_Assets_From_Xml_Management(&$pm)
	{
		$this->Asset_Management($pm);

	}//end constructor

    /**
	* Gets a list of all files that the bodycopy wants web accessable
	*
	* @return array
	* @access private
	*/
	function _getFileList()
	{
		return Array(
				'files/example.xml',
			   );

	}//end _getFileList()

}//end class

?>
