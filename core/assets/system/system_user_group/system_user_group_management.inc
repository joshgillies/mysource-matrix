<?php

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* System_User_Group_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class System_User_Group_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function System_User_Group_Management(&$pm)
	{
		$this->Asset_Management($pm);

	}//end constructor

}//end class
?>
