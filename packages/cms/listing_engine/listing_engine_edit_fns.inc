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
* $Id: listing_engine_edit_fns.inc,v 1.16 2005/05/23 06:01:19 tbarrett Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page_edit_fns.inc';


/**
* Listing_Engine
*
* Purpose
*     Provides common listing services to a range of CMS and search
*     related assets, such as Asset Listing and Search Page. This is
*     a 'base asset' that should not be created directly - rather you
*     should create its children
*
*
* @author  Luke Wright   <lwright@squiz.net>
* @author  David Heppell <dheppell@squiz.net>
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.16 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Listing_Engine_Edit_Fns extends Page_Edit_Fns
{


	/**
	* Constructor
	*/
	function Listing_Engine_Edit_Fns($assetid=0)
	{
		$this->Page_Edit_Fns($assetid);

	}//end constructor


	/**
	* Paint the interface for choosing which asset types to list
	*
	* @param object Listing_Engine		&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintAssetTypes(&$asset, &$o, $prefix)
	{
		$types = $asset->attr('types');
		if ($asset->writeAccess('content')) {
			// print the form fields
			$display_values = Array('type_code' => Array(), 'inherit' => Array());
			foreach ($types as $type => $inherit) {
				$display_values['type_code'][] = $type;
				$display_values['inherit'][] = $inherit;
			}
			asset_type_chooser($prefix.'_types', true, $display_values, null, true, true);
			return true;
		} else {
			// print the read-only version
			if (empty($types)) {
				echo '<p class="sq-backend-warning">'.translate('cms_listing_no_asset_types_chosen').'</p>';
			} else {
				$type_names = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo(array_keys($types), 'name');
				?>
					<table class="sq-backend-table">
						<tr>
							<th><?php echo translate('type') ?></th>
							<th><?php echo translate('inherit_question') ?></th>
						</tr>
				<?php
				$inherit_image_path = sq_web_path('lib').'/web/images/';
				foreach ($types as $type => $inherit) {
					$inherit_image = $inherit_image_path.($inherit ? 'tick' : 'cross').'.gif';
					$inherit_alt = $inherit ? translate('yes') : translate('no');
					?>
						<tr>
							<td><?php echo $type_names[$type]; ?></td>
							<td><img src="<?php echo $inherit_image; ?>" alt="<?php echo $inherit_alt; ?>" /></td>
						</tr>
					<?php
				}
				?>
					</table>
				<?php
			}
			return false;
		}

	}//end paintAssetTypes()


	/**
	* Process the asset types to list
	*
	* @param object Listing_Engine		&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processAssetTypes(&$asset, &$o, $prefix)
	{
		if (isset($_POST[$prefix.'_types'])) {
			$existing_types = $asset->attr('types');
			$submitted_types = Array();
			foreach ($_POST[$prefix.'_types']['type_code'] as $i => $type_code) {
				if (!empty($type_code)) {
					$submitted_types[$type_code] = $_POST[$prefix.'_types']['inherit'][$i];
				}
			}

			$folder = &$asset->getFolder('type_formats');
			$formats = $asset->getFormats('type_formats');

			// See if any types have been removed
			foreach ($existing_types as $type => $inherit) {
				if (!isset($submitted_types[$type])) {
					if (in_array($type, $formats)) {
						// send the type format bodycopy to the trash
						$link = $GLOBALS['SQ_SYSTEM']->am->getLink($folder->id, SQ_LINK_TYPE_2, 'bodycopy', true, $type);
						if (!empty($link)) {
							$folder->deleteLink($link['linkid']);
						}
					}
				}
			}

			$asset->setAttrValue('types', $submitted_types);
			return true;
		} else {
			return false;
		}

	}//end processAssetTypes()


	/**
	* Paint the interface for editing custom asset positions
	*
	* @param object Listing_Engine		&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintAssetPositions(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$asset_positions = $asset->attr('asset_positions');

		if (!empty($asset_positions)) {
			ksort($asset_positions, SORT_NUMERIC);
			if (!$write_access) {
				$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_values($asset_positions));
			}
			?>
			<table class="sq-backend-table">
				<tr>
					<td class="sq-backend-table-header"><?php echo translate('position'); ?></td>
					<td class="sq-backend-table-header"><?php echo translate('asset'); ?></td>
					<?php
					if ($write_access) {
						?><td class="sq-backend-table-header"><?php echo translate('delete_question'); ?></td><?php
					}
					?>
				</tr>
				<?php
				foreach ($asset_positions as $position => $assetid) {
					?>
					<tr>
						<td class="sq-backend-table-cell"><?php echo $position; ?></td>
						<td class="sq-backend-table-cell">
							<?php
							if ($write_access) {
								asset_finder($prefix.'_asset_position['.$position.']', $assetid, Array('asset' => 'D'));
							} else {
								$info = array_get_index($asset_info, $assetid, Array());
								if (empty($info)) {
									echo translate('unknown_asset');
								} else {
									echo $info['short_name'].' (#'.$assetid.')';
								}
							}
							?>
						</td>
						<?php
						if ($write_access) {
							?>
							<td class="sq-backend-table-cell">
								<?php
								check_box($prefix.'_delete_asset_position['.$position.']');
								?>
							</td>
							<?php
						}
						?>
					</tr>
					<?php
				}//end foreach
				?>
			</table>
			<?php
		} else {
			echo translate('cms_listing_no_positions_defined');
		}

		if ($write_access) {
			echo '<p><b>'.translate('cms_listing_new_asset_position').'</b> ';
			text_box($prefix.'_new_asset_position_pos', '', 5);
			echo ' ';
			asset_finder($prefix.'_new_asset_position_asset', '', Array('asset' => 'D'));
			echo '</p>';
		}

		return $write_access;

	}//end paintAssetPositions()


	/**
	* Process the interface for editing custom asset positions
	*
	* @param object Listing_Engine		&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processAssetPositions(&$asset, &$o, $prefix)
	{
		$asset_positions = Array();

		$new_position_pos = (int) array_get_index($_POST, $prefix.'_new_asset_position_pos', 0);
		if ($new_position_pos > 0) {
			$new_position_data = array_get_index($_POST, $prefix.'_new_asset_position_asset', Array());
			if ($new_position_assetid = $new_position_data['assetid']) {
				$asset_positions[$new_position_pos] = $new_position_assetid;
			}
		}

		$current_asset_positions = array_get_index($_POST, $prefix.'_asset_position', Array());
		foreach ($current_asset_positions as $position => $data) {
			if ($data['assetid']) {
				$asset_positions[$position] = $data['assetid'];
			}
		}

		$delete_asset_position = array_get_index($_POST, $prefix.'_delete_asset_position', Array());
		foreach ($delete_asset_position as $position => $on) {
			unset($asset_positions[$position]);
		}

		return $asset->setAttrValue('asset_positions', $asset_positions);

	}//end processAssetPositions()


	/**
	* Paints the interface for setting the number of columns to display per row
	*
	* @param object	Listing_Engine		&$rootid	asset being painted
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
				<td class="sq-backend-table-header"><?php echo translate('row'); ?></td>
				<td class="sq-backend-table-header"><?php echo translate('column(s)'); ?></td>
				<?php
				if ($write_access) {
					?>
					<td class="sq-backend-table-header"><?php echo translate('delete_question') ?></td>
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
				<td class="sq-backend-table-cell"><?php echo translate('cms_listing_all_remaining'); ?></td>
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
			$o->openField(translate('cms_listing_customise_row'));
				echo strtolower(translate('row')).' ';
				text_box($prefix.'_new_row', '', 5);
				echo ' '.strtolower(translate('columns')).' ';
				text_box($prefix.'_new_column', '', 5);
			// field closed by asset_edit_interface
		}

		return $write_access;

	}//end paintColumnSettings()


	/**
	* Processes the interface for setting the number of columns to display per row
	*
	* @param object	Listing_Engine		&$rootid	asset being painted
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
			if (isset($column_layout[$row])) {
				unset($column_layout[$row]);
			}
		}

		// add a new column setting
		$new_row = (int)array_get_index($_POST, $prefix.'_new_row', 0);
		$new_column = (int)array_get_index($_POST, $prefix.'_new_column', 0);
		if ($new_row > 0) {
			if ($new_column > 0) {
				if (!isset($column_layout[$new_row])) {
					$column_layout[$new_row] = $new_column;
				} else {
					trigger_localised_error('CMS0040', E_USER_WARNING, $new_row);
				}
			} else {
				trigger_localised_error('CMS0041', E_USER_WARNING, $new_row);
			}
		}

		return $asset->setAttrValue('column_layout', $column_layout);

	}//end processColumnSettings()


	/**
	* Paints the interface for previewing the column layout
	*
	* @param object	Listing_Engine		&$rootid	asset being painted
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
					if ($num_per_page > 0 && $cell_num > $num_per_page) {
						$class .= '-alt';
					}
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
					<?php echo translate('cms_listing_column_all_remaining_rows', $column_layout[0]); ?>
					<?php
					if ($num_per_page > 0 && $cell_num > $num_per_page) {
						echo '<br /><b>'.translate('cms_listing_column_page_note', $num_per_page).'</b>';
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
	* @param object	Listing_Engine		&$rootid	asset being painted
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


	/**
	* Paint links to the defaut formats
	*
	* @param object Asset				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintDefaultFormatLinks(&$asset, &$o, $prefix)
	{
		?>
		<table class="sq-backend-table">
			<tr>
				<td class="sq-backend-table-header"><?php echo translate('edit_format') ?></td>
				<td class="sq-backend-table-header"><?php echo translate('note') ?></td>
			</tr>
			<?php
			$link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_TYPE_2, 'bodycopy', true, 'page_contents');
			if (!empty($link)) {
				$info = Array($link['minorid'] => 'contents');
				$href = $GLOBALS['SQ_SYSTEM']->am->getAssetBackendHref($info);
				?>
				<tr>
					<td class="sq-backend-table-cell">
						<a href="<?php echo $href[$link['minorid']]; ?>"><?php echo translate('cms_page_contents_bodycopy') ?></a>
					</td>
					<td class="sq-backend-table-cell">
						<?php echo translate('cms_listing_page_contents_desc') ?>
					</td>
				</tr>
				<?php
			}

			$folder = &$asset->getFolder('type_formats');
			$link = $GLOBALS['SQ_SYSTEM']->am->getLink($folder->id, SQ_LINK_TYPE_2, 'bodycopy', true, 'default_format');
			if (!empty($link)) {
				$info = Array($link['minorid'] => 'contents');
				$href = $GLOBALS['SQ_SYSTEM']->am->getAssetBackendHref($info);
				?>
				<tr>
					<td class="sq-backend-table-cell">
						<a href="<?php echo $href[$link['minorid']]; ?>"><?php echo translate('cms_listing_default_asset_format'); ?></a>
					</td>
					<td class="sq-backend-table-cell">
						<?php echo translate('cms_listing_default_asset_desc') ?>
					</td>
				</tr>
				<?php
			}
			?>
		</table>
		<?php

		return false;

	}//end paintDefaultFormatLinks()


	/**
	* Paint the interface for editing custom type formats
	*
	* @param object Listing_Engine		&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintTypeFormats(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('links');
		$types = $asset->attr('types');
		$formats = $asset->getFormats('type_formats');

		if (count($formats) > 1) {
			?>
			<table class="sq-backend-table">
				<tr>
					<td class="sq-backend-table-header"><?php echo translate('asset_type') ?></td>
					<td class="sq-backend-table-header"><?php echo translate('edit_format') ?></td>
					<?php
					if ($write_access) {
						?>
						<td class="sq-backend-table-header"><?php echo translate('delete_question') ?></td>
						<?php
					}
					?>
				</tr>
				<?php
				foreach ($formats as $minorid => $value) {
					if ($value == 'default_format') continue;

					$info = Array($minorid => 'contents');
					$href = $GLOBALS['SQ_SYSTEM']->am->getAssetBackendHref($info);
					$type_name = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($value, 'name');
					?>
					<tr>
						<td class="sq-backend-table-cell" style="width: 200px;">
							<?php
							echo $type_name;
							?>
						</td>
						<td class="sq-backend-table-cell">
							<?php
							echo '<a href="'.$href[$minorid].'">'.translate('cms_listing_type_format', $type_name).'</a>';
							?>
						</td>
						<?php
						if ($write_access) {
							?>
							<td class="sq-backend-table-cell" style="width: 100px;">
								<?php
								check_box($prefix.'_delete_type['.$value.']');
								?>
							</td>
							<?php
						}
						?>
					</tr>
					<?php
				}//end foreach
				?>
			</table>
			<?php
		} else {
			echo translate('cms_listing_no_type_formats');
		}//end if cutomised formats

		if ($write_access) {
			$new_format_options = Array('' => '');
			foreach ($types as $type => $inherit) {
				if (in_array($type, $formats)) continue;
				$new_format_options[$type] = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type, 'name');
			}
			if (count($new_format_options) > 1) {
				echo '<p><b>'.translate('cms_listing_new_type_format').'</b> ';
				combo_box($prefix.'_new_type', $new_format_options, false, '');
				echo '</p>';
			}
		}
		return false;

	}//end paintTypeFormats()


	/**
	* Process the interface for editing custom type formats
	*
	* @param object Listing_Engine		&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processTypeFormats(&$asset, &$o, $prefix)
	{
		$folder = &$asset->getFolder('type_formats');
		$types = $asset->attr('types');

		$deleted_formats = array_get_index($_POST, $prefix.'_delete_type', Array());
		$new_type = array_get_index($_POST, $prefix.'_new_type', '');
		if (isset($deleted_formats[$new_type])) {
			trigger_localised_error('CMS0042', E_USER_WARNING, $new_type);
			unset($deleted_formats[$new_type]);
			$new_type = '';
		}

		foreach ($deleted_formats as $type => $on) {
			$current_link = $GLOBALS['SQ_SYSTEM']->am->getLink($folder->id, SQ_LINK_TYPE_2, 'bodycopy', true, $type);
			if (empty($current_link)) {
				trigger_localised_error('CMS0043', E_USER_WARNING, $type);
			} else {
				if (!$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($current_link['linkid'])) {
					trigger_localised_error('CMS0044', E_USER_WARNING, $type);
				}
			}
		}

		// check that the new type is valid
		if ($new_type != '' && !isset($types[$new_type])) {
			trigger_localised_error('CMS0045', E_USER_WARNING, $new_type);
			$new_type = '';
		}

		if ($new_type != '') {
			// check that this type isnt already customised
			$current_link = $GLOBALS['SQ_SYSTEM']->am->getLink($folder->id, SQ_LINK_TYPE_2, 'bodycopy', true, $new_type);
			if (empty($current_link)) {
				// create a new format bodycopy for this type
				$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
				$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');

				$copy_link = Array('asset' => &$folder, 'value' => $new_type, 'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);

				$bodycopy = new Bodycopy();
				$bodycopy->setAttrValue('name', $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($new_type, 'name').' Format');

				if (!$bodycopy->create($copy_link)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return false;
				}
				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			} else {
				trigger_localised_error('CMS0046', E_USER_WARNING, $new_type);
			}
		}
		if (isset($asset->_tmp['formats']['type_formats'])) {
			unset($asset->_tmp['formats']['type_formats']);
		}

		return false;

	}//end processTypeFormats()


	/**
	* Paint the interface for editing custom position formats
	*
	* @param object Listing_Engine		&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintPositionFormats(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('links');
		$formats = $asset->getFormats('position_formats');

		if (!empty($formats)) {
			?>
			<table class="sq-backend-table">
				<tr>
					<td class="sq-backend-table-header"><?php echo translate('position') ?></td>
					<td class="sq-backend-table-header"><?php echo translate('edit_format') ?></td>
					<?php
					if ($write_access) {
						?><td class="sq-backend-table-header"><?php echo translate('delete_question') ?></td><?php
					}
					?>
				</tr>
				<?php
				foreach ($formats as $minorid => $value) {
					$info = Array($minorid => 'contents');
					$href = $GLOBALS['SQ_SYSTEM']->am->getAssetBackendHref($info);
					$position = substr($value, 9);
					?>
					<tr>
						<td class="sq-backend-table-cell" style="width: 200px;">
							<?php
							echo $position;
							?>
						</td>
						<td class="sq-backend-table-cell">
							<?php
							echo '<a href="'.$href[$minorid].'">'.translate('cms_listing_position_number_format', $position).'</a>';
							?>
						</td>
						<?php
						if ($write_access) {
							?>
							<td class="sq-backend-table-cell" style="width: 100px;">
								<?php
								check_box($prefix.'_delete_position['.$position.']');
								?>
							</td>
							<?php
						}
						?>
					</tr>
					<?php
				}//end foreach
				?>
			</table>
			<?php
		} else {
			echo translate('cms_listing_no_position_formats');
		}

		if ($write_access) {
			echo '<p><b>'.translate('cms_listing_new_position_format').'</b> ';
			text_box($prefix.'_new_position', '', 5);
			echo '</p>';
		}

		return $write_access;

	}//end paintPositionFormats()


	/**
	* Process the interface for editing custom position formats
	*
	* @param object Listing_Engine		&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processPositionFormats(&$asset, &$o, $prefix)
	{
		$folder = &$asset->getFolder('position_formats');

		$deleted_formats = array_get_index($_POST, $prefix.'_delete_position', Array());
		$new_position = (int) array_get_index($_POST, $prefix.'_new_position', 0);
		if (isset($deleted_formats[$new_position])) {
			trigger_localised_error('CMS0047', E_USER_WARNING, $new_position);
			unset($deleted_formats[$new_position]);
			$new_position = 0;
		}

		foreach ($deleted_formats as $position => $on) {
			$current_link = $GLOBALS['SQ_SYSTEM']->am->getLink($folder->id, SQ_LINK_TYPE_2, 'bodycopy', true, 'position_'.$position);
			if (empty($current_link)) {
				trigger_localised_error('CMS0048', E_USER_WARNING, $position);
			} else {
				if (!$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($current_link['linkid'])) {
					trigger_localised_error('CMS0049', E_USER_WARNING, $position);
				}
			}
		}

		if ($new_position > 0) {
			// check that this position isnt already customised
			$current_link = $GLOBALS['SQ_SYSTEM']->am->getLink($folder->id, SQ_LINK_TYPE_2, 'bodycopy', true, 'position_'.$new_position);
			if (empty($current_link)) {

				// create a new format bodycopy for this list position
				$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
				$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');

				$copy_link = Array('asset' => &$folder, 'value' => 'position_'.$new_position, 'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);

				$bodycopy = new Bodycopy();
				$bodycopy->setAttrValue('name', translate('cms_listing_position_number_format', $new_position));

				if (!$bodycopy->create($copy_link)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return false;
				}
				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			} else {
				trigger_localised_error('CMS0050', E_USER_POSITION, $new_position);
			}
		}
		if (isset($asset->_tmp['formats']['position_formats'])) {
			unset($asset->_tmp['formats']['position_formats']);
		}
		return false;

	}//end processPositionFormats()


	/**
	* Paints the interface for selecting sort field
	*
	* @param object	Listing_Engine		&$asset		asset being painted
	* @param object Backend_Outputter	&$o			backend outputter
	* @param string						$prefix		prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintSortBy(&$asset, &$o, $prefix)
	{
		$write_access    = $asset->writeAccess('attributes');
		$sort_by_list    = $asset->attr('sort_by');
		$sort_by_options = $this->_getSortByOptions($asset);

		$o->openField(translate('cms_listing_new_sort_by_field'));

			if ($asset->attr('group_by') == 'random') {
				echo translate('cms_listing_random_selected_no_sort');
				return false;
			} else {
				$sort_by_options = $this->_getSortByOptions($asset);
				$sort_by_options[''] = '-- '.translate('select').' --';
			}

			if ($write_access) {
				echo ' '.translate('name').' ';
				text_box($prefix.'_name', '');
				echo ' '.translate('sort_by').' ';
				combo_box($prefix.'_sort_by', $sort_by_options, false, '');
			}

		$o->closeField();
		$o->openField('');

			if (empty($sort_by_list)) {
				$o->closeField();
				return $write_access;
			}

			$o->openSection(translate('cms_listing_sort_by_asset_field'));
				$o->openField('');

					$fields = Array();
					foreach ($sort_by_list as $option_id => $info) {
						if ($info['type'] == 'field') {
							$fields[$option_id] = $info;
						}
					}
					?>
				<?php

					if (!empty($fields)) {
						?>
						<table class="sq-backend-table">
							<tr>
								<td class="sq-backend-table-header" style="width: 200px;"><?php echo translate('asset_field'); ?></td>
								<td class="sq-backend-table-header"><?php echo translate('display_name') ?></td>
								<td class="sq-backend-table-header">&nbsp;</td>
								<td class="sq-backend-table-header" style="width: 200px;"><?php echo translate('remove_question'); ?></td>
							</tr>
						<?php

						$combo_list = Array('' => '');
						foreach ($fields as $id => $info) {
							?>
								<tr>
									<td class="sq-backend-table-cell">
										<?php
											hidden_field($prefix.'_fields['.$id.'][params][field]', $info['params']['field']);
											hidden_field($prefix.'_fields['.$id.'][type]', 'field');
											echo $sort_by_options[$info['params']['field']];
										?>
									</td>
									<td class="sq-backend-table-cell">
										<?php
											if ($write_access) {
												text_box($prefix.'_fields['.$id.'][name]', $info['name']);
											} else {
												echo array_get_index($info, 'name', '');
											}
										?>
									</td>
									<td class="sq-backend-table-cell"></td>
									<td class="sq-backend-table-cell">
										<?php
											if ($write_access) {
												check_box($prefix.'_remove['.$id.']', '');
											}
										?>
									</td>
								</tr>
							<?php
						}
						?>
						</table>
					<?php
					} else {
						echo translate('cms_listing_no_field_sort_options');
					}

				$o->closeField();
			$o->closeSection();

			$o->openSection(translate('cms_listing_sort_by_attribute'));
				$o->openField('');

					$attrs = Array();
					foreach ($sort_by_list as $id => $info) {
						if ($info['type'] == 'asset_attrib') {
							$attrs[$id] = $info;
						}
					}

					if (!empty($attrs)) {

						?>
						<table class="sq-backend-table">
							<tr>
								<td class="sq-backend-table-header" style="width: 200px;"><?php echo translate('asset_type'); ?></td>
								<td class="sq-backend-table-header"><?php echo translate('display_name') ?></td>
								<td class="sq-backend-table-header"><?php echo translate('attribute_name'); ?></td>
								<td class="sq-backend-table-header" style="width: 200px;"><?php echo translate('remove_question'); ?></td>
								</tr>
							<?php

						foreach ($attrs as $id => $info) {
							hidden_field($prefix.'_fields['.$id.'][params][type_code]', $info['params']['type_code']);
							hidden_field($prefix.'_fields['.$id.'][type]', 'asset_attrib');
							?>
								<tr>
									<td class="sq-backend-table-cell">
										<?php
											if ($write_access) {
												$types = $asset->attr('types');
												$type_info = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo(array_keys($types), 'name');
												combo_box($prefix.'_fields['.$id.'][params][type_code]', $type_info, false, $info['params']['type_code']);
											} else {
												echo $info['params']['type_code'];
											}
										?>
									</td>
									<td class="sq-backend-table-cell">
										<?php
											if ($write_access) {
												text_box($prefix.'_fields['.$id.'][name]', $info['name']);
											} else {
												echo array_get_index($info, 'name', '');
											}
										?>
									</td>
									<td class="sq-backend-table-cell">
										<?php
											$attributes = $GLOBALS['SQ_SYSTEM']->am->getAssetTypeAttributes($info['params']['type_code']);
											$display_attributes = Array();
											foreach (array_keys($attributes) as $attribute) {
												$display_attributes[$attribute] = ucwords(str_replace('_', ' ', $attribute));
											}
											if ($write_access) {
												combo_box($prefix.'_fields['.$id.'][params][attribute]', $display_attributes, false, $info['params']['attribute']);
											} else {
												echo $display_attributes[$info['params']['attribute']];
											}
										?>
									</td>
									<td class="sq-backend-table-cell">
										<?php
											if ($write_access) {
												check_box($prefix.'_remove['.$id.']', '');
											}
										?>
									</td>
								</tr>
							<?php
						}//end foreach
						?>
							</table>
						<?php
					} else {
						echo translate('cms_listing_no_attribute_sort_options');
					}

				$o->closeField();
			$o->closeSection();
		$o->closeField();

		return $write_access;

	}//end paintSortBy()


	/**
	* Paints the interface for selecting sort field
	*
	* @param object	Listing_Engine		&$asset		asset being painted
	* @param object Backend_Outputter	&$o			backend outputter
	* @param string						$prefix		prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processSortBy(&$asset, &$o, $prefix)
	{
		$sort_by_list = $asset->attr('sort_by');

		$sort_by = array_get_index($_POST, $prefix.'_sort_by', '');
		$name    = array_get_index($_POST, $prefix.'_name', '');

		$options = $this->_getSortByOptions($asset);
		$save    = false;
		$invalid_name = false;

		$remove      = array_get_index($_POST, $prefix.'_remove', Array());
		$posted_list = array_get_index($_POST, $prefix.'_fields', Array());

		$save_list = Array();
		foreach ($posted_list as $id => $info) {
			// skip ones that we are removing
			if (in_array($id, array_keys($remove))) {
				$save = true;
				continue;
			}
			// skip the new one if we already have a name with that id
			if ($info['name'] == $name) $invalid_name = true;

			// set the attribute type for the chosen attribute if there is one
			if ($info['type'] == 'asset_attrib') {
				$attributes = $GLOBALS['SQ_SYSTEM']->am->getAssetTypeAttributes($info['params']['type_code']);
				$info['params']['attr_type'] = $attributes[$info['params']['attribute']];
			}

			$save_list[$id] = $info;
			$save = true;
		}

		if (!empty($sort_by) && !empty($name)) {
			if (!$invalid_name) {
				$invalid = false;
				$new_index = count($save_list);
				$save_list[$new_index] = Array();
				$save_list[$new_index]['params'] = Array();
				$save_list[$new_index]['name'] = $name;

				if ($sort_by == 'asset_attrib') {
					$type = 'asset_attrib';

					// set a default type and attribute
					$types = $asset->attr('types');

					if (!empty($types)) {
						$default_type = array_shift(array_keys($types));
						$attributes   = $GLOBALS['SQ_SYSTEM']->am->getAssetTypeAttributes($default_type);
						$save_list[$new_index]['params']['type_code'] = $default_type;
						$save_list[$new_index]['params']['attribute'] = array_shift(array_keys($attributes));
						$save_list[$new_index]['params']['attr_type'] = array_shift($attributes);
					} else {
						trigger_localised_error('CMS0051', E_USER_WARNING, $name);
						$invalid = true;
					}
				} else {
					$type = 'field';
					$save_list[$new_index]['params']['field'] = $sort_by;
				}

				if (!$invalid) {
					$save_list[$new_index]['type'] = $type;
					$save = true;
				} else {
					unset($save_list[$new_index]);
				}
			}
		}//end if

		// re-order the list so that the indexes are aligned
		ksort($save_list);
		$reordered_list = Array();
		$i = 0;
		foreach ($save_list as $id => $info) {
			$reordered_list[$i++] = $info;
		}

		if ($save) {
			$asset->setAttrValue('sort_by', $reordered_list);
			return true;
		}

		return false;

	}//end processSortBy()


	/**
	* Paints the inteface for choosing a default sort by field
	*
	* @param object	Listing_Engine		&$asset		asset being painted
	* @param object Backend_Outputter	&$o			backend outputter
	* @param string						$prefix		prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintDefaultSortByOption(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$sort_by_list = $asset->attr('sort_by');

		$o->openField(translate('cms_listing_default_sort_by_option'));

			if (empty($sort_by_list)) {
				echo translate('cms_listing_no_sort_by_options');
				return false;
			}

			$names = Array();
			$default =  $asset->attr('default_sort_by');
			foreach ($sort_by_list as $id => $info) {
				$names[$id] = $info['name'];
			}

			if ($write_access) {
				combo_box($prefix.'_default', $names, '', $default);
			} else {
				echo $sort_by_list[$default]['name'];
			}
		$o->closeField();

		return $write_access;

	}//end paintDefaultSortByOption()


	/**
	* Processes the default sory by option
	*
	* @param object	Listing_Engine		&$asset		asset being painted
	* @param object Backend_Outputter	&$o			backend outputter
	* @param string						$prefix		prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processDefaultSortByOption(&$asset, &$o, $prefix)
	{
		// check to see if the default has changed
		if (isset($_POST[$prefix.'_default'])) {
			$default = $asset->attr('default_sort_by');
			if ($_POST[$prefix.'_default'] != $default) {
				$asset->setAttrValue('default_sort_by', $_POST[$prefix.'_default']);
				return true;
			}
		}
		return false;

	}//end processDefaultSortByOption()


	/**
	* Paints the interface for re ordering the sort options
	*
	* @param object	Listing_Engine		&$asset		asset being painted
	* @param object Backend_Outputter	&$o			backend outputter
	* @param string						$prefix		prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintOrderSortOptions(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$sort_by_list = $asset->attr('sort_by');

		if (empty($sort_by_list)) return false;

		$o->openField(translate('cms_listing_list_sort_order'));

			?>
			<script language="JavaScript">
				function sortOrderMoveDown(currentOrder) {
					if (!document.getElementById) {
						alert(js_translate('brower_does_not_support_getelementbyid'));
						return;
					}

					var currentElement = document.getElementById('<?php echo $prefix ?>_sort_order_' + currentOrder);
					var nextElement    = document.getElementById('<?php echo $prefix ?>_sort_order_' + (currentOrder + 1));

					// if there is no next element, then this is the last one; we shouldn't be running
					if (!nextElement) return;

					if (!currentElement.innerHTML) {
						alert(js_translate('brower_does_not_support_innerhtml'));
						return;
					}

					// switch the question names
					var temp = currentElement.innerHTML;
					currentElement.innerHTML = nextElement.innerHTML;
					nextElement.innerHTML = temp;

					var currOrder = document.getElementById('<?php echo $prefix; ?>_sort_order_list[' + currentOrder +']');
					var prevOrder = document.getElementById('<?php echo $prefix; ?>_sort_order_list[' + (currentOrder + 1) +']');

					var temp = prevOrder.value;
					prevOrder.value = currOrder.value;
					currOrder.value = temp;
				}

				function sortOrderMoveUp(currentOrder) {
					if (!document.getElementById) {
						alert(js_translate('brower_does_not_support_getelementbyid'));
						return;
					}

					if (currentOrder == 0) return;

					var currentElement = document.getElementById('<?php echo $prefix ?>_sort_order_' + currentOrder);
					var prevElement    = document.getElementById('<?php echo $prefix ?>_sort_order_' + (currentOrder - 1));

					if (!currentElement.innerHTML) {
						alert(js_translate('brower_does_not_support_innerhtml'));
						return;
					}

					// switch the question names
					var temp = currentElement.innerHTML;
					currentElement.innerHTML = prevElement.innerHTML;
					prevElement.innerHTML = temp;

					var currOrder = document.getElementById('<?php echo $prefix; ?>_sort_order_list[' + currentOrder +']');
					var prevOrder = document.getElementById('<?php echo $prefix; ?>_sort_order_list[' + (currentOrder - 1) +']');

					var temp = prevOrder.value;
					prevOrder.value = currOrder.value;
					currOrder.value = temp;
				}
			</script>
			<table class="sq-backend-table" style="width: 50%">
				<tr>
					<td class="sq-backend-table-header"><?php echo translate('cms_listing_sort_option_name'); ?></td>
					<?php
						if ($write_access) {
							?><td class="sq-backend-table-header"><?php echo translate('order'); ?></td><?php
						}
				?>
				</tr>
					<?php
						$i = 0;
						foreach ($sort_by_list as $id => $info) {
							?>
							<tr>
								<td class="sq-backend-table-cell">
									<?php hidden_field($prefix.'_sort_order_list['.$id.']', $id); ?>
									<span id ="<?php echo $prefix ?>_sort_order_<?php echo $id; ?>"><?php echo $info['name']; ?></span>
								</td>
								<?php
								if ($write_access) {
									?>
									<td class="sq-backend-table-cell">
										<?php
											if ($i != 0) {
												?><a href="#" onclick="sortOrderMoveUp(<?php echo $id ?>); return false;"><script language="JavaScript" type="text/javascript">sq_print_icon("<?php echo sq_web_path('lib').'/web/images/icons/up_arrow.png' ?>", "16", "16", "<?php echo translate('move_up'); ?>");</script></a><?php
											}

											if ($i != count($sort_by_list) - 1) {
												?><a href="#" onclick="sortOrderMoveDown(<?php echo $id ?>); return false;"><img src="<?php echo sq_web_path('lib').'/web/images/icons/down_arrow.png' ?>" width="16" height="16" border="0"alt="<?php echo translate('move_down'); ?>" /></a><?php
											}
											?>
									</td>
									<?php
								}
								?>

							</tr>
							<?php

							$i++;
						}
					?>
			</table>
		<?php
		$o->closeField();

		return $write_access;

	}//end paintOrderSortOptions()


	/**
	* Processes the interface for re ordering the sort options
	*
	* @param object	Listing_Engine		&$asset		asset being painted
	* @param object Backend_Outputter	&$o			backend outputter
	* @param string						$prefix		prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processOrderSortOptions(&$asset, &$o, $prefix)
	{
		$sort_by_list = $asset->attr('sort_by');
		$sort_order = array_get_index($_POST, $prefix.'_sort_order_list', Array());

		$save = false;
		$save_list = Array();
		$i = 0;
		foreach ($sort_order as $id => $order) {
			// check to see if they have re-ordered anything
			// we can rely on the name being unique
			if (isset($sort_by_list[$order]) && isset($sort_by_list[$id])) {
				if ($sort_by_list[$order]['name'] != $sort_by_list[$id]['name']) {
					$save = true;
				}
				$save_list[$i++] = $sort_by_list[$order];
			}
		}

		if ($save) {
			$asset->setAttrValue('sort_by', $save_list);
			return true;
		}
		return false;

	}//end processOrderSortOptions()


	/**
	* Paints the inteface for changing the default sort direction text
	*
	* @param object	Listing_Engine		&$asset		asset being painted
	* @param object Backend_Outputter	&$o			backend outputter
	* @param string						$prefix		prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintDefaultSortDirection(&$asset, &$o, $prefix)
	{
		$write_access   = $asset->writeAccess('attributes');
		$sort_direction = $asset->attr('reverse_sort');
		$sort_asc_text  = $asset->attr('sort_direction_asc_text');
		$sort_desc_text = $asset->attr('sort_direction_desc_text');

		$options = Array(
					'0'	=> $sort_asc_text,
					'1'	=> $sort_desc_text,
				   );

		$o->openField('Default Sort Direction');
			if ($write_access) {
				combo_box($prefix.'_default_sort_direction', $options, false, $sort_direction);
			} else {
				echo (!$sort_direction) ? $sort_asc_text : $sort_desc_text;
			}
		$o->closeField();

		return $write_access;

	}//end paintDefaultSortDirection()


	/**
	* Processes the inteface for changing the default sort direction text
	*
	* @param object	Listing_Engine		&$asset		asset being painted
	* @param object Backend_Outputter	&$o			backend outputter
	* @param string						$prefix		prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processDefaultSortDirection(&$asset, &$o, $prefix)
	{
		$direction = array_get_index($_POST, $prefix.'_default_sort_direction');
		if (!is_null($direction)) {
			$direction = (int)(bool)$direction;
			$asset->setAttrValue('reverse_sort', $direction);
			return true;
		}
		return false;

	}//end processDefaultSortDirection()


	/**
	* Get the possible 'sort by' options
	*
	* @return Array(string => string)
	* @access protected
	*/
	function _getSortByOptions(&$asset)
	{
		if ($asset->attr('group_by') == 'letter') {
			return Array(
					''				=> '-- '.translate('no_sorting').' --',
					'name'			=> translate('name'),
					'short_name'	=> translate('short_name'),
				   );
		} else {
			return Array(
					''				=> '-- '.translate('no_sorting').' --',
					'assetid'		=> translate('asset_id'),
					'version'		=> translate('version'),
					'name'			=> translate('name'),
					'short_name'	=> translate('short_name'),
					'created'		=> translate('created_date'),
					'updated'		=> translate('last_updated_date'),
					'published'		=> translate('last_published_date'),
					'type_code'		=> translate('type_code'),
					'asset_attrib'		=> translate('cms_listing_asset_attribute(s)'),
				   );
		}

	}//end _getSortByOptions()


}//end class
?>
