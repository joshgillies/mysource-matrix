<?php

require_once SQ_CORE_PACKAGE_PATH.'/user_group/user_group.inc';

/**
* System_User_Group
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Resolve::packages::__core__
*/
class System_User_Group extends User_Group
{

	/**
	* Indicates whether we are currently in create() or not
	*
	* @see setAttrValue()
	* @var boolean
	*/
	var $_in_create = false;


	/**
	* Constructor
	*
	* @param int    $assetid      the asset id to be loaded
	*
	*/
	function System_User_Group($assetid=0) 
	{
		$this->User_Group($assetid);
	}


	/**
	* Create this asset
	*
	* @param array(string)	$link	information used to create the initial link
	*
	* @return boolean
	* @access public
	*/
	function create($link)
	{
		$this->_in_create = true;

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = 'SELECT count(*) FROM sq_asset WHERE type_code = '.$db->quote($this->type());
		$exists = $db->getOne($sql);
		if (DB::isError($exists)) trigger_error($exists->getMessage().'<br/>'.$exists->getUserInfo(), E_USER_ERROR);

		if ($exists) {
			trigger_error('Unable to create another System User Group, you are only allowed one', E_USER_WARNING);
			return false;
		}
		
		$ret_val = parent::create($link, 'System Administrators');
		$this->_in_create = false;
		return $ret_val;

	}//end create()


	/**
	* Creates a duplicate of this asset. It DOES NOT duplicate the links that this asset has either to it or from it
	* Returns a reference to the new object or if an error occured FALSE
	*
	* @param bool	$dupe_directory	whether or not to duplicate thia assets data directory
	*
	* @return object Asset | NULL
	* @access public
	* @see Asset::duplicate()
	*/
	function &duplicate($dupe_directory=true)
	{
		trigger_error('Unable to duplicate System User Group, you are only allowed one', E_USER_WARNING);
		$null = NULL; // 'cause we need to return by reference :)
		return $null;
	}

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
	}


	/**
	* Set the value for the passed variable to the passed value
	*
	* @access public
	*/
	function setAttrValue($name, $value) {

		if ($name == 'name' && !$this->_in_create) {
			trigger_error('You are not able to alter the name for the system administrators user group', E_USER_WARNING);
			return false;
		}

		return parent::setAttrValue($name, $value);

	}//end setAttrValue()

}//end class
?>
