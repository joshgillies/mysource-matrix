<?php
/**
* +--------------------------------------------------------------------+
* | MySource 3 - MySource Matrix                                       |
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
* $Id: design_area_constant_button_edit_fns.inc,v 1.11 2003/11/18 15:37:24 brobertson Exp $
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
			asset_finder($prefix.'_assetid', $asset->attr('assetid'));

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
