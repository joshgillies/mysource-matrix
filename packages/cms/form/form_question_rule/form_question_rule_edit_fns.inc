<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: form_question_rule_edit_fns.inc,v 1.3 2003/09/26 05:26:41 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Form_Question_Rule_Edit_Fns
*
* Purpose
*
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Question_Rule_Edit_Fns extends Asset_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function Form_Question_Rule_Edit_Fns()
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
