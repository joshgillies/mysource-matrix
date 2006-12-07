<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +--------------------------------------------------------------------+
*
* $Id: tool_clear_squid_cache_management.inc,v 1.2 2006/12/07 02:54:14 bcaldwell Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Tool_Clear_Squid_Cache_Management
*
* @see Asset_Management
*
* @author Rayn Ong <rong@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage squid
*/
class Tool_Clear_Squid_Cache_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Tool_Clear_Squid_Cache_Management(&$pm)
	{
		$this->Asset_Management($pm);

	}//end constructor


}//end class

?>