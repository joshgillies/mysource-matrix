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
* $Id: page_asset_listing_edit_fns.inc,v 1.9 2005/01/17 21:59:21 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page_edit_fns.inc';

/**
* Page_Asset_Listing_Edit_Fns
*
* Purpose
*
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Asset_Listing_Edit_Fns extends Page_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Page_Asset_Listing_Edit_Fns()
	{
		$this->Page_Edit_Fns();
		$this->static_screens['details']['force_unlock'] = false;
		$this->static_screens['details']['lock_type'] = 'content';

	}//end constructor


	/**
	* Paint some information about replacing keywords
	*
	* @param object Form				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintBodycopyInformation(&$asset, &$o, $prefix)
	{
		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_TYPE_2, 'bodycopy', true, 'page_contents');
		if (empty($link)) return false;
		$info = Array($link['minorid'] => 'contents');
		$href = $GLOBALS['SQ_SYSTEM']->am->getAssetBackendHref($info);

		echo 'The contents of the asset listing format can be '.
			'replaced into the <a href="'.$href[$link['minorid']].'">Page Contents Bodycopy</a> by using the %asset_listing% keyword';

	}//end paintBodycopyInformation()


	/**
	* Does nothing
	*
	* @return boolean
	* @access public
	*/
	function processBodycopyInformation()
	{
		return false;

	}//end processBodycopyInformation()


	/**
	* Paint a link to the default format
	*
	* @param object Form				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintDefaultFormatLink(&$asset, &$o, $prefix)
	{
		$folder = &$asset->getFolder('type_formats');
		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($folder->id, SQ_LINK_TYPE_2, 'bodycopy', true, 'default_format');
		if (empty($link)) return false;

		$info = Array($link['minorid'] => 'contents');
		$href = $GLOBALS['SQ_SYSTEM']->am->getAssetBackendHref($info);
		echo '<a href="'.$href[$link['minorid']].'">Edit the Default Format</a>';

		return true;

	}//end paintDefaultFormatLink()


	/**
	* Does nothing
	*
	* @return boolean
	* @access public
	*/
	function processDefaultFormatLink()
	{
		return false;

	}//end processDefaultFormatLink()


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
	* Paint the current asset types that have been added
	*
	* @param object Form				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintCurrentAssetTypes(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('content');
		$folder = &$asset->getFolder('type_formats');

		// do a check to see if any of the bodycopies have been removed manually
		$customised = $asset->attr('customised');
		if (!empty($customised)) {
			$deleted = false;
			foreach ($customised as $type) {
				$link = $GLOBALS['SQ_SYSTEM']->am->getLink($folder->id, SQ_LINK_TYPE_2, 'bodycopy', true, $type);
				if (empty($link)) {
					$deleted = true;
					$key = array_search($type, $customised, true);
					unset($customised[$key]);
				}
			}
			if ($deleted) {
				$asset->setAttrValue('customised', $customised);
				$asset->saveAttributes();
			}
		}

		$types = $asset->attr('types');
		if (empty($types)) {
			echo 'There are no Types Defined';
			return false;
		}

		?>
		<table class="sq-backend-table">
		<tr>
			<td class="sq-backend-table-header">Asset Type</td>
			<td class="sq-backend-table-header">Inherit Types</td>
			<td class="sq-backend-table-header">Edit Format</td>
			<?php
			if ($write_access) {
				?>
				<td class="sq-backend-table-header">Customise</td>
				<td class="sq-backend-table-header">Remove?</td>
				<?php
			}
			?>
		</tr>
			<?php
			foreach ($types as $type => $inherit) {
				?>
				<tr>
					<td class="sq-backend-table-cell">
						<?php
						echo ucwords(str_replace('_',' ', $type));
						?>
					</td>
					<td class="sq-backend-table-cell">
						<?php
						if ($write_access) {
							check_box($prefix.'_inherit_types[]', $type, $inherit);
						} else {
							echo ($inherit) ? 'Yes' : 'No';
						}
						?>
					</td>
					<td class="sq-backend-table-cell">
						<?php
							$link = $GLOBALS['SQ_SYSTEM']->am->getLink($folder->id, SQ_LINK_TYPE_2, 'bodycopy', true, $type);
							$is_customised = false;
							if (!empty($link)) {
								$is_customised = true;
								$info = Array($link['minorid'] => 'contents');
								$href = $GLOBALS['SQ_SYSTEM']->am->getAssetBackendHref($info);

								$content = '<a href="'.$href[$link['minorid']].'">'.ucwords(str_replace('_',' ', $type)).' Format</a>';
							} else {
								$content = 'Default Format';
							}
							echo $content;
						?>
					</td>
					<?php
					if ($write_access) {
						?>
						<td class="sq-backend-table-cell">
							<?php
							check_box($prefix.'_customise[]', $type, $is_customised);
							?>
						</td>
						<td class="sq-backend-table-cell">
							<?php
							check_box($prefix.'_remove[]', $type, false);
							?>
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

	}//end paintCurrentAssetTypes()


	/**
	* Process the current asset types that have been added
	*
	* @param object Form				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processCurrentAssetTypes(&$asset, &$o, $prefix)
	{
		$customised = $asset->attr('customised');
		$types = $asset->attr('types');
		$folder = &$asset->getFolder('type_formats');

		// we are looking to see if any types have been removed
		if (isset($_POST[$prefix.'_remove'])) {

			foreach ($_POST[$prefix.'_remove'] as $type) {
				if (isset($types[$type])) {
					unset($types[$type]);

					// remove from the customise list if its in here
					if (in_array($type, $customised)) {
						$key = array_search($type, $customised, true);
						unset($customised[$key]);

						// we also want to send the bodycopy to the trash
						$link = $GLOBALS['SQ_SYSTEM']->am->getLink($folder->id, SQ_LINK_TYPE_2, 'bodycopy', true, $type);
						if (empty($link)) {
							trigger_error('Format for type '.$type.' not found', E_USER_WARNING);
						}
						// back up in your trash with the resurection
						$folder->deleteLink($link['linkid']);
					}
				}
			}
		}


		if (!isset($_POST[$prefix.'_inherit_types'])) $_POST[$prefix.'_inherit_types'] = Array();
		for (reset($types); null !== ($k = key($types)); next($types)) {
			$types[$k] = (in_array($k, $_POST[$prefix.'_inherit_types'])) ? true : false;
		}
		$asset->setAttrValue('types', $types);


		if (isset($_POST[$prefix.'_customise'])) {

			if (empty($_POST[$prefix.'_customise'])) return false;

			foreach ($_POST[$prefix.'_customise'] as $type) {
				if (in_array($type, $customised)) continue;

				if (isset($_POST[$prefix.'_remove']) && in_array($type, $_POST[$prefix.'_remove'])) continue;

				$customised[] = $type;

				$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
				$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');
				$bodycopy = new Bodycopy();
				$copy_link = Array('asset' => &$folder, 'value' => $type, 'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);

				$bodycopy->setAttrValue('name', ucwords(str_replace('_',' ', $type)).' Format');

				if (!$bodycopy->create($copy_link)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return false;
				}
				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			}
		}

		if (!empty($customised)) {
			foreach ($customised as $type) {
				if (!isset($_POST[$prefix.'_customise']) || !in_array($type, $_POST[$prefix.'_customise'])) {

					if (isset($_POST[$prefix.'_remove']) && in_array($type, $_POST[$prefix.'_remove'])) continue;

					$link = $GLOBALS['SQ_SYSTEM']->am->getLink($folder->id, SQ_LINK_TYPE_2, 'bodycopy', false, $type);
					$asset->deleteLink($link['linkid']);
					$key = array_search($type, $customised, true);
					unset($customised[$key]);
				}
			}
		}

		$asset->setAttrValue('customised', $customised);
		return true;

	}//end processCurrentAssetTypes()


	/**
	* Paint section to add an asset type
	*
	* @param object Form				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintAddAssetType(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('content')) return false;

		$types = $GLOBALS['SQ_SYSTEM']->am->getTypeList();
		$types = array_diff($types, $asset->attr('types'));
		asort($types);

		$uc_types = Array('' => '');
		foreach ($types as $type) $uc_types[$type] = ucwords(str_replace('_',' ', $type));
		combo_box($prefix.'_new_type', $uc_types, false, '');

	}//end paintAddAssetType()


	/**
	* Process the section to add a new format for an asset
	*
	* @param object Form				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processAddAssetType(&$asset, &$o, $prefix)
	{
		if (isset($_POST[$prefix.'_new_type'])) {
			$type = trim($_POST[$prefix.'_new_type']);
			if (empty($type)) return false;
			$types = $asset->attr('types');
			if (!isset($types[$type])) {
				$types[$type] = true;
				$asset->setAttrValue('types', $types);
				return true;
			}
		}
		return false;

	}//end processAddAssetType()


	/**
	* Paints the interface for setting the number of columns to display per row
	*
	* @param object	Asset				&$rootid	asset being painted
	* @param object Backend_Outputter	&$o			backend outputter
	* @param string						$prefix		prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintColumnSettings(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$current = $asset->attr('column_layout');
		
		?>
		<table class="sq-backend-table">
			<tr>
				<td class="sq-backend-table-header">Row</td>
				<td class="sq-backend-table-header">Column(s)</td>
				<?php
				if ($write_access) {
					?>
					<td class="sq-backend-table-header">Delete ?</td>
					<?php
				}
				?>
			</tr>
			<?php
			if (count($current) > 1) {
				foreach ($current as $row => $columns) {
					if ($row == 0) continue;
					?>
					<tr>
						<td class="sq-backend-table-cell"><?php echo $row; ?></td>
						<td class="sq-backend-table-cell">
							<?php
							if ($write_access) {
								text_box($prefix.'_columns['.$row.']', $columns, 5);
							} else {
								echo $columns;
							}
							?>
						</td>
						<?php
						if ($write_access) {
							?>
							<td class="sq-backend-table-cell">
								<?php check_box($prefix.'_delete_row['.$row.']'); ?>
							</td>
							<?php
						}
					?>
					</tr>
					<?php
				}//end foreach
			}//end if

			?>
			<tr>
				<td class="sq-backend-table-cell">All remaining</td>
				<td class="sq-backend-table-cell">
					<?php
					if ($write_access) {
						text_box($prefix.'_columns[0]', $current['0'], 5);
					} else {
						echo $current['0'];
					}
					?>
				</td>
				<?php
				if ($write_access) {
					?><td class="sq-backend-table-cell">&nbsp;</td><?php
				}
				?>
			</tr>
		</table>
		<?php

		if ($write_access) {
			$o->closeField();
			$o->openField('Customise Row');
				echo 'row ';
				text_box($prefix.'_new_row', '', 5);
				echo ' columns ';
				text_box($prefix.'_new_column', '', 5);
			// field closed by asset_edit_interface
		}

		return $write_access;

	}//end paintColumnSettings()


	/**
	* Processes the interface for setting the number of columns to display per row
	*
	* @param object	Asset				&$rootid	asset being painted
	* @param object Backend_Outputter	&$o			backend outputter
	* @param string						$prefix		prefix for form elements
	*
	* @return void
	* @access public
	*/
	function processColumnSettings(&$asset, &$o, $prefix)
	{
		// change existing column settings
		$column_layout = array_get_index($_POST, $prefix.'_columns', Array());
		$deleted_rows  = array_get_index($_POST, $prefix.'_delete_row', Array());
		foreach ($deleted_rows as $row => $on) {
			if (isset($column_layout[$row])) unset($column_layout[$row]);
		}
		
		// add a new column setting
		$new_row = (int)array_get_index($_POST, $prefix.'_new_row', 0);
		$new_column = (int)array_get_index($_POST, $prefix.'_new_column', 0);
		if ($new_row > 0) {
			if ($new_column > 0) {
				if (!isset($column_layout[$new_row])) {
					$column_layout[$new_row] = $new_column;
				} else {
					trigger_error('Failed adding customisation for row '.$new_row.', row has already been customised', E_USER_WARNING);
				}
			} else {
				trigger_error('Failed adding customisation for row '.$new_row.', number of columns must be greater than zero', E_USER_WARNING);
			}
		}
		
		return $asset->setAttrValue('column_layout', $column_layout);

	}//end processColumnSettings()


	/**
	* Paints the interface for previewing the column layout
	*
	* @param object	Asset				&$rootid	asset being painted
	* @param object Backend_Outputter	&$o			backend outputter
	* @param string						$prefix		prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintColumnPreview(&$asset, &$o, $prefix)
	{
		$column_layout = $asset->attr('column_layout');
		$num_per_page  = $asset->attr('num_per_page');

		$num_rows = max(array_keys($column_layout)) + 1;
		$num_columns = max(array_values($column_layout));
		
		?>
		<table class="sq-backend-table">
			<?php
			$cell_num = 1;
			for ($i = 1; $i <= $num_rows; $i++) {
				echo '<tr>';
				$cols_in_row = array_get_index($column_layout, $i, $column_layout['0']);
				$each_colspan = (int)($num_columns / $cols_in_row);
				$first_colspan = $each_colspan + $num_columns - ($cols_in_row * $each_colspan);

				for ($c = 1; $c <= $cols_in_row; $c++) {
					$colspan = ($c == 1) ? $first_colspan : $each_colspan;
					$class = 'sq-backend-table-cell';
					if ($num_per_page > 0 && $cell_num > $num_per_page) $class .= '-alt';
					?>
					<td class="<?php echo $class; ?>" style="<?php echo ($c < $cols_in_row) ? ' border-right: 1px solid #CCCCCC;' : ''; ?>"<?php echo ($colspan > 1) ? ' colspan="'.$colspan.'"' : ''; ?>>
						<b><?php echo $cell_num; ?></b>
					</td>
					<?php
					$cell_num++;
				}
				echo '</tr>';
			}
			?>
			<tr>
				<td class="sq-backend-table-cell" style="text-align: center;" colspan="<?php echo $num_columns; ?>">
					All remaining rows will have <?php echo $column_layout[0]; ?> column(s)
					<?php
					if ($num_per_page > 0 && $cell_num > $num_per_page) {
						echo '<br /><b>Note: This asset has been configured to list '.$num_per_page.' assets per page, so the highlighted cells will not be displayed</b>';
					}
					?>
				</td>
			</tr>
		</table>
		<?php

	}//end paintColumnPreview()


	/**
	* Processes the interface for previewing the column layout
	*
	* @param object	Asset				&$rootid	asset being painted
	* @param object Backend_Outputter	&$o			backend outputter
	* @param string						$prefix		prefix for form elements
	*
	* @return void
	* @access public
	*/
	function processColumnPreview(&$asset, &$o, $prefix)
	{
		return false;

	}//end processColumnPreview()


}//end class

?>
