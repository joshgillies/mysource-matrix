<?php

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Workspace_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Resolve::packages::__core__
*/
class Workspace_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function File_Management(&$pm)
	{
		$this->Asset_Management($pm);
	}//end constructor

}//end class
?>
