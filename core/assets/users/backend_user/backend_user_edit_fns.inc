<?php

require_once SQ_CORE_PACKAGE_PATH.'/users/user/user_edit_fns.inc';

/**
* Backend_User_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Backend_User_Edit_Fns extends User_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function Backend_User_Edit_Fns() 
	{
		$this->User_Edit_Fns();
	}

}//end class
?>
