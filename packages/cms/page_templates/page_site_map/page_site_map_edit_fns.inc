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
* $Id: page_site_map_edit_fns.inc,v 1.9 2004/06/16 06:12:06 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page_edit_fns.inc';

/**
* Page_Site_Map_Edit_Fns
*
* Purpose
*
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Site_Map_Edit_Fns extends Page_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Page_Site_Map_Edit_Fns()
	{
		$this->Page_Edit_Fns();
		$this->static_screens['details']['force_unlock'] = false;
		$this->static_screens['details']['lock_type'] = 'content';

	}//end constructor


	/**
	* Paints the root node selection box
	*
	* @param object	Asset				&$rootid	asset being painted
	* @param object Backend_Outputter	&$o			backend outputter
	* @param string						$prefix		prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function paintRootNode(&$asset, &$o, $prefix)
	{
		$root_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, '', false, 'root');
		$write_access = $asset->writeAccess('links');

		bam($root_link);
		
		if (!$write_access) {
			$root_asset = null;
			if (!empty($root_link)) $root_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($root_link['minorid']);
			if (!is_null($root_asset)) echo $root_asset->name . ' (Id: #' . $root_asset->id . ')';
			else echo 'No root node has been selected';
		} else {
			asset_finder($prefix.'_rootid', (!empty($root_link)) ? $root_link['minorid'] : '');
		}

		return $write_access;

	}//end paintRootNode()


	/**
	* Processes the value input from root node selection box
	*
	* @param object	Asset				&$rootid	asset being painted
	* @param object Backend_Outputter	&$o			backend outputter
	* @param string						$prefix		prefix for the html doc element name
	*
	* @return void
	* @access public
	*/
	function processRootNode(&$asset, &$o, $prefix)
	{
		if (!isset($_POST[$prefix.'_rootid']['assetid'])) return false;
		$new_root  = $_POST[$prefix.'_rootid']['assetid'];

		// getting existing root link
		$root_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, '', false, 'root');

		// dont process anything if the root node has not changed
		if (!empty($root_link) && $root_link['minorid'] == $new_root) return false;

		$ok = true;

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// deleting old root link if any found...
		if (!empty($root_link) && !$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($root_link['linkid'])) {
			$ok = false;
		} else if ($new_root) {
			$root = &$GLOBALS['SQ_SYSTEM']->am->getAsset($new_root);
			if (!is_null($root)) {
				if (!$asset->createLink($root, SQ_LINK_NOTICE, 'root')) {
					$ok = false;
					trigger_error('Failed adding new root node "'.$root->name.'" [#'.$root->id.']', E_USER_WARNING);
				}
			}
		}

		if ($ok) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			return true;
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

	}//end processRootNode()


	/**
	* paints the exclude list and the asset finder for adding to the exclude list
	*
	* @param object Form				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintExcludeList(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$curr_list = $asset->attr('exclude_list');

		if (empty($curr_list)) {
			echo '<b>No assets are currently excluded</b>';
		} else {
			$curr_list_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_keys($curr_list));
			?>
			<table class="sq-backend-table">
				<tr>
					<td class="sq-backend-table-header" width="100%">Asset Name</td>
					<?php
					if ($write_access) {
						?><td class="sq-backend-table-header" width="80">Remove?</td><?php
					}
					?>
				</tr>
				<?php
				foreach ($curr_list_info as $assetid => $asset_info) {
					?>
					<tr>
						<td class="sq-backend-table-cell"><?php echo $asset_info['name']; ?></td>
						<?php
						if ($write_access) {
							?>
							<td class="sq-backend-table-cell">
								<?php check_box($prefix.'_curr_exclude_list[]', $assetid, false); ?>
							</td>
							<?php
						}
						?>
					</tr>
					<?php
				}
				?>
			</table>
			<?php
		}

		if ($write_access) {
			echo '<br/><br/>choose an asset to exclude<br/>';
			asset_finder($prefix.'_exclude_list');
		}

		return $write_access;

	}//end paintExcludeList()


	/**
	* processes the exclude list and the asset finder for adding to the exclude list
	*
	* @param object Form				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processExcludeList(&$asset, &$o, $prefix)
	{
		$exclude_list = $asset->attr('exclude_list');
		if (isset($_POST[$prefix.'_exclude_list']['assetid']) && $_POST[$prefix.'_exclude_list']['assetid']) {
			$exclude_id = (int)$_POST[$prefix.'_exclude_list']['assetid'];
			$exclude_list[$exclude_id] = $exclude_id;
		}
		if (isset($_POST[$prefix.'_curr_exclude_list'])) {
			foreach ($_POST[$prefix.'_curr_exclude_list'] as $id) {
				unset($exclude_list[$id]);
			}
		}
		return $asset->setAttrValue('exclude_list', $exclude_list);

	}//end processExcludeList()


}//end class

?>