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
* $Id: tool_export_files_management.inc,v 1.1.2.1 2008/05/26 07:02:52 bpearson Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Tool_Export_Files_User_Management
*
* @see Asset_Management
*
* @author Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.1.2.1 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Tool_Export_Files_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Tool_Export_Files_Management(&$pm)
	{
		$this->Asset_Management($pm);


	}//end constructor


}//end class

?>
