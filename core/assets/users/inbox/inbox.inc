<?php

require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Inbox
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
* @subpackage __core__
*/
class Inbox extends Asset
{

	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Inbox($assetid=0)
	{
		$this->Asset($assetid);
	}//end constructor


	/**
	* Gets the user that this inbox is attached to
	*
	* @return object User
	* @access private
	*/
	function &_getUser()
	{
		$link = $this->getLink(SQ_LINK_TYPE_2, 'user', false, '', 'minor', '1');
		$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['majorid'], $link['type_code']);
		return $user;
	}


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
	* Returns true if at least one of the passed assetids has the passed permission
	* Does not check current status of lock
	*
	* @param int	$perm		the permission to check access for
	* @param array	$assetids	An array of user_group and/or user ids to check for permission
	*							If the array is empty, the current user will be checked
	*
	* @return boolean
	* @access public
	*/
	function _checkPermissionAccess($perm, $assetids=Array())
	{
		// root can do anything
		if ($GLOBALS['SQ_SYSTEM']->userRoot()) return true;

		$user = &$this->_getUser();
		if ($GLOBALS['SQ_SYSTEM']->userRoot($user)) {
			// we have to be root to view root's inbox
			return false;
		} else if ($GLOBALS['SQ_SYSTEM']->userSystemAdmin($user)) {
			// this is the inbox of a sysadmin so they need to
			// be root or the sysadmin themselves
			if (!$GLOBALS['SQ_SYSTEM']->userSystemAdmin()) return false;
			if ($user->id == $GLOBALS['SQ_SYSTEM']->curretUserId()) return true;
			return false;
		} else {
			// any other user's inbox
			if ($GLOBALS['SQ_SYSTEM']->userSystemAdmin()) return true;
			if ($user->id == $GLOBALS['SQ_SYSTEM']->curretUserId()) return true;
		}

		return false;
	}//end checkAccess()


	/**
	* Called by the design to print the body of this asset
	*
	* @access public
	*/
	function printBody()
	{
		$edit = $this->getEditFns();
		$null = '';
		$edit->paintMail($this, $null, 'inbox');
	}// end printBody()


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the short name or the full name
	*
	* @see Asset::_getName()
	* @access public
	*/
	function _getName($short_name=false)
	{
		return 'Inbox';
	}//end _getName();

}//end class
?>
