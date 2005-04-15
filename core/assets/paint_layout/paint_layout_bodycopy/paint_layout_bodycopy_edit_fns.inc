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
* $Id: paint_layout_bodycopy_edit_fns.inc,v 1.3 2005/04/15 00:37:26 lwright Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/bodycopy/bodycopy/bodycopy_edit_fns.inc';

/**
* Paint_Layout_Bodycopy_Edit_Fns
*
* Purpose
*
*
* @author Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Paint_Layout_Bodycopy_Edit_Fns extends Bodycopy_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Paint_Layout_Bodycopy_Edit_Fns()
	{
		$this->Bodycopy_Edit_Fns();

	}//end constructor


	/**
	* Paints adding a new user condition
	*
	* @access public
	* @return void
	*/
	function paintConditionalKeywords(&$asset, &$o, $prefix)
	{
		$allowed_keywords = Array();
		$asset->getKeywordsDescription($allowed_keywords, $asset->getAssociatedAssetType());
		$allowed_keywords = array_keys($allowed_keywords);

		$write_access = $asset->writeAccess('attributes');
		$condition_list = new Conditions_List($asset, 'conditional_keywords');
		return $condition_list->paintConditionsList($asset, $o, $prefix, $write_access, $allowed_keywords);

	}


	/**
	* Processses adding a new user condition
	*
	* @access public
	* @return void
	*/
	function processConditionalKeywords(&$asset, &$o, $prefix)
	{
		$allowed_keywords = Array();		
		$asset->getKeywordsDescription($allowed_keywords, $asset->getAssociatedAssetType());
		$allowed_keywords = array_keys($allowed_keywords);

		$write_access = $asset->writeAccess('attributes');
		$condition_list = new Conditions_List($asset, 'conditional_keywords');
		return $condition_list->processConditionsList($asset, $o, $prefix, $write_access, $allowed_keywords);

	}


	/**
	* Processes the interface for creating containers in the bodycopy
	*
	* @param object Asset				$asset	the asset to which we belong
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processBodycopy(&$asset, &$o, $prefix)
	{
		if (!parent::processBodycopy($asset, $o, $prefix)) return false;

		$content = $asset->getRawBodycopyContent();
		$keywords = retrieve_keywords_replacements($content);

		$stack = Array();

		$validated = true;

		// we rely on the fact that the retrieve_keywords_replacements() returns
		// all keywords (even duplicates) in the order they exist in content
		foreach ($keywords as $keyword) {
			if (preg_match('/^('.SQ_KEYWORD_CONDITION_BEGIN.'|'.SQ_KEYWORD_CONDITION_END.'|'.SQ_KEYWORD_CONDITION_ELSE.')_(.*)$/', $keyword, $condition_parts)) {
				$test_keyword = $condition_parts[2];
				switch ($condition_parts[1]) {
					case SQ_KEYWORD_CONDITION_BEGIN :
						array_push($stack, $test_keyword);
						break;
					case SQ_KEYWORD_CONDITION_ELSE :
						$current_scope = end($stack);
						if (end($stack) != $test_keyword) {
							trigger_localised_error("CORE0202", E_USER_WARNING, $keyword, SQ_KEYWORD_CONDITION_ELSE.'_'.$current_scope, SQ_KEYWORD_CONDITION_END.'_'.$current_scope, SQ_KEYWORD_CONDITION_BEGIN.'_'.$test_keyword);
							$validated = false;
							break 2; //exit out of loop
						}
						break;
					case SQ_KEYWORD_CONDITION_END :
						$current_scope = array_pop($stack);
						if ($current_scope != $test_keyword) {
							trigger_localised_error("CORE0202", E_USER_WARNING, $keyword, SQ_KEYWORD_CONDITION_ELSE.'_'.$current_scope, SQ_KEYWORD_CONDITION_END.'_'.$current_scope, SQ_KEYWORD_CONDITION_BEGIN.'_'.$test_keyword);
							$validated = false;
							break 2; //exit out of loop
						}
						break;
				}//end switch
		 	}// end if
		}// end foreach

		if (!$asset->setAttrValue('validated', $validated)) return false;

		// even if the validation failed we will still save the contents for the user
		return true;

	}//end processBodycopy()


}//end class

?>
