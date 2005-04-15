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
* $Id: user.inc,v 1.84 2005/04/15 00:37:27 lwright Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* User
*
* Purpose
*     Processing interfaces for the (standard) user asset - one that can log in
*     but has no backend editing privileges. This also defines the baseline
*     interface for all user assets in the system (whether they be standard,
*     backend, system admin, etc).
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.84 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class User extends Asset
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	**/
	function User($assetid=0)
	{
		$this->_ser_attrs = true;
		$this->Asset($assetid);

	}//end constructor


	/**
	* Perform any pre-validation required during the creation of this asset
	*
	* Users ensure a username and password has been set before they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @access private
	* @return boolean
	*/
	function _preCreateCheck(&$link)
	{
		if (!parent::_preCreateCheck($link)) return false;

		$username = trim($this->attr('username'));
		$password = $this->attr('password');

		if (!$this->_catchBlankUserNamePass($username, $password)) {
			trigger_localised_error("CORE0084", E_USER_WARNING, $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name'));
			return false;
		}

		return true;

	}//end _preCreateCheck()


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* Standard pages create a bodycopy asset when they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @access private
	* @return boolean
	*/
	function _createAdditional(&$link)
	{
		if (!parent::_createAdditional($link)) return false;

		// set an initial web path
		$initial_path = strtolower($this->attr('username'));
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		$valid_paths = make_valid_web_paths(Array($initial_path));
		$good_paths = $GLOBALS['SQ_SYSTEM']->am->webPathsInUse($link['asset'], $valid_paths, $this->id, true);
		if (!$this->saveWebPaths($good_paths)) {
			trigger_localised_error("CORE0019", E_USER_WARNING, $this->name);
			return false;
		}

		if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($this->id, 'permissions')) {
			trigger_localised_error("CORE0014", E_USER_WARNING, $this->name);
			return false;
		}

		// give the user admin access to him/herself
		if (!$GLOBALS['SQ_SYSTEM']->am->setPermission($this->id, $this->id, SQ_PERMISSION_ADMIN, true)) {
			trigger_localised_error("CORE0015", E_USER_WARNING, $this->name);
			return false;
		}

		if (!$GLOBALS['SQ_SYSTEM']->am->releaseLock($this->id, 'permissions')) {
			trigger_localised_error("CORE0013", E_USER_WARNING, $this->name);
			return false;
		}

		$user_link = Array('asset' => &$this, 'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);

		// create an inbox
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('inbox');
		$inbox = new Inbox();
		if (!$inbox->create($user_link)) return false;

		// create a workspace
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('workspace');
		$workspace = new Workspace();
		$workspace->setAttrValue('name', 'Workspace');
		if (!$workspace->create($user_link)) return false;

		return true;

	}//end _createAdditional()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks()
	{
		$allowed = parent::_getAllowedLinks();
		$allowed[SQ_LINK_TYPE_2]['inbox']     = Array('card' => 1, 'exclusive' => true);
		$allowed[SQ_LINK_TYPE_2]['workspace'] = Array('card' => 1, 'exclusive' => true);
		return $allowed;

	}//end _getAllowedLinks()


	/**
	* Set the value for the passed variable to the passed value
	* Overwritten here to check inputs (username and password) and to
	* encrypt password before setting value
	*
	* @param string		$name				the name of the attribute
	* @param string		$value				the new value of the attribute
	*
	* @return boolean
	* @access public
	*/
	function setAttrValue($name, $value)
	{
		// if this is the password, encrypt it
		if ($name == 'password') {
			$value = crypt($value); // let salt be generated

		} else if ($name == 'username') {
			if (!$this->_catchBlankSetUserName(trim($value))) {
				trigger_localised_error("CORE0110", E_USER_WARNING);
				return false;
			}
		}

		if (!parent::setAttrValue($name, $value)) return false;

		// tell the workspace (or anyone else listening) that the username has been updated
		if ($name == 'username') {
			$em = &$GLOBALS['SQ_SYSTEM']->getEventManager();
			$em->broadcastEvent($this, 'usernameUpdated', Array('username' => $value));
		}
		return true;

	}//end setAttrValue()


	/**
	* Returns whether the passed password matches this users' password
	*
	* @access public
	*/
	function comparePassword($entered_password)
	{
		$password = $this->attr('password');

		if (!trim($password)) {
			return false;
		}

		return (crypt($entered_password,$password) == $password);

	}//end comparePassword()


	/**
	* Returns name of the user
	*
	* @param bool	$short_name	whether or not we are after the shortname or the full name
	*
	* @see Asset::_getName()
	* @access private
	*/
	function _getName($short_name=false)
	{
		return ($this->attr('first_name') || $this->attr('last_name')) ? $this->attr('first_name').' '.$this->attr('last_name') : $this->attr('username');

	}//end _getName()


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
		if (empty($assetids)) {
			// root can do anything
			if ($GLOBALS['SQ_SYSTEM']->userRoot()) return true;

			if ($GLOBALS['SQ_SYSTEM']->userRoot($this)) {
				// we have to be root to view root's details
				return false;
			} else if ($GLOBALS['SQ_SYSTEM']->userSystemAdmin($this)) {
				// this is a sysadmin so they need to
				// be root or the sysadmin themselves
				if (!$GLOBALS['SQ_SYSTEM']->userSystemAdmin()) return false;
				if ($this->id == $GLOBALS['SQ_SYSTEM']->currentUserId()) return true;
				return false;
			} else {
				// any other user's details
				if ($GLOBALS['SQ_SYSTEM']->userSystemAdmin()) return true;
				if ($this->id == $GLOBALS['SQ_SYSTEM']->currentUserId()) return true;
			}

			return false;
		} else {
			// a user always has full access to themselves
			if (in_array($this->id, $assetids)) return true;

			return parent::_checkPermissionAccess($perm, $assetids);
		}

	}//end _checkPermissionAccess()


	/**
	* Returns TRUE if we can delete the passed link, or a string with the error msg
	*
	* @param int	$linkid	the link id of the link to remove
	*
	* @return mixed TRUE or error msg string
	* @access public
	*/
	function isDeletableLink($linkid)
	{
		if ($GLOBALS['SQ_PURGING_TRASH']) return true;

		// we dont want our inbox or workspace deleted
		$inbox_link = $this->getInboxLink();
		if ($inbox_link['linkid'] == $linkid) return 'You cant delete the link between a user and their inbox';

		$workspace_link = $this->getWorkspaceLink();
		if ($workspace_link['linkid'] == $linkid) return 'You cant delete the link between a user and their workspace';

		return parent::isDeletableLink($linkid);

	}//end isDeletableLink()


	/**
	* Check if this user can log in
	* Note that this is not for backend logins only, it applys to all logins
	*
	* @return boolean
	* @access public
	*/
	function canLogin()
	{
		// check that this user is live
		if ($this->status & SQ_SC_STATUS_NOT_LIVE) return false;

		// check that this user is not only in the trash
		if ($GLOBALS['SQ_SYSTEM']->am->assetInTrash($this->id, true)) return false;

		return true;

	}//end canLogin()


	/**
	* Can we set this as the current user?
	*
	* @return boolean
	* @access public
	*/
	function canSetAsCurrentUser()
	{
		return $this->canLogin();

	}//end canSetAsCurrentUser()


	/**
	* Gets an array of groups that this user is in
	*
	* The result is stored in the session for faster access
	*
	* @return array
	* @access public
	*/
	function getGroups()
	{
		if (!isset($_SESSION['sq_user_groups'])) {
			$_SESSION['sq_user_groups'] = array_keys($GLOBALS['SQ_SYSTEM']->am->getParents($GLOBALS['SQ_SYSTEM']->currentUserId(), Array('user', 'user_group'), false));
		}
		return $_SESSION['sq_user_groups'];

	}//end getGroups()


	/**
	* Gets an array of groups that satisfy user conditions
	*
	* Returns an array of all the Groups where we have met the
	* User Conditions associated with this Group for this User
	*
	* @return array $groups array of group asset ids
	* @access public
	*/
	function getUserGroups()
	{
		$groups = Array();
		$groupids = $GLOBALS['SQ_SYSTEM']->am->getParents($this->id, 'user_group', false);

		foreach ($groupids as $key => $value) {
			// if we pass the conditions, let us belong to this group
			if ($this->_belongsToGroup($key)) {
				array_push($groups, intval($key));
			}
		}

		return $groups;

	}//end getUserGroups()


	/**
	* Verify if a user belongs to a given group
	*
	* Here is where we call the user conditions and certify the result.
	* When a new user condition is added, please add the specifics for calling it here.
	*
	* @param string $group_assetid  assetid of the group in question
	* @access private
	* @return boolean true if were allowed to belong to this gorup
	*/
	function _belongsToGroup($group_assetid)
	{
		$restrictions = $this->attr('restrictions');

		// if this group isnt in the restrictions array
		// then we just belong to it and so return
		if (!in_array($group_assetid, array_keys($restrictions))) {
			return true;
		}

		// for each restriction associated to this group
		foreach ($restrictions[$group_assetid] as $restriction) {

			// grab the condition name, make an object
			$condition_type_code = $restriction['name'];
			$GLOBALS['SQ_SYSTEM']->am->includeAsset($condition_type_code);
			$condition = new $condition_type_code();

			$match = $condition->evaluate($this, $restriction['condition_data']);

			// they don't match up, then get the hell out of mah group!!
			if ($match != $restriction['match']) return false;

		}

		// if we got this far, we've passed all restrictions
		return true;

	}//end _belongsToGroup()


	/**
	* Returns the link to the inbox
	*
	* @return array
	* @access public
	*/
	function getInboxLink()
	{
		return $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'inbox');

	}//end getInboxLink()


	/**
	* Returns the link to the workspace
	*
	* @return array
	* @access public
	*/
	function getWorkspaceLink()
	{
		return $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'workspace');

	}//end getWorkspaceLink()


	/**
	* Returns this user's Workspace
	*
	* @return &object Asset the user's workspace
	* @access public
	*/
	function &getWorkspace()
	{
		$null = null;
		$link = $this->getWorkspaceLink();

		if (empty($link)) {
			trigger_localised_error("CORE0109", E_USER_WARNING,$this->name);
			return $null;
		}

		$workspace = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid']);
		if (is_null($workspace)) {
			trigger_localised_error("CORE0109", E_USER_WARNING,$this->name);
			return $null;
		}
		return $workspace;

	}//end getWorkspace()


	/**
	* Called by the design to print the body of this asset
	*
	* @access public
	*/
	function printBody()
	{
		?>
		<table border="0" cellspacing="2" cellpadding="2">
			<tr>
				<td><b>Username</b></td>
				<td><?php echo $this->attr('username'); ?></td>
			</tr>
			<tr>
				<td><b>First Name</b></td>
				<td><?php echo $this->attr('first_name'); ?></td>
			</tr>
			<tr>
				<td><b>Last Name</b></td>
				<td><?php echo $this->attr('last_name'); ?></td>
			</tr>
			<tr>
				<td><b>Email</b></td>
				<td><?php echo $this->attr('email'); ?></td>
			</tr>
		</table>
		<?php

	}//end printBody()


	/**
	* Catches blank username/password combination made during create()
	*
	* Override this and return true if a specific subclass wants to skip this check
	*
	* @param string	$username	the passed user name
	* @param string $password	the passed pass
	*
	* @return boolean	overridden so always returns true
	* @access public
	*/
	function _catchBlankUserNamePass($username, $password)
	{
		return (($username != '') && ($password != ''));

	}//end _catchBlankUserNamePass()


	/**
	* Catches blank username being set during setAttrValue()
	*
	* Override this and return true if a specific subclass wants to skip this check
	*
	* @param string	$username	the passed user name
	*
	* @return boolean	overridden so always returns true
	* @access public
	*/
	function _catchBlankSetUserName($username)
	{
		return ($username != '');

	}//end _catchBlankSetUserName()


}//end class

?>