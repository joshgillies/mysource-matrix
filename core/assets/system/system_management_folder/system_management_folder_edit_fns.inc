<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: system_management_folder_edit_fns.inc,v 1.1 2003/10/20 07:23:49 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/folder/folder_edit_fns.inc';

/**
* System_Management_Folder_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class System_Management_Folder_Edit_Fns extends Folder_Edit_Fns
{
	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function System_Management_Folder_Edit_Fns($assetid=0)
	{
		$this->Folder_Edit_Fns($assetid);
		unset($this->static_screens['permissions']);
		unset($this->static_screens['workflow']);
		unset($this->static_screens['metadataSchemas']);
		unset($this->static_screens['metadata']);
		unset($this->static_screens['dependants']);
		unset($this->static_screens['linking']);
		unset($this->static_screens['settings']);
		unset($this->static_screens['history']);

	}


}//end class
?>
