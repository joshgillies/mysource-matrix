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
* $Id: system_user_group.inc,v 1.21 2003/11/18 15:37:29 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/users/user_group/user_group.inc';

/**
* System_User_Group
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class System_User_Group extends User_Group
{

	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function System_User_Group($assetid=0)
	{
		$this->User_Group($assetid);

	}//end constructor


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
		require_once SQ_CORE_PACKAGE_PATH.'/system/system_asset_fns.inc';
		if (!system_asset_fns_create_pre_check($this)) return false;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

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
	* Creates a duplicate of this asset. It DOES NOT duplicate the links that this asset has either to it or from it
	* Returns a reference to the new object or if an error occured FALSE
	*
	* @param bool	$dupe_directory	whether or not to duplicate thia assets data directory
	*
	* @return mixed object Asset or NULL
	* @access public
	* @see Asset::duplicate()
	*/
	function &duplicate($dupe_directory=true)
	{
		trigger_error('Unable to duplicate '.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name').', you are only allowed one', E_USER_WARNING);
		$null = NULL; // 'cause we need to return by reference :)
		return $null;

	}//end duplicate()


	/**
	* Morph's this asset to either one of it's parent type or one of it's children types
	* Only succeeds if $type_code asset's is allowed to link to all children that this asset has
	*
	* @param string	$new_type_code	the new type_code to attempt to cast this asset to
	*
	* @return bool
	* @access public
	* @see Asset::morph()
	*/
	function morph($new_type_code)
	{
		trigger_error('Unable to morph '.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name'), E_USER_WARNING);
		return false;

	}//end morph()


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

		if ($name == 'name') {
			trigger_error('You are not able to alter the name for the '.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name'), E_USER_WARNING);
			return false;
		}

		return parent::setAttrValue($name, $value);

	}//end setAttrValue()


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
		// we have to be root to do anything in here
		if ($GLOBALS['SQ_SYSTEM']->userRoot()) return true;
		// if we are just reading then, we need to be a system admin
		if (($perm & SQ_PERMISSION_READ) && $GLOBALS['SQ_SYSTEM']->userSystemAdmin()) return true;
		return false;

	}//end checkAccess()


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks()
	{
		return Array(SQ_LINK_TYPE_1 => Array('system_user' => Array('card' => 'M', 'exclusive' => false)),
					 SQ_LINK_TYPE_2 => Array('system_user' => Array('card' => 'M', 'exclusive' => false)),
					 SQ_LINK_TYPE_3 => Array(),
					 SQ_LINK_NOTICE => Array()
					);

	}//end _getAllowedLinks()


}//end class

?>