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
* @package Resolve::packages::__core__
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


	/**
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

			$this->setAttrValue('username',   $username);
			$this->setAttrValue('password',   $password);
			$this->setAttrValue('first_name', $first_name);
			$this->setAttrValue('last_name',  $last_name);
			$this->setAttrValue('email',      $email);

			// create an inbox
			$GLOBALS['SQ_SYSTEM']->am->includeAsset('inbox');
			$inbox = new Inbox();
			$inbox->create();
			$this->createLink($inbox, SQ_LINK_EXCLUSIVE);

			return true;

		}//end if

		return false;

	}//end create()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks()
	{
		return Array(SQ_LINK_EXCLUSIVE => Array('inbox' => '1', 'workspace' => '1'),
					 SQ_LINK_UNITE     => Array(),
					 SQ_LINK_USES      => Array(),
					 SQ_LINK_NOTICE    => Array()
					);
	}//end _getAllowedLinks()


	/**
	* Set the value for the passed variable to the passed value
	* Overwritten here to check inputs (username and password) and to
	* encrypt password before setting value
	*
	* @access public
	*/
	function setAttrValue($name, $value) {

		trigger_error('CHECK USER INPUTS FOR USERNAME, PASSWORD ETC', E_USER_NOTICE);

		if ($name == 'password') {
			$value = crypt($value); // let salt be generated
		}

		return parent::setAttrValue($name, $value);

	}//end setAttrValue()


	/**
	* Returns whether the passed password matches this users' password
	*
	* @access public
	*/
	function comparePassword($entered_password)
	{

		$password = $this->attr('password');

		if (!trim($password)) {
			return false;
		}

		return (crypt($entered_password,$password) == $password);

	}//end comparePassword()


	/**
	* Returns name of the user
	*
	* @param bool	$short_name	whether or not we are after the shortname or the full name
	*
	* @see Asset::_getName()
	* @access public
	*/
	function _getName($short_name=false)
	{
		return ($this->attr('first_name') || $this->attr('last_name')) ? $this->attr('first_name').' '.$this->attr('last_name') : $this->attr('username');
	}//end _getName()


	/**
	* Attempts to find a user that matches the passed username and login, returning it's assetid
	*
	* @param string	$username
	* @param string	$password
	*
	* @return Array(int,string)
	* @access public
	*/
	function find($username, $password) {
		require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = 'SELECT a.assetid, a.type_code
				FROM sq_asset a, sq_asset_attribute at, sq_asset_attribute_value av
				WHERE a.type_code IN (~SQ0~) 
				  AND at.name         = '.$db->quote('username').'
				  AND at.attributeid  = av.attributeid
				  AND av.assetid      = a.assetid
				  and av.custom_value = '.$db->quote($username);

		$subs = Array( 'SELECT type_code FROM sq_asset_type_inherited 
						WHERE inherited_type_code = '.$db->quote('user').'
						   OR type_code = '.$db->quote('user'));

		$sql = db_extras_subquery($db, $sql, $subs);

		$result = $db->getRow($sql);
		if (DB::isError($result)) trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);

		if (!$result['assetid']) return array(0,'');

		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($result['assetid']);
		if (is_null($asset) || !$asset->comparePassword($password)) {
			return array(0,'');
		}

		return array($result['assetid'], $result['type_code']);

	}//end find();


	/**
	* Processes a backend submission from this asset, returns true if all OK
	*
	* @return boolean
	* @access public
	*/
	function processBackend() {

		switch($_POST['asset_action']) {
			case 'create' :
				$ei = &$this->getEI();
				$ei->process($this, true);

				$username = $this->getAttribute('username');
				$password = $this->getAttribute('password');
				if ($username->processed && $password->processed) {
					pre_echo('$this->create('.$username->value.', '.$password->value.')');
					return $this->create($username->value, $password->value);
				} else {
					return false;
				}

				break;

			default :
				return parent::processBackend();

		}// end switch

	}// end processBackend()


}//end class
?>
