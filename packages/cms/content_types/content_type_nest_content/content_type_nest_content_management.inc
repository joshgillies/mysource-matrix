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
* $Id: content_type_nest_content_management.inc,v 1.11 2006/12/07 00:04:16 emcdonald Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Content_Type_Nest_Content_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.11 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Content_Type_Nest_Content_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Content_Type_Nest_Content_Management(&$pm)
	{
		$this->Asset_Management($pm);

	}//end constructor


}//end class

?>