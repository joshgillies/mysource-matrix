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

		}#end if

		return false;

	}#end create()

	/*
	* Set the value for the passed variable to the passed value
	*
	* @access public
	*/
	function setAttribute($name, $value) {

		trigger_error('CHECK USER INPUTS FOR USERNAME, PASSWORD ETC');

		if ($name == 'password') {
			$value = crypt($value); # let salt be generated
		}

		return parent::setAttribute($name, $value);

	}#end setAttribute()

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

	}#end comparePassword()

	/*
	* Returns name of the user
	*
	* @ see Asset::name()
	* @access public
	*/
	function name() {
		return $this->attr('first_name').' '.$this->attr('last_name');
	}#end name();

}#end class
?>
