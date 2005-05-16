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
* $Id: authentication.inc,v 1.7 2005/05/16 06:59:10 tbarrett Exp $
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
* @version $Revision: 1.7 $
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
	* @return bool
	* @access public
	*/
	function morph($new_type_code)
	{
		trigger_localised_error('CORE0091', E_USER_WARNING, $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name'));
		return false;

	}//end morph()


	/**
	* Can this asset have its last significant link removed (putting it in the trash)?
	*
	* @return boolean
	* @access public
	*/
	function canDelete()
	{
		return false;

	}//end canDelete()


	/**
	* returns FALSE always because we don't want to be able to clone a authentication
	*
	* @return boolean
	* @access public
	*/
	function canClone()
	{
		return false;

	}//end canClone()


	/**
	* Returns name of the asset
	*
	* @param bool	$short_name	whether or not we are after the shortname or the full name
	*
	* @see Asset::_getName()
	* @access private
	*/
	function _getName($short_name=false)
	{
		return $this->attr('name');

	}//end _getName()


	/**
	* Attempts to find a user that matches the passed username and password
	*
	* If the wrong password is supllied, the user account will be locked after
	* a system defined number of incorrect login attempts
	*
	* @param string	$username	the user name to search for
	* @param string	$password	the password to match
	*
	* @return object User | NULL
	* @access public
	*/
	function &authenticateUser($username, $password)
	{
		$null = null;
		return $null;

	}//end authenticateUser()


	/**
	* If this authentication has a user matching the username in $_SERVER['REMOTE_USER'], return that user
	*
	* @access public
	* @return Object | Null
	*/
	function &authenticateHttpUser()
	{
		$null = null;
		return $null;

	}//end authenticateHttpUser()


}//end class

?>