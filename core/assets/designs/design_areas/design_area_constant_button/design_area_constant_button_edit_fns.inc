<?php

require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area_edit_fns.inc';

/**
* Design_Area_Constant_Button_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve::packages::__core__
*/
class Design_Area_Constant_Button_Edit_Fns extends Design_Area_Edit_Fns
{

	/**
	* Prints the interface for uploading a file
	*
	* @param object Image				&$asset	the file asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintAssetid(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess()) {
			$type_codes = $asset->attr('type_codes');
			$type_code_restrictions = Array();
			if (is_array($type_codes)) {
				foreach($type_codes as $type_code) $type_code_restrictions[$type_code] = 'D';
			}
			asset_finder($prefix.'_assetid', $asset->attr('assetid'), $type_code_restrictions);

		} elseif ($asset->attr('assetid')) {
			$link_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($asset->attr('assetid'));
			if (!is_null($link_asset)) echo $link_asset->name, '(Id: #', $link_asset->id, ')';

		}// end if
		return true;
	}//end paintLink()


	/**
	* Processes the interface for uploading a file
	*
	* @param object Image				&$asset	the file asset
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processAssetid(&$asset, &$o, $prefix)
	{
		if (!isset($_POST[$prefix.'_assetid'])) return false;

		if ($asset->attr('assetid') == $_POST[$prefix.'_assetid']) return false;

		$asset->_tmp['regenerate_design'] = true;
		return $asset->setAttrValue('assetid', (int) $_POST[$prefix.'_assetid']);

	}//end processAssetidLink()


}//end class
?>
