<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: root_folder_management.inc,v 1.9 2003/09/26 05:26:28 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Root_Folder_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Root_Folder_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Root_Folder_Management(&$pm)
	{
		$this->Asset_Management($pm);

		// only need to have the default here because the other information will be 
		// gleaned from the 'folder' type
		$this->vars = Array('name' => Array('default' => '/'));

	}//end constructor

}//end class
?>
