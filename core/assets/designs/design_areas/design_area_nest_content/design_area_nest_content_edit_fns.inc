<?php

require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area_edit_fns.inc';

/**
* Design_Area_Nest_Content_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
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

		if ($asset->writeAccess()) {
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
		if (!isset($_POST[$prefix.'_assetid'])) return false;

		if ($asset->_getNestAssetInfo('minorid') == $_POST[$prefix.'_assetid']) return false;

		$assetid = (int) $_POST[$prefix.'_assetid'];

		// open the transaction
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// remove the old link first
		if ($old_linkid = $asset->_getNestAssetInfo('linkid')) {
		pre_echo("DELETE $old_linkid");
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
		pre_echo("CREATE $old_linkid");
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
