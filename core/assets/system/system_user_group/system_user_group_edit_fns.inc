<?php

require_once SQ_CORE_PACKAGE_PATH.'/user_group/user_group_edit_fns.inc';

/**
* System_User_Group_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
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
