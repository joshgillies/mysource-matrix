<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: form_question_rule_edit_fns.inc,v 1.10 2004/07/07 01:05:29 lwright Exp $
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
	function paintInlineBackend(&$asset, &$o, $prefix)
	{
		return true;
	}


	/**
	* processes the section in the question to change the options of this rule
	* 
	* @param object Form				&$asset		the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o			the outputter class
	* @param string						$prefix		prefix for the form elements
	*
	* @access public
	* @return boolean
	*/
	function processInlineBackend(&$asset, &$o, $prefix)
	{
		if ($this->processDelete($asset, $o, $prefix)) return true;

		if (isset($_POST[$prefix.'_operator'])) {
			$asset->setAttrValue('operator', $_POST[$prefix.'_operator']);
		}
		if (isset($_POST[$prefix.'_value'])) {
			$asset->setAttrValue('value', $_POST[$prefix.'_value']);
		}
		if (isset($_POST[$prefix.'_comparison_question_id'])) {
			$asset->setAttrValue('comparison_question_id', $_POST[$prefix.'_comparison_question_id']);
		}

		return $asset->saveAttributes();

	}//end processInlineBackend()


	/**
	* deletes rules
	* 
	* @param object Form				&$asset		the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o			the outputter class
	* @param string						$prefix		prefix for the form elements
	*
	* @access public
	* @return boolean
	*/
	function processDelete(&$asset, &$o, $prefix)
	{
		$my_question = &$asset->getMyQuestion();
		bam($my_question->id);
		if (isset($_POST['delete_rule'])) {
			$ruleids = $_POST['delete_rule'];
			$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($my_question->id, SQ_LINK_TYPE_3);
			bam($links);
			foreach ($links as $link) {
				bam('here');
				if (empty($link)) continue;
				bam($link);
				if(in_array($link['minorid'], $_POST['delete_rule'])) { 
					$asset->deleteLink($link['linkid']);
				}
			}
			return true;
		}

		return false;
	}//end processDelete()

}//end class
?>