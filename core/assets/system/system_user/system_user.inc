<?php

require_once SQ_CORE_PACKAGE_PATH.'/backend_user/backend_user.inc';


/**
* System_User
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Resolve::packages::__core__
*/
class System_User extends Backend_User
{

	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function System_User($assetid=0)
	{
		$this->Backend_User($assetid);
	}

}//end class
?>
