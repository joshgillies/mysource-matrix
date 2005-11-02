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
* $Id: page_multiple_page_edit_fns.inc,v 1.1 2005/11/02 05:55:02 dmckee Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page_edit_fns.inc';

/**
* Page_Multiple_Page_Edit_Fns
*
* Purpose
*
*
* @author  Darren McKee <dmckee@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Multiple_Page_Edit_Fns extends Page_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Page_Multiple_Page_Edit_Fns()
	{
		$this->Page_Edit_Fns();

	}//end constructor


	/**
	* Paint the interface for creating containers in the bodycopy
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintCurrentPages(&$asset, &$o, $prefix)
	{
		$asset_ids = $asset->getPageIds();
		$asset_infos = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($asset_ids);
		$edit_ids = Array();
		foreach ($asset_ids as $id) {
			$edit_ids[$id] = 'contents';
		}
		$edit_links = $GLOBALS['SQ_SYSTEM']->am->getAssetBackendHref($edit_ids);
		$write_access = $asset->writeAccess('content');

		require_once SQ_LIB_PATH.'/html_form/html_form.inc';

		?>
		<table class="sq-backend-table">
			<tr>
				<th class="sq-backend-table-header"><?php echo translate('name'); ?></th>
				<th class="sq-backend-table-header"><?php echo translate('link'); ?></th>
				<?php
				if ($write_access) {
					?>
					<th class="sq-backend-table-header"><?php echo translate('delete'); ?>?</th>
					<?php
				}
				?>
			</tr>
			<?php
			if (empty($asset_ids)) {
				?>
				<tr>
					<td colspan="5" style="text-align: center;"><?php echo translate('currently_no_pages_to_display'); ?></td>
				</tr>
				<?php
			}
			foreach ($asset_ids as $id) {
			?>
				<tr>
					<td class="sq-backend-table-cell">
						<?php echo $asset_infos[$id]['name']; ?>
					</td>
					<td class="sq-backend-table-cell">
						<a href="<?php echo $edit_links[$id]; ?>"><?php echo translate('contents'); ?></a>
					</td>
					<?php
					if ($write_access) {
						?>
						<td class="sq-backend-table-cell">
							<?php check_box($prefix.'_delete_page['.$id.']'); ?>
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

		if ($write_access) {
			?>
			<table>
				<tr>
				<td>
					<?php echo translate('create_new_page'); ?>
				</td>
				<td>
					<?php text_box($prefix.'_new_page', '', '50', '50'); ?>
				</td>
				</tr>
			</table>
			<?php
		}

		return true;
	}//end paintCurrentPages()


	/**
	* Paint the interface for creating containers in the bodycopy
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processCurrentPages(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('content');

		$am = $GLOBALS['SQ_SYSTEM']->am;

		if ($write_access && isset($_POST[$prefix.'_new_page']) && !empty($_POST[$prefix.'_new_page'])) {
			$asset->createChildPage($_POST[$prefix.'_new_page']);
		}

		if (isset($_POST[$prefix.'_delete_page'])) {

			//Set the following flag, so the updating of lookups doesnt cause dramas
			$asset->_tmp['deleting_links'] = true;
			foreach ($_POST[$prefix.'_delete_page'] as $id => $flag) {
				$doomed_link = $am->getLinkByAsset($asset->id, $id);
				$am->moveLink($doomed_link['linkid'], $am->getSystemAssetid('trash_folder'), SQ_LINK_TYPE_2, '0');
				$deleted =& $am->getAsset($doomed_link['minorid']);
				$deleted->saveWebPaths(Array());
			}
		}
		//Restore the flag set above
		$asset->_tmp['deleting_links'] = false;

		$asset->linksUpdated();

		return $write_access;
	}//end processCurrentPages()


}//end class

?>