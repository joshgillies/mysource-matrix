<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: workspace_management.inc,v 1.5 2003/09/26 05:26:29 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/folder/folder_management.inc';

/**
* Workspace_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Workspace_Management extends Folder_Management
{

	/**
	* Constructor
	*
	*/
	function Workspace_Management(&$pm)
	{
		$this->Folder_Management($pm);
	}//end constructor

}//end class
?>
