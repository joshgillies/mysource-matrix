<?php

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
* @subpackage __core__
*/
class Design_Customisation_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Design_Customisation_Management(&$pm)
	{
		$this->Asset_Management($pm);
	}//end constructor

}//end class
?>
