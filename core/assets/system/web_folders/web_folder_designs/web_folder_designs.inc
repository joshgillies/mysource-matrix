<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: web_folder_designs.inc,v 1.1 2005/02/07 05:02:51 gsherwood Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/web_folders/web_folder/web_folder.inc';

/**
* Web_Folder_Designs
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Web_Folder_Designs extends Web_Folder
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Web_Folder_Designs($assetid=0)
	{
		$this->Web_Folder($assetid);

	}//end constructor


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @see Web_Folder::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks()
	{
		$allowed_links = parent::_getAllowedLinks();
		$allowed_links[SQ_LINK_TYPE_1]['design'] = Array('card' => 'M', 'exclusive' => false);
		$allowed_links[SQ_LINK_TYPE_2]['design'] = Array('card' => 'M', 'exclusive' => false);
		return $allowed_links;

	}//end _getAllowedLinks()


}//end class

?>
