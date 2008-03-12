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
* $Id: tool_clear_squid_cache_management.inc,v 1.2.8.1 2008/03/12 04:05:13 lwright Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Tool_Clear_Squid_Cache_Management
*
* @see Asset_Management
*
* @author Rayn Ong <rong@squiz.net>
* @version $Revision: 1.2.8.1 $
* @package MySource_Matrix_Packages
* @subpackage squid
*/
class Tool_Clear_Squid_Cache_Management extends Asset_Management
{


	/**
	* Constructor
	*
	* @param Package_Manager	$pm	The package manager installing this asset
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);

	}//end constructor


}//end class

?>