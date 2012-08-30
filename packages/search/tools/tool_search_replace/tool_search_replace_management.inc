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
* $Id: tool_search_replace_management.inc,v 1.7 2012/08/30 00:59:16 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Tool_Search_Replace_Management
*
* @see Asset_Management
*
* @author  Nathan de Vries <ndvries@squiz.net>
* @author  David Heppell <dheppell@squiz.net>
* @version $Revision: 1.7 $
* @package MySource_Matrix_Packages
* @subpackage search
*/
class Tool_Search_Replace_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Tool_Search_Replace_Management(&$pm)
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
		return Array(
				'js/search_and_replace.js',
				'css/search_and_replace.css',
			   );

	}//end _getFileList()


}//end class

?>
