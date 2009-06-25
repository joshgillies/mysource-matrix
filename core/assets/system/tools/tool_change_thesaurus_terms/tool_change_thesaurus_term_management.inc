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
* $Id: tool_change_thesaurus_term_management.inc,v 1.1.2.1 2009/06/25 06:55:34 bpearson Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Tool_Change_Thesaurus_Term_Management
*
* @see Asset_Management
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.1.2.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Tool_Change_Thesaurus_Term_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Tool_Change_Thesaurus_Term_Management(&$pm)
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
