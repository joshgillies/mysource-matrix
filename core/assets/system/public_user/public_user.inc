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
* $Id: public_user.inc,v 1.2 2005/03/21 06:18:44 gsherwood Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/users/user/user.inc';


/**
* Public_User
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Public_User extends User
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Public_User($assetid=0)
	{
		$this->User($assetid);

	}//end constructor


	/**
	* Create this asset
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return mixed (int|boolean) the new link id if successful or false on fail
	* @access public
	*/
	function create(&$link)
	{
		require_once SQ_CORE_PACKAGE_PATH.'/system/system_asset_fns.inc';
		if (!system_asset_fns_create_pre_check($this)) return false;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		parent::setAttrValue('username', '');
		parent::setAttrValue('first_name', 'Public');
		parent::setAttrValue('last_name', 'User');

		if ($linkid = parent::create($link)) {
			if (!system_asset_fns_create_cleanup($this)) {
				$linkid = false;
			}
		}

		if ($linkid) $GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		else $GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');

		return $linkid;

	}//end create()


	/**
	* Morph's this asset to either one of it's parent type or one of it's children types
	* Only succeeds if $type_code asset's is allowed to link to all children that this asset has
	*
	* @param string	$new_type_code	the new type_code to attempt to cast this asset to
	*
	* @return boolean
	* @access public
	*/
	function morph($new_type_code)
	{
		// trying to morph an unprivileged public user... why would you bother?
		trigger_error('Unable to morph '.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name'), E_USER_WARNING);
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
	* returns FALSE always because we don't want to be able to clone a root user
	*
	* @return boolean
	* @access public
	*/
	function canClone()
	{
		return false;

	}//end canClone()


	/**
	* Returns true if this user can log in
	* Note that this is not for backend logins only, it applys to all logins
	*
	* @return boolean
	* @access public
	*/
	function canLogin()
	{
		return false;

	}//end canLogin()


	/**
	* Check if this user can be set as the current user
	*
	* Although we can't log in with the public user we DO need to be able to set
	* this as the current user for things such as locking et al.
	*
	* @return boolean
	* @access public
	*/
	function canSetAsCurrentUser()
	{
		return true;
	
	}//end canSetAsCurrentUser()
	
	
	/**
	* Set the value for the passed variable to the passed value
	*
	* @param string		$name				the name of the attribute
	* @param string		$value				the new value of the attribute
	*
	* @return boolean
	* @access public
	*/
	function setAttrValue($name, $value)
	{
		if ($name == 'username') {
			trigger_error('You are not able to alter the username for the "'.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name').'"', E_USER_WARNING);
			return false;
		}

		return parent::setAttrValue($name, $value);

	}//end setAttrValue()


	/**
	* Catches blank username/password combination made during User::create()
	*
	* As it makes no sense for a public access user to have a user name or
	* password, we will skip this check
	* 
	* @param string	$username	the passed user name
	* @param string $password	the passed pass
	*
	* @return boolean	overridden so always returns true
	* @access public
	*/
	function _catchBlankUserNamePass($username, $password)
	{
		return true;
		
	}//end _catchBlankUserNamePass()
	
	
	/**
	* Catches blank username being set during User::setAttrValue()
	*
	* As it makes no sense for a public access user to have a user name, we will
	* skip this check
	*
	* @param string	$username	the passed user name
	* 
	* @return boolean	overridden so always returns true
	* @access public
	*/
	function _catchBlankSetUserName($username)
	{
		return true;
		
	}//end _catchBlankSetUserName()
	
	
}//end class
?>
