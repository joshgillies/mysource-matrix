<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: content_type_nest_content_management.inc,v 1.10 2005/04/29 05:40:28 gsherwood Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Content_Type_Nest_Content_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.10 $
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