<?php

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* User_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve::packages::__core__
*/
class User_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function User_Management(&$pm)
	{
		$this->Asset_Management(&$pm);

		$this->vars = Array('username' => Array(
												'added'     => '0.1',
												'type'      => 'text', 'default'   => ''
												),
							'password' => Array(
												'added'     => '0.1',
												'type'      => 'password', 'default'   => ''
												),
							'first_name' => Array(
												'added'     => '0.1',
												'type'      => 'text', 'default'   => ''
												),
							'last_name' => Array(
												'added'     => '0.1',
												'type'      => 'text', 'default'   => ''
												),
							'email'     => Array(
												'added'     => '0.1',
												'type'      => 'email',    'default'   => ''
												),
							'expiry_date' => Array(
												'added'     => '0.1',
												'type'      => 'date',    'default'   => ''
												)
							);
	}#end constructor

	/**
	* Install
	*
	*/
	function _install() 
	{
		$ok = parent::_install();
		pre_echo($this->getAssetType().' Specific install stuff here');
		return $ok;

	}#end install


	function _upgrade($current_version) {

		$ok = parent::_upgrade($current_version);
		pre_echo($this->getAssetType().' Specific upgrade stuff here');

		return $ok;

	}#end upgrade()

}#end class
?>
