<?php

require_once SQ_CORE_PACKAGE_PATH.'/users/user/user.inc';


/**
* Backend_User
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
* @subpackage __core__
*/
class Backend_User extends User
{

	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Backend_User($assetid=0)
	{
		$this->User($assetid);
	}

}//end class
?>
