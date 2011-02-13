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
* $Id: tool_export_assets_to_xml_management.inc,v 1.1 2011/02/13 23:42:09 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Tool_Import_Assets_From_Xml_Management
*
* @see Asset_Management
*
* @author  Edison Wang <ewang@squiz.com.au>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage import_tools
*/
class Tool_Export_Assets_To_Xml_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Tool_Export_Assets_To_Xml_Management(&$pm)
	{
		$this->Asset_Management($pm);

	}//end constructor

  

}//end class

?>
