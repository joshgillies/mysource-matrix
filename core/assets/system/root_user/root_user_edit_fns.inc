<?php

require_once SQ_CORE_PACKAGE_PATH.'/user/user_edit_fns.inc';

/**
* Root_User_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
*/
class Root_User_Edit_Fns extends User_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function Root_User_Edit_Fns() 
	{
		$this->User_Edit_Fns();
	}

}//end class
?>
