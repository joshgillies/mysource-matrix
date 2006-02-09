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
* $Id: paint_layout_bodycopy_edit_fns.inc,v 1.4.2.1 2006/02/09 06:36:59 lwright Exp $
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
* @version $Revision: 1.4.2.1 $
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


}//end class

?>
