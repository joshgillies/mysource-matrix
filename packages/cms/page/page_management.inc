<?php

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve::packages::cms
*/
class Page_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Page_Management(&$pm)
	{
		$this->Asset_Management(&$pm);

		$this->vars = Array('short_name' => Array(
												'added'     => '0.0.1', 'removed'   => '9.9.0',
												'type'      => 'text',  'default'   => '',
												'update_fn' => ''
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

		switch($current_version) {
			case '0.1': 
				# 0.1 to 0.2 upgrade stuff here
			case '0.2': 
				# 0.2 to 0.3 upgrade stuff here
			case '0.3': 
				# 0.3 to 0.4 upgrade stuff here
			case '0.4': 
				# 0.4 to 0.5 upgrade stuff here
		}

		return $ok;

	}#end upgrade()

	function update_short_name() {

		pre_echo('UPDATE SHORT NAME');

		return true;

	}#end update_short_name()

}#end class
?>
