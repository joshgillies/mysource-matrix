<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: system_management_folder_management.inc,v 1.1 2003/10/20 07:23:49 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* System_Management_Folder_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class System_Management_Folder_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function System_Management_Folder_Management(&$pm)
	{
		$this->Asset_Management($pm);
		$this->vars =	Array(
							'name' => Array(
								'default' => 'System Management',
							),
						);

	}//end constructor

}//end class
?>
