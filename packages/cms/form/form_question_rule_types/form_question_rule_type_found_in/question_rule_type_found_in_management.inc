<?php

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Question_Rule_Type_Found_In_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
* @subpackage __core__
*/
class Question_Rule_Type_Found_In_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Question_Rule_Type_Found_In_Management(&$pm)
	{
		$this->Asset_Management($pm);

	}//end constructor

}//end class
?>
