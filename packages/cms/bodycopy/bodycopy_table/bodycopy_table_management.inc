<?php

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Bodycopy_Table_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Resolve::packages::cms
*/
class Bodycopy_Table_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Bodycopy_Table_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array('name' => Array(
												'added'     => '0.0.1',
												'type'      => 'text',  'default'   => '',
												'update_fn' => ''
												),
							'rows' => Array(
												'added'     => '0.0.1',
												'type'      => 'array',  'default'   => '',
												'update_fn' => ''
												),
							'attributes' => Array(
												'added'     => '0.0.1',
												'type'      => 'array',  'default'   => '',
												'update_fn' => ''
												)
							);
	}//end constructor


	/**
	* Install
	*
	* @access public
	*/
	function _install()
	{
		$ok = parent::_install();
		pre_echo($this->getAssetType().' Specific install stuff here');
		return $ok;

	}//end install


	/**
	* Upgrade
	*
	* @param string	$current_version	the version of the asset that is currenly installed
	*
	* @return boolean
	* @access public
	*/
	function _upgrade($current_version)
	{
		$ok = parent::_upgrade($current_version);
		pre_echo($this->getAssetType().' Specific upgrade stuff here');

		switch($current_version) {
			case '0.1': 
				// 0.1 to 0.2 upgrade stuff here
			case '0.2': 
				// 0.2 to 0.3 upgrade stuff here
			case '0.3': 
				// 0.3 to 0.4 upgrade stuff here
			case '0.4': 
				// 0.4 to 0.5 upgrade stuff here
		}

		return $ok;

	}//end upgrade()

}//end class
?>
