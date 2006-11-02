<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: authentication.inc,v 1.10 2006/11/02 23:56:08 tbarrett Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';


/**
* Authentication
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.10 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Authentication extends Asset
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Authentication($assetid=0)
	{
		$this->Asset($assetid);

	}//end constructor


	/**
	* Morphs this asset to either one of its parent type or one of its children types
	*
	* Only succeeds if $type_code assets are allowed to link to all children that this asset has
	*
	* @param string	$new_type_code	the new type_code to attempt to cast this asset to
	*
	* @return boolean
	* @access public
	*/
	function morph($new_type_code)
	{
		trigger_localised_error('CORE0091', E_USER_WARNING, $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name'));
		return FALSE;

	}//end morph()


	/**
	* Can this asset have its last significant link removed (putting it in the trash)?
	*
	* @return boolean
	* @access public
	*/
	function canDelete()
	{
		return FALSE;

	}//end canDelete()


	/**
	* returns FALSE always because we don't want to be able to clone a authentication
	*
	* @return boolean
	* @access public
	*/
	function canClone()
	{
		return FALSE;

	}//end canClone()


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	function _getName($short_name=FALSE)
	{
		return $this->attr('name');

	}//end _getName()


	/**
	* Given a username, attempts to locate and return a user object.
	*
	* @param string	$username	The username of the user to locate
	*
	* @return object
	* @access public
	*/
	function &locateUser($username)
	{
		$null = NULL;
		return $null;

	}//end locateUser()


	/**
	* Attempts to find a user that matches the passed username and password
	*
	* If the wrong password is supllied, the user account will be locked after
	* a system defined number of incorrect login attempts
	*
	* @param string	$username	the user name to search for
	* @param string	$password	the password to match
	*
	* @return mixed object|NULL
	* @access public
	*/
	function &authenticateUser($username, $password)
	{
		$null = NULL;
		return $null;

	}//end authenticateUser()


	/**
	* Given a username supplied by HTTP, attempts to locate and return a user object
	*
	* This function is used when processing a pre-authenticated username supplied
	* via HTTP headers or server vars.
	*
	* @param string	$username	The username to search for
	*
	* @return mixed object|NULL
	* @access public
	*/
	function &authenticateHttpUser($username)
	{
		$null = NULL;
		return $null;

	}//end authenticateHttpUser()


}//end class

?>