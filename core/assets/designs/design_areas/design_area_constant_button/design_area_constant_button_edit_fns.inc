<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: design_area_constant_button_edit_fns.inc,v 1.8 2003/10/14 05:07:56 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area_edit_fns.inc';

/**
* Design_Area_Constant_Button_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Constant_Button_Edit_Fns extends Design_Area_Edit_Fns
{
	/**
	* Returns an array of vars that are setable, used to create the parseString
	*
	* @param object Asset	$asset	the asset whose interface we are painting
	*
	* @return Array
	* @access public
	* @see createParseString()
	*/
	function _getSetableVars(&$asset)
	{
		$vars = parent::_getSetableVars($asset);
		unset($vars['asset_contents']);
		unset($vars['divider_contents']);
		return $vars;

	}// end _getSetableVars()


	/**
	* Prints the interface for linking to an assetid
	*
	* @param object Asset				&$asset	the owning asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintAssetid(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess('attributes')) {
			// if they are trying to set the type_codes, split the string
			$type_codes = preg_split('/[ ,]+/', (string) $asset->attr('type_codes'), -1, PREG_SPLIT_NO_EMPTY);
			$type_code_restrictions = Array();
			foreach($type_codes as $type_code) $type_code_restrictions[$type_code] = 'D';
			asset_finder($prefix.'_assetid', $asset->attr('assetid'), $type_code_restrictions);

		} elseif ($asset->attr('assetid')) {
			$link_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($asset->attr('assetid'));
			if (!is_null($link_asset)) echo $link_asset->name, '(Id: #', $link_asset->id, ')';

		}// end if
		return true;
	}//end paintAssetid()


	/**
	* Processes the interface for linking to an asset
	*
	* @param object Asset				&$asset	the owning asset
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

		return $asset->setAttrValue('assetid', (int) $_POST[$prefix.'_assetid']);

	}//end processAssetid()


}//end class
?>
