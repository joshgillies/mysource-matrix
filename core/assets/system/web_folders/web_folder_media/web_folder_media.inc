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
* $Id: web_folder_media.inc,v 1.3 2006/12/05 23:41:49 bcaldwell Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/web_folders/web_folder/web_folder.inc';

/**
* Web_Folder_Media
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Web_Folder_Media extends Web_Folder
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Web_Folder_Media($assetid=0)
	{
		$this->Web_Folder($assetid);

	}//end constructor


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Web_Folder::_getAllowLinks()
	*/
	function _getAllowedLinks()
	{
		$allowed_links = parent::_getAllowedLinks();
		$allowed_links[SQ_LINK_TYPE_1]['file'] = Array('card' => 'M', 'exclusive' => FALSE);
		$allowed_links[SQ_LINK_TYPE_2]['file'] = Array('card' => 'M', 'exclusive' => FALSE);
		return $allowed_links;

	}//end _getAllowedLinks()


}//end class

?>
