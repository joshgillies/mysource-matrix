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
* @package Resolve::packages::__core__
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
	* @param array(string)	$link	information used to create the initial link
	* @param string			$email
	*
	* @return boolean
	* @access public
	*/
	function create($link, $email='')
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
		
		$ret_val = parent::create($link, 'root', 'root', 'Root', 'User', $email);
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
	* Set the value for the passed variable to the passed value
	*
	* @access public
	*/
	function setAttrValue($name, $value) {

		if ($name == 'username' && !$this->_in_create) {
			trigger_error('You are not able to alter the username for the root user', E_USER_WARNING);
			return false;
		}

		return parent::setAttrValue($name, $value);

	}//end setAttrValue()

}//end class
?>
