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
* $Id: content_type_raw_html_management.inc,v 1.3 2005/02/11 04:58:24 tbarrett Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Content_Type_Raw_HTML_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Content_Type_Raw_HTML_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Content_Type_Raw_HTML_Management(&$pm)
	{
		$this->Asset_Management($pm);

	}//end constructor


	/**
	* Gets a list of all metadata files that we want web accessable
	*
	* @access private
	* @return array(string)
	*/
	function _getFileList()
	{
		return Array(
					 'js/raw_html_insert_keyword.js',
					 );

	}//end _getFileList()

}//end class

?>