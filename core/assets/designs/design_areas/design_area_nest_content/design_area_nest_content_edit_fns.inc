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
* $Id: design_area_nest_content_edit_fns.inc,v 1.8.2.1 2004/03/02 18:35:51 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area_edit_fns.inc';

/**
* Design_Area_Nest_Content_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Nest_Content_Edit_Fns extends Design_Area_Edit_Fns
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
		if ($asset->writeAccess('links')) {
			$type_codes = $asset->attr('type_codes');
			$type_code_restrictions = Array();
			if (is_array($type_codes)) {
				foreach($type_codes as $type_code) $type_code_restrictions[$type_code] = 'D';
			}
			asset_finder($prefix.'_assetid', $asset->_getNestAssetInfo('minorid'), $type_code_restrictions);

		} elseif ($asset->_getNestAssetInfo('minorid')) {
			$link_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($asset->_getNestAssetInfo('minorid'));
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
		if (!isset($_POST[$prefix.'_assetid']['assetid'])) return false;

		if ($asset->_getNestAssetInfo('minorid') == $_POST[$prefix.'_assetid']['assetid']) return false;

		$assetid = $_POST[$prefix.'_assetid']['assetid'];

		// open the transaction
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// remove the old link first
		if ($old_linkid = $asset->_getNestAssetInfo('linkid')) {
			if (!$asset->deleteLink($old_linkid)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}
		}

		// now create the new link
		if ($assetid) {
			$link_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			if (is_null($link_asset)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}
			if (!$asset->createLink($link_asset, SQ_LINK_NOTICE, 'nest_content_asset')) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}
		}

		// If we get this far, then it's all OK
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		return true;

	}//end processAssetid()

}//end class

?>