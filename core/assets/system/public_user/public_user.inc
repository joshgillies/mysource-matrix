<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: public_user.inc,v 1.8 2012/08/30 01:09:11 ewang Exp $
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
* @version $Revision: 1.8 $
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
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Create this asset
	*
	* Return linkid or FALSE on failure
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return mixed int|boolean
	* @access public
	*/
	public function create(Array &$link)
	{
		require_once SQ_CORE_PACKAGE_PATH.'/system/system_asset_fns.inc';
		if (!system_asset_fns_create_pre_check($this)) {
			return FALSE;
		}
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		parent::setAttrValue('username', '');
		parent::setAttrValue('first_name', 'Public');
		parent::setAttrValue('last_name', 'User');

		if ($linkid = parent::create($link)) {
			if (!system_asset_fns_create_cleanup($this)) {
				$linkid = FALSE;
			}
		}

		if ($linkid) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		}

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
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
	public function morph($new_type_code)
	{
		// trying to morph an unprivileged public user... why would you bother?
		trigger_localised_error('CORE0091', sprintf(translate('Unable to morph %s'), $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name')), E_USER_WARNING);
		return FALSE;

	}//end morph()


	/**
	* Can this asset have its last significant link removed (putting it in the trash)?
	*
	* @return boolean
	* @access public
	*/
	public function canDelete()
	{
		return FALSE;

	}//end canDelete()


	/**
	* returns FALSE always because we don't want to be able to clone a root user
	*
	* @return boolean
	* @access public
	*/
	public function canClone()
	{
		return FALSE;

	}//end canClone()


	/**
	* Returns TRUE if this user can log in
	* Note that this is not for backend logins only, it applys to all logins
	*
	* @return boolean
	* @access public
	*/
	public function canLogin()
	{
		return FALSE;

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
	public function canSetAsCurrentUser()
	{
		return TRUE;

	}//end canSetAsCurrentUser()


	/**
	* Set the value for the passed variable to the passed value
	*
	* @param string	$name	the name of the attribute
	* @param string	$value	the new value of the attribute
	*
	* @return boolean
	* @access public
	*/
	public function setAttrValue($name, $value, $filter=NULL)
	{
		if ($name == 'username') {
			trigger_localised_error('CORE0113', sprintf(translate('You are not able to alter the username for the "%s"'), $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name')), E_USER_WARNING);
			return FALSE;
		}

		return parent::setAttrValue($name, $value, $filter);

	}//end setAttrValue()


	/**
	* Catches blank username/password combination made during User::create()
	*
	* As it makes no sense for a public access user to have a user name or
	* password, we will skip this check and always return TRUE
	*
	* @param string	$username	the passed user name
	* @param string	$password	the passed pass
	*
	* @return boolean
	* @access public
	*/
	protected function _catchBlankUserNamePass($username, $password)
	{
		return TRUE;

	}//end _catchBlankUserNamePass()


	/**
	* Catches blank username being set during User::setAttrValue()
	*
	* As it makes no sense for a public access user to have a user name, we will
	* skip this check and always return TRUE
	*
	* @param string	$username	the passed user name
	*
	* @return boolean
	* @access public
	*/
	protected function _catchBlankSetUserName($username)
	{
		return TRUE;

	}//end _catchBlankSetUserName()

	/**
	* Provides extra checking and disable invalid status change on errors
	*
	* @param int		$new_status		the new status to set for this asset
	* @param boolean	$update_parents	Specify if we want to update the parent as well
	* @param boolean	$run_updated	specify it we want to run _upated
	*
	* @return boolean
	* @access public
	*/
	function processStatusChange($new_status, $update_parents=TRUE, $run_updated=TRUE)
	{
		// left blank to overide processStatusChange
	}//end processStatusChange()

}//end class
?>
