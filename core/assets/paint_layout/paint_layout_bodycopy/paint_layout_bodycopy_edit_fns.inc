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
* $Id: paint_layout_bodycopy_edit_fns.inc,v 1.13 2012/02/15 04:42:34 akarelia Exp $
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
* @version $Revision: 1.13 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Paint_Layout_Bodycopy_Edit_Fns extends Bodycopy_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

	}//end constructor


	/**
	* Paints adding a new user condition
	*
	* @param Paint_Layout_Bodycopy	$asset	the asset to which we belong
	* @param Backend_Outputter		$o		the backend outputter object
	* @param string					$prefix	prefix for the form element
	*
	* @return void
	* @access public
	*/
	public function paintConditionalKeywords(Paint_Layout_Bodycopy $asset, Backend_Outputter $o, $prefix)
	{
		$allowed_keywords = Array();
		$asset->getKeywordsDescription($allowed_keywords, $asset->getAssociatedAssetType());
		$allowed_keywords = array_keys($allowed_keywords);

		if ($asset->getAssociatedAssetType() == 'form_submission') {
			foreach ($allowed_keywords as $idx => $allowed_keyword) {
				$allowed_keywords[$idx] = preg_replace('/(.*_)(X_qY)$/', '$1', $allowed_keyword);
			}
		}

		$write_access = $asset->writeAccess('attributes');
		$condition_list = new Conditions_List($asset, 'conditional_keywords');
		return $condition_list->paintConditionsList($asset, $o, $prefix, $write_access, $allowed_keywords);

	}//end paintConditionalKeywords()


	/**
	* Processses adding a new user condition
	*
	* @param Paint_Layout_Bodycopy	$asset	the asset to which we belong
	* @param Backend_Outputter		$o		the backend outputter object
	* @param string					$prefix	prefix for the form element
	*
	* @return void
	* @access public
	*/
	public function processConditionalKeywords(Paint_Layout_Bodycopy $asset, Backend_Outputter $o, $prefix)
	{
		$allowed_keywords = Array();
		$asset->getKeywordsDescription($allowed_keywords, $asset->getAssociatedAssetType());
		$allowed_keywords = array_keys($allowed_keywords);

		$write_access = $asset->writeAccess('attributes');
		$condition_list = new Conditions_List($asset, 'conditional_keywords');
		return $condition_list->processConditionsList($asset, $o, $prefix, $write_access, $allowed_keywords);

	}//end processConditionalKeywords()


}//end class

?>
