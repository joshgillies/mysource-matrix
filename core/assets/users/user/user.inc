<?php

require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* User
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
*/
class User extends Asset
{

	/**
	* Constructor
	*
	* @param int    $assetid      the asset id to be loaded
	*
	*/
	function User($assetid=0) 
	{
		$this->Asset($assetid);
	}


	/*
	* Create this asset
	*
	* @param string $username   the unique username 
	* @param string $password   
	* @param string $first_name 
	* @param string $last_name  
	* @param string $email      
	*
	* @return boolean
	* @access public
	*/
	function create($username, $password, $first_name='', $last_name='', $email='')
	{

		if (parent::create()) {

			$this->setAttribute('username',   $username);
			$this->setAttribute('password',   $password);
			$this->setAttribute('first_name', $first_name);
			$this->setAttribute('last_name',  $last_name);
			$this->setAttribute('email',      $email);
			return true;

		}//end if

		return false;

	}//end create()

	/*
	* Set the value for the passed variable to the passed value
	*
	* @access public
	*/
	function setAttribute($name, $value) {

		trigger_error('CHECK USER INPUTS FOR USERNAME, PASSWORD ETC');

		if ($name == 'password') {
			$value = crypt($value); // let salt be generated
		}

		return parent::setAttribute($name, $value);

	}//end setAttribute()

	/*
	* returns whether the passed password is this users
	*
	* @access public
	*/
	function comparePassword($entered_password) {

		$password = $this->attr('password');

		if (!trim($password)) {
			return false;
		}

		return (crypt($entered_password,$password) == $password);

	}//end comparePassword()

	/*
	* Returns name of the user
	*
	* @see Asset::_getName()
	* @access public
	*/
	function _getName() {
		return $this->attr('first_name').' '.$this->attr('last_name');
	}//end _getName();

	/*
	* Attempts to find a user that matches the passed username and login, returning it's assetid
	*
	* @param string	$username
	* @param string	$password
	*
	* @return int
	* @access public
	*/
	function find($username, $password) {
		include_once SQ_LIB_PATH.'/db_extras/db_extras.inc';

		$db = &$GLOBALS['SQ_RESOLVE']->getDb();

		$sql = 'SELECT a.assetid
				FROM sq_asset a, sq_asset_attribute at, sq_asset_attribute_value av
				WHERE at.type_code = '.$db->quote('user').'
				  AND a.type_code IN (~SQ0~) 
				  AND at.name     = '.$db->quote('username').'
				  AND at.attributeid = av.attributeid
				  AND av.assetid  = a.assetid
				  and av.value    = '.$db->quote($username);

		$subs = Array( 'SELECT type_code FROM sq_asset_type_inherited 
						WHERE inherited_type_code = '.$db->quote('user').'
						   OR type_code = '.$db->quote('user'));

		$sql = db_extras_subquery($db, $sql, $subs);

		$result = $db->getOne($sql);
		if (DB::isError($result)) trigger_error($result->getMessage().'<br>'.$result->getUserInfo(), E_USER_ERROR);

		if (!$result) return 0;

		$asset = $GLOBALS['SQ_RESOLVE']->am->getAsset($result);
		if (is_null($asset) || !$asset->comparePassword($password)) {
			return 0;
		}

		return $result;

	}//end find();


}//end class
?>
