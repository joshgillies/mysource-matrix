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
* $Id: root_user.inc,v 1.30 2003/11/18 15:37:28 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/system/system_user/system_user.inc';


/**
* Root_User
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Root_User extends System_User
{
	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Root_User($assetid=0)
	{
		$this->System_User($assetid);
	}


	/**
	* Create this asset
	*
	* @param array(string)	&$link	information used to create the initial link
	*
	* @return boolean
	* @access public
	*/
	function create(&$link)
	{
		require_once SQ_CORE_PACKAGE_PATH.'/system/system_asset_fns.inc';
		if (!system_asset_fns_create_pre_check($this)) return false;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		parent::setAttrValue('username', 'root');
		if ($linkid = parent::create($link)) {
			if (!system_asset_fns_create_cleanup($this))  {
				$linkid = false;
			}
		}

		if ($linkid) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		}

		return $linkid;

	}//end create()


	/**
	* Morph's this asset to either one of it's parent type or one of it's children types
	* Only succeeds if $type_code asset's is allowed to link to all children that this asset has
	*
	* @param string	$new_type_code	the new type_code to attempt to cast this asset to
	*
	* @return bool
	* @access public
	*/
	function morph($new_type_code)
	{
		// you cant morph a bloody root user... GEZZZ get serious
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
	* Returns true if this user can log in
	* Note that this is not for backend logins only, it applys to all logins
	*
	* @return boolean
	* @access public
	*/
	function canLogin()
	{
		return true;

	}//end canLogin()


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

}//end class
?>
