<?php

require_once SQ_CORE_PACKAGE_PATH.'/users/backend_user/backend_user_edit_fns.inc';

/**
* System_User_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
* @subpackage __core__
*/
class System_User_Edit_Fns extends Backend_User_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function System_User_Edit_Fns() 
	{
		$this->Backend_User_Edit_Fns();
	}

}//end class
?>
