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
* $Id: tool_mass_clone_management.inc,v 1.2 2006/12/06 00:13:03 bcaldwell Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Tool_Mass_Clone_Management
*
* @see Asset_Management
*
* @author  Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage search
*/
class Tool_Mass_Clone_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Tool_Mass_Clone_Management(&$pm)
	{
		$this->Asset_Management($pm);


	}//end constructor


	/**
	* Returns all files this asset wants installed/updated into the data directory
	*
	* @return array
	* @access private
	*/
	function _getFileList()
	{
		return Array();

	}//end _getFileList()


}//end class

?>
