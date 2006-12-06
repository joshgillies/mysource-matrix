<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: paint_layout_bodycopy_edit_fns.inc,v 1.10 2006/12/06 02:53:10 emcdonald Exp $
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
* @version $Revision: 1.10 $
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
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the backend outputter object
	* @param string	$prefix	prefix for the form element
	*
	* @return void
	* @access public
	*/
	function paintConditionalKeywords(&$asset, &$o, $prefix)
	{
		$allowed_keywords = Array();
		$asset->getKeywordsDescription($allowed_keywords, $asset->getAssociatedAssetType());
		$allowed_keywords = array_keys($allowed_keywords);

		$write_access = $asset->writeAccess('attributes');
		$condition_list =& new Conditions_List($asset, 'conditional_keywords');
		return $condition_list->paintConditionsList($asset, $o, $prefix, $write_access, $allowed_keywords);

	}//end paintConditionalKeywords()


	/**
	* Processses adding a new user condition
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the backend outputter object
	* @param string	$prefix	prefix for the form element
	*
	* @return void
	* @access public
	*/
	function processConditionalKeywords(&$asset, &$o, $prefix)
	{
		$allowed_keywords = Array();
		$asset->getKeywordsDescription($allowed_keywords, $asset->getAssociatedAssetType());
		$allowed_keywords = array_keys($allowed_keywords);

		$write_access = $asset->writeAccess('attributes');
		$condition_list =& new Conditions_List($asset, 'conditional_keywords');
		return $condition_list->processConditionsList($asset, $o, $prefix, $write_access, $allowed_keywords);

	}//end processConditionalKeywords()


}//end class

?>
