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
* $Id: inbox_management.inc,v 1.14 2013/09/24 01:20:42 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Inbox_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.14 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Inbox_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Inbox_Management(&$pm)
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
				'js/inbox.js',
			   );

	}//end _getFileList()


}//end class
?>
