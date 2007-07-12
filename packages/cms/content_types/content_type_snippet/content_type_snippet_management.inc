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
* $Id: content_type_snippet_management.inc,v 1.1 2007/07/12 01:13:12 rong Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Content_Type_Raw_HTML_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Content_Type_Snippet_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Content_Type_Snippet_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars['view_style'] = Array(
										'type'		=> 'text',
										'default'	=> 'source',
										'added'		=> '0.1',
									);

	}//end constructor


}//end class
?>