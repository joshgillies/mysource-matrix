<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: root_user_edit_fns.inc,v 1.9 2003/09/26 05:26:28 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/system/system_user/system_user_edit_fns.inc';

/**
* Root_User_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Root_User_Edit_Fns extends System_User_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function Root_User_Edit_Fns()
	{
		$this->System_User_Edit_Fns();
	}

}//end class
?>
