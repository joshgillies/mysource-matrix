<?php

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
		$this->_tmp[__CLASS__.'_in_create'] = true;

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$where = 'type_code = '.$db->quote($this->type());
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);
		$sql = 'SELECT COUNT(*) FROM '.SQ_TABLE_RUNNING_PREFIX.'asset '.$where;
		$exists = $db->getOne($sql);
		if (DB::isError($exists)) trigger_error($exists->getMessage().'<br/>'.$exists->getUserInfo(), E_USER_ERROR);

		if ($exists) {
			trigger_error('Unable to create another System User Group, you are only allowed one', E_USER_WARNING);
			return false;
		}

		$this->setAttrValue('name', 'System Administrators');
		$ret_val = parent::create($link);
		unset($this->_tmp[__CLASS__.'_in_create']);
		return $ret_val;

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
		trigger_error('Unable to duplicate System User Group, you are only allowed one', E_USER_WARNING);
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
		trigger_error('Unable to morph System User Group', E_USER_WARNING);
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
			trigger_error('You are not able to alter the name for the System Administrators User Group', E_USER_WARNING);
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
		return false;

	}//end checkAccess()

}//end class

?>