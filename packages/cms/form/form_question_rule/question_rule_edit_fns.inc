<?php

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Question_Rule_Edit_Fns
*
* Purpose
*
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
* @subpackage cms
*/
class Question_Rule_Edit_Fns extends Asset_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function Question_Rule_Edit_Fns()
	{
		$this->Asset_Edit_Fns();
	}


	/**
	* @abstract
	*/
	function paintInlineBackend()
	{
		return true;
	}


	/**
	* @abstract
	*/
	function processInlineBackend()
	{
		return true;
	}

	function processDelete()
	{
		if (isset($_POST['delete_rule'])) {
			$ruleids = $_POST['delete_rule'];
			bam($ruleids);
		}
	}

}//end class
?>
