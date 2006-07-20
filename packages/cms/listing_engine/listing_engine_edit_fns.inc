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
* $Id: listing_engine_edit_fns.inc,v 1.55.2.4 2006/07/20 00:03:51 rong Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page_edit_fns.inc';
require_once SQ_FUDGE_PATH.'/general/general.inc';


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
* @author Luke Wright   <lwright@squiz.net>
* @author David Heppell <dheppell@squiz.net>
* @author Marc McIntyre <mmcintyre@squiz.net>
* @author Greg Sherwood <greg@squiz.net>
* @author Andrei Railean <arailean@squiz.net>
* @author Robert Howard <rhoward@squiz.net>
* @version $Revision: 1.55.2.4 $
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
		$this->static_screens['details']['force_unlock'] = FALSE;
		$this->static_screens['details']['lock_type'] = 'content';

	}//end constructor


	/**
	* Paint the interface for choosing which asset types to list
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
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
			asset_type_chooser($prefix.'_types', TRUE, $display_values, NULL, TRUE, TRUE);
			return TRUE;
		} else {
			// print the read-only version
			if (empty($types)) {
				echo '<p class="sq-backend-warning">'.translate('cms_listing_no_asset_types_selected').'</p>';
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
							<td><?php
								echo get_asset_type_icon($type);
								echo $type_names[$type];
							?></td>
							<td><img src="<?php echo $inherit_image; ?>" alt="<?php echo $inherit_alt; ?>" /></td>
						</tr>
					<?php
				}
				?>
					</table>
				<?php
			}
			return FALSE;
		}//end else - if write access to content

	}//end paintAssetTypes()


	/**
	* Process the asset types to list
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
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

			$folder =& $asset->getFolder('type_formats');
			$formats = $asset->getFormats('type_formats');

			// See if any types have been removed
			foreach ($existing_types as $type => $inherit) {
				if (!isset($submitted_types[$type])) {
					if (in_array($type, $formats)) {
						// send the type format bodycopy to the trash
						$link = $GLOBALS['SQ_SYSTEM']->am->getLink($folder->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, $type);
						if (!empty($link)) {
							$folder->deleteLink($link['linkid']);
						}
					}
				}
			}

			$asset->setAttrValue('types', $submitted_types);
			return TRUE;
		} else {
			return FALSE;
		}

	}//end processAssetTypes()


	/**
	* Paint the interface for editing custom asset positions
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintAssetPositions(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$asset_positions = $asset->attr('asset_positions');

		// if were using custom grouping, this option is useless
		$groups = $asset->attr('asset_grouping');
		if ($asset->attr('group_by') == 'grouped' && !empty($groups)) {
			$o->note(translate('cms_listing_cannot_use_asset_postitions_with_groups'));
		} else {
			if (!empty($asset_positions)) {
				ksort($asset_positions, SORT_NUMERIC);
				if (!$write_access) {
					$asset_ids = Array();
					foreach ($asset_positions as $asset_data) {
						$asset_ids[] = $asset_data['id'];
					}
					$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($asset_ids);
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
					foreach ($asset_positions as $position => $asset_data) {
							$assetid = $asset_data['id'];
						?>
						<tr>
							<td class="sq-backend-table-cell"><?php echo $position; ?></td>
							<td class="sq-backend-table-cell">
								<?php
								if ($write_access) {
									asset_finder($prefix.'_asset_position['.$position.']', $assetid, Array('asset' => 'D'));
								} else {
									echo get_asset_tag_line($assetid);
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
		}//end else - if grouped

		return $write_access;

	}//end paintAssetPositions()


	/**
	* Process the interface for editing custom asset positions
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processAssetPositions(&$asset, &$o, $prefix)
	{
		$asset_positions = Array();

		// first, update the existing positions
		$current_asset_positions = array_get_index($_POST, $prefix.'_asset_position', Array());
		foreach ($current_asset_positions as $position => $data) {
			if ($data['assetid']) {
				$asset_positions[$position] = $data['assetid'];
			}
		}

		// and delete, the unneded ones
		$delete_asset_position = array_get_index($_POST, $prefix.'_delete_asset_position', Array());
		foreach ($delete_asset_position as $position => $on) {
			unset($asset_positions[$position]);
		}

		// now, process new asset
		$new_position_pos = (int) array_get_index($_POST, $prefix.'_new_asset_position_pos', 0);
		if ($new_position_pos > 0) {
			$new_position_data = array_get_index($_POST, $prefix.'_new_asset_position_asset', Array());
			if ($new_position_assetid = $new_position_data['assetid']) {
				$asset_positions[$new_position_pos] = $new_position_assetid;
			}
		}

		$asset_types = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_values($asset_positions), 'asset', FALSE, 'type_code');
		foreach ($asset_positions as $position=>$id) {
			$asset_positions[$position] = Array('id'=>$id, 'type'=>$asset_types[$id]);
		}
		ksort($asset_positions, SORT_NUMERIC);

		return $asset->setAttrValue('asset_positions', $asset_positions);

	}//end processAssetPositions()


	/**
	* Paints the interface for setting the number of columns to display per row
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for form elements
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
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for form elements
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

		// Check if the existing column settings are at least 1 column
		for (reset($column_layout); NULL !== ($row = key($column_layout)); next($column_layout)) {
			if ((int)$column_layout[$row] < 1) {
				// Tailor the error message depending on whether it's a customised
				// row or an "all remaining rows" list
				if ($row == 0) {
					trigger_localised_error('CMS0061', E_USER_NOTICE);
				} else {
					trigger_localised_error('CMS0060', E_USER_NOTICE, $row);
				}
				$column_layout[$row] = 1;
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
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for form elements
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
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function processColumnPreview(&$asset, &$o, $prefix)
	{
		return FALSE;

	}//end processColumnPreview()


	/**
	* Paint links to the defaut formats
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
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
			$this->_paintDefaultFormatLink($asset->id, 'page_contents', translate('cms_listing_page_contents_desc'));
			$this->_paintDefaultFormatLink($asset->id, 'no_results', translate('cms_listing_no_results_bodycopy_desc'));

			$folder =& $asset->getFolder('type_formats');
			$this->_paintDefaultFormatLink($folder->id, 'default_format', translate('cms_listing_default_asset_desc'));
			?>
		</table>
		<?php

		return FALSE;

	}//end paintDefaultFormatLinks()


	/**
	* Prints the table containing the links to the bodycopies (and default type format)
	*
	* @param string $bodycopy_name			the name of the bodycopy to print, eg. 'no_results'
	* @param string $bodycopy_description	the description to label the bodycopy with
	* @param string	$root_assetid			the id of the asset to look under for bodycopies; NULL indicates the listing_engine asset itself
	*
	* @returns void
	* @access private
	*/
	function _paintDefaultFormatLink($root_assetid, $bodycopy_name, $bodycopy_description)
	{
		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($root_assetid, SQ_LINK_TYPE_2, 'bodycopy', TRUE, $bodycopy_name);
		if (!empty($link)) {
			$info = Array($link['minorid'] => 'contents');
			$href = $GLOBALS['SQ_SYSTEM']->am->getAssetBackendHref($info);
			?>
			<tr>
				<td class="sq-backend-table-cell">
					<?php echo get_asset_tag_line($link['minorid'], 'contents'); ?>
				</td>
				<td class="sq-backend-table-cell">
					<?php echo $bodycopy_description; ?>
				</td>
			</tr>
			<?php
		}
	}

	/**
	* Paint the interface for Group Formats
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	 */
	function paintGroupFormatLinks(&$asset, &$o, $prefix)
	{
		// first lets get the bodycopies we want
		$formats = $asset->getFormats('group_formats');

		if (empty($formats)) {
			echo translate('cms_listing_no_groups');
		} else {
			?>
			<table class="sq-backend-table">
				<tr>
					<td class="sq-backend-table-header"><?php echo translate('edit_format'); ?></td>
				</tr>
				<?php
				foreach (array_keys($formats) as $assetid) {
					$format = reset($GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($assetid)));
					$link = $GLOBALS['SQ_SYSTEM']->am->getAssetBackendHref(Array($assetid => 'contents'))
					?>
					<tr>
						<td class="sq-backend-table-cell"><?php echo get_asset_tag_line($assetid, 'contents'); ?></td>
					</tr>
				<?php
				}
				?>
				</table>
			<?php
		}

		return TRUE;

	}//end paintGroupFormatLinks()


	/**
	* Gets a list of the available asset types that can be use for type formats.
	* The reason it's used is so that assets (such as Search_Page) don't need to
	* override the whole paint/process functions if they've got special asset
	* type logic (Eg. proxy asset types)
	*
	* @param object	&$asset	the asset whose interface we are painting
	*
	* @return boolean
	* @access public
	*/
	function _getAvailableTypes(&$asset)
	{
		return $asset->attr('types');

	}//end _getAvailableTypes()


	/**
	* Paint the interface for editing custom type formats
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintTypeFormats(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('links');
		$types = $this->_getAvailableTypes($asset);
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
							echo get_asset_type_icon($value);
							echo $type_name;
							?>
						</td>
						<td class="sq-backend-table-cell">
							<?php
							echo get_asset_tag_line($minorid, 'contents');
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
				combo_box($prefix.'_new_type', $new_format_options, FALSE, '');
				echo '</p>';
			}
		}
		return FALSE;

	}//end paintTypeFormats()


	/**
	* Process the interface for editing custom type formats
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processTypeFormats(&$asset, &$o, $prefix)
	{
		$folder =& $asset->getFolder('type_formats');
		$types = $this->_getAvailableTypes($asset);

		$deleted_formats = array_get_index($_POST, $prefix.'_delete_type', Array());
		$new_type = array_get_index($_POST, $prefix.'_new_type', '');
		if (isset($deleted_formats[$new_type])) {
			trigger_localised_error('CMS0042', E_USER_WARNING, $new_type);
			unset($deleted_formats[$new_type]);
			$new_type = '';
		}

		foreach ($deleted_formats as $type => $on) {
			$current_link = $GLOBALS['SQ_SYSTEM']->am->getLink($folder->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, $type);
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
			$current_link = $GLOBALS['SQ_SYSTEM']->am->getLink($folder->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, $new_type);
			if (empty($current_link)) {
				// create a new format bodycopy for this type
				$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
				$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
				$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');

				$copy_link = Array('asset' => &$folder, 'value' => $new_type, 'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);

				$bodycopy =& new Bodycopy();
				$bodycopy->setAttrValue('name', $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($new_type, 'name').' Format');

				if (!$bodycopy->create($copy_link)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return FALSE;
				}
				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			} else {
				trigger_localised_error('CMS0046', E_USER_WARNING, $new_type);
			}
		}
		if (isset($asset->_tmp['formats']['type_formats'])) {
			unset($asset->_tmp['formats']['type_formats']);
		}

		return FALSE;

	}//end processTypeFormats()


	/**
	* Paint the interface for editing custom position formats
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
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
							echo get_asset_tag_line($minorid, 'contents');
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
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processPositionFormats(&$asset, &$o, $prefix)
	{
		$folder =& $asset->getFolder('position_formats');

		$deleted_formats = array_get_index($_POST, $prefix.'_delete_position', Array());
		$new_position = (int) array_get_index($_POST, $prefix.'_new_position', 0);
		if (isset($deleted_formats[$new_position])) {
			trigger_localised_error('CMS0047', E_USER_WARNING, $new_position);
			unset($deleted_formats[$new_position]);
			$new_position = 0;
		}

		foreach ($deleted_formats as $position => $on) {
			$current_link = $GLOBALS['SQ_SYSTEM']->am->getLink($folder->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, 'position_'.$position);
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
			$current_link = $GLOBALS['SQ_SYSTEM']->am->getLink($folder->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, 'position_'.$new_position);
			if (empty($current_link)) {

				// create a new format bodycopy for this list position
				$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
				$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
				$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');

				$copy_link = Array('asset' => &$folder, 'value' => 'position_'.$new_position, 'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);

				$bodycopy =& new Bodycopy();
				$bodycopy->setAttrValue('name', translate('cms_listing_position_number_format', $new_position));

				if (!$bodycopy->create($copy_link)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return FALSE;
				}
				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			} else {
				trigger_localised_error('CMS0050', E_USER_POSITION, $new_position);
			}
		}
		if (isset($asset->_tmp['formats']['position_formats'])) {
			unset($asset->_tmp['formats']['position_formats']);
		}
		return FALSE;

	}//end processPositionFormats()


	/**
	* Paints the interface for selecting sort field
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for form elements
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
				return FALSE;
			} else {
				$sort_by_options = $this->_getSortByOptions($asset);
			}
			if ($write_access) {
				echo ' '.translate('name').' ';
				text_box($prefix.'_name', '');
				echo ' '.translate('sort_by').' ';
				combo_box($prefix.'_sort_by', $sort_by_options, FALSE, '');
			}

		$o->closeField();

		// Sort by standard asset field
		$o->openField('');

			if (!empty($sort_by_list)) {
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
								if (empty($sort_by_options[$info['params']['field']])) {
									continue;
								}
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
							}//end foreach
							?>
							</table>
						<?php
						} else {
							echo translate('cms_listing_no_field_sort_options');
						}

					$o->closeField();
				$o->closeSection();

			// Sort by attribute
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
												combo_box($prefix.'_fields['.$id.'][params][type_code]', $type_info, FALSE, $info['params']['type_code']);
											} else {
												$type_code = $info['params']['type_code'];
												$type_name = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code, 'name');
												echo get_asset_type_icon($type_code);
												echo $type_name;
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
												combo_box($prefix.'_fields['.$id.'][params][attribute]', $display_attributes, FALSE, $info['params']['attribute']);
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
						echo translate('cms_listing_no_attribute_sort_options').'<br /><br /><br />';
					}

				$o->closeField();
			$o->closeSection();
			} else {
				$o->closeField();
			}

		// Sort by metadata
		$o->openField(translate('cms_listing_new_sort_by_metadata'));
			if ($write_access) {
				echo ' '.translate('name').' ';
				text_box($prefix.'_new_metadata', '');
				echo ' '.translate('metadata_field').' ';
				asset_finder($prefix.'_new_metadata_fieldid',0,Array('metadata_field' => 'D'));
			}
		$o->closeField();

		if (!empty($sort_by_list)) {
			$o->openSection(translate('cms_listing_sort_by_metadata'));
				$o->openField('');

					$meta_datas = Array();
					foreach ($sort_by_list as $id => $info) {
						if ($info['type'] == 'metadata') {
							$meta_datas[$id] = $info;
						}
					}

					if (!empty($meta_datas)) {
						?>
						<table class="sq-backend-table">
							<tr>

								<td class="sq-backend-table-header" style="width: 200px;"><?php echo translate('metadata_field'); ?></td>
								<td class="sq-backend-table-header"><?php echo translate('display_name') ?></td>
								<td class="sq-backend-table-header" style="width: 200px;"><?php echo translate('remove_question'); ?></td>
								</tr>
							<?php

						foreach ($meta_datas as $id => $info) {
							hidden_field($prefix.'_metadatas['.$id.'][type]', 'metadata');
							?>
							<tr>
								<td class="sq-backend-table-cell">
									<?php
										if ($write_access) {
											asset_finder($prefix.'_metadatas['.$id.'][field]',$info['field'],Array('metadata_field' => 'D'));
										} else {
											echo get_asset_tag_line($info['field']);
										}
									?>
								</td>
								<td class="sq-backend-table-cell">
									<?php
										if ($write_access) {
											text_box($prefix.'_metadatas['.$id.'][name]', array_get_index($info,'name',''));
										} else {
											echo array_get_index($info, 'name', '');
										}
									?>
								</td>
							<?php

							?>
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
						echo translate('cms_listing_no_metadata_sort_options');
					}
				$o->closeField();
			$o->closeSection();
		}//end if

		$o->closeField();

		return $write_access;

	}//end paintSortBy()


	/**
	* Paints the interface for selecting sort field
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processSortBy(&$asset, &$o, $prefix)
	{
		$sort_by = array_get_index($_POST, $prefix.'_sort_by', '');

		$name    = array_get_index($_POST, $prefix.'_name', '');
		$meta_data = array_get_index($_POST, $prefix.'_new_metadata', '');
		$new_metadata = array_get_index($_POST, $prefix.'_new_metadata_fieldid', '');
		if (isset($new_metadata['assetid'])) {
			$metadata_fieldid = $new_metadata['assetid'];
		}

		$options = $this->_getSortByOptions($asset);
		$save    = TRUE;
		$invalid_name = FALSE;

		$remove      = array_get_index($_POST, $prefix.'_remove', Array());
		$posted_list = array_get_index($_POST, $prefix.'_fields', Array());

		$save_list = Array();

		foreach ($posted_list as $id => $info) {
			// skip ones that we are removing
			if (in_array($id, array_keys($remove))) {
				$save = TRUE;
				continue;
			}
			// skip the new one if we already have a name with that id
			if ($info['name'] == $name) $invalid_name = TRUE;

			// set the attribute type for the chosen attribute if there is one
			if ($info['type'] == 'asset_attrib') {
				$attributes = $GLOBALS['SQ_SYSTEM']->am->getAssetTypeAttributes($info['params']['type_code']);
				$info['params']['attr_type'] = $attributes[$info['params']['attribute']];
			}

			$save_list[$id] = $info;
			$save = TRUE;
		}

		// -- MetaData -- //
		$posted_list = array_get_index($_POST, $prefix.'_metadatas', Array());

		foreach ($posted_list as $id => $info) {
			// skip ones that we are removing
			if (in_array($id, array_keys($remove))) {
				$save = TRUE;
				continue;
			}

			// dont allow blank names
			if (!isset($info['name']) || !strlen($info['name'])) {
				continue;
			}

			$info['field'] = $info['field']['assetid'];

			$save_list[$id] = $info;
			$save = TRUE;
		}

		// add a new metadata field
		if (!empty($sort_by) && !empty($meta_data)  && !empty($metadata_fieldid)) {
			$type = 'metadata';
			$new_index = count($save_list);
			$save_list[$new_index] = Array();
			$save_list[$new_index]['name'] = $meta_data;
			$save_list[$new_index]['type'] = $type;
			$save_list[$new_index]['field'] = $metadata_fieldid;
		}

		// --- Fields --- //
		if (!empty($sort_by) && !empty($name)) {
			if (!$invalid_name) {
				$invalid = FALSE;
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
						$invalid = TRUE;
					}
				} else {
					$type = 'field';
					$save_list[$new_index]['params']['field'] = $sort_by;
				}

				if (!$invalid) {
					$save_list[$new_index]['type'] = $type;
					$save = TRUE;
				} else {
					unset($save_list[$new_index]);
				}
			}//end if not invalid name
		}//end if

		$empty_sort = Array (
						'params'	=> Array (
										'field'	=> '',
									   ),
						'type'		=> '',
						'name'		=> 'No Sorting',
					  );
		$save_list[] = $empty_sort;

		// re-order the list so that the indexes are aligned
		ksort($save_list);
		$reordered_list = Array();
		$i = 0;
		foreach ($save_list as $id => $info) {
			$reordered_list[$i++] = $info;
		}

		if ($save) {
			$asset->setAttrValue('sort_by', $reordered_list);
			return TRUE;
		}

		return FALSE;

	}//end processSortBy()


	/**
	* Paints the inteface for choosing a default sort by field
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for form elements
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
				$asset->setAttrValue('default_sort_by', 0);
				echo 'No Sorting';
				return FALSE;
			}

			$names = Array();
			$default =  $asset->attr('default_sort_by');
			$last_index = count($sort_by_list) - 1;
			
			// bug fix 1513, move 'No Sorting' option to the end
			if ($sort_by_list[$last_index]['name'] != 'No Sorting') {
				// if 'No Sort' is not at the end, find and move it
				$reordered = Array();
				$no_sort = Array();
				foreach ($sort_by_list as $id => $info) {
					if ($info['name'] == 'No Sorting') {
						// update the default index accordingly
						if ($default > $id) {
							$default--;
						} else if ($default == $id) {
							$default = $last_index;
						}
						$no_sort = $info;
					} else {
						$reordered[] = $info;
					}
					
				}
				
				// move 'No Sorting' to the end, and update the attributes
				$reordered[] = $no_sort;
				$sort_by_list = $reordered;
				$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
				$asset->setAttrValue('default_sort_by', $default);
				$asset->setAttrValue('sort_by', $sort_by_list);
				$asset->saveAttributes();
				$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
			}

			foreach ($sort_by_list as $id => $info) {
				$names[$id] = $info['name'];
			}

			if ($write_access) {
				combo_box($prefix.'_default', $names, FALSE, $default);
			} else {
				if (isset($sort_by_list[$default]['name'])) {
					echo $sort_by_list[$default]['name'];
				}
			}
		$o->closeField();

		return $write_access;

	}//end paintDefaultSortByOption()


	/**
	* Processes the default sory by option
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for form elements
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
				return TRUE;
			}
		}
		return FALSE;

	}//end processDefaultSortByOption()


	/**
	* Paints the interface for re ordering the sort options
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintOrderSortOptions(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$sort_by_list = $asset->attr('sort_by');

		if (empty($sort_by_list)) return FALSE;

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
									<?php
									hidden_field($prefix.'_sort_order_list['.$id.']', $id); ?>
									<span id ="<?php echo $prefix ?>_sort_order_<?php echo $id; ?>"><?php echo $info['name']; ?></span>
								</td>
								<?php
								if ($write_access) {
									?>
									<td class="sq-backend-table-cell">
										<?php
											// bug fix 1513
											// 'No Sorting' should always stay as the last option, no reorder available
											if ($info['name'] != 'No Sorting') {
												if ($i != 0) {
													?><a href="#" onclick="sortOrderMoveUp(<?php echo $id ?>); return false;"><script language="JavaScript" type="text/javascript">sq_print_icon("<?php echo sq_web_path('lib').'/web/images/icons/up_arrow.png' ?>", "16", "16", "<?php echo translate('move_up'); ?>");</script></a><?php
												}
	
												if ($i != count($sort_by_list) - 2) {
													?><a href="#" onclick="sortOrderMoveDown(<?php echo $id ?>); return false;"><script language="JavaScript" type="text/javascript">sq_print_icon("<?php echo sq_web_path('lib').'/web/images/icons/down_arrow.png' ?>", "16", "16", "<?php echo translate('move_down'); ?>");</script></a><?php
												}
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
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processOrderSortOptions(&$asset, &$o, $prefix)
	{
		$sort_by_list = $asset->attr('sort_by');
		$sort_order = array_get_index($_POST, $prefix.'_sort_order_list', Array());

		$save = FALSE;
		$save_list = Array();
		$i = 0;
		foreach ($sort_order as $id => $order) {
			// check to see if they have re-ordered anything
			// we can rely on the name being unique
			if (isset($sort_by_list[$order]) && isset($sort_by_list[$id])) {
				if ($sort_by_list[$order]['name'] != $sort_by_list[$id]['name']) {
					$save = TRUE;
				}
				$save_list[$i++] = $sort_by_list[$order];
			}
		}

		if ($save) {
			$asset->setAttrValue('sort_by', $save_list);
			
			// list order changed
			$default_sort_by = $asset->attr('default_sort_by');
			if ($sort_order[$default_sort_by] != $default_sort_by) {
				foreach ($sort_order as $key => $val) {
					if ($val == $default_sort_by) {
						// update the default sort by index too
						$asset->setAttrValue('default_sort_by', $key);
					}
				}
			}
			
			return TRUE;
		}
		return FALSE;

	}//end processOrderSortOptions()


	/**
	* Paints the inteface for changing the default sort direction text
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for form elements
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
				combo_box($prefix.'_default_sort_direction', $options, FALSE, $sort_direction);
			} else {
				echo (!$sort_direction) ? $sort_asc_text : $sort_desc_text;
			}
		$o->closeField();

		return $write_access;

	}//end paintDefaultSortDirection()


	/**
	* Processes the inteface for changing the default sort direction text
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for form elements
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
			return TRUE;
		}
		return FALSE;

	}//end processDefaultSortDirection()


	/**
	* Get the possible 'sort by' options
	*
	* @param object	&$asset	asset being painted
	*
	* @return array
	* @access protected
	*/
	function _getSortByOptions(&$asset)
	{
		if ($asset->attr('group_by') == 'letter') {
			return Array(
					'name'			=> translate('name'),
					'short_name'	=> translate('short_name'),
				   );
		} else {
			return Array(
					'assetid'		=> translate('asset_id'),
					'version'		=> translate('version'),
					'name'			=> translate('name'),
					'short_name'	=> translate('short_name'),
					'created'		=> translate('created_date'),
					'updated'		=> translate('last_updated_date'),
					'published'		=> translate('last_published_date'),
					'type_code'		=> translate('type_code'),
					'asset_attrib'	=> translate('cms_listing_asset_attribute(s)'),
				   );
		}

	}//end _getSortByOptions()


	/**
	* Paints the root node selection box
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function paintRootNode(&$asset, &$o, $prefix)
	{
		// get root links
		$root_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_NOTICE, '', FALSE, 'major', 'root');
		$root_asset_ids = Array();
		foreach ($root_links as $root_link) {
			$root_asset_ids[$root_link['minorid']] = $root_link['minor_type_code'];
		}

		if ($asset->writeAccess('links')) {
			multiple_asset_finder($prefix.'_root_nodes', array_keys($root_asset_ids));
		} else {
			$infos = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_keys($root_asset_ids));
			?><ul style="margin: 1px 15px;"><?php
			foreach ($infos as $assetid => $info) {
				echo '<li>'.get_asset_tag_line($assetid).'</li>';
			}
			?></ul><?php
		}
		return TRUE;

	}//end paintRootNode()


	/**
	* Processes the value input from root node selection box
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return void
	* @access public
	*/
	function processRootNode(&$asset, &$o, $prefix)
	{
		if (!isset($_POST[$prefix.'_root_nodes'])) {
			return FALSE;
		}
		$new_root_nodes = $_POST[$prefix.'_root_nodes'];
		foreach ($new_root_nodes as $new_root_node) {
			$new_root_node_ids[] = $new_root_node['assetid'];
		}

		// get existing root links
		$root_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_NOTICE, '', FALSE, 'major', 'root');

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$ok = TRUE;

		// lets delete the root nodes that have been removed
		foreach ($root_links as $root_link) {
			if (!in_array($root_link['minorid'], $new_root_node_ids)) {
				if (!$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($root_link['linkid'])) {
					$ok = FALSE;
				}
			}
		}

		// now, create the new node links
		foreach ($new_root_nodes as $index => $node) {
			// first test to see if the node already exists -- if so, ignore it
			foreach ($root_links as $root_link) {
				if ($root_link['minorid'] == $node['assetid']) {
					continue 2;
				}
			}
			// otherwise, create the appropriate notice link
			if ($node['assetid'] != 0) {
				$new_root =& $GLOBALS['SQ_SYSTEM']->am->getAsset($node['assetid']);
				if (!$asset->createLink($new_root, SQ_LINK_NOTICE, 'root')) {
					$ok = FALSE;
					trigger_localised_error('CMS0016', E_USER_WARNING, $new_root->name, $new_root->id);
					break;
				}
			}
		}

		if ($ok) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		}

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return $ok;

	}//end processRootNode()


	/**
	* Paint the Group By field
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function paintGroupBy(&$asset, &$o, $prefix)
	{
		$asset->reindexGroupFormats();

		$write_access = $asset->writeAccess('attributes');
		$group_by = $asset->attr('asset_grouping');
		$commit_button = FALSE;

		foreach ($group_by as $level => $group_by_option) {

			switch ($group_by_option['group_type']) {
				case 'parent_asset':
					$option_name = 'cms_listing_parent_asset';
				break;

				case 'metadata':
					$option_name = 'asset_metadata';
				break;

				case 'standard_asset':
					$option_name = 'standard_asset_field';
				break;

				case 'attribute':
					$option_name = 'asset_attribute';
				break;

				default:
					$option_name = '';
				break;
			}

			$o->openSection('Group Level '.($level+1).': '.translate($option_name));

				switch ($group_by_option['group_type']) {
					case 'parent_asset':
						$commit_button |= $this->paintGroupByParentAsset($asset, $o, $prefix.'['.$level.']', $group_by_option);
					break;

					case 'metadata':
						$commit_button |= $this->paintGroupByMetadata($asset, $o, $prefix.'['.$level.']', $group_by_option);
					break;

					case 'standard_asset':
						$commit_button |= $this->paintGroupByStandardAssetField($asset, $o, $prefix.'['.$level.']', $group_by_option);
					break;

					case 'attribute':
						// implementation reserved
					break;
				}

				// common options - sort order, start group on a new page, delete
				$o->openField(translate('cms_listing_sort_order'));

					$options = Array(
								'asc'	=> translate('cms_listing_ascending_order'),
								'desc'	=> translate('cms_listing_descending_order'),
							   );

					if ($write_access) {
						combo_box($prefix.'['.$level.'][sort_order]', $options, FALSE, array_get_index($group_by_option, 'sort_order', 'asc'));
					} else {
						echo $options[array_get_index($group_by_option, 'sort_order', 'asc')];
					}

				$o->closeField();

				$o->openField(translate('cms_listing_max_children_per_group'));
				$max_children = array_get_index($group_by_option, 'max_children', NULL);
					if ($write_access) {
						text_box($prefix.'['.$level.'][max_children]', $max_children, 5);
					} else {
						if (is_null($max_children)) {
							echo '<em>'.translate('unlimited').'</em>';
						} else {
							echo $max_children;
						}
					}
				$o->closeField();


				// only display Delete option if we have write access
				if ($write_access) {

					$o->openField(translate('delete_question'));

						check_box($prefix.'['.$level.'][delete]', '1', array_get_index($group_by_option, 'new_page', FALSE));
						label(translate('cms_listing_delete_group_format'), $prefix.'['.$level.'][delete]');

					$o->closeField();

					// link to group format bodycopy
					$o->openField('');
						$link = $GLOBALS['SQ_SYSTEM']->am->getAssetBackendHref(Array($group_by_option['format_assetid'] => 'contents'));
						?><a href="<?php echo $link[$group_by_option['format_assetid']]; ?>"><?php echo translate('cms_listing_edit_group_format'); ?></a><?php
					$o->closeField();

				}


			$o->closeSection();
		}//end foreach

		$o->sectionNote(translate('cms_listing_engine_grouping_section_note'));

		return $commit_button;

	}//end paintGroupBy()


	/**
	* Process the Group By field
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function processGroupBy(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;

		$group_by_details = $asset->attr('asset_grouping');
		$posted_options = array_get_index($_POST, $prefix, Array());
		$folder =& $asset->getFolder('group_formats');

		// go through and process deletions of groups
		foreach ($posted_options as $key => $posted_option) {
			if (is_array($posted_option) && (array_get_index($posted_option, 'delete', 0) == 1)) {
				$format_assetid = $group_by_details[$key]['format_assetid'];
				$current_link = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($folder->id, $format_assetid);

				if (empty($current_link)) {
					trigger_error('Group format does not seem to exist for group level '.($key+1), E_USER_WARNING);
				} else {
					if (!$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($current_link['linkid'])) {
						trigger_error('Could not delete link between group level '.($key+1).' and group formats folder', E_USER_WARNING);
					}
				}

				unset($group_by_details[$key]);
			}
		}

		foreach ($group_by_details as $group_by_key => $group_by_option) {
			switch ($group_by_option['group_type']) {
				case 'parent_asset':
					$this->processGroupByParentAsset($asset, $o, $prefix, $group_by_key,  $group_by_details[$group_by_key]);
				break;

				case 'metadata':
					$this->processGroupByMetadata($asset, $o, $prefix, $group_by_key, $group_by_details[$group_by_key]);
				break;

				case 'standard_asset':
					$this->processGroupByStandardAssetField($asset, $o, $prefix, $group_by_key, $group_by_details[$group_by_key]);
				break;

				case 'attribute':
					// $this->processGroupByAttribute($asset, $o, $prefix, $group_by_key, $group_by_details[$group_by_key]);
				break;
			}
		}

		// merge the array with nothing, to re-order the keys of the array back
		// to a numerical order
		$group_by_details = array_merge($group_by_details, Array());
		$asset->reindexGroupFormats();

		// process new stuff here
		$asset->setAttrValue('asset_grouping', $group_by_details);
		return $asset->saveAttributes();

	}//end processGroupBy()


	/**
	* Paint 'Group By Metadata' Option
	*
	* @param object	&$asset				asset being painted
	* @param object	&$o					backend outputter
	* @param string	$prefix				prefix for the html doc element name
	* @param array	$group_by_option	the 'group by' array element
	*
	* @return boolean
	* @access public
	*/
	function paintGroupByMetadata(&$asset, &$o, $prefix, $group_by_option)
	{
		$write_access = $asset->writeAccess('attributes');

		$o->openField(translate('metadata_field'));
			$field_assetid = array_get_index($group_by_option, 'metadata_field', 0);

			if ($write_access) {
				asset_finder($prefix.'[metadata_field]', $field_assetid, Array('metadata_field' => 'D'));
			} else {
				if ($field_assetid) {
					echo get_asset_tag_line($field_assetid);
				}
			}

			$o->note(translate('cms_listing_group_metadata_field_note'));
		$o->closeField();

		return $write_access;

	}//end paintGroupByMetadata()


	/**
	* Paint 'Group By Metadata' Option
	*
	* @param object	&$asset				asset being painted
	* @param object	&$o					backend outputter
	* @param string	$prefix				prefix for the html doc element name
	* @param int	$key				key
	* @param array	&$group_by_option	the 'group by' array element
	*
	* @return boolean
	* @access public
	*/
	function processGroupByMetadata(&$asset, &$o, $prefix, $key, &$group_by_option)
	{
		$posted_option = array_get_index($_POST[$prefix], $key, NULL);
		if (!is_array($posted_option)) return FALSE;

		if (isset($posted_option['metadata_field'])) {
			$group_by_option['metadata_field'] = $posted_option['metadata_field']['assetid'];
		}
		$group_by_option['sort_order'] = $posted_option['sort_order'];

		if (empty($posted_option['max_children'])) {
			$group_by_option['max_children'] = NULL;
		} else {
			$group_by_option['max_children'] = $posted_option['max_children'];
		}

		return TRUE;

	}//end processGroupByMetadata()


	/**
	* Paint 'Group By Standard Asset Field' Option
	*
	* @param object	&$asset				asset being painted
	* @param object	&$o					backend outputter
	* @param string	$prefix				prefix for the html doc element name
	* @param array	$group_by_option	the 'group by' array element
	*
	* @return boolean
	* @access public
	*/
	function paintGroupByStandardAssetField(&$asset, &$o, $prefix, $group_by_option)
	{
		$write_access = $asset->writeAccess('attributes');

		$options = Array(
					'assetid'			=> translate('asset_id'),
					'version'			=> translate('version'),
					'name'				=> translate('name'),
					'short_name'		=> translate('short_name'),
					'status'			=> translate('asset_status'),
					'created'			=> translate('created_date'),
					'updated'			=> translate('last_updated_date'),
					'published'			=> translate('last_published_date'),
					'status_changed'	=> translate('status_changed_date'),
					'type_code'			=> translate('type_code'),
				   );
		$field = array_get_index($group_by_option, 'field', '');

		$o->openField(translate('standard_asset_field'));

			if ($write_access) {
				combo_box($prefix.'[standard_asset_field]', $options, FALSE, $field);
			} else {
				if (empty($field)) {
					echo '<em>'.translate('cms_listing_no_field_selected').'</em>';
				} else {
					echo $options[$field];
				}
			}

		$o->closeField();

		return $write_access;

	}//end paintGroupByStandardAssetField()


	/**
	* Process 'Group By Standard Asset Field' Option
	*
	* @param object	&$asset				asset being painted
	* @param object	&$o					backend outputter
	* @param string	$prefix				prefix for the html doc element name
	* @param int	$key				key
	* @param array	&$group_by_option	the 'group by' array element
	*
	* @return boolean
	* @access public
	*/
	function processGroupByStandardAssetField(&$asset, &$o, $prefix, $key, &$group_by_option)
	{
		$posted_option = array_get_index($_POST[$prefix], $key, NULL);
		if (!is_array($posted_option)) return FALSE;

		if (isset($posted_option['standard_asset_field'])) {
			$group_by_option['field'] = $posted_option['standard_asset_field'];
		}
		$group_by_option['sort_order'] = $posted_option['sort_order'];

		if (empty($posted_option['max_children'])) {
			$group_by_option['max_children'] = NULL;
		} else {
			$group_by_option['max_children'] = $posted_option['max_children'];
		}

		return TRUE;

	}//end processGroupByStandardAssetField()


	/**
	* Paint 'Group By Parent Asset' Option
	*
	* @param object	&$asset				asset being painted
	* @param object	&$o					backend outputter
	* @param string	$prefix				prefix for the html doc element name
	* @param array	$group_by_option	the 'group by' array element
	*
	* @return boolean
	* @access public
	*/
	function paintGroupByParentAsset(&$asset, &$o, $prefix, $group_by_option)
	{
		$write_access = $asset->writeAccess('attributes');

		$o->openField(translate('cms_listing_restrict_types'));

			if ($write_access) {
				asset_type_chooser($prefix.'[restrict_types]', TRUE, array_get_index($group_by_option, 'restrict_types', Array()), NULL, FALSE, TRUE);
			} else {
				if (empty($group_by_option['restrict_types']['type_code'])) {
					echo '<i>'.translate('cms_listing_no_types_selected').'</i>';
				} else {
					?><ul style="margin: 1px 15px"><?php
					$type_names = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($group_by_option['restrict_types']['type_code'], 'name');

					foreach ($group_by_option['restrict_types']['type_code'] as $key => $type_code) {
						?><li><?php
							echo get_asset_type_icon($type_code);
							echo $type_names[$type_code];
							if ($group_by_option['restrict_types']['inherit'][$key]) {
								echo ' ('.strtolower(translate('cms_listing_inherits_child_types')).')';
							}
						?></li><?php
					}
					?></ul><?php
				}
			}

			$o->note(translate('cms_listing_group_asset_restriction_note'));
		$o->closeField();

		$o->openField(translate('sort_by'));

			// for now we will only sort by the original set of options (not
			// attributes - it would clutter the page for one)
			$sort_by_options = $this->_getSortByOptions($asset);
			$sort_by_options[''] = '-- '.translate('select').' --';
			unset($sort_by_options['asset_attrib']);

			$current_option = array_get_index($group_by_option, 'sort_by', '');

			if ($write_access) {
				combo_box($prefix.'[sort_by]', $sort_by_options, FALSE, $current_option);
			} else {

				if (empty($current_option)) {
					echo '<i>'.'No sorting applied'.'</i>';
				} else {
					echo $sort_by_options[$current_option];
				}
			}

		$o->closeField();

		return $write_access;

	}//end paintGroupByParentAsset()


	/**
	* Process 'Group By Parent Asset' Option
	*
	* @param object	&$asset				asset being painted
	* @param object	&$o					backend outputter
	* @param string	$prefix				prefix for the html doc element name
	* @param int	$key				key
	* @param array	&$group_by_option	the 'group by' array element
	*
	* @return boolean
	* @access public
	*/
	function processGroupByParentAsset(&$asset, &$o, $prefix, $key, &$group_by_option)
	{
		$posted_option = array_get_index($_POST[$prefix], $key, NULL);
		if (!is_array($posted_option)) return FALSE;

		if (isset($posted_option['restrict_types'])) {
			$group_by_option['restrict_types'] = $posted_option['restrict_types'];

			// the asset type chooser is prone to giving blank asset types if
			// 'more' fields are left blank, so strip them out right here
			foreach ($group_by_option['restrict_types']['type_code'] as $key => $type_code) {
				if (empty($type_code)) {
					unset($group_by_option['restrict_types']['type_code'][$key]);
					unset($group_by_option['restrict_types']['inherit'][$key]);
				}
			}

			$group_by_option['sort_by']		= $posted_option['sort_by'];
			$group_by_option['sort_order']	= $posted_option['sort_order'];

			if (empty($posted_option['max_children'])) {
				$group_by_option['max_children'] = NULL;
			} else {
				$group_by_option['max_children'] = $posted_option['max_children'];
			}
		}

		return TRUE;

	}//end processGroupByParentAsset()


	/**
	* Paint a place to add a new 'Group By' field
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function paintNewGroupBy(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess('attributes')) {
			$group_by = $asset->attr('asset_grouping');

			check_box($prefix.'[new]');
			?> Add a new group level&nbsp;<?php
			$options = Array(0 => 'at the top level');
			for ($i = 1; $i <= count($group_by); $i++) {
				$options[$i] = 'after group level '.$i;
			}
			combo_box($prefix.'[new_after]', $options, FALSE, count($group_by));

			?> of type <?php
			$options = Array(
						''					=> '-- '.translate('select').' --',
						'parent_asset'		=> translate('cms_listing_parent_asset'),
						'metadata'			=> translate('asset_metadata'),
						'standard_asset'	=> translate('standard_asset_field'),
					   );

			combo_box($prefix.'[new_type]', $options, FALSE, '');

		}

		return $asset->writeAccess('attributes');

	}//end paintNewGroupBy()


	/**
	* Process the 'new Group By field' field
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function processNewGroupBy(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess('attributes')) {

			if (isset($_POST[$prefix]['new'])) {

				$new_type = array_get_index($_POST[$prefix], 'new_type', NULL);
				$new_position = array_get_index($_POST[$prefix], 'new_after', NULL);

				if (!is_null($new_type) && !is_null($new_position)) {

					// need to work out where the correct position is, taking
					// into account group deletions

					$posted_options = array_get_index($_POST, $prefix, Array());
					$pos_adjust = 0;
					$group_by_options = $asset->attr('asset_grouping');

					foreach ($group_by_options as $key => $group_by_option) {
						if (isset($posted_options[$key]['delete'])) {
							// if the deleted item's position is above where the
							// new group is going to be placed
							if ((int)$key <= $new_position)$pos_adjust++;
						}
					}

					// $new_position now holds the correct position for inserting
					// the new group
					$new_position -= $pos_adjust;

					// create a new group format bodycopy
					$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
					$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
					$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');
					$folder =& $asset->getFolder('group_formats');


					$copy_link = Array('asset' => &$folder, 'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1, 'sort_order' => $new_position);

					$bodycopy =& new Bodycopy();
					$bodycopy->setAttrValue('name', 'Group Level '.($new_position+1).' Format');
					$group_listing_data = Array('content' => '%group_listing%');

					if (!$bodycopy->create($copy_link, $group_listing_data)) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
						return FALSE;
					}

					// if we lost the lock while creating the bodycopy
					if (!$asset->writeAccess('attributes')) {
						$GLOBALS['SQ_SYSTEM']->am->acquireLock($asset->id, 'attributes');
					}

					// update the grouping list
					$new_group = Array(Array('group_type' => $new_type, 'format_assetid' => $bodycopy->id));
					array_splice($group_by_options, $new_position, 0, $new_group);
					$asset->setAttrValue('asset_grouping', $group_by_options);

					// adjust the other group formats so their names are correct
					$asset->reindexGroupFormats();
					if (!$asset->saveAttributes()) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
						return FALSE;
					}


					$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return TRUE;
				}//end if

			}//end if

		}//end if

		return FALSE;

	}//end processNewGroupBy()


	/**
	* Paint the current Asset Selections
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function paintSelections(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		$selections = $asset->attr('asset_selections');
		$input_types = Array(
						'radio'		=> translate('cms_listing_radio_buttons'),
						'selection'	=> translate('cms_listing_drop_down_list'),
						'textbox'	=> translate('cms_listing_text_field'),
					   );
		$defaults = $asset->getAssetSelectionDefaults();

		// print existing selections
		foreach ($selections as $name => $settings) {
			?>
			<table class="sq-backend-table">
				<tr>
					<td class="sq-backend-table-header"><?php echo $name; ?></td>
					<td class="sq-backend-table-header" style="text-align: right;">
					<?php
						hidden_field($prefix.'_sel['.$name.'][permanent]', array_get_index($settings, 'permanent', $defaults['permanent']));
						if ($write_access && !array_get_index($settings, 'permanent', $defaults['permanent'])) {
							check_box($prefix.'_sel_delete['.$name.']', '1');
							label(translate('delete_question'), $prefix.'_sel_delete['.$name.']');
						}
					?>
					</td>
				</tr>
				<tr>
					<td class="sq-backend-table-cell"><?php echo translate('cms_listing_input_style') ?></td>
					<td class="sq-backend-table-cell">
					<?php
						if ($write_access) {
							combo_box($prefix.'_sel['.$name.'][type]', $input_types, FALSE, array_get_index($settings, 'type', $defaults['type']));
						} else {
							echo $input_types[array_get_index($settings, 'type', $defaults['type'])];
						}
					?>
					</td>
				</tr>
				<tr>
					<td class="sq-backend-table-cell"><?php echo translate('options') ?></td>
					<td class="sq-backend-table-cell">

						<?php
							$options = array_get_index($settings, 'options', Array());
						?>

						<table style="width: 100%; border: 0;">
							<tr>
								<td>
								<?php
									if ($write_access) {
										check_box($prefix.'_sel['.$name.'][options][allow_multiple]', '1', array_get_index($options, 'allow_multiple', $defaults['options']['allow_multiple']));
										label(translate('cms_listing_allow_multiple'), $prefix.'_sel['.$name.'][options][allow_multiple]');
									} else {
										?><img src="<?php echo sq_web_path('lib'); ?>/web/images/<?php echo (array_get_index($options, 'allow_multiple', $defaults['options']['allow_multiple']) ? 'tick' : 'cross'); ?>.gif" width="15" height="15" /><?php
										echo ' '.translate('cms_listing_allow_multiple');
									}
								?>
								</td>
								<td style="text-align: right;"><?php echo translate('cms_listing_radio_dd_only') ?></td>
							</tr>
							<tr>
								<td>
								<?php
									if ($write_access) {
										check_box($prefix.'_sel['.$name.'][options][allow_negative]', '1', array_get_index($options, 'allow_negative', $defaults['options']['allow_negative']));
										label(translate('cms_listing_allow_negative_values'), $prefix.'_sel['.$name.'][options][allow_negative]');
									} else {
										?><img src="<?php echo sq_web_path('lib'); ?>/web/images/<?php echo (array_get_index($options, 'allow_negative', $defaults['options']['allow_negative']) ? 'tick' : 'cross'); ?>.gif" width="15" height="15" /><?php
										echo ' '.translate('cms_listing_allow_negative_values');
									}
								?>
								</td>
								<td style="text-align: right;"><?php echo translate('cms_listing_text_dd_only') ?></td>
							</tr>
							<tr>
								<td>
									<?php
										$restrict_range = array_get_index($options, 'restrict_range', Array());
										if ($write_access) {
											check_box($prefix.'_sel['.$name.'][options][restrict_range][enable]', '1', array_get_index($restrict_range, 'enable', $defaults['options']['restrict_range']['enable']));
											label(translate('cms_listing_restrict_values_to_range').':', $prefix.'_sel['.$name.'][options][restrict_range][enable]');
										} else {
											?><img src="<?php echo sq_web_path('lib'); ?>/web/images/<?php echo (array_get_index($restrict_range, 'enable', $defaults['options']['restrict_range']['enable']) ? 'tick' : 'cross'); ?>.gif" width="15" height="15" /><?php
											echo translate('cms_listing_restrict_values_to_range') ?>:<?php
										}

										// From box
										ob_start();
											if ($write_access) {
												int_text_box($prefix.'_sel['.$name.'][options][restrict_range][min]', array_get_index($restrict_range, 'min', $defaults['options']['restrict_range']['min']), TRUE, 3);
											} else {
												echo '<strong>'.array_get_index($restrict_range, 'min', $defaults['options']['restrict_range']['min']).'</strong>';
											}
											$from_box = ob_get_contents();
										ob_end_clean();

										// To box
										ob_start();
											if ($write_access) {
												int_text_box($prefix.'_sel['.$name.'][options][restrict_range][max]', array_get_index($restrict_range, 'max', $defaults['options']['restrict_range']['max']), TRUE, 3);
											} else {
												echo '<strong>'.array_get_index($restrict_range, 'max', $defaults['options']['restrict_range']['max']).'</strong>';
											}
											$to_box = ob_get_contents();
										ob_end_clean();

										echo ' '.translate('cms_listing_from_to_range', $from_box, $to_box);
									?>
								</td>
								<td style="text-align: right; vertical-align: top;">
									<?php echo translate('cms_listing_text_dd_only_dd_req') ?><br/>
									<?php echo translate('cms_listing_override_by_previous_options') ?>

								</td>
							</tr>
						</table>

					</td>
				</tr>
			</table><br />
			<?php
		}//end foreach $selections

		// new
		if ($write_access) {
			$o->closeField();
			$o->openField(translate('cms_listing_add_new_selection_question'));

			text_box($prefix.'_sel_new', '');
		}

		return $write_access;

	}//end paintSelections()


	/**
	* Process Asset Selection modifications and deletions
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function processSelections(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		if ($write_access) {
			$selections = $asset->attr('asset_selections');
			$defaults = $asset->getAssetSelectionDefaults();

			// modify values
			if (!empty($_POST[$prefix.'_sel'])) {
				foreach ($_POST[$prefix.'_sel'] as $name => $settings) {

					// only modify, not add
					if (isset($selections[$name])) {

						$selections[$name] = array_merge_multi($defaults, $settings);

						$type           = array_get_index($selections[$name], 'type', $defaults['type']);
						$options        = array_get_index($selections[$name], 'options', $defaults['options']);
						$restrict_range = array_get_index($options, 'restrict_range', $defaults['options']['restrict_range']);

						// check for invalid combinations of settings, and change them if required
						// ... allow_multiple - radio & selection only
						if (array_get_index($options, 'allow_multiple', $defaults['options']['allow_multiple']) && ($type != 'radio' && $type != 'selection')) {
							$selections[$name]['options']['allow_multiple'] = 0;
						}

						// ... allow_negative - textbox & selection only
						if (array_get_index($options, 'allow_negative', $defaults['options']['allow_negative']) && $type != 'textbox') {
							$selections[$name]['options']['allow_negative'] = 0;
						}

						// ... restrict_range - textbox & selection only
						if (array_get_index($restrict_range, 'enable', $defaults['options']['restrict_range']['enable']) && ($type != 'textbox' && $type != 'selection')) {
							$selections[$name]['options']['restrict_range']['enable'] = 0;
						}

						// turn the range if selection was selected
						if ($type == 'selection') {
							$selections[$name]['options']['restrict_range']['enable'] = 1;
						}

						// fix negative min or max when 'allow_negative' is unchecked
						if (array_get_index($restrict_range, 'min', $defaults['options']['restrict_range']['min']) < 0 && !array_get_index($options, 'allow_negative', $defaults['options']['allow_negative'])) {
							$selections[$name]['options']['restrict_range']['min'] = 0;
						}
						if (array_get_index($restrict_range, 'max', $defaults['options']['restrict_range']['max']) < 0 && !array_get_index($options, 'allow_negative', $defaults['options']['allow_negative'])) {
							$selections[$name]['options']['restrict_range']['max'] = 0;
						}

						// min > max
						if (array_get_index($restrict_range, 'min', $defaults['options']['restrict_range']['min']) > array_get_index($restrict_range, 'max', $defaults['options']['restrict_range']['max'])) {
							$selections[$name]['options']['restrict_range']['min'] = array_get_index($restrict_range, 'max', $defaults['options']['restrict_range']['max']);
						}

					}//end if

				}//end foreach
			}//end if

			// process deletions
			if (!empty($_POST[$prefix.'_sel_delete'])) {
				foreach ($_POST[$prefix.'_sel_delete'] as $name => $value) {
					if (isset($selections[$name])) {
						unset($selections[$name]);
					}
				}
			}

			if (!empty($_POST[$prefix.'_sel_new'])) {
				// set the defaults
				$name = trim($_POST[$prefix.'_sel_new']);
				$name = str_replace(' ','_',$name);
				$name = str_replace('%','_',$name);

				$selections[$name] = $defaults;
			}

			$asset->setAttrValue('asset_selections', $selections);
		}//end if


		return $write_access;

	}//end processSelections()


	/**
	* Paint the current Asset Targets
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function paintTargets(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		$targets  = $asset->attr('asset_targets');
		$defaults = $asset->getAssetTargetDefaults();

		// print existing selections
		foreach ($targets as $name => $settings) {
			?>
			<table class="sq-backend-table">
				<tr>
					<td class="sq-backend-table-header"><?php echo $name; ?></td>
					<td class="sq-backend-table-header" style="text-align: right;">
					<?php
						hidden_field($prefix.'_target['.$name.'][permanent]', array_get_index($settings, 'permanent', $defaults['permanent']));
						if ($write_access && !array_get_index($settings, 'permanent', $defaults['permanent'])) {
							check_box($prefix.'_target_delete['.$name.']', '1');
							label(translate('delete_question'), $prefix.'_target_delete['.$name.']');
						}
					?>
					</td>
				</tr>
				<tr>
					<td class="sq-backend-table-cell"><?php echo translate('cms_listing_button_label') ?>:</td>
					<td class="sq-backend-table-cell">
					<?php
						if ($write_access) {
							text_box($prefix.'_target['.$name.'][label]', array_get_index($settings, 'label', $defaults['label']));
						} else {
							echo array_get_index($settings, 'label', $defaults['label']);
						}
					?>
					</td>
				</tr>
				<tr>
					<td class="sq-backend-table-cell"><?php echo translate('cms_listing_target') ?>:</td>
					<td class="sq-backend-table-cell">

					<?php
						$assetid = array_get_index(array_get_index($settings, 'target_asset', $defaults['target_asset']), 'assetid', $defaults['target_asset']['assetid']);

						if ($write_access) {
							?>
							<table style="width: 100%; border: 0;">
								<tr>
									<td style="width: 100px"><?php echo translate('url') ?>:</td>
									<td><?php text_box($prefix.'_target['.$name.'][target_url]', array_get_index($settings, 'target_url', $defaults['target_url'])); ?></td>
								</tr>
								<tr>
									<td style="width: 100px"><?php echo translate('cms_listing_or_asset') ?>:</td>
									<td><?php asset_finder($prefix.'_target['.$name.'][target_asset]', $assetid); ?></td>
								</tr>
							</table>
							<?php

						} else {

							if (!empty($settings['target_url'])) {

								?>
								<table style="width: 100%; border: 0;">
									<tr>
										<td style="width: 100px"><?php echo translate('url') ?>:</td>
										<td><?php echo array_get_index($settings, 'target_url', $defaults['target_url']); ?></td>
									</tr>
								</table>
								<?php

							} else if (!empty($settings['target_asset'])) {

								?>
								<table style="width: 100%; border: 0;">
									<tr>
										<td style="width: 100px"><?php echo translate('asset') ?>:</td>
										<td>
										<?php
											echo get_asset_tag_line($assetid);
										?>
										</td>
									</tr>
								</table>
								<?php

							}

						}
					?>

					</td>
				</tr>
			</table><br />
			<?php
		}//end foreach $targets

		// new
		if ($write_access) {
			$o->closeField();
			$o->openField(translate('cms_listing_add_new_target_question'));

			text_box($prefix.'_target_new', '');
		}

		return $write_access;

	}//end paintTargets()


	/**
	* Process Asset Target modifications and deletions
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function processTargets(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		if ($write_access) {

			$targets = $asset->attr('asset_targets');
			$defaults = $asset->getAssetTargetDefaults();

			// modify values
			if (!empty($_POST[$prefix.'_target'])) {
				foreach ($_POST[$prefix.'_target'] as $name => $settings) {
					// only modify, not add
					if (isset($targets[$name])) {
						$targets[$name] = array_merge_multi($defaults, $settings);

						// if url is set, clear the asset selection
						if (!empty($targets[$name]['target_url'])) {
							$targets[$name]['target_asset']['assetid'] = 0;
						}
					}
				}
			}

			// process deletions
			if (!empty($_POST[$prefix.'_target_delete'])) {
				foreach ($_POST[$prefix.'_target_delete'] as $name => $value) {
					if (isset($targets[$name])) unset($targets[$name]);
				}
			}

			// new
			if (!empty($_POST[$prefix.'_target_new'])) {
				// set the defaults
				$name = trim($_POST[$prefix.'_target_new']);
				$name = str_replace(' ','_',$name);
				$name = str_replace('%','_',$name);

				$targets[$name] = $defaults;
			}

			$asset->setAttrValue('asset_targets', $targets);
		}//end if

		return $write_access;

	}//end processTargets()


	/**
	* Paint the interface for selecting searched statuses
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintStatus(&$asset, &$o, $prefix)
	{
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		$write_access = $asset->writeAccess('links');

		$current = $asset->attr('statuses');

		if ($write_access) {
			require_once SQ_LIB_PATH.'/html_form/html_form.inc';
			$statuses = get_constant_values('SQ_STATUS');

			$options = Array();
			foreach ($statuses as $constant => $value) {
				$options[$value] = get_status_description($value);
			}

			combo_box($prefix.'_statuses', $options, TRUE, $current);
		} else {
			if (empty($current)) {
				echo translate('cms_listing_no_status_selected');
			} else {
				?><ul style="margin: 1px 15px;"><?php
				foreach ($current as $status) {
					echo '<li>';
					echo get_asset_status_icon($status);
					echo get_status_description($status);
					echo '</li>';
				}
				?></ul><?php
			}
		}

		return $write_access;

	}//end paintStatus()


	/**
	* Processes the interface for selecting searched statuses
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processStatus(&$asset, &$o, $prefix)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('attributes')) return FALSE;

		$statuses = Array();
		if (isset($_POST[$prefix.'_statuses'])) {
			foreach ($_POST[$prefix.'_statuses'] as $status) {
				$statuses[] = $status;
			}
			$statuses = array_unique($statuses);
		}

		return $asset->setAttrValue('statuses', $statuses);

	}//end processStatus()


	/**
	* Paint the interface for changing the dropdown lists options
	*
	* @param object	&$asset		the asset whose interface we are painting
	* @param object	&$o			the outputter class
	* @param string	$prefix		prefix for the form elements
	* @param array	&$options	options
	*
	* @return boolean
	* @access public
	*/
	function _paintStructuredTypes(&$asset, &$o, $prefix, &$options)
	{

		$types = $options['asset_types'];
		if ($asset->writeAccess('content')) {
			// print the form fields
			$display_values = Array('type_code' => Array(), 'inherit' => Array());
			foreach ($types as $type => $inherit) {
				$display_values['type_code'][] = $type;
				$display_values['inherit'][] = $inherit;
			}
			asset_type_chooser($prefix.'_structured_types', TRUE, $display_values, NULL, TRUE, TRUE);
			return TRUE;
		} else {
			// print the read-only version
			if (empty($types)) {
				echo '<p class="sq-backend-warning">'.translate('cms_listing_no_asset_types_selected').'</p>';
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
			return FALSE;
		}

		return TRUE;

	}//end _paintStructuredTypes()


	/**
	* Process the interface for editing the dropdown lists options
	*
	* @param object	&$asset		the asset whose interface we are painting
	* @param object	&$o			the outputter class
	* @param string	$prefix		prefix for the form elements
	* @param array	&$options	options
	*
	* @return boolean
	* @access public
	*/
	function _processStructuredTypes(&$asset, &$o, $prefix, &$options)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('attributes')) return FALSE;

		if (isset($_POST[$prefix.'_structured_types'])) {
			$existing_types = $options['asset_types'];
			$submitted_types = Array();
			foreach ($_POST[$prefix.'_structured_types']['type_code'] as $i => $type_code) {
				if (!empty($type_code)) {
					$submitted_types[$type_code] = $_POST[$prefix.'_structured_types']['inherit'][$i];
				}
			}
			$options['asset_types'] = $submitted_types;
			$asset->setAttrValue('structured_dropdown_options', $options);
			return TRUE;
		} else {
			return FALSE;
		}

	}//end _processStructuredTypes()


	/**
	* Paint the interface for a single set of selectable create locations
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return void
	* @access public
	*/
	function paintStructuredDropdownOptions(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$options = $asset->attr('structured_dropdown_options');
		?>
		<table class="sq-backend-table">
			<tr>
				<td class="sq-backend-table-header">
					<?php echo translate('name') ?>:
				</td>
				<td colspan="2" class="">
					<?php
						if ($write_access) {
							text_box($prefix.'_structured_name', $options['name']);
						} else {
							echo $options['name'];
						}
					?>
					<br />&nbsp;&nbsp;&nbsp;*<?php echo translate('must_be_selected_dynamic_parameter'); ?>
				</td>
			</tr>
			<tr>
				<td class="sq-backend-table-header">
					<b><?php echo translate('root_node') ?>:</b>
				</td>
				<td >
					<?php
						if ($write_access) {
							asset_finder($prefix.'_structured_root', $options['root_node']);
						} else {
							if ($options['root_node'] != 0) {
								$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($options['root_node']));
								if (isset($asset_info[$options['root_node']]['name'])) {
									echo get_asset_tag_line($options['root_node']);
								} else {
									echo translate('core_no_selectes_root_node');
								}
							} else {
								echo translate('core_no_selectes_root_node');
							}
						}
					?>
				</td>
			</tr>
			<tr>
				<td class="sq-backend-table-header">
					<b><?php echo translate('depth') ?></b>
				</td>

				<td class="sq-backend-table-header">
				<?php
					if ($write_access) {
						int_text_box($prefix.'_structured_max_depth', $options['max_depth'], FALSE, 4, 0);
					} else {
						echo ($options['max_depth'] == '0' ? translate('infinite').' '.translate('depth') : $options['max_depth']);
					}
				?>
				</td>
			</tr>
			<tr>
				<td class="sq-backend-table-header">
					<b><?php echo translate('asset_types') ?>:</b>
				</td>
				<td colspan="2" class="">
					<?php
						$this->_paintStructuredTypes($asset, $o, $prefix, $options);
					?>
				</td>
			</tr>
			<tr>
				<td class="sq-backend-table-header">
					<b><?php echo translate('style') ?></b>
				</td>

				<td >
				<?php if ($write_access) {
					?>
					<select name="<?php echo $prefix.'_structured_input_style'; ?>">
						<option value="FALSE" <?php echo $asset->attr('check_boxes')== FALSE ? 'selected="selected"' : ''; ?> ><?php echo translate('dropdown_list'); ?></option>
						<option value="TRUE" <?php echo $asset->attr('check_boxes')== TRUE ? 'selected="selected"' : ''; ?> ><?php echo translate('radio_buttons'); ?></option>
					</select>
				<?php } else {
						echo $asset->attr('check_boxes') == FALSE ? translate('dropdown_list') : translate('radio_buttons');;
					  }

				?>
				</td>

			</tr>
			<tr>
				<td class="sq-backend-table-header">
					<b><?php echo translate('width') ?></b>
				</td>
				<td >
				<?php
					if ($write_access) {
						int_text_box($prefix.'_structured_width', $options['width'], FALSE, 4, 0);
					} else {
						echo $options['width'] == 0 ? translate('fit_to_contents') : $options['width'];
					}
				?>
				</td>
			</tr>
			<tr>
				<td class="sq-backend-table-header">
					<b><?php echo translate('height') ?></b>
				</td>
				<td >
				<?php
					if ($write_access) {
						int_text_box($prefix.'_structured_height', $options['height'], FALSE, 4, 0);
					} else {
						echo $options['height'] == 0 ? translate('not_specified') : $options['height'];
					}
				?>
				</td>
			</tr>
			<tr>
				<td class="sq-backend-table-header">
					<b><?php echo translate('seperator') ?></b>
				</td>

				<td >
				<?php
					if ($write_access) {
						text_box($prefix.'_structured_seperator', $options['seperator'], FALSE, 10, 0);
					} else {
						echo $options['seperator'];
					}
				?>
				</td>
			</tr>
		</table>
		<br />
		<?php
		return $write_access;

	}//end paintStructuredDropdownOptions()


	/**
	* Paint the interface for a single set of selectable create locations
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return void
	* @access public
	*/
	function processStructuredDropdownOptions(&$asset, &$o, $prefix)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('attributes')) return FALSE;

		// Get current options
		$options = $asset->attr('structured_dropdown_options');

		// Now step thrugh and see which stuff is set. If it isn't set, the value will stay the same

		if (isset($_REQUEST[$prefix.'_structured_root'])) {
			$options['root_node'] = $_REQUEST[$prefix.'_structured_root']['assetid'];
		}

		$this->_processStructuredTypes($asset, $o, $prefix, $options);

		if (isset($_REQUEST[$prefix.'_structured_name'])) {
			$options['name'] = $_REQUEST[$prefix.'_structured_name'];
		}
		if (isset($_REQUEST[$prefix.'_structured_width'])) {
			$options['width'] = $_REQUEST[$prefix.'_structured_width'];
		}
		if (isset($_REQUEST[$prefix.'_structured_height'])) {
			$options['height'] = $_REQUEST[$prefix.'_structured_height'];
		}
		if (isset($_REQUEST[$prefix.'_structured_max_depth'])) {
			$options['max_depth'] = $_REQUEST[$prefix.'_structured_max_depth'];
		}
		if (isset($_REQUEST[$prefix.'_structured_seperator'])) {
			$options['seperator'] = $_REQUEST[$prefix.'_structured_seperator'];
		}
		if (isset($_REQUEST[$prefix.'_structured_input_style'])) {
			$asset->setAttrValue('check_boxes', $_REQUEST[$prefix.'_structured_input_style']);
		}

		$asset->setAttrValue('structured_dropdown_options', $options);
		return TRUE;

	}//end processStructuredDropdownOptions()


	/**
	* Returns TRUE if this is A-Z listing format
	*
	* Even though it looks trivial, this is required to perfom some show_if checks on the backend edit interface
	*
	* @param object	&$asset	The asset that we are using for this interface
	* @param string	$prefix	the prefix for this asset
	*
	* @return boolean
	* @access public
	*/
	function isAZListing(&$asset, $prefix)
	{
		return ($asset->attr('group_by') == 'letter');

	}//end isAZListing()


	/**
	* Process use format bodycopy dropdown
	*
	* This function returns boolean FALSE if write access to links cannot be
	* gained on the listing engine asset, TRUE otherwise.
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintUseNoResultsBodycopy(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('links');
		$bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_TYPE_2, 'bodycopy', FALSE, 'no_results');
		$enabled = !empty($bodycopy_link);

		if ($write_access) {
			$options = Array(
						'1'	=> translate('yes'),
						'0'	=> translate('no'),
					   );

			combo_box($prefix.'_use_bodycopy', $options, FALSE, Array($enabled));

		} else {
			if ($enabled) {
				echo translate('yes').' - '.get_asset_tag_line($bodycopy_link['minorid'], 'contents');
			} else {
				echo translate('no');
			}
		}

		return $write_access;

	}//end paintUseNoResultsBodycopy()


	/**
	* Process use format bodycopy
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processUseNoResultsBodycopy(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('links')) return FALSE;
		$enabled = $_POST[$prefix.'_use_bodycopy'];

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if (!$asset->setUseNoResultsBodycopy($enabled)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return TRUE;

	}//end processUseNoResultsBodycopy()


}//end class


?>
