<?php

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Root_User_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve::packages::__core__
*/
class Root_User_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Root_User_Management(&$pm)
	{
		$this->Asset_Management(&$pm);

	}#end constructor

}#end class
?>
