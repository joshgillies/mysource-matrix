<?php

require_once SQ_CORE_PACKAGE_PATH.'/system_user/system_user.inc';


/**
* Root_User
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
* @subpackage __core__
*/
class Root_User extends System_User
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

		$this->_in_create = true;

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = 'SELECT count(*) FROM sq_asset WHERE type_code = '.$db->quote($this->type());
		$exists = $db->getOne($sql);
		if (DB::isError($exists)) trigger_error($exists->getMessage().'<br/>'.$exists->getUserInfo(), E_USER_ERROR);

		if ($exists) {
			trigger_error('Unable to create another Root User, you are only allowed one', E_USER_WARNING);
			return false;
		}
		
		$this->setAttrValue('username',   'root');
		$this->setAttrValue('password',   'root');
		$this->setAttrValue('first_name', 'Root');
		$this->setAttrValue('last_name',  'User');

		$ret_val = parent::create($link);
		$this->_in_create = false;
		return $ret_val;

	}//end create()


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
	function canLogin() {
		return true;
	}


	/**
	* Set the value for the passed variable to the passed value
	*
	* @param string		$name				the name of the attribute
	* @param string		$value				the new value of the attribute
	* @param boolean	$dont_run_updated	if true this->_updated() won't be run (THIS IS ONLY EVER CALLED FROM Asset::create())
	*
	* @return boolean
	* @access public
	*/
	function setAttrValue($name, $value, $dont_run_updated=false)
	{

		if ($name == 'username' && !$this->_in_create) {
			trigger_error('You are not able to alter the username for the root user', E_USER_WARNING);
			return false;
		}

		return parent::setAttrValue($name, $value, $dont_run_updated);

	}//end setAttrValue()

}//end class
?>
