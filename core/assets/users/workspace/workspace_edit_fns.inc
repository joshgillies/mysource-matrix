<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: workspace_edit_fns.inc,v 1.5 2003/09/26 05:26:29 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/folder/folder_edit_fns.inc';

/**
* Workspace_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Workspace_Edit_Fns extends Folder_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function Workspace_Edit_Fns()
	{
		$this->Folder_Edit_Fns();
	}//end constructor

}//end class
?>
