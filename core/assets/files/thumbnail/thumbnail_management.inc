<?php

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Thumbnail_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Resolve::packages::__core__
*/
class Thumbnail_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Thumbnail_Management(&$pm)
	{
		$this->Asset_Management($pm);
	}//end constructor

}//end class
?>
