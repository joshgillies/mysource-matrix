<?php
/**
* +--------------------------------------------------------------------+
* | MySource 3 - MySource Matrix                                       |
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
* $Id: user.inc,v 1.63 2003/11/25 05:52:10 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* User
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class User extends Asset
{

	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function User($assetid=0)
	{
		$this->Asset($assetid);
	
	} // end User


	/**
	* Create this asset
	*
	* @param array()	&$link	information used to create the initial link
	*
	* @see Asset::create()
	* @return mixed int or false
	* @access public
	*/
	function create(&$link)
	{
		$username = trim($this->attr('username'));
		$password = $this->attr('password');

		if ($username == '' || $password == '') {
			trigger_error('Unable to create a '.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name').' without a username and password', E_USER_WARNING);
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$linkid = parent::create($link);

		if (!$linkid) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$this->id = 0;
			return false;
		}

		$user_link = Array('asset' => &$this, 'link_type' => SQ_LINK_TYPE_2, 'dependant' => 1, 'exclusive' => 1);

		// create an inbox
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('inbox');
		$inbox = new Inbox();
		if (!$inbox->create($user_link)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$this->id = 0;
			return false;
		}

		// create a workspace
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('workspace');
		$workspace = new Workspace();
		$workspace->setAttrValue('name', 'Workspace');
		if (!$workspace->create($user_link)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$this->id = 0;
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}//end create()


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
	* @access public
	*/
	function _getName($short_name=false)
	{
		return ($this->attr('first_name') || $this->attr('last_name')) ? $this->attr('first_name').' '.$this->attr('last_name') : $this->attr('username');

	}//end _getName()


	/**
	* Attempts to find a user that matches the passed username and password, returning it's assetid
	*
	* @param string	$username	the user name to search for
	* @param string	$password	the password to match (only if not null)
	* @param string	$type_code	the asset type to search for
	*
	* @return object User
	* @access public
	*/
	function &find($username, $password=null, $type_code='user')
	{
		require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$where = 'a.type_code IN (~SQ0~)
				  AND at.name         = '.$db->quote('username').'
				  AND at.attributeid  = av.attributeid
				  AND av.assetid      = a.assetid
				  AND av.custom_value = '.$db->quote($username);
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'av');
		$sql = 'SELECT a.assetid, a.type_code
				FROM '.SQ_TABLE_RUNNING_PREFIX.'asset a, '.SQ_TABLE_PREFIX.'asset_attribute at, '.SQ_TABLE_RUNNING_PREFIX.'asset_attribute_value av
				'.$where;

		$subs = Array( 'SELECT type_code
						FROM '.SQ_TABLE_PREFIX.'asset_type_inherited
						WHERE inherited_type_code = '.$db->quote($type_code)
						);

		$sql = db_extras_subquery($db, $sql, $subs);

		$result = $db->getRow($sql);
		if (DB::isError($result)) trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);

		if (!$result['assetid']) return $null = null;

		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($result['assetid']);
		if (is_null($asset) || (!is_null($password) && !$asset->comparePassword($password))) {
			return $null = null;
		}

		return $asset;

	}//end find()


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

	}//end checkAccess()


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

	}// end printBody()


}//end class

?>