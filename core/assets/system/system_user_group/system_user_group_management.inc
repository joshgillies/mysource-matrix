<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: system_user_group_management.inc,v 1.5 2003/10/15 11:05:15 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* System_User_Group_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class System_User_Group_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function System_User_Group_Management(&$pm)
	{
		$this->Asset_Management($pm);
		$this->vars =	Array(
							'name' => Array(
								'default' => 'System Administrators',
							),
						);

	}//end constructor

}//end class
?>
