<?php

require_once SQ_CORE_PACKAGE_PATH.'/system_user/system_user_edit_fns.inc';

/**
* Root_User_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
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
