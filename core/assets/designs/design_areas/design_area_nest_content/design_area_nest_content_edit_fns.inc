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
* $Id: design_area_nest_content_edit_fns.inc,v 1.15 2006/12/05 22:42:13 emcdonald Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area_edit_fns.inc';

/**
* Design_Area_Nest_Content_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.15 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Nest_Content_Edit_Fns extends Design_Area_Edit_Fns
{


	/**
	* Prints the interface for linking to an assetid
	*
	* @param object	&$asset	the owning asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
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
				foreach ($type_codes as $type_code) {
					$type_code_restrictions[$type_code] = 'D';
				}
			}
			asset_finder($prefix.'_assetid', $asset->_getNestAssetInfo('minorid'), $type_code_restrictions);

		} else if ($asset->_getNestAssetInfo('minorid')) {
			$link_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($asset->_getNestAssetInfo('minorid'));
			if (!is_null($link_asset)) {
				echo get_asset_tag_line($link_asset->id);
			}

		}// end if

		return TRUE;

	}//end paintAssetid()


	/**
	* Processes the interface for linking to an asset
	*
	* @param object	&$asset	the owning asset
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processAssetid(&$asset, &$o, $prefix)
	{
		if (!isset($_POST[$prefix.'_assetid']['assetid'])) {
			return FALSE;
		}

		if ($asset->_getNestAssetInfo('minorid') == $_POST[$prefix.'_assetid']['assetid']) {
			return FALSE;
		}

		$assetid = (int) $_POST[$prefix.'_assetid']['assetid'];

		// open the transaction
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// remove the old link first
		if ($old_linkid = $asset->_getNestAssetInfo('linkid')) {
			if (!$asset->deleteLink($old_linkid)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}
		}

		// now create the new link
		if ($assetid) {
			$link_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			if (is_null($link_asset)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}
			if (!$asset->createLink($link_asset, SQ_LINK_NOTICE, 'nest_content_asset')) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}
		}

		// If we get this far, then it's all OK
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return TRUE;

	}//end processAssetid()


}//end class
?>