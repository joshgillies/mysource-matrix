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
* $Id: system_management_folder.inc,v 1.5.2.1 2004/02/18 11:39:05 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_CORE_PACKAGE_PATH.'/folder/folder.inc';

/**
* System_Management_Folder
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class System_Management_Folder extends Folder
{
	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function System_Management_Folder($assetid=0)
	{
		$this->Folder($assetid);
	}


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
	* Creates a duplicate of this asset
	* It DOES NOT duplicate the links that this asset has either to it or from it
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
	* Creates a link between this asset and the passed asset, of a certain type
	* and returns the linkid of the created link
	*
	* @param object Asset	$minor		the minor asset that we are linking to
	* @param string			$link_type	the type of link this is
	* @param string			$value		the value that is to be associated with this link
	* @param string			$sort_order	the position in the links list that this link should take, 
	*									if null or less than zero places at end of list
	* @param string			$dependant	'0' / '1' on whether the this asset is dependant on the 
	*									asset that will be linked by this new link
	* @param string			$exclusive	'0' / '1' on whether the this asset is linked exclusivly 
	*									to the asset that will be linked by this new link
	*
	* @return int
	* @access public
	*/
	function createLink(&$minor, $link_type, $value='', $sort_order=null, $dependant='0', $exclusive='0')
	{
		if (empty($GLOBALS['SQ_INSTALL'])) {
			trigger_error('You cannot create a new link to the '.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name'), E_USER_WARNING);
			return 0;
		
		} else {
			return parent::createLink($minor, $link_type, $value, $sort_order, $dependant, $exclusive);
		}

	}//end createLink()


	/**
	* Removes a link where this asset is the major player
	*
	* @param int	$linkid	the link id of the link to remove
	*
	* @return boolean
	* @access public
	*/
	function deleteLink($linkid)
	{
		trigger_error('You cannot delete anything from the '.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name'), E_USER_WARNING);
		return false;

	}// end deleteLink()


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


	/*
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks()
	{
		return Array(SQ_LINK_TYPE_1 => Array(
											'system_user_group'     => Array('card' => '1', 'exclusive' => true),
											'designs_folder'        => Array('card' => '1', 'exclusive' => true),
											'authentication_folder' => Array('card' => '1', 'exclusive' => true),
											'search_manager'        => Array('card' => '1', 'exclusive' => true),
											'layout_manager'        => Array('card' => '1', 'exclusive' => true),
											'remap_manager'         => Array('card' => '1', 'exclusive' => true),
											'cron_manager'          => Array('card' => '1', 'exclusive' => true),
										),
					);

	}//end _getAllowedLinks()


	/**
	* Returns true if the passed assetid has write access to this asset
	*
	* @param string	$lock_type	the type of write access required - if blank returns a plain check against
	*							permissions set for this asset, otherwise returns effective access based on locking 
	*							and status
	* @param Array	$assetids	An array of user_group and/or user ids to check for write access
	*							If the array is empty, the current user will be checked
	*
	* @return boolean
	* @access public
	*/
	function writeAccess($lock_type, $assetids=Array())
	{
		// only root can write to this folder so we add this extra checking
		// before going up to our parent to check access
		if (empty($assetids)) {
			if ($GLOBALS['SQ_SYSTEM']->userRoot()) {
				return parent::writeAccess($lock_type, $assetids);
			}
		} else {
			$root_user = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
			if (in_array($root_user->id, $assetids)) {
				return parent::writeAccess($lock_type, $assetids);
			}
		}
		return false;
	}//end writeAccess()


}//end class
?>
