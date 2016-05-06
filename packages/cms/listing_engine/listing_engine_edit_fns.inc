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
* $Id: listing_engine_edit_fns.inc,v 1.105 2013/07/22 07:52:51 ewang Exp $
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
* @version $Revision: 1.105 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Listing_Engine_Edit_Fns extends Page_Edit_Fns
{


	/**
	* Constructor
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);
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
				echo '<p class="sq-backend-warning">'.translate('No asset types have been selected').'</p>';

			} else {
				$type_names = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo(array_keys($types), 'name');
				?>
					<table class="sq-backend-table">
						<tr>
							<th><?php echo translate('Type') ?></th>

							<th><?php echo translate('Inherit?') ?></th>

						</tr>
				<?php
				$inherit_image_path = sq_web_path('lib').'/web/images/';
				foreach ($types as $type => $inherit) {
					$inherit_image = $inherit_image_path.($inherit ? 'tick' : 'cross').'.png';
					$inherit_alt = $inherit ? translate('Yes') : translate('No');


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
	* When a type is removed from the listing, the type format bodycopy is
	* removed. We need to be able to have type formats for the children of all
	* our proxy asset types.
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
		$proxy_settings = $asset->attr('proxy_types');

		if (isset($_POST[$prefix.'_types'])) {
			$existing_types = $asset->attr('types');

			$submitted_types = Array();
			foreach ($_POST[$prefix.'_types']['type_code'] as $i => $type_code) {
				if (!empty($type_code)) {
					$submitted_types[$type_code] = $_POST[$prefix.'_types']['inherit'][$i];
				}
			}

			$proxy_child_types = Array();
			$delete_proxy_types = Array();
			foreach ($proxy_settings as $proxy_type => $child_types) {
				if (!empty($child_types)) {
					if (in_array($proxy_type, array_keys($submitted_types))) {
						$proxy_child_types = array_merge($proxy_child_types, array_keys($child_types));
					} else {
						$delete_proxy_types[] = $proxy_type;
					}
					$existing_types = array_merge($existing_types, $child_types);
				}
			}

			$folder = $asset->getFolder('type_formats');
			$formats = $asset->getFormats('type_formats');

			// See if any types have been removed
			foreach ($existing_types as $type => $inherit) {
				// added second condition to prevent accidental deletion of proxied children type formats
				if (!isset($submitted_types[$type]) && !in_array($type, $proxy_child_types)) {
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

			// also remove associated proxy_types when the type is removed
			foreach ($delete_proxy_types as $delete_proxy_type) {
				unset($proxy_settings[$delete_proxy_type]);
			}
			$asset->setAttrValue('proxy_types', $proxy_settings);

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
			$o->note(translate('Custom Grouping is currently selected. Asset positions are not used for Custom Grouping'));

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
						<th><?php echo translate('Position'); ?></th>

						<th><?php echo translate('Asset'); ?></th>

						<?php
						if ($write_access) {
							?><th><?php echo translate('Delete?'); ?></th><?php

						}
						?>
					</tr>
					<?php
					foreach ($asset_positions as $position => $asset_data) {
							$assetid = $asset_data['id'];
						?>
						<tr>
							<td><?php echo $position; ?></td>
							<td>
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
								<td>
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
				echo translate('There are no asset positions defined. All assets will be listed using the sorting settings.');

			}

			if ($write_access) {
				echo '<p><b>'.translate('New Asset Position:').'</b> ';

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

		// if there's noting in the POST array, do nothing
		if(!isset($_POST[$prefix.'_asset_position']) && !isset($_POST[$prefix.'_delete_asset_position']) && !isset($_POST[$prefix.'_new_asset_position_pos'])) return FALSE;

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
				<th><?php echo translate('Row'); ?></th>

				<th><?php echo translate('Column(s)'); ?></th>

				<?php
				if ($write_access) {
					?>
					<th><?php echo translate('Delete?') ?></th>

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
						<td><?php echo $row; ?></td>
						<td>
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
							<td>
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
				<td><?php echo translate('All remaining'); ?></td>

				<td>
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
					?><td>&nbsp;</td><?php
				}
				?>
			</tr>
		</table>
		<?php

		if ($write_access) {
			$o->closeField();
			$o->openField(translate('Customise row'));

				echo strtolower(translate('Row')).' ';

				text_box($prefix.'_new_row', '', 5);
				echo ' '.strtolower(translate('Columns')).' ';

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
					trigger_localised_error('CMS0061', translate('The number of columns for "all remaining rows" must be at least 1. Resetting number of columns to 1.'), E_USER_NOTICE);
				} else {
					trigger_localised_error('CMS0060', sprintf(translate('The number of columns for row %s must be at least 1. Resetting number of columns to 1.'), $row), E_USER_NOTICE);
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
					trigger_localised_error('CMS0040', sprintf(translate('Failed adding customisation for row %s, row has already been customised'), $new_row), E_USER_WARNING);
				}
			} else {
				trigger_localised_error('CMS0041', sprintf(translate('Failed adding customisation for row %s, number of columns must be greater than zero'), $new_row), E_USER_WARNING);
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
				<td style="text-align: center;" colspan="<?php echo $num_columns; ?>">
					<?php echo sprintf(translate('All remaining rows will have %s column(s)'), $column_layout[0]); ?>

					<?php
					if ($num_per_page > 0 && $cell_num > $num_per_page) {
						echo '<br /><b>'.sprintf(translate('Note: This asset has been configured to list %s\' assets per page, so the highlighted cells will not be displayed'), $num_per_page).'</b>';

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
	* Paint links to the default formats
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
				<th><?php echo translate('Edit Format') ?></th>

				<th><?php echo translate('Note') ?></th>

			</tr>
			<?php
			$this->_paintDefaultFormatLink($asset->id, 'page_contents', translate('This format defines the layout of the page. Ensure you use the %asset_listing% keyword to include the list of assets on the page.'));
			$this->_paintDefaultFormatLink($asset->id, 'no_results', translate('This format defines the layout of the page when the <b>Customise No Results Bodycopy</b> option is turned on, and there are no assets to list.'));

			$folder = $asset->getFolder('type_formats');
			$this->_paintDefaultFormatLink($folder->id, 'default_format', translate('This is the default format for all assets in the list if they do not have a customised format for their asset type or position.'));

			?>
		</table>
		<?php

		return FALSE;

	}//end paintDefaultFormatLinks()


	/**
	* Prints the table containing the links to the bodycopies (and default type format)
	*
	* @param string	$root_assetid			the id of the asset to look under for bodycopies; NULL indicates the listing_engine asset itself
	* @param string	$bodycopy_name			the name of the bodycopy to print, eg. 'no_results'
	* @param string	$bodycopy_description	the description to label the bodycopy with
	* @param string	$type_code				the type_code of the format to the print (by default, 'bodycopy')
	* @param string	$screen					the edit screen to link to
	*
	* @return void
	* @access private
	*/
	function _paintDefaultFormatLink($root_assetid, $bodycopy_name, $bodycopy_description, $type_code='bodycopy', $screen='contents')
	{
		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($root_assetid, SQ_LINK_TYPE_2, $type_code, TRUE, $bodycopy_name);
		if (!empty($link)) {
			$info = Array($link['minorid'] => 'contents');
			$href = $GLOBALS['SQ_SYSTEM']->am->getAssetBackendHref($info);
			?>
			<tr>
				<td>
					<?php echo get_asset_tag_line($link['minorid'], $screen); ?>
				</td>
				<td>
					<?php echo $bodycopy_description; ?>
				</td>
			</tr>
			<?php
		}

	}//end _paintDefaultFormatLink()


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
			echo translate('There are no groups defined');

		} else {
			?>
			<table class="sq-backend-table">
				<tr>
					<th><?php echo translate('Edit Format'); ?></th>

				</tr>
				<?php
				foreach (array_keys($formats) as $assetid) {
					$_format = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($assetid));
					$format = reset($_format);
					$link = $GLOBALS['SQ_SYSTEM']->am->getAssetBackendHref(Array($assetid => 'contents'))
					?>
					<tr>
						<td><?php echo get_asset_tag_line($assetid, 'contents'); ?></td>
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
	*
	* @param object	&$asset	the asset whose interface we are painting
	*
	* @return boolean
	* @access public
	*/
	function _getAvailableTypes(&$asset)
	{
		$types = $asset->attr('types');
		$proxy_settings = $asset->attr('proxy_types');

		foreach ($proxy_settings as $proxy_type => $proxy_children) {
			if (!empty($proxy_children)) {
				$types = array_merge($proxy_children, $types);
			}
		}

		return $types;

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
					<th><?php echo translate('Asset Type') ?></th>

					<th><?php echo translate('Edit Format') ?></th>

					<?php
					if ($write_access) {
						?>
						<th><?php echo translate('Delete?') ?></th>

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
						<td style="width: 200px;">
							<?php
							echo get_asset_type_icon($value);
							echo $type_name;
							?>
						</td>
						<td>
							<?php
							echo get_asset_tag_line($minorid, 'contents');
							?>
						</td>
						<?php
						if ($write_access) {
							?>
							<td style="width: 100px;">
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
			echo translate('There are no custom type formats defined');

		}//end if cutomised formats

		if ($write_access) {
			$new_format_options = Array('' => '');
			foreach ($types as $type => $inherit) {
				if (in_array($type, $formats)) continue;
				$new_format_options[$type] = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type, 'name');
			}
			if (count($new_format_options) > 1) {
				echo '<p><b>'.translate('New Type Format:').'</b> ';

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
		$folder = $asset->getFolder('type_formats');
		$types = $this->_getAvailableTypes($asset);

		$deleted_formats = array_get_index($_POST, $prefix.'_delete_type', Array());
		$new_type = array_get_index($_POST, $prefix.'_new_type', '');
		if (isset($deleted_formats[$new_type])) {
			trigger_localised_error('CMS0042', sprintf(translate('You cant delete a format and create it at the same time. The current type format for type %s has not been deleted.'), $new_type), E_USER_WARNING);
			unset($deleted_formats[$new_type]);
			$new_type = '';
		}

		foreach ($deleted_formats as $type => $on) {
			$current_link = $GLOBALS['SQ_SYSTEM']->am->getLink($folder->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, $type);
			if (empty($current_link)) {
				trigger_localised_error('CMS0043', sprintf(translate('Failed deleting format for type %s, couldnt get format bodycopy link'), $type), E_USER_WARNING);
			} else {
				if (!$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($current_link['linkid'])) {
					trigger_localised_error('CMS0044', sprintf(translate('Failed deleting format for type %s'), $type), E_USER_WARNING);
				}
			}
		}

		// check that the new type is valid
		if ($new_type != '' && !isset($types[$new_type])) {
			trigger_localised_error('CMS0045', sprintf(translate('Cannot customise the format of type %s, type is not valid'), $new_type), E_USER_WARNING);
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

				$bodycopy = new Bodycopy();
				$bodycopy->setAttrValue('name', $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($new_type, 'name').' Format');

				if (!$bodycopy->create($copy_link)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return FALSE;
				}
				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			} else {
				trigger_localised_error('CMS0046', sprintf(translate('Cannot customise type %s, it is already customised'), $new_type), E_USER_WARNING);
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
					<th><?php echo translate('Position') ?></th>

					<th><?php echo translate('Edit Format') ?></th>

					<?php
					if ($write_access) {
						?><th><?php echo translate('Delete?') ?></th><?php

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
						<td style="width: 200px;">
							<?php
							echo $position;
							?>
						</td>
						<td>
							<?php
							echo get_asset_tag_line($minorid, 'contents');
							?>
						</td>
						<?php
						if ($write_access) {
							?>
							<td style="width: 100px;">
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
			echo translate('There are no custom position formats defined');

		}

		if ($write_access) {
			echo '<p><b>'.translate('New Position Format:').'</b> ';

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
		$folder = $asset->getFolder('position_formats');

		$deleted_formats = array_get_index($_POST, $prefix.'_delete_position', Array());
		$new_position = (int) array_get_index($_POST, $prefix.'_new_position', 0);
		if (isset($deleted_formats[$new_position])) {
			trigger_localised_error('CMS0047', sprintf(translate('You cant delete a format and create it at the same time. The current list position format for position %s has not been deleted.'), $new_position), E_USER_WARNING);
			unset($deleted_formats[$new_position]);
			$new_position = 0;
		}

		foreach ($deleted_formats as $position => $on) {
			$current_link = $GLOBALS['SQ_SYSTEM']->am->getLink($folder->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, 'position_'.$position);
			if (empty($current_link)) {
				trigger_localised_error('CMS0048', sprintf(translate('Failed deleting format for position %s, couldnt get format bodycopy link'), $position), E_USER_WARNING);
			} else {
				if (!$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($current_link['linkid'])) {
					trigger_localised_error('CMS0049', sprintf(translate('Failed deleting format for position %s'), $position), E_USER_WARNING);
				}
			}
		}

		if ($new_position != 0) {
			// check that this position isnt already customised
			$current_link = $GLOBALS['SQ_SYSTEM']->am->getLink($folder->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, 'position_'.$new_position);
			if (empty($current_link)) {

				// create a new format bodycopy for this list position
				$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
				$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
				$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');

				$copy_link = Array('asset' => &$folder, 'value' => 'position_'.$new_position, 'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);

				$bodycopy = new Bodycopy();
				$bodycopy->setAttrValue('name', sprintf(translate('Position %s Format'), $new_position));


				if (!$bodycopy->create($copy_link)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return FALSE;
				}
				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			} else {
				trigger_localised_error('CMS0050', sprintf(translate('Cannot customise list position %s, it is already customised'), $new_position), E_USER_POSITION);
			}
		}
		if (isset($asset->_tmp['formats']['position_formats'])) {
			unset($asset->_tmp['formats']['position_formats']);
		}
		return FALSE;

	}//end processPositionFormats()


	/**
	* Paints the interface for managing sort-by options
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintSortOptions(&$asset, &$o, $prefix)
	{
		// If random grouping is on, no sorting allowed
		if ($asset->attr('group_by') == 'random') {
			$o->openRaw();
			echo translate('Random list format is currently selected. Sorting is disabled.');

			$o->closeRaw();
			return FALSE;
		}

		// If no types, no sorting!
		$types = $asset->attr('types');
		if (empty($types)) {
			$o->openRaw();
			echo translate('There are no types defined. No sorting can be applied.');

			$o->closeRaw();
			return FALSE;
		}

		$wa = $asset->writeAccess('attributes');

		$o->openSection(translate('Sorting by Standard Asset Field Value'));

			$this->_paintStandardFieldSortOptions($asset, $o, $prefix.'_std_sort_options');
		$o->closeSection(); // end standard asset fields

		$o->openSection(translate('Sorting by Asset Attribute Value'));

			$this->_paintAttributeSortOptions($asset, $o, $prefix.'_attr_sort_options');
		$o->closeSection(); // end asset attributes

		$o->openSection(translate('Sorting by Metadata Field Value'));

			$this->_paintMetadataSortOptions($asset, $o, $prefix.'_metadata_sort_options');
		$o->closeSection(); // end metadata

		$o->openSection(('Sorting by Asset Keyword Value'));
			$this->_paintKeywordSortOptions($asset, $o, $prefix.'_keyword_sort_options');
		$o->closeSection(); // end keyword options

		$o->openSection(translate('Extra Sort-by options'));

			$this->_paintExtraSortOptions($asset, $o, $prefix.'_extra_sort_options');
		$o->closeSection(); // end extra options



		return $wa;

	}//end paintSortOptions()


	/**
	* Process the inteface for managing sort-by options
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access private
	*/
	function processSortOptions(&$asset, &$o, $prefix)
	{
		$this->_tmp['previous_sort_options'] = $asset->attr('sort_by');

		$res = FALSE;
		$res = $this->_processStandardFieldSortOptions($asset, $o, $prefix.'_std_sort_options') || $res;
		$res = $this->_processAttributeSortOptions($asset, $o, $prefix.'_attr_sort_options') || $res;
		$res = $this->_processMetadataSortOptions($asset, $o, $prefix.'_metadata_sort_options') || $res;
		$res = $this->_processKeywordSortOptions($asset, $o, $prefix.'_keyword_sort_options') || $res;
		$res = $this->_processExtraSortOptions($asset, $o, $prefix.'_extra_sort_options') || $res;

		if ($res) {
			// renumber
			return $asset->setAttrValue('sort_by', array_values($asset->attr('sort_by')));
		}
		return FALSE;

	}//end processSortOptions()


	/**
	* Paint the inteface for configuring standard asset field sort-by options
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access private
	*/
	function _paintStandardFieldSortOptions(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		$sort_by_options = $this->_getStandardSortByOptions($asset);
		$o->openField($wa ? translate('Current') : '');

			$fields = Array();
			foreach ($asset->attr('sort_by') as $option_id => $info) {
				if ($info['type'] == 'field') {
					$fields[$option_id] = $info;
				}
			}
			if (!empty($fields)) {
				?>
				<table class="sq-backend-table" >
					<tr>
						<th style="width: 60ex"><?php echo translate('Asset Field'); ?></td>

						<th><?php echo translate('Display Name') ?></th>

					<?php
					if ($wa) {
						?>
						<th style="width: 12ex"><?php echo translate('Remove?'); ?></td>

						<?php
					}
					?>
					</tr>
				<?php
				foreach ($fields as $id => $info) {
					if (empty($sort_by_options[$info['params']['field']])) {
						continue;
					}
					?>
					<tr>
						<td>
							<?php
							echo $sort_by_options[$info['params']['field']];
							?>
						</td>
						<td>
							<?php
							if ($wa) {
								text_box($prefix.'_current['.$id.'][name]', $info['name']);
							} else {
								echo array_get_index($info, 'name', '');
							}
							?>
						</td>
					<?php
					if ($wa) {
						?>
						<td>
							<?php
							check_box($prefix.'_current['.$id.'][remove]', '1');
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
				echo translate('There are no standard asset field sort options');

			}
		$o->closeField();

		if ($wa) {
			$o->openField(translate('New'));

				?>
				<table class="sq-backend-table" style="width: 60ex">
					<tr>
						<th style="width: 20ex"><?php echo translate('Asset Field'); ?></th>

						<td>
							<?php
							combo_box($prefix.'_new[field]', $sort_by_options, FALSE, '');
							?>
						</td>
					</tr>
					<tr>
						<th><?php echo translate('Display Name'); ?></th>

						<td><?php text_box($prefix.'_new[name]', ''); ?></td>
					</tr>
				</table>
				<?php
			$o->closeField();
		}

	}//end _paintStandardFieldSortOptions()


	/**
	* Process the inteface for configuring standard asset field sort-by options
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access private
	*/
	function _processStandardFieldSortOptions(&$asset, &$o, $prefix)
	{
		$val = $asset->attr('sort_by');
		$res = FALSE;
		if (!empty($_POST[$prefix.'_current'])) {
			foreach ($_POST[$prefix.'_current'] as $i => $details) {
				if (!empty($details['remove'])) {
					unset($val[$i]);
				} else {
					$val[$i]['name'] = $details['name'];
				}
			}
			$res = TRUE;
		}
		if (!empty($_POST[$prefix.'_new']['name'])) {
			$option = Array(
						'type'		=> 'field',
						'name'		=> $_POST[$prefix.'_new']['name'],
						'params'	=> Array('field' => $_POST[$prefix.'_new']['field']),
					  );
			$this->_addNewSortOption($val, $option);
			$res = TRUE;
		}
		if ($res) {
			return $asset->setAttrValue('sort_by', $val);
		}
		return FALSE;

	}//end _processStandardFieldSortOptions()


	/**
	* Paint the inteface for configuring asset-attribute sort-by options
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access private
	*/
	function _paintAttributeSortOptions(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		$types = $asset->attr('types');
		$type_info = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo(array_keys($types), 'name');


		$o->openField($wa ? translate('Current') : '');


			$attrs = Array();
			foreach ($asset->attr('sort_by') as $id => $info) {
				if ($info['type'] == 'asset_attrib') {
					$attrs[$id] = $info;
				}
			}

			if (!empty($attrs)) {
				?>
				<table class="sq-backend-table">
					<tr>
						<th style="width: 30ex"><?php echo translate('Asset Type'); ?></th>

						<th style="width: 30ex; padding-left: 0px; padding-right: 0px"><?php echo translate('Attribute Name'); ?></th>

						<th><?php echo translate('Display Name') ?></th>

					<?php
					if ($wa) {
						?>
						<th style="width: 12ex"><?php echo translate('Remove?'); ?></th>

						<?php
					}
					?>
					</tr>
				<?php
				foreach ($attrs as $id => $info) {
					?>
					<tr>
						<td>
							<?php
							if ($wa) {
								combo_box($prefix.'_current['.$id.'][type_code]', $type_info, FALSE, $info['params']['type_code']);
							} else {
								echo get_asset_type_icon($info['params']['type_code']);
								echo $type_info[$info['params']['type_code']];
							}
							?>
						</td>
						<td>
							<?php
							$attributes = $GLOBALS['SQ_SYSTEM']->am->getAssetTypeAttributes($info['params']['type_code']);
							$display_attributes = Array();
							foreach (array_keys($attributes) as $attribute) {
								$display_attributes[$attribute] = ucwords(str_replace('_', ' ', $attribute));
							}
							if ($wa) {
								combo_box($prefix.'_current['.$id.'][attribute]', $display_attributes, FALSE, $info['params']['attribute']);
							} else {
								echo $display_attributes[$info['params']['attribute']];
							}
							?>
						</td>
						<td>
							<?php
							if ($wa) {
								text_box($prefix.'_current['.$id.'][name]', $info['name']);
							} else {
								echo array_get_index($info, 'name', '');
							}
							?>
						</td>
					<?php
					if ($wa) {
						?>
						<td>
							<?php
							check_box($prefix.'_current['.$id.'][remove]', '1');
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
				echo translate('There are no asset attribute sort options');

			}
		$o->closeField();

		if ($wa) {
			$o->openField(translate('New'));

				?>
				<table class="sq-backend-table" style="width: 60ex">
					<tr>
						<th style="width: 20ex"><?php echo translate('Asset Type'); ?></th>

						<td>
							<?php
							$types = $asset->attr('types');
							$type_info = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo(array_keys($types), 'name');
							combo_box($prefix.'_new[type_code]', $type_info, FALSE);
							?>
						</td>
					</tr>
					<tr>
						<th><?php echo translate('Display Name'); ?></th>

						<td><?php text_box($prefix.'_new[name]', ''); ?></td>
					</tr>
				</table>
				<?php
			$o->closeField();
		}

	}//end _paintAttributeSortOptions()


	/**
	* Process the inteface for configuring asset-attribute sort-by options
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access private
	*/
	function _processAttributeSortOptions(&$asset, &$o, $prefix)
	{
		$val = $asset->attr('sort_by');
		$res = FALSE;

		if (!empty($_POST[$prefix.'_current'])) {
			foreach ($_POST[$prefix.'_current'] as $i => $details) {
				if (!empty($details['remove'])) {
					unset($val[$i]);
				} else {
					$val[$i]['name'] = $details['name'];
					$val[$i]['params']['type_code'] = $details['type_code'];
					$attributes = $GLOBALS['SQ_SYSTEM']->am->getAssetTypeAttributes($details['type_code']);
					if (isset($attributes[$details['attribute']])) {
						$val[$i]['params']['attribute'] = $details['attribute'];
						$val[$i]['params']['attr_type'] = $attributes[$val[$i]['params']['attribute']];
					} else {
						$val[$i]['params']['attribute'] = reset(array_keys($attributes));
					}
				}
			}
			$res = TRUE;
		}

		if (!empty($_POST[$prefix.'_new']['name'])) {
			$attributes = $GLOBALS['SQ_SYSTEM']->am->getAssetTypeAttributes($_POST[$prefix.'_new']['type_code']);
			if (empty($attributes)) {
				trigger_localised_error('CMS0110', translate('The selected asset type does not have any attributes to be used for sorting.'), E_USER_WARNING);
			} else {
				$default_attr = array_shift(array_keys($attributes));
				$option = Array(
							'type'		=> 'asset_attrib',
							'name'		=> $_POST[$prefix.'_new']['name'],
							'params'	=> Array(
											'type_code'	=> $_POST[$prefix.'_new']['type_code'],
											'attribute'	=> $default_attr,
											'attr_type'	=> $attributes[$default_attr],
										   ),
						  );
				$this->_addNewSortOption($val, $option);
				$res = TRUE;
			}//end if
		}

		if ($res) {
			return $asset->setAttrValue('sort_by', $val);
		}
		return FALSE;

	}//end _processAttributeSortOptions()


	/**
	* Paint the inteface for configuring metadata-field-value sort-by options
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access private
	*/
	function _paintMetadataSortOptions(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		$o->openField($wa ? translate('Current') : '');

			$meta_datas = Array();
			foreach ($asset->attr('sort_by') as $id => $info) {
				if ($info['type'] == 'metadata') {
					$meta_datas[$id] = $info;
				}
			}

			if (!empty($meta_datas)) {
				?>
				<table class="sq-backend-table">
					<tr>
						<th style="width: 60ex"><?php echo translate('Metadata Field'); ?></th>

						<th><?php echo translate('Display Name') ?></th>

					<?php
					if ($wa) {
						?>
						<th style="width: 12ex"><?php echo translate('Remove?'); ?></th>

						<?php
					}
					?>
					</tr>
				<?php
				foreach ($meta_datas as $id => $info) {
					?>
					<tr>
						<td>
							<?php
							if ($wa) {
								asset_finder($prefix.'_current['.$id.'][field]',$info['field'],Array('metadata_field' => 'D'));
							} else {
								echo get_asset_tag_line($info['field']);
							}
							?>
						</td>
						<td>
							<?php
							if ($wa) {
								text_box($prefix.'_current['.$id.'][name]', array_get_index($info,'name',''));
							} else {
								echo array_get_index($info, 'name', '');
							}
							?>
						</td>
					<?php
					if ($wa) {
						?>
						<td>
							<?php
							check_box($prefix.'_current['.$id.'][remove]', '1');
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
				echo translate('There are no metadata field sort options');

			}
		$o->closeField();
		if ($wa) {
			$o->openField(translate('New'));

				?>
				<table class="sq-backend-table" style="width: 60ex">
					<tr>
						<th style="width: 20ex"><?php echo translate('Metadata Field'); ?></th>

						<td><?php asset_finder($prefix.'_new[field]',0,Array('metadata_field' => 'D')); ;?></td>
					</tr>
					<tr>
						<th><?php echo translate('Display Name'); ?></th>

						<td><?php text_box($prefix.'_new[name]', ''); ?></td>
					</tr>
				</table>
				<?php
			$o->closeField();
		}

	}//end _paintMetadataSortOptions()


	/**
	* Process the inteface for configuring metadata-field-value sort-by options
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access private
	*/
	function _processMetadataSortOptions(&$asset, &$o, $prefix)
	{
		$val = $asset->attr('sort_by');
		$res = FALSE;

		if (!empty($_POST[$prefix.'_current'])) {
			foreach ($_POST[$prefix.'_current'] as $i => $details) {
				if (!empty($details['remove'])) {
					unset($val[$i]);
				} else {
					$val[$i]['name'] = $details['name'];
					if (!empty($details['field']['assetid'])) {
						$val[$i]['field'] = $details['field']['assetid'];
					}
				}
			}
			$res = TRUE;
		}

		if (!empty($_POST[$prefix.'_new']['name']) && !empty($_POST[$prefix.'_new']['field']['assetid'])) {
			$option = Array(
						'type'		=> 'metadata',
						'name'		=> $_POST[$prefix.'_new']['name'],
						'field'		=> $_POST[$prefix.'_new']['field']['assetid'],
						'params'	=> Array(),
					  );
			$this->_addNewSortOption($val, $option);
			$res = TRUE;
		}

		if ($res) {
			return $asset->setAttrValue('sort_by', $val);
		}
		return FALSE;

	}//end _processMetadataSortOptions()


	/**
	* Paint the inteface for configuring keyword sort fields
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access private
	*/
	function _paintKeywordSortOptions(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		$o->openField($wa ? translate('Current') : '');

			$keyword_datas = Array();
			foreach ($asset->attr('sort_by') as $id => $info) {
				if ($info['type'] == 'keyword') {
					$keyword_datas[$id] = $info;
				}
			}

			if (!empty($keyword_datas)) {
				?>
				<table class="sq-backend-table">
					<tr>
						<th style="width: 60ex"><?php echo ('Keyword'); ?></th>
						<th><?php echo translate('Display Name') ?></th>

					<?php
					if ($wa) {
						?>
						<th style="width: 12ex"><?php echo translate('Remove?'); ?></th>

						<?php
					}
					?>
					</tr>
				<?php
				foreach ($keyword_datas as $id => $info) {
					?>
					<tr>
						<td>
							<?php
							if ($wa) {
								text_box($prefix.'_current['.$id.'][keyword]', array_get_index($info,'keyword',''));
							} else {
								echo array_get_index($info, 'name', '');
							}
							?>
						</td>
						<td>
							<?php
							if ($wa) {
								text_box($prefix.'_current['.$id.'][name]', array_get_index($info,'name',''));
							} else {
								echo array_get_index($info, 'name', '');
							}
							?>
						</td>
					<?php
					if ($wa) {
						?>
						<td>
							<?php
							check_box($prefix.'_current['.$id.'][remove]', '1');
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
				echo translate('There are no keyword sort options');

			}
		$o->closeField();
		if ($wa) {
			$o->openField(translate('New'));

				?>
				<table class="sq-backend-table" style="width: 60ex">
					<tr>
						<th style="width: 20ex"><?php echo translate('Keyword'); ?></th>

						<td><?php text_box($prefix.'_new[keyword]', ''); ?></td>
					</tr>
					<tr>
						<th><?php echo translate('Display Name'); ?></th>

						<td><?php text_box($prefix.'_new[name]', ''); ?></td>
					</tr>
				</table>
				<?php
			$o->closeField();
		}

	}//end _paintKeywordSortOptions()


	/**
	* Process the inteface for configuring keyword sort fields
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access private
	*/
	function _processKeywordSortOptions(&$asset, &$o, $prefix)
	{
		$val = $asset->attr('sort_by');
		$res = FALSE;

		if (!empty($_POST[$prefix.'_current'])) {
			foreach ($_POST[$prefix.'_current'] as $i => $details) {
				if (!empty($details['remove'])) {
					unset($val[$i]);
				} else {
					$val[$i]['name'] = $details['name'];
					if (!empty($details['keyword'])) {
						$val[$i]['keyword'] = $details['keyword'];
					}
				}
			}
			$res = TRUE;
		}

		if (!empty($_POST[$prefix.'_new']['name']) && !empty($_POST[$prefix.'_new']['keyword'])) {
			$option = Array(
						'type'		=> 'keyword',
						'name'		=> $_POST[$prefix.'_new']['name'],
						'keyword'	=> $_POST[$prefix.'_new']['keyword'],
						'params'	=> Array(),
					  );
			$this->_addNewSortOption($val, $option);
			$res = TRUE;
		}

		if ($res) {
			return $asset->setAttrValue('sort_by', $val);
		}
		return FALSE;


	}//end _processKeywordSortOptions()


	/**
	* Paint the inteface for configuring extra sort fields
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access private
	*/
	function _paintExtraSortOptions(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		if ($wa) $o->addHiddenField($prefix.'_submitted', 1);

		$o->openField(translate('Random Sorting'));

			$random_enabled = FALSE;
			foreach ($asset->attr('sort_by') as $v) {
				if ($v['name'] == 'Random') {
					$random_enabled = TRUE;
					break;
				}
			}
			if ($wa) {
				check_box($prefix.'_enable_random', '1', $random_enabled);
				label(translate('Enable "Random Order" option'), $prefix.'_enable_random');

			} else {
				$filename = $random_enabled ? 'tick' : 'cross';
				echo '<img src="'.sq_web_path('lib').'/web/images/'.$filename.'.png" /> ';
				echo translate('Enable "Random Order" option');

			}
		$o->closeField();
		$o->openField(translate('No Sorting'));

			$nosort_enabled = FALSE;
			foreach ($asset->attr('sort_by') as $v) {
				if ($v['name'] == 'No Sorting') {
					$nosort_enabled = TRUE;
					break;
				}
			}
			if ($wa) {
				check_box($prefix.'_enable_nosort', '1', $nosort_enabled);
				label(translate('Enable "No Sorting" option'), $prefix.'_enable_nosort');

			} else {
				$filename = $nosort_enabled ? 'tick' : 'cross';
				echo '<img src="'.sq_web_path('lib').'/web/images/'.$filename.'.png" /> ';
				echo translate('Enable "No Sorting" option');

			}
		$o->closeField();

	}//end _paintExtraSortOptions()


	/**
	* Process the inteface for configuring extra sort fields
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access private
	*/
	function _processExtraSortOptions(&$asset, &$o, $prefix)
	{
		$val = $asset->attr('sort_by');
		if (isset($_POST[$prefix.'_submitted'])) {
			$want_random = !empty($_POST[$prefix.'_enable_random']);
			$random_index = FALSE;
			foreach ($val as $i => $v) {
				if ($v['name'] == 'Random') {
					$random_index = $i;
					break;
				}
			}
			if (($random_index !== FALSE) && !$want_random) {
				unset($val[$random_index]);
			} else if (($random_index === FALSE) && $want_random) {
				$val[] = Array(
							'name'		=> 'Random',
							'type'		=> '',
							'field'		=> '',
							'params'	=> '',
						 );
			}

			$want_nosort = !empty($_POST[$prefix.'_enable_nosort']);
			$nosort_index = FALSE;
			foreach ($val as $i => $v) {
				if ($v['name'] == 'No Sorting') {
					$nosort_index = $i;
					break;
				}
			}
			if (($nosort_index !== FALSE) && !$want_nosort) {
				unset($val[$nosort_index]);
			} else if (($nosort_index === FALSE) && $want_nosort) {
				$val[] = Array(
							'name'		=> 'No Sorting',
							'type'		=> '',
							'field'		=> '',
							'params'	=> '',
						 );
			}
			return $asset->setAttrValue('sort_by', $val);
		}//end if
		return FALSE;

	}//end _processExtraSortOptions()


	/**
	* Add an option to the sort-by array
	*
	* The new option is added in such a way as to preserve the positioning of
	* 'no sorting' and 'random' at the end of the list, if that's their current position
	*
	* @param array	&$options	The current list of options
	* @param array	$new_option	Option to add
	*
	* @return void
	* @access public
	*/
	function _addNewSortOption(&$options, $new_option)
	{
		$last = end($options);
		$ending = Array();
		while (($last['name'] == 'Random') || ($last['name'] == 'No Sorting')) {
			array_unshift($ending, $last);
			array_pop($options);
			$last = end($options);
		}
		$options = array_merge($options, Array($new_option), $ending);

	}//end _addNewSortOption()


	/**
	* Paints the inteface for choosing a sort by raw or presentation value
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintMetadataSortType(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$sort_by = $asset->attr('metadata_sort_type');

		$option = Array(
					'raw'			=> 'Raw Value',
					'presentation'	=> 'Presentation Value',
				  );

		if ($write_access) {
			combo_box($prefix.'_metadata_sort_type', $option, FALSE, $sort_by);
		} else {
			echo ($sort_by == 'raw') ? 'Raw Value' : 'Presentation Value';
		}

		return $write_access;

	}//end paintMetadataSortType()


	/**
	* Processes the sort by raw/presentation value option
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processMetadataSortType(&$asset, &$o, $prefix)
	{
		// check to see if the default has changed
		if (isset($_POST[$prefix.'_metadata_sort_type'])) {
			return $asset->setAttrValue('metadata_sort_type', $_POST[$prefix.'_metadata_sort_type']);
		}
		return FALSE;

	}//end processMetadataSortType()


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

		if (empty($sort_by_list)) {
			echo translate('No sorting applied');

			return FALSE;
		}
		$last_index = count($sort_by_list) - 1;
		$names = Array();
		$default =  $asset->attr('default_sort_by');

		foreach ($sort_by_list as $id => $info) {
			$names[$id] = $info['name'];
		}

		if ($write_access) {
			combo_box($prefix.'_default_sort', $names, FALSE, $default);
		} else {
			if (isset($sort_by_list[$default]['name'])) {
				echo $sort_by_list[$default]['name'];
			}
		}

		return $write_access;

	}//end paintDefaultSortByOption()


	/**
	* Processes the default sort by option
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
		if (isset($_POST[$prefix.'_default_sort'])) {
			$new_default_index = $_POST[$prefix.'_default_sort'];
			$new_sort_order = $asset->attr('sort_by');
			$old_sort_order = $this->_tmp['previous_sort_options'];
			$new_default_entry = array_get_index($new_sort_order, $new_default_index, -1);
			$old_default_entry = $old_sort_order[$new_default_index];
			unset($new_default_entry['name']);
			unset($old_default_entry['name']);
			if ($new_default_entry != $old_default_entry) {
				// the chosen default has been moved to a new spot,
				// or the number of options has changed,
				// so we'll have to search for the new index by characteristics
				// If they've renamed it this will still work, but if they've
				// changed its characteristics we won't find it, bad luck.
				foreach ($new_sort_order as $i => $v) {
					unset($v['name']);
					if ($v == $old_default_entry) {
						// everything matches except maybe the name
						$new_default_index = $i;
						break;
					}
				}
			}
			return $asset->setAttrValue('default_sort_by', $new_default_index);
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
	function paintSortOptionOrder(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$sort_by_list = $asset->attr('sort_by');

		if (empty($sort_by_list)) {
			echo translate('There are no sort-by options');

			return FALSE;
		}

		// If no types, no sorting!
		$types = $asset->attr('types');
		if (empty($types)) {
			echo translate('There are no types defined. No sorting can be applied.');

			return FALSE;
		}

		?>
		<script type="text/javascript">
			function sortOrderMoveDown(currentOrder) {
				if (!document.getElementById) {
					alert(js_translate('Browser cannot support script - Get Element By ID is not defined'));

					return;
				}

				var currentElement = document.getElementById('<?php echo $prefix ?>_sort_order_' + currentOrder);
				var nextElement    = document.getElementById('<?php echo $prefix ?>_sort_order_' + (currentOrder + 1));

				// if there is no next element, then this is the last one; we shouldn't be running
				if (!nextElement) return;

				if (!currentElement.innerHTML) {
					alert(js_translate('Browser cannot support script - Inner HTML is not defined'));

					return;
				}

				var table = currentElement.parentNode;
				while (table.tagName != 'TABLE') {
					table = table.parentNode;
				}
				table.style.border = '2px dashed #7B9EBD';

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
					alert(js_translate('Browser cannot support script - Get Element By ID is not defined'));

					return;
				}

				if (currentOrder == 0) return;

				var currentElement = document.getElementById('<?php echo $prefix ?>_sort_order_' + currentOrder);
				var prevElement    = document.getElementById('<?php echo $prefix ?>_sort_order_' + (currentOrder - 1));

				if (!currentElement.innerHTML) {
					alert(js_translate('Browser cannot support script - Inner HTML is not defined'));

					return;
				}

				var table = currentElement.parentNode;
				while (table.tagName != 'TABLE') {
					table = table.parentNode;
				}
				table.style.border = '2px dashed #7B9EBD';

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
		<table class="sq-backend-table" style="width: 50%;">
			<tr>
				<th><?php echo translate('Sort-by Option Name'); ?></th>

				<?php
					if ($write_access) {
						?><th colspan="2" style="width: 8ex"><?php echo translate('Order'); ?></th><?php

					}
			?>
			</tr>
				<?php
					$i = 0;
					foreach ($sort_by_list as $id => $info) {
						?>
						<tr>
							<td>
								<?php
								hidden_field($prefix.'_sort_order_list['.$id.']', $id); ?>
								<span id ="<?php echo $prefix ?>_sort_order_<?php echo $id; ?>"><?php echo $info['name']; ?></span>
							</td>
							<?php
							if ($write_access) {
								?>
								<td>
									<?php
									if ($i != 0) {
										sq_print_icon(sq_web_path('lib').'/web/images/icons/up_arrow.png', 16, 16, translate('Move Up'), NULL, 'class="clickable" onclick="sortOrderMoveUp('.$id.')"');

									}
									?>
								</td>
								<td>
									<?php
									if ($i != count($sort_by_list)-1) {
										sq_print_icon(sq_web_path('lib').'/web/images/icons/down_arrow.png', 16, 16, translate('Move Down'), NULL, 'class="clickable" onclick="sortOrderMoveDown('.$id.')"');

									}
									?>
								</td>
								<?php
							}
							?>
						</tr>
						<?php

						$i++;
					}//end foreach
				?>
		</table>
		<?php
		return $write_access;

	}//end paintSortOptionOrder()


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
	function processSortOptionOrder(&$asset, &$o, $prefix)
	{
		$sort_by_list = $asset->attr('sort_by');
		if (isset($_POST[$prefix.'_sort_order_list'])) {
			$sort_order = $_POST[$prefix.'_sort_order_list'];
			$save_list = Array();
			$i = 0;
			foreach ($sort_order as $id => $order) {
				if (empty($sort_by_list[$order])) {
					unset($sort_by_list[$order]);
				}
				if (isset($sort_by_list[$order])) {
					$save_list[$i++] = $sort_by_list[$order];
					unset($sort_by_list[$order]);
				}
			}
			// $sort_by_list now contains only the newly-added fields, which we'll put last
			return $asset->setAttrValue('sort_by', array_merge($save_list, $sort_by_list));
		}
		return FALSE;

	}//end processSortOptionOrder()


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

		if ($write_access) {
			combo_box($prefix.'_default_sort_direction', $options, FALSE, $sort_direction);
		} else {
			echo ($sort_direction) ? $sort_desc_text : $sort_asc_text;
		}

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
	function _getStandardSortByOptions(&$asset)
	{
		if ($asset->attr('group_by') == 'letter') {
			return Array(
					'Name'			=> translate('Name'),

					'Short Name'	=> translate('Short Name'),

				   );
		} else {
			return $GLOBALS['SQ_SYSTEM']->am->getAssetInfoFields();
		}

	}//end _getStandardSortByOptions()


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
		if (!isset($_POST[$prefix.'_root_nodes']) || (isset($asset->_tmp['reverting_to_system_version']) && $asset->_tmp['reverting_to_system_version'])) {
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

			if($node['assetid'] === '0') continue 1;
			// otherwise, create the appropriate notice link
			if (is_numeric($node['assetid']) && count($GLOBALS['SQ_SYSTEM']->am->getAssetInfo($node['assetid'])) !== 0) {
				$new_root = $GLOBALS['SQ_SYSTEM']->am->getAsset($node['assetid']);
				if (!$asset->createLink($new_root, SQ_LINK_NOTICE, 'root')) {
					$ok = FALSE;
					trigger_localised_error('CMS0016', sprintf(translate('Failed adding new root node "%1$s" [#%2$s]'), $new_root->name, $new_root->id), E_USER_WARNING);
				}
			} else {
				trigger_error(sprintf(translate('Failed adding Root Node. This #%1$s asset id is not a valid asset id.'), $node['assetid']), E_USER_WARNING);
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
					$option_name = translate('Parent Asset');
				break;

				case 'metadata':
					$option_name = translate('Asset Metadata');
				break;

				case 'standard_asset':
					$option_name = translate('Standard Asset Field');
				break;

				case 'attribute':
					$option_name = translate('Asset Attribute');
				break;

				case 'keyword':
					$option_name = translate('Keyword');
				break;

				default:
					$option_name = '';
				break;
			}

			$group_name = sprintf(translate('Group Level %s: %s'), ($level+1), $option_name);
			$o->openSection($group_name);

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

					case 'keyword':
						$commit_button |= $this->paintGroupByKeyword($asset, $o, $prefix.'['.$level.']', $group_by_option);
					break;

					case 'attribute':
						// implementation reserved
					break;
				}

				$o->openField(translate('Allow Blank Group Name'));


				$group_by_blank = array_get_index($group_by_option, 'group_by_blank', 1);
				$option = Array(
							1			=> translate('Yes'),

							0	=> translate('No'),

						  );

				if ($write_access) {
					combo_box($prefix.'['.$level.'][group_by_blank]', $option, FALSE, $group_by_blank);
				} else {
					echo ($group_by_blank == 1) ? translate('Yes') : translate('No');


				}

					$o->note(translate('If the grouping source returns a blank value, consider it as a valid group name.'));

				$o->closeField();

				$o->openField(translate('Group Name Match Search Query'));


				$group_name_match_query = array_get_index($group_by_option, 'group_name_match_query', 0);
				$option = Array(
							1			=> translate('Yes'),

							0	=> translate('No'),

						  );

				if ($write_access) {
					combo_box($prefix.'['.$level.'][group_name_match_query]', $option, FALSE, $group_name_match_query);
				} else {
					echo ($group_name_match_query == 1) ? translate('Yes') : translate('No');


				}

					$o->note(translate('On a Search Page, consider it as a valid group only if group name matches with any search query string.'));

				$o->closeField();


				// common options - sort order, start group on a new page, delete
				$o->openField(translate('Sort Order'));


					$options = Array(
								'asc'	=> translate('Ascending Order'),

								'desc'	=> translate('Descending Order'),

							   );

					if ($write_access) {
						combo_box($prefix.'['.$level.'][sort_order]', $options, FALSE, array_get_index($group_by_option, 'sort_order', 'asc'));
					} else {
						echo $options[array_get_index($group_by_option, 'sort_order', 'asc')];
					}

				$o->closeField();

				$o->openField(translate('Max Children Per Group'));

				$max_children = array_get_index($group_by_option, 'max_children', NULL);
					if ($write_access) {
						text_box($prefix.'['.$level.'][max_children]', $max_children, 5);
					} else {
						if (is_null($max_children)) {
							echo '<em>'.translate('Unlimited').'</em>';

						} else {
							echo $max_children;
						}
					}
				$o->closeField();




				// only display Delete option if we have write access
				if ($write_access) {

					$o->openField(translate('Delete?'));


						check_box($prefix.'['.$level.'][delete]', '1', array_get_index($group_by_option, 'new_page', FALSE));
						label(translate('Delete this grouping and its group format'), $prefix.'['.$level.'][delete]');


					$o->closeField();

					// link to group format bodycopy
					$o->openField('');
						$link = $GLOBALS['SQ_SYSTEM']->am->getAssetBackendHref(Array($group_by_option['format_assetid'] => 'contents'));
						?><a href="<?php echo $link[$group_by_option['format_assetid']]; ?>"><?php echo translate('Edit Group Format'); ?></a><?php

					$o->closeField();

				}


			$o->closeSection();
		}//end foreach

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
		$folder = $asset->getFolder('group_formats');

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

				case 'keyword':
					$this->processGroupByKeyword($asset, $o, $prefix, $group_by_key, $group_by_details[$group_by_key]);
				break;


				case 'attribute':
					// $this->processGroupByAttribute($asset, $o, $prefix, $group_by_key, $group_by_details[$group_by_key]);
				break;
			}

			$posted_option = array_get_index($_POST[$prefix], $group_by_key, NULL);
			if (isset($posted_option['group_by_blank'])) {
			    $group_by_details[$group_by_key]['group_by_blank'] = $posted_option['group_by_blank'];
			}
			if (isset($posted_option['group_name_match_query'])) {
			    $group_by_details[$group_by_key]['group_name_match_query'] = $posted_option['group_name_match_query'];
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

		$o->openField(translate('Metadata Field'));

			$field_assetid = array_get_index($group_by_option, 'metadata_field', Array());
			if(is_scalar($field_assetid)) $field_assetid = Array($field_assetid);

			if ($write_access) {
				multiple_asset_finder($prefix.'[metadata_field]', $field_assetid, Array('metadata_field' => 'D'));
			} else {
				if ($field_assetid) {
					echo '<ul>';
					foreach ($field_assetid as $assetid) {
							echo '<li>'.get_asset_tag_line($assetid).'</li>';
					}
					echo '</ul>';
				}
			}

			$o->note(translate('If the metadata schema that contains this field is not applied to an asset, it will be omitted from the asset listing'));

		$o->closeField();

		$o->openField(translate('Default Metadata Sort Type'));


		$sort_by = array_get_index($group_by_option, 'metadata_sort_type', 'raw');
		$option = Array(
					'raw'			=> translate('Raw Value'),

					'presentation'	=> translate('Presentation Value'),

				  );

		if ($write_access) {
			combo_box($prefix.'[metadata_sort_type]', $option, FALSE, $sort_by);
		} else {
			echo ($sort_by == 'raw') ? translate('Raw Value') : translate('Presentation Value');


		}

			$o->note(translate('To sort by the raw metadata value entered on the metadata screen, select Raw Value. To sort by the value displayed on the frontend, select Presentation Value. Sorting by Raw Value will perform the sort before keywords are replaced and presentation formats are applied.'));

		$o->closeField();


		$o->openField(translate('Group By Individual Entry'));


		$group_by_individual = array_get_index($group_by_option, 'group_by_individual', 1);
		$option = Array(
					1			=> translate('Yes'),

					0	=> translate('No'),

				  );

		if ($write_access) {
			combo_box($prefix.'[group_by_individual]', $option, FALSE, $group_by_individual);
		} else {
			echo ($group_by_individual == 1) ? translate('Yes') : translate('No');


		}

			$o->note(translate('If the Metadata Field contains mutiple entries such as a Multiple Selection Field or a Multiple Text Field, use individual entries as grouping source.'));

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
			$group_by_option['metadata_field'] = Array();
			foreach ($posted_option['metadata_field'] as $data) {
			    if(empty($data['assetid'])) continue;
			    $group_by_option['metadata_field'][] = $data['assetid'];
			}
		}
		$group_by_option['sort_order'] = $posted_option['sort_order'];

		if (empty($posted_option['max_children'])) {
			$group_by_option['max_children'] = NULL;
		} else {
			$group_by_option['max_children'] = $posted_option['max_children'];
		}

		if (isset($posted_option['metadata_sort_type'])) {
			$group_by_option['metadata_sort_type'] = $posted_option['metadata_sort_type'];
		}

		if (isset($posted_option['group_by_individual'])) {
			$group_by_option['group_by_individual'] = $posted_option['group_by_individual'];
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
					'assetid'			=> translate('Asset ID'),

					'version'			=> translate('Version'),

					'name'				=> translate('Name'),

					'short_name'		=> translate('Short Name'),

					'status'			=> translate('Asset Status'),

					'created'			=> translate('Created Date'),

					'updated'			=> translate('Last updated date'),

					'published'			=> translate('Last published date'),

					'status_changed'	=> translate('Status Changed Date'),

					'type_code'			=> translate('Type Code'),

				   );
		$field = array_get_index($group_by_option, 'field', '');

		$o->openField(translate('Standard Asset Field'));


			if ($write_access) {
				combo_box($prefix.'[standard_asset_field]', $options, FALSE, $field);
			} else {
				if (empty($field)) {
					echo '<em>'.translate('No field has been selected').'</em>';

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

		$o->openField(translate('Restrict Types'));


			if ($write_access) {
				asset_type_chooser($prefix.'[restrict_types]', TRUE, array_get_index($group_by_option, 'restrict_types', Array()), NULL, FALSE, TRUE);
			} else {
				if (empty($group_by_option['restrict_types']['type_code'])) {
					echo '<i>'.translate('No types have been selected').'</i>';

				} else {
					?><ul style="margin: 1px 15px"><?php
					$type_names = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($group_by_option['restrict_types']['type_code'], 'name');

					foreach ($group_by_option['restrict_types']['type_code'] as $key => $type_code) {
						?><li><?php
							echo get_asset_type_icon($type_code);
							echo $type_names[$type_code];
							if ($group_by_option['restrict_types']['inherit'][$key]) {
								echo ' ('.strtolower(translate('Inherits Child Types')).')';

							}
						?></li><?php
					}
					?></ul><?php
				}
			}

			$o->note(translate('If no asset types are chosen, no asset type restriction will be enforced, and all parents will be included by default'));

		$o->closeField();

		// option to include only assets with direct parent that matches one of the restricted types
		$o->openField('Direct Parent Only ?');
			if ($write_access) {
				check_box($prefix.'[direct_parent_only]', '1', array_get_index($group_by_option, 'direct_parent_only', FALSE));
				label('Group assets by direct parent asset', $prefix.'[direct_parent_only]');
			} else {
				?>
				<img src="<?php echo sq_web_path('lib'); ?>/web/images/<?php echo array_get_index($group_by_option, 'direct_parent_only', FALSE) ? 'tick' : 'cross'; ?>.png" width="15" height="15" />
				<?php
				echo 'Group assets by direct parent asset';
			}
		$o->closeField();

		$o->openField(translate('Sort By'));


			// for now we will only sort by the original set of options (not
			// attributes - it would clutter the page for one)
			$sort_by_options = $this->_getStandardSortByOptions($asset);
			$sort_by_options['sorting_metadata_field'] = 'Metadata Field';
			$sort_by_options[''] = '-- '.translate('Select').' --';


			$current_option = array_get_index($group_by_option, 'sort_by', '');

			if ($write_access) {
				combo_box($prefix.'[sort_by]', $sort_by_options, FALSE, $current_option);
				if ($current_option == 'sorting_metadata_field') {
					echo '<br>';
					$field_assetid = array_get_index($group_by_option, 'sorting_metadata_field', 0);
					asset_finder($prefix.'[sorting_metadata_field]',$field_assetid);
				}
				$o->note('Select the order in which the parent assets should be displayed');
			} else {

				if (empty($current_option)) {
					echo '<i>'.translate('No sorting applied').'</i>';

				} else {
					echo $sort_by_options[$current_option];
				}
			}

		$o->closeField();


		if (isset($group_by_option['sort_by']) && $group_by_option['sort_by'] == 'sorting_metadata_field') {
			$o->openField(translate('Default Metadata Sort Type'));


			$sort_by = array_get_index($group_by_option, 'metadata_sort_type', 'raw');
			$option = Array(
						'raw'			=> translate('Raw Value'),

						'presentation'	=> translate('Presentation Value'),

					  );

			if ($write_access) {
				combo_box($prefix.'[metadata_sort_type]', $option, FALSE, $sort_by);
			} else {
				echo ($sort_by == 'raw') ? translate('Raw Value') : translate('Presentation Value');


			}

			$o->note(translate('To sort by the raw metadata value entered on the metadata screen, select Raw Value. To sort by the value displayed on the frontend, select Presentation Value. Sorting by Raw Value will perform the sort before keywords are replaced and presentation formats are applied.'));

			$o->closeField();
		}


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

			if (isset($posted_option['sorting_metadata_field'])) {
				$group_by_option['sorting_metadata_field'] = $posted_option['sorting_metadata_field']['assetid'];
			}

			$group_by_option['direct_parent_only'] = array_get_index($posted_option, 'direct_parent_only', FALSE);
			$group_by_option['sort_by']		= $posted_option['sort_by'];
			$group_by_option['sort_order']	= $posted_option['sort_order'];

			if (isset($posted_option['metadata_sort_type'])) {
				$group_by_option['metadata_sort_type'] = $posted_option['metadata_sort_type'];
			}

			if (empty($posted_option['max_children'])) {
				$group_by_option['max_children'] = NULL;
			} else {
				$group_by_option['max_children'] = $posted_option['max_children'];
			}
		}

		return TRUE;

	}//end processGroupByParentAsset()


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
	function paintGroupByKeyword(&$asset, &$o, $prefix, $group_by_option)
	{
		$write_access = $asset->writeAccess('attributes');

		$o->openField(translate('Keyword'));

		$keyword = array_get_index($group_by_option, 'keyword', NULL);
			if ($write_access) {
				text_box($prefix.'[keyword]', $keyword, 50);
			} else {
				if (is_null($keyword)) {
					echo '<em>'.translate('No keyword specified').'</em>';

				} else {
					echo $keyword;
				}
			}
		$o->closeField();

		return $write_access;

	}//end paintGroupByKeyword()


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
	function processGroupByKeyword(&$asset, &$o, $prefix, $key, &$group_by_option)
	{
		$posted_option = array_get_index($_POST[$prefix], $key, NULL);
		if (!is_array($posted_option)) return FALSE;

		if (isset($posted_option['keyword'])) {
			$group_by_option['keyword'] = $posted_option['keyword'];
		}
		$group_by_option['sort_order'] = $posted_option['sort_order'];

		if (empty($posted_option['max_children'])) {
			$group_by_option['max_children'] = NULL;
		} else {
			$group_by_option['max_children'] = $posted_option['max_children'];
		}

		return TRUE;

	}//end processGroupByKeyword()


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
						''					=> '-- '.translate('Select').' --',

						'parent_asset'		=> translate('Parent Asset'),

						'metadata'			=> translate('Asset Metadata'),

						'standard_asset'	=> translate('Standard Asset Field'),

						'keyword'			=> translate('Keyword'),

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
					$folder = $asset->getFolder('group_formats');


					$copy_link = Array('asset' => &$folder, 'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1, 'sort_order' => $new_position);

					$bodycopy = new Bodycopy();
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
						'radio'		=> translate('Radio Buttons'),

						'selection'	=> translate('Drop-Down List'),

						'textbox'	=> translate('Text Field'),

					   );
		$defaults = $asset->getAssetSelectionDefaults();

		// print existing selections
		foreach ($selections as $name => $settings) {
			?>
			<table class="sq-backend-table">
				<tr>
					<th><?php echo $name; ?></th>
					<td class="sq-backend-table-header" style="text-align: right;">
					<?php
						hidden_field($prefix.'_sel['.$name.'][permanent]', array_get_index($settings, 'permanent', $defaults['permanent']));
						if ($write_access && !array_get_index($settings, 'permanent', $defaults['permanent'])) {
							check_box($prefix.'_sel_delete['.$name.']', '1');
							label(translate('Delete?'), $prefix.'_sel_delete['.$name.']');

						}
					?>
					</td>
				</tr>
				<tr>
					<td><?php echo translate('Input Style') ?></td>

					<td>
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
					<td><?php echo translate('Options') ?></td>

					<td>

						<?php
							$options = array_get_index($settings, 'options', Array());
						?>

						<table style="width: 100%; border: 0;">
							<tr>
								<td>
								<?php
									if ($write_access) {
										check_box($prefix.'_sel['.$name.'][options][allow_multiple]', '1', array_get_index($options, 'allow_multiple', $defaults['options']['allow_multiple']));
										label(translate('Allow multiple selections'), $prefix.'_sel['.$name.'][options][allow_multiple]');

									} else {
										?><img src="<?php echo sq_web_path('lib'); ?>/web/images/<?php echo (array_get_index($options, 'allow_multiple', $defaults['options']['allow_multiple']) ? 'tick' : 'cross'); ?>.png" width="15" height="15" /><?php
										echo ' '.translate('Allow multiple selections');

									}
								?>
								</td>
								<td style="text-align: right;"><?php echo translate('(Radio Buttons and Drop-down Lists only)') ?></td>

							</tr>
							<tr>
								<td>
								<?php
									if ($write_access) {
										check_box($prefix.'_sel['.$name.'][options][allow_negative]', '1', array_get_index($options, 'allow_negative', $defaults['options']['allow_negative']));
										label(translate('Allow negative values'), $prefix.'_sel['.$name.'][options][allow_negative]');

									} else {
										?><img src="<?php echo sq_web_path('lib'); ?>/web/images/<?php echo (array_get_index($options, 'allow_negative', $defaults['options']['allow_negative']) ? 'tick' : 'cross'); ?>.png" width="15" height="15" /><?php
										echo ' '.translate('Allow negative values');

									}
								?>
								</td>
								<td style="text-align: right;"><?php echo translate('(Text Fields and Drop-down Lists only)') ?></td>

							</tr>
							<tr>
								<td>
									<?php
										$restrict_range = array_get_index($options, 'restrict_range', Array());
										if ($write_access) {
											check_box($prefix.'_sel['.$name.'][options][restrict_range][enable]', '1', array_get_index($restrict_range, 'enable', $defaults['options']['restrict_range']['enable']));
											label(translate('Restrict values to a range').':', $prefix.'_sel['.$name.'][options][restrict_range][enable]');

										} else {
											?><img src="<?php echo sq_web_path('lib'); ?>/web/images/<?php echo (array_get_index($restrict_range, 'enable', $defaults['options']['restrict_range']['enable']) ? 'tick' : 'cross'); ?>.png" width="15" height="15" /><?php
											echo translate('Restrict values to a range') ?>:<?php

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

										echo ' '.sprintf(translate('(From %1$s to %2$s)'), $from_box, $to_box);

									?>
								</td>
								<td style="text-align: right; vertical-align: top;">
									<?php echo translate('(Text Fields and Drop-down Lists only - required for Drop-down Lists)') ?><br/>

									<?php echo translate('(Note, this option will be overridden by the previous options)') ?>


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
			$o->openField(translate('Add New Selection?'));


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
					<th><?php echo $name; ?></th>
					<td class="sq-backend-table-header" style="text-align: right;">
					<?php
						hidden_field($prefix.'_target['.$name.'][permanent]', array_get_index($settings, 'permanent', $defaults['permanent']));
						if ($write_access && !array_get_index($settings, 'permanent', $defaults['permanent'])) {
							check_box($prefix.'_target_delete['.$name.']', '1');
							label(translate('Delete?'), $prefix.'_target_delete['.$name.']');

						}
					?>
					</td>
				</tr>
				<tr>
					<td><?php echo translate('Button Label') ?>:</td>

					<td>
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
					<td><?php echo translate('Target') ?>:</td>

					<td>

					<?php
						$assetid = array_get_index(array_get_index($settings, 'target_asset', $defaults['target_asset']), 'assetid', $defaults['target_asset']['assetid']);

						if ($write_access) {
							?>
							<table style="width: 100%; border: 0;">
								<tr>
									<td style="width: 100px"><?php echo translate('URL') ?>:</td>

									<td><?php text_box($prefix.'_target['.$name.'][target_url]', array_get_index($settings, 'target_url', $defaults['target_url'])); ?></td>
								</tr>
								<tr>
									<td style="width: 100px"><?php echo translate('or Asset') ?>:</td>

									<td><?php asset_finder($prefix.'_target['.$name.'][target_asset]', $assetid); ?></td>
								</tr>
							</table>
							<?php

						} else {

							if (!empty($settings['target_url'])) {

								?>
								<table style="width: 100%; border: 0;">
									<tr>
										<td style="width: 100px"><?php echo translate('URL') ?>:</td>

										<td><?php echo array_get_index($settings, 'target_url', $defaults['target_url']); ?></td>
									</tr>
								</table>
								<?php

							} else if (!empty($settings['target_asset'])) {

								?>
								<table style="width: 100%; border: 0;">
									<tr>
										<td style="width: 100px"><?php echo translate('Asset') ?>:</td>

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
			$o->openField(translate('Add New Target?'));


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
		$write_access = $asset->writeAccess('attributes');

		$current = $asset->attr('statuses');

		if ($write_access) {
			require_once SQ_LIB_PATH.'/html_form/html_form.inc';
			$statuses = get_constant_values('SQ_STATUS');
    	$options = Array();
			foreach ($statuses as $constant => $value) {
				$options[$value] = get_status_description($value);
			}

			combo_box($prefix.'_statuses', $options, TRUE, $current);
			hidden_field($prefix.'_statuses_submitted', '1');
		} else {
			if (empty($current)) {
				echo translate('No statuses are selected. Only assets for which the user has sufficient read permissions will be listed.');

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

		// must make sure if the form 'statuses to list' was actually submitted
		if (!isset($_POST[$prefix.'_statuses_submitted'])) return FALSE;

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
	* Paint the interface for selecting searched permissions
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintPermissions(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		$perm_options = Array(
					SQ_PERMISSION_READ  => translate('Read'),
					SQ_PERMISSION_WRITE => translate('Write'),
					SQ_PERMISSION_ADMIN => translate('Admin'),
				 );
		$conf_perm = $asset->attr('list_permission');
		$conf_eff  = $asset->attr('list_effective');

		if ($write_access) {
			combo_box($prefix.'_permissions[list]', $perm_options, FALSE, $conf_perm);

			echo '&nbsp;';

			// checkbox for effective permissions
			check_box($prefix.'_permissions[effective]', 1, $conf_eff);
			label(translate('Include effective permissions'), $prefix.'_permissions[effective]');

		} else {
				// display effective permission status
				if ($conf_eff) {
					echo sprintf(translate('Effective %1$s'), $perm_options[$conf_perm]);

				} else {
					echo $perm_options[$conf_perm];
				}
		}
		return $asset->writeAccess('attributes');

	}//end paintPermissions()


	/**
	* Processes the interface for selecting searched permissions
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processPermissions(&$asset, &$o, $prefix)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('attributes')) return FALSE;
		if (!isset($_POST[$prefix.'_permissions'])) return FALSE;

		$permission = array_get_index($_POST[$prefix.'_permissions'], 'list');
		$effective  = array_get_index($_POST[$prefix.'_permissions'], 'effective', 0);

		if (is_null($permission)) return FALSE;

		$asset->setAttrValue('list_permission', $permission);
		$asset->setAttrValue('list_effective', $effective);

		return TRUE;

	}//end processPermissions()


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
				echo '<p class="sq-backend-warning">'.translate('No asset types have been selected').'</p>';

			} else {
				$type_names = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo(array_keys($types), 'name');
				?>
					<table class="sq-backend-table">
						<tr>
							<th><?php echo translate('Type') ?></th>

							<th><?php echo translate('Inherit?') ?></th>

						</tr>
				<?php
				$inherit_image_path = sq_web_path('lib').'/web/images/';
				foreach ($types as $type => $inherit) {
					$inherit_image = $inherit_image_path.($inherit ? 'tick' : 'cross').'.png';
					$inherit_alt = $inherit ? translate('Yes') : translate('No');


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
					<?php echo translate('Name') ?>:

				</td>
				<td colspan="2" class="">
					<?php
						if ($write_access) {
							text_box($prefix.'_structured_name', $options['name']);
						} else {
							echo $options['name'];
						}
					?>
					<br />&nbsp;&nbsp;&nbsp;*<?php echo translate('This must be selected as a dynamic parameter above'); ?>

				</td>
			</tr>
			<tr>
				<td class="sq-backend-table-header">
					<b><?php echo translate('Root Node') ?>:</b>

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
									echo translate('No root node has been selected');

								}
							} else {
								echo translate('No root node has been selected');

							}
						}
					?>
				</td>
			</tr>
			<tr>
				<td class="sq-backend-table-header">
					<b><?php echo translate('Depth') ?></b>

				</td>

				<td>
				<?php
					if ($write_access) {
						int_text_box($prefix.'_structured_max_depth', $options['max_depth'], FALSE, 4, 0);
					} else {
						echo ($options['max_depth'] == '0' ? translate('Infinite').' '.translate('Depth') : $options['max_depth']);


					}
				?>
				</td>
			</tr>
			<tr>
				<td class="sq-backend-table-header">
					<b><?php echo translate('Asset Types') ?>:</b>

				</td>
				<td colspan="2" class="">
					<?php
						$this->_paintStructuredTypes($asset, $o, $prefix, $options);
					?>
				</td>
			</tr>
			<tr>
				<td class="sq-backend-table-header">
					<b><?php echo translate('Style') ?></b>

				</td>

				<td >
				<?php if ($write_access) {
					?>
					<select name="<?php echo $prefix.'_structured_input_style'; ?>">
						<option value="0" <?php echo $asset->attr('check_boxes')== 0 ? 'selected="selected"' : ''; ?> ><?php echo translate('Drop-down List'); ?></option>

						<option value="1" <?php echo $asset->attr('check_boxes')== 1 ? 'selected="selected"' : ''; ?> ><?php echo translate('Radio Buttons'); ?></option>

						<option value="2" <?php echo $asset->attr('check_boxes')== 2 ? 'selected="selected"' : ''; ?> ><?php echo translate('Check Boxes'); ?></option>

					</select>
				<?php } else {
						echo $asset->attr('check_boxes') == 0 ? translate('Drop-down List') : $asset->attr('check_boxes') == 1 ? translate('Radio Buttons') : translate('Check Boxes');
					  }

				?>
				</td>

			</tr>
			<tr>
				<td class="sq-backend-table-header">
					<b><?php echo translate('Width') ?></b>

				</td>
				<td >
				<?php
					if ($write_access) {
						int_text_box($prefix.'_structured_width', $options['width'], FALSE, 4, 0);
					} else {
						echo $options['width'] == 0 ? translate('Fit to contents') : $options['width'];

					}
				?>
				</td>
			</tr>
			<tr>
				<td class="sq-backend-table-header">
					<b><?php echo translate('Height') ?></b>

				</td>
				<td >
				<?php
					if ($write_access) {
						int_text_box($prefix.'_structured_height', $options['height'], FALSE, 4, 0);
					} else {
						echo $options['height'] == 0 ? translate('Not Specified') : $options['height'];

					}
				?>
				</td>
			</tr>
			<tr>
				<td class="sq-backend-table-header">
					<b><?php echo translate('Separator') ?></b>

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
						'1'	=> translate('Yes'),

						'0'	=> translate('No'),

					   );

			combo_box($prefix.'_use_bodycopy', $options, FALSE, Array($enabled));

		} else {
			if ($enabled) {
				echo translate('Yes').' - '.get_asset_tag_line($bodycopy_link['minorid'], 'contents');

			} else {
				echo translate('No');

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


	/**
	* Paints the list of current proxy asset types (when one of these
	* asset returns a list of result, the children are listed rather than the
	* proxy itself)
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return void
	* @access public
	*/
	function paintProxyTypes(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$current_types = $asset->attr('proxy_types');

		$am = $GLOBALS['SQ_SYSTEM']->am;

		$o->openField(translate('Proxy Asset Types'));

			if (empty($current_types)) {
				echo translate('There are no current types defined');

			} else {
				?>
					<table class="sq-backend-table">
						<tr>
							<td class="sq-backend-table-header" style="width: 200px;"><?php echo translate('Proxy Type Code') ?></td>

							<td class="sq-backend-table-header"><?php echo translate('Child Type Codes') ?></td>

							<?php
								if ($write_access) {
									?><td class="sq-backend-table-header" style="width: 20px;"><?php echo translate('Delete') ?></td><?php

								}
							?>

						</tr>
						<?php
							foreach ($current_types as $current_type => $type_settings) {
								?>
								<tr>
									<td class="sq_backend-data"><?php
										echo get_asset_type_icon($current_type);
										echo $am->getTypeInfo($current_type, 'name');
									?></td>
									<td class="sq_backend-data"><?php
										if ($write_access) {
											$i = 0;
											$options = Array();
											foreach ($type_settings as $type_code => $inherit) {
												$options['type_code'][$i] = $type_code;
												$options['inherit'][$i] = $inherit;
												$i++;
											}
											asset_type_chooser($prefix.'_'.$current_type.'_children', TRUE, $options, NULL, TRUE, TRUE);
										} else {
											$type_names = $am->getTypeInfo(array_keys($type_settings), 'name');
											?><ul style="margin: 1px 15px"><?php
											foreach ($type_settings as $type_code => $inherits) {
												echo '<li>';
												echo get_asset_type_icon($type_code);
												echo $type_names[$type_code];
												if ($inherits) {
													echo ' ('.strtolower(translate('Inherits Child Types')).')';

												}
												echo '</li>';
											}
											?></ul><?php
										}
									?></td>
									<?php
										if ($write_access) {
											?><td class="sq_backend-data"><?php check_box($prefix.'_'.$current_type.'_delete'); ?></td><?php
										}
									?>
								<tr>
								<?php
							}//end foreach
						?>
					</table>
				<?php
			}//end else
		$o->closeField();

		$search_types = array_keys($asset->attr('types'));
		if ($write_access && !empty($search_types)) {
			$o->openField(translate('New Proxy Asset Type'));

				$types = array_diff($search_types, array_keys($current_types));

				if (empty($types)) {
					echo translate('All available type codes have been selected as proxies');

				} else {
					echo '&nbsp;';
					$options = Array('' => '');
					$options += $am->getTypeInfo($types, 'name');
					combo_box($prefix.'_new_type', $options, FALSE, count($types));
				}
			$o->closeField();
		}

		return $write_access;

	}//end paintProxyTypes()


	/**
	* Processes the list of current proxy asset types
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return void
	* @access public
	*/
	function processProxyTypes(&$asset, &$o, $prefix)
	{
		$current_types = $asset->attr('proxy_types');

		foreach ($current_types as $current_type => $type_settings) {
			// First check to see if any have been deleted
			if (isset($_REQUEST[$prefix.'_'.$current_type.'_delete'])) {
				unset($current_types[$current_type]);
				continue;
			}

			// Set the new types or set to an empty array if they've all been cleared
			if (!empty($_REQUEST[$prefix.'_'.$current_type.'_children'])) {
				$children = $_REQUEST[$prefix.'_'.$current_type.'_children'];

				$current_types[$current_type] = Array();
				foreach ($children['type_code'] as $index => $type_code) {
					if (empty($type_code)) continue;
					$current_types[$current_type][$type_code] = (empty($children['inherit'][$index])) ? FALSE : TRUE;
				}

			} else {
				$current_types[$current_type]['children'] = Array();
			}
		}

		if (!empty($_REQUEST[$prefix.'_new_type'])) {
			$current_types[$_REQUEST[$prefix.'_new_type']] = Array();
		}

		return $asset->setAttrValue('proxy_types', $current_types);

	}//end processProxyTypes()


	/**
	* Paint the interface for selecting assets to be excluded
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintAssetExclusion(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		$exclude_assets = $asset->attr('exclude_assets');

		if ($write_access) {
			require_once SQ_LIB_PATH.'/html_form/html_form.inc';

			multiple_asset_finder($prefix.'_exclude_asset', $exclude_assets);
		} else {
				echo '<ul>';
				foreach ($exclude_assets as $index => $assetid) {
						echo '<li>'.get_asset_tag_line($assetid).'</li>';
				}
				echo '</ul>';
		}

		return $write_access;

	}//end paintAssetExclusion()




	/**
	* Processes the interface for selecting assets to be excluded
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processAssetExclusion(&$asset, &$o, $prefix)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('attributes')) return FALSE;
		if (empty($_POST[$prefix.'_exclude_asset'])) return FALSE;

		$exclude_assets = $_POST[$prefix.'_exclude_asset'];
		$save_exclude_assets = Array();
		foreach ($exclude_assets as $index => $asset_info) {
			if (!empty($asset_info['assetid'])) {
				if (!is_numeric($asset_info['assetid']) || count($GLOBALS['SQ_SYSTEM']->am->getAssetInfo($asset_info['assetid'])) === 0) {
					trigger_error(sprintf(translate('Failed adding Exculded Assets root node. This #%1$s asset id is not a valid asset id.'), $asset_info['assetid']), E_USER_WARNING);
				} else {
					$save_exclude_assets[] = $asset_info['assetid'];
				}
			}//end if
		}//end foreach

		return $asset->setAttrValue('exclude_assets', $save_exclude_assets);

	}//end processAssetExclusion()




	/**
	* Paint the interface for selecting root nodes to exclude. All children of root nodes will be excluded
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintRootNodeExclusion(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		$exclude_assets = $asset->attr('exclude_root_nodes');

		if ($write_access) {
			require_once SQ_LIB_PATH.'/html_form/html_form.inc';

			multiple_asset_finder($prefix.'_exclude_root_node', $exclude_assets);
		} else {
				echo '<ul>';
				foreach ($exclude_assets as $index => $assetid) {
						echo '<li>'.get_asset_tag_line($assetid).'</li>';
				}
				echo '</ul>';
		}

		return $write_access;

	}//end paintRootNodeExclusion()


	/**
	* Processes the interface for selecting root nodes to exclude. All children of root nodes will be excluded
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processRootNodeExclusion(&$asset, &$o, $prefix)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('attributes')) return FALSE;
		if (empty($_POST[$prefix.'_exclude_root_node'])) return FALSE;

		$exclude_assets = $_POST[$prefix.'_exclude_root_node'];
		$save_exclude_assets = Array();
		foreach ($exclude_assets as $index => $asset_info) {
			if (!empty($asset_info['assetid'])) {
				if (!is_numeric($asset_info['assetid']) || count($GLOBALS['SQ_SYSTEM']->am->getAssetInfo($asset_info['assetid'])) === 0) {
					trigger_error(sprintf(translate('Failed adding Exculde Root Node. This #%1$s asset id is not a valid asset id.'), $asset_info['assetid']), E_USER_WARNING);
				} else {
					$save_exclude_assets[] = $asset_info['assetid'];
				}
			}//end if
		}//end foreach

		return $asset->setAttrValue('exclude_root_nodes', $save_exclude_assets);

	}//end processRootNodeExclusion()



}//end class


?>
