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
* $Id: design_area_constant_button_edit_fns.inc,v 1.21 2007/12/06 04:28:48 lwright Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area_edit_fns.inc';

/**
* Design_Area_Constant_Button_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.21 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Constant_Button_Edit_Fns extends Design_Area_Edit_Fns
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
	* Prints the interface for linking to an assetid
	*
	* @param Design_Area_Constant_Button	$asset	the owning asset
	* @param Backend_Outputter				$o		the backend outputter
	* @param string							$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	public function paintAssetid(Design_Area_Constant_Button $asset, Backend_Outputter $o, $prefix)
	{
		if ($asset->writeAccess('attributes')) {
			// if they are trying to set the type_codes, split the string
			asset_finder($prefix.'_assetid', $asset->attr('assetid'));

		} else if ($asset->attr('assetid')) {
			$link_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($asset->attr('assetid'));
			if (!is_null($link_asset)) {
				echo get_asset_tag_line($link_asset->id);
			}

		}// end if

		return TRUE;

	}//end paintAssetid()


	/**
	* Processes the interface for linking to an asset
	*
	* @param Design_Area_Constant_Button	$asset	the owning asset
	* @param Backend_Outputter				$o		the backend outputter
	* @param string							$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	public function processAssetid(Design_Area_Constant_Button $asset, Backend_Outputter $o, $prefix)
	{
		if (!isset($_POST[$prefix.'_assetid']['assetid'])) {
			return FALSE;
		}

		if ($asset->attr('assetid') == $_POST[$prefix.'_assetid']['assetid']) {
			return FALSE;
		}

		return $asset->setAttrValue('assetid', (int) $_POST[$prefix.'_assetid']['assetid']);

	}//end processAssetid()


}//end class
?>