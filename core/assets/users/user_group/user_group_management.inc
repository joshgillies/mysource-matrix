<?php

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* User_Group_Management
*
* @see Asset_Management
*
* @author  Greg Serwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
* @subpackage __core__
*/
class User_Group_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function User_Group_Management(&$pm)
	{
		$this->Asset_Management($pm);

	}//end constructor

}//end class
?>
