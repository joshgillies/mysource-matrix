<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: system_user_group_edit_fns.inc,v 1.5 2003/09/26 05:26:29 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/users/user_group/user_group_edit_fns.inc';

/**
* System_User_Group_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class System_User_Group_Edit_Fns extends User_Group_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function System_User_Group_Edit_Fns() 
	{
		$this->User_Group_Edit_Fns();
	}

}//end class
?>
