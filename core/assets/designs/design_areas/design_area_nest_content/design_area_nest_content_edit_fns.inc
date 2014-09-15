<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: design_area_nest_content_edit_fns.inc,v 1.19 2012/08/30 01:09:06 ewang Exp $
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
* @version $Revision: 1.19 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Nest_Content_Edit_Fns extends Design_Area_Edit_Fns
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
	* @param Design_Area_Nest_Content	$asset	the owning asset
	* @param Backend_Outputter			$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintAssetid(Design_Area_Nest_Content $asset, Backend_Outputter $o, $prefix)
	{
		$type_codes = $asset->attr('type_codes');
		$paint_with = $asset->attr('paint_with');
		$paint_layout = $asset->attr('paint_layout');
		$type_code_restrictions = Array();
		if (is_array($type_codes)) {
			foreach ($type_codes as $type_code) {
				$type_code_restrictions[$type_code] = 'D';
			}
		}
		if ($asset->writeAccess('links')) {
			?>
			<table class="sq-backend-table sq-nest-content-table">
				<tr>
					<th><?php echo translate('nest_content_menu_nesting') ?></th>
				</tr>
				<tr>
					<td class="sq-backend-table-cell">
					<?php
						asset_finder($prefix.'_assetid', $asset->_getNestAssetInfo('minorid'), $type_code_restrictions);
					?>
					</td>
				</tr>
				<tr>
					<th><?php echo translate('nest_content_menu_paint_with'); ?></th>
				</tr>
				<tr>
					<td class="sq-backend-table-cell">
					<input name="<?php echo $prefix ?>_paint_with" id="<?php echo $prefix ?>_paint_with_0" type="radio" value="raw" <?php if ($paint_with == 'raw') { ?>checked="checked"<?php } ?>>
					<?php label(translate('nest_content_menu_paint_raw'), $prefix.'_paint_with_0', 'style="display: inline-block; margin-bottom: 5px;"'); ?>
					<br />
					<input name="<?php echo $prefix ?>_paint_with" id="<?php echo $prefix ?>_paint_with_1" type="radio" value="default" <?php if ($paint_with == 'default') { ?>checked="checked"<?php } ?>>
					<?php label(translate('nest_content_menu_paint_default'), $prefix.'_paint_with_1'); ?>
					<br />
					<input name="<?php echo $prefix ?>_paint_with" id="<?php echo $prefix ?>_paint_with_2" type="radio" value="arbitrary" <?php if ($paint_with == 'arbitrary') { ?>checked="checked"<?php } ?>>
					<?php label(translate('nest_content_menu_paint_arbitrary'), $prefix.'_paint_with_2'); ?>
					<?php asset_finder($prefix.'_paint_layout', $paint_layout, Array('paint_layout_page' => 'D'));
					?><br />
					</td>
				</tr>
			</table>
			<?php
		} else if ($asset->_getNestAssetInfo('minorid')) {
			$link_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($asset->_getNestAssetInfo('minorid'));
			if (!is_null($link_asset)) {
				echo get_asset_tag_line($link_asset->id);
			}
		}

		return TRUE;

	}//end paintAssetid()


	/**
	* Processes the interface for linking to an asset
	*
	* @param Design_Area_Nest_Content	$asset	the owning asset
	* @param Backend_Outputter			$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	public function processAssetid(Design_Area_Nest_Content $asset, Backend_Outputter $o, $prefix)
	{
		if (!isset($_POST[$prefix.'_assetid']['assetid'])) {
			return FALSE;
		}

		$assetid = (int) $_POST[$prefix.'_assetid']['assetid'];

		// open the transaction
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// Save the paint layout information
		if (isset($_POST[$prefix.'_paint_layout']['assetid'])) {
			$paint_layout = $_POST[$prefix.'_paint_layout']['assetid'];
			$asset->setAttrValue('paint_layout', $paint_layout);
		}//end if

		if (isset($_POST[$prefix.'_paint_with'])) {
			$paint_with = $_POST[$prefix.'_paint_with'];
			if (($paint_with != ('raw'||'default'||'arbitrary'))) {
				// Something went very wrong, use the default
				$paint_with = 'raw';
			}
			$asset->setAttrValue('paint_with', $paint_with);
		}//end if

		// Update the nested content asset
		if ($asset->_getNestAssetInfo('minorid') != $_POST[$prefix.'_assetid']['assetid']) {
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
				$link_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
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
		}//end if

		// If we get this far, then it's all OK
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return TRUE;

	}//end processAssetid()


}//end class
?>
