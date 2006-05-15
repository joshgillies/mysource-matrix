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
* $Id: structured_asset_info.inc,v 1.1 2006/05/15 06:13:31 emcdonald Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/report/report.inc';

/**
* Structured_Asset_Info
*
* Reports assets according to their current status and the time of their last status change
*
*
* @author  Elden McDonald <emcdonald@squiz.net>
* @author  Robert Howard <rhoward@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Structured_Asset_Info extends Report
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Structured_Asset_Info($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		$this->Report($assetid);

	}//end constructor


}//end class

?>
