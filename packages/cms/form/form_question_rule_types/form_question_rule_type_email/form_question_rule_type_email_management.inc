<?php

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Form_Question_Rule_Type_Email_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
* @subpackage __core__
*/
class Form_Question_Rule_Type_Email_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Form_Question_Rule_Type_Email_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array();
	}//end constructor

}//end class
?>
