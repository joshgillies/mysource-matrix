<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: backend_user.inc,v 1.6 2003/09/26 05:26:29 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/users/user/user.inc';


/**
* Backend_User
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
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
