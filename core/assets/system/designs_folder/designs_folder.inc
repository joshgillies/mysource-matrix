<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: designs_folder.inc,v 1.13 2003/10/08 00:51:05 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/folder/folder.inc';

/**
* Designs_Folder
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Designs_Folder extends Folder
{
	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Designs_Folder($assetid=0)
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

		$this->_tmp[__CLASS__.'_in_create'] = true;

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$where = 'type_code = '.$db->quote($this->type());
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);
		$sql = 'SELECT COUNT(*) FROM '.SQ_TABLE_RUNNING_PREFIX.'asset '.$where;
		$exists = $db->getOne($sql);
		if (DB::isError($exists)) trigger_error($exists->getMessage().'<br/>'.$exists->getUserInfo(), E_USER_ERROR);

		if ($exists) {
			trigger_error('Unable to create another System Designs Folder, you are only allowed one', E_USER_WARNING);
			return false;
		}

		$this->setAttrValue('name', 'System Designs');

		$ret_val = parent::create($link);
		unset($this->_tmp[__CLASS__.'_in_create']);
		return $ret_val;

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
		trigger_error('Unable to duplicate Systems Designs Folder, you are only allowed one', E_USER_WARNING);
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
		trigger_error('Unable to morph Designs Folder', E_USER_WARNING);
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
		if ($name == 'name' && empty($this->_tmp[__CLASS__.'_in_create'])) {
			trigger_error('You are not able to alter the name for the designs folder', E_USER_WARNING);
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
			trigger_error('You cannot create a new link to the designs folder', E_USER_WARNING);
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
		trigger_error('You cannot delete anything from the designs folder', E_USER_WARNING);
		return false;

	}// end deleteLink()


	/*
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks()
	{
		return Array(SQ_LINK_TYPE_1 => Array('design' => Array('card' => 'M', 'exclusive' => true)));

	}//end _getAllowedLinks()


	/**
	* Returns true if the passed assetid has write access to this asset
	*
	* @param array		$assetids	An array of user_group and/or user ids to check for write access
	*								If the array is empty, the current user will be checked
	* @param boolean	$effective	effective access based on locking and status
	* @param boolean	$editing	effective access based on live edit access
	*
	* @return boolean
	* @access public
	*/
	function writeAccess($assetids=Array(), $effective=true, $editing=true)
	{
		// only root can write to this folder so we add this extra checking
		// before going up to our parent to check access
		if (empty($assetids)) {
			if ($GLOBALS['SQ_SYSTEM']->userRoot()) {
				return parent::writeAccess($assetids, $effective, $editing);
			}
		} else {
			$root_user = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
			if (in_array($root_user->id, $assetids)) {
				return parent::writeAccess($assetids, $effective, $editing);
			}
		}
		return false;
	}//end writeAccess()


}//end class
?>
