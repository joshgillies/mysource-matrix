<?php

require_once SQ_CORE_PACKAGE_PATH.'/folder/folder.inc';

/**
* Workspace
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
* @subpackage __core__
*/
class Workspace extends Folder
{

	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Workspace($assetid=0)
	{
		$this->Folder($assetid);

	}//end constructor


	/**
	* Gets the user that this workspace is attached to
	*
	* @return object User
	* @access private
	*/
	function &_getUser()
	{
		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'user', false, '', 'minor', '1');
		$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['majorid'], $link['major_type_code']);
		return $user;

	}//end _getUser()


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
		$user = &$this->_getUser();
		if (empty($assetids)) {
			// root can do anything
			if ($GLOBALS['SQ_SYSTEM']->userRoot()) return true;

			if ($GLOBALS['SQ_SYSTEM']->userRoot($user)) {
				// we have to be root to view root's workspace
				return false;
			} else if ($GLOBALS['SQ_SYSTEM']->userSystemAdmin($user)) {
				// this is the workspace of a sysadmin so they need to
				// be root or the sysadmin themselves
				if (!$GLOBALS['SQ_SYSTEM']->userSystemAdmin()) return false;
				if ($user->id == $GLOBALS['SQ_SYSTEM']->currentUserId()) return true;
				return false;
			} else {
				// any other user's workspace
				if ($GLOBALS['SQ_SYSTEM']->userSystemAdmin()) return true;
				if ($user->id == $GLOBALS['SQ_SYSTEM']->currentUserId()) return true;
			}

			return false;
		} else {
			// a user always has full access to their own workspace
			if (in_array($user->id, $assetids)) return true;

			return parent::_checkPermissionAccess($perm, $assetids);
		}

	}//end checkAccess()


	/**
	* Called by the design to print the body of this asset
	*
	* @access public
	*/
	function printBody()
	{
		bam('WORKSPACE printBody(): NOT DONE YET');

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
		return 'Workspace';

	}//end _getName();

}//end class

?>