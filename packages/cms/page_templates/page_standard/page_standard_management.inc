<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: page_standard_management.inc,v 1.6 2003/09/26 05:26:42 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Standard_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Standard_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Page_Standard_Management(&$pm)
	{
		$this->Asset_Management($pm);

	}//end constructor

}//end class

?>