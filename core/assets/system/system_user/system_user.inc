<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: system_user.inc,v 1.5 2003/09/26 05:26:29 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/users/backend_user/backend_user.inc';


/**
* System_User
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
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
