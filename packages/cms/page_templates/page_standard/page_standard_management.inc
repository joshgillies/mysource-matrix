<?php

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Standard_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
* @subpackage cms
*/
class Page_Standard_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Page_Standard_Management(&$pm)
	{
		$this->Asset_Management($pm);
	}//end constructor

}//end class
?>
