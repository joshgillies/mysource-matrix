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
* $Id: search_page_edit_fns.inc,v 1.86 2012/08/30 00:59:15 ewang Exp $
*
*/

require_once SQ_PACKAGES_PATH.'/cms/listing_engine/listing_engine_edit_fns.inc';

/**
* Search_Page_Edit_Fns
*
*     Edit functions for search page
*
* @author  Dominic Wong <dwong@squiz.net>
* @version $Revision: 1.86 $
* @package MySource_Matrix_Packages
* @subpackage search
*/
class Search_Page_Edit_Fns extends Listing_Engine_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();
		$this->static_screens['details']['lock_type'] = 'content';
		$this->static_screens['details']['force_unlock'] = FALSE;

	}//end constructor


	/**
	* Paints the root node selection box
	*
	* @param object		&$asset		asset being painted
	* @param object		&$o			backend outputter
	* @param string		$prefix		prefix for the html doc element name
	* @param string		$category	the category we are painting for
	*								(if blank we paint for the whole search page)
	* @param boolean	$read_only	force read only view
	*
	* @return boolean
	* @access public
	*/
	function paintRootAsset(&$asset, &$o, $prefix, $category='', $read_only=FALSE)
	{
		$link_value = 'root';
		if (!empty($category)) $link_value .= '_'.$category;
		$root_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, 'asset', FALSE, $link_value);

		if ($read_only) {
			$write_access = FALSE;
		} else {
			$write_access = $asset->writeAccess('links');
		}

		if (!$write_access) {
			$root_asset = NULL;
			if (!empty($root_link)) {
				$root_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($root_link['minorid']);
			}
			if (!is_null($root_asset)) {
				echo $root_asset->name.' (Id: #'.$root_asset->id.')';
			} else {
				echo translate('No root node has been selected');

			}
		} else {
			asset_finder($prefix.'_rootid', (!empty($root_link)) ? $root_link['minorid'] : '');
		}

		return $write_access;

	}//end paintRootAsset()


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
	function processRootAsset(&$asset, &$o, $prefix)
	{
		if (!isset($_POST[$prefix.'_rootid']['assetid'])) {
			return FALSE;
		}
		$new_root  = $_POST[$prefix.'_rootid']['assetid'];

		// getting existing root link
		$link_value = 'root';
		$root_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, '', FALSE, $link_value);

		// dont process anything if the root node has not changed
		if (!empty($root_link) && $root_link['minorid'] == $new_root) {
			return FALSE;
		}

		$ok = TRUE;

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// deleting old root link if any found...
		if (!empty($root_link) && !$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($root_link['linkid'])) {
			$ok = FALSE;
		} else if ($new_root) {
			$root = $GLOBALS['SQ_SYSTEM']->am->getAsset($new_root);
			if (!is_null($root)) {
				if (!$asset->createLink($root, SQ_LINK_NOTICE, $link_value)) {
					$ok = FALSE;
					trigger_localised_error('SCH0012', sprintf(translate('Failed adding new root node "%1$s" [#%2$s]'), $root->name, $root->id), E_USER_WARNING);
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

	}//end processRootAsset()


	/**
	* Paints the interface for adding and configuring search fields.
	*
	* @param object	&$asset	the search page asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function paintFields(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('all');
		$fields = $asset->attr('fields');

		$query_types = Array(
						'none'			=> '-- '.ucfirst(translate('Add Data Source').' --'),

						'include_all'	=> ucfirst(translate('Include All')),

						'standard'		=> ucfirst(translate('Standard Asset Field')),

						'asset_attrib'	=> ucfirst(translate('Asset Attribute')),

						'metadata'		=> ucfirst(translate('Asset Metadata Field')),

					   );

		?>
		<table class ="sq-backend-table" style="border-bottom: none;">
			<tr>
				<th rowspan="2"><?php echo translate('Name'); ?></th>

				<th colspan="<?php echo $write_access ? 3 : 2; ?>"><?php echo translate('Data Sources'); ?></th>

				<th rowspan="2" style="text-align:center;"><?php echo translate('Word Logic'); ?></th>

				<?php
				if ($write_access) {
					?>
					<th rowspan="2" style="text-align:center;"><?php echo translate('Delete'); ?>?</th>

					<?php
				}
				?>
			</tr>
			<tr>
				<th><?php echo translate('Type');?></th>

				<th class="force-right-border" <?php if (!$write_access) echo 'colspan="1"';?>><?php echo translate('Details');?> </th>

				<?php
				if ($write_access) {
					?>
					<th class="force-right-border" style="text-align:center;"><?php echo translate('Delete'); ?>?</th>

					<?php
				}
				?>
			</tr>
			<?php

			foreach ($fields as $field_name => $field_data) {
				if ($field_data['exclude_field']) continue;

				$field_datasources = $field_data['data_sources'];
				if ((!empty($field_datasources)) && ($field_datasources[0]['type'] == 'exclude')) {
					continue;
				}
				if ($write_access) $field_datasources[] = 'new';
				foreach ($field_datasources as $index => $field_datasource) {
					?>
					<tr class="force-bottom-border">
						<?php
						if ($index == 0) {
							?>
							<td rowspan="<?php echo count($field_datasources);?>"><strong><?php echo $field_data['name'] ?></strong></td>
							<?php
						}
						?>
						<td class="sq-backend-data <?php if ($field_datasource == 'new') echo 'no-right-border';?>">
							<?php
							if ($field_datasource == 'new') {
								if ($write_access) {
									combo_box($prefix.'_fields['.$field_name.'][new_data_source_type]', $query_types, FALSE,'none');
								}
							} else {
								$type = array_get_index($field_datasource, 'type');
								echo $query_types[$type];
							}
							?>
						</td>
						<td class="<?php echo $field_datasource == 'new' ? 'no-right-border' : 'force-right-border';?>">
							<?php
							if ($field_datasource != 'new') {
								switch ($type) {

									case 'asset_attrib':
										if (!isset($field_datasource['params'])) {
											$field_datasources[$index]['params'] = Array();
										}
										$search_types = $asset->attr('types');
										$search_type = array_get_index($field_datasources[$index]['params'], 'asset_type', '0');
										$type_names = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo(array_keys($search_types), 'name');
										if ($write_access) {
											$options = Array('' => '-- '.translate('Select Asset Type').' --');

											foreach (array_keys($search_types) as $this_type) {
												$options[$this_type] = $type_names[$this_type];
											}
											combo_box($prefix.'_fields['.$field_name.'][data_sources]['.$index.'][params][asset_type]', $options, FALSE, $search_type);
										} else {
											echo '<strong>'.translate('Asset Type').':</strong> ';


											// check to see if any of the assets have been selected
											// else dont bother about printing its details
											if ($search_type !== '0') {
												echo sq_print_icon($GLOBALS['SQ_SYSTEM']->am->getAssetIconURL($search_type), 16, 16, $type_names[$search_type], $type_names[$search_type], ' align="absmiddle"');
												echo $type_names[$search_type];
											} else {
												echo translate('No asset selected');

											}
										}
										// printing out attribute selection combo box
										if (!isset($field_datasource['params']['attrid']) || empty($search_type)) {
											$selected_attr = '';
										} else {
											$selected_attr = $field_datasource['params']['attrid'];
										}
										if (!empty($search_type)) {
											$asset_type_attrs = $GLOBALS['SQ_SYSTEM']->am->getAssetTypeAttributes($search_type, Array('attrid', 'name'));
											if ($write_access) {
												$type_attrs = Array('' => '-- '.translate('Select Attribute').' --');

												foreach ($asset_type_attrs as $attr_id => $attr_name) {
													$type_attrs[$attr_id] = $attr_name['name'];
												}
												combo_box($prefix.'_fields['.$field_name.'][data_sources]['.$index.'][params][attrid]', $type_attrs, FALSE, $selected_attr);
											} else if ($selected_attr != '') {
												echo '<br /><strong>'.translate('Attribute').':</strong>  '.$asset_type_attrs[$selected_attr]['name'];

											}
										}

										if (!empty($selected_attr)) {
											$search_style_options = Array(
												'exact'  => translate('Provide a single text box for exact numeric searching'),
												'range'  => translate('Provide two text boxes (upper and lower bound) for range searching'),
												'custom' => translate('Provide a drop-down list of named ranges'),
											);
											$attr_info = $GLOBALS['SQ_SYSTEM']->am->getAttributeInfo(Array($selected_attr));
											$attr_info = $attr_info[$selected_attr];

											if (($attr_info['type'] == 'int') || ($attr_info['type'] == 'float')) {
												$current_search = array_get_index($field_datasource['params'], 'numeric_search', 'exact');
												?><br/><?php
												if ($write_access) {
													radio_button($prefix.'_fields['.$field_name.'][data_sources]['.$index.'][params][numeric_search]', 'exact', ($current_search == 'exact'));
													echo $search_style_options['exact'];

													?><br/><?php

													radio_button($prefix.'_fields['.$field_name.'][data_sources]['.$index.'][params][numeric_search]', 'range', ($current_search == 'range'));
													echo $search_style_options['range'];

													?><br/><?php

													radio_button($prefix.'_fields['.$field_name.'][data_sources]['.$index.'][params][numeric_search]', 'custom', ($current_search == 'custom'));
													echo $search_style_options['custom'];

													?><br/><?php
												} else {
													?><strong><?php echo translate('Search Style:'); ?></strong> <?php

													echo $search_style_options[$current_search];
												}

												if ($current_search == 'custom') {
													$custom_ranges = array_get_index($field_datasource['params'], 'custom_num_search', Array());
													$custom_ranges[] = Array();
													?><table class="sq-backend-table" style="width:90%">
													<thead>
													<tr>
														<th class="sq-backend-table-header">Range Name</th>
														<th class="sq-backend-table-header">Minimum</th>
														<th class="sq-backend-table-header">Maximum</th>
													</tr>
													</thead>
													<tbody>
													<?php
													foreach ($custom_ranges as $key => $custom_range) {
														if ($key != max(array_keys($custom_ranges)) && (empty($custom_range) || (empty($custom_range['name']) && empty($custom_range['min']) && empty($custom_range['max'])))) {
															continue;
														}

														?><tr><td class="sq-backend-table-cell"><?php
														if ($write_access) {
															echo text_box($prefix.'_fields['.$field_name.'][data_sources]['.$index.'][params][custom_num_search]['.$key.'][name]', array_get_index($custom_range, 'name', ''), 20);
														} else {
															echo array_get_index($custom_range, 'name', '');
														}
														?></td><td class="sq-backend-table-cell"><?php
														if ($write_access) {
															echo text_box($prefix.'_fields['.$field_name.'][data_sources]['.$index.'][params][custom_num_search]['.$key.'][min]', array_get_index($custom_range, 'min', ''), 7);
														} else {
															echo array_get_index($custom_range, 'min', '');
														}
														?></td><td class="sq-backend-table-cell"><?php
														if ($write_access) {
															echo text_box($prefix.'_fields['.$field_name.'][data_sources]['.$index.'][params][custom_num_search]['.$key.'][max]', array_get_index($custom_range, 'max', ''), 7);
														} else {
															echo array_get_index($custom_range, 'max', '');
														}
														?></td></tr><?php
													}
													?></tbody></table><?php
												}//end if

											}//end if
										}//end if
										?>
										<?php
									break;

									case 'metadata':
										if (!isset($field_datasource['params'])) {
											$field_datasource['params'] = Array();
										}
										$metadata_field = array_get_index($field_datasource['params'], 'assetid', '0');
										if ($write_access) {
											asset_finder($prefix.'_fields['.$field_name.'][data_sources]['.$index.'][params][metadata_field]', $metadata_field, Array('metadata_field' => 'D'));
										} else {
											if ($metadata_field > 0) {
												$field_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($metadata_field);
												if (!is_null($metadata_field)) {
													echo get_asset_tag_line($field_asset->id);
												} else {
													echo translate('No metadata field specified');

												}
											} else {
												echo translate('No metadata field specified');

											}
										}
									break;

									case 'standard':
										if (!isset($field_datasource['params'])) {
											$field_datasources[$index]['params'] = Array();
										}
										if (isset($field_datasource['params']['field'])) {
											$selected_attr_name = $field_datasource['params']['field'];
										} else {
											$selected_attr_name = '';
										}
										if ($write_access) {
											$type_attrs = Array('' => '-- '.translate('Select Attribute').' --');

											$sm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('search_manager');
											$attributes = $sm->standard_date_fields + $sm->standard_text_fields;
											combo_box($prefix.'_fields['.$field_name.'][data_sources]['.$index.'][params][field]', $attributes, FALSE, $selected_attr_name);
										} else if ($selected_attr_name != '') {
											echo translate('Field').':  '.$selected_attr_name;

										}
									break;

									case 'include_all':
										echo '&nbsp;';
										hidden_field($prefix.'_fields['.$field_name.'][data_sources]['.$index.'][params]', '');
									break;

								}//end switch
							}//end if
							?>
						</td>
						<?php
						if ($write_access) {
							// Delete Data Source Cell
							?>
							<td class="sq-backend-data force-right-border" style="text-align:center;">
								<?php
								if ($field_datasource != 'new') {
									check_box($prefix.'_fields['.$field_name.'][data_sources]['.$index.'][delete]');
								}
								?>
							</td>
							<?php
						}
						if ($index == 0) {
							// Word Logic Cell
							?>
							<td rowspan="<?php echo count($field_datasources);?>" style="text-align:center;">
								<?php
								$word_logic_options = Array('AND' => translate('All Words'), 'OR' => translate('Any Word'));


								if ($write_access) {
									combo_box($prefix.'_fields['.$field_name.'][word_logic]', $word_logic_options, FALSE, array_get_index($field_data, 'word_logic', 'AND'), 0, 'title="'.translate('Find assets with ALL or ANY of the entered words?').'"');

								} else {
									echo array_get_index($field_data, 'word_logic', 'AND');
								}
								?>
							</td>
							<?php
						}

						if (($index == 0) && $write_access) {
							// Delete field overall cell
							?>
							<td rowspan="<?php echo count($field_datasources);?>" style="text-align:center;">
								<?php
								check_box($prefix.'_fields['.$field_name.'][delete]');
								?>
							</td>
							<?php
						}
						?>
						<?php
					}//end foreach
				}//end foreach
				?>
			</tr>

		</table>
		<?php
		if ($write_access) {
			label(translate('New Search Field Name').':', $prefix.'_new_field_name');

			echo '&nbsp;';
			text_box($prefix.'_new_field_name', '', 20);
		}
		return TRUE;

	}//end paintFields()


	/**
	* Processes the interface for adding and configuring search fields.
	*
	* @param object	&$asset	the search page asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processFields(&$asset, &$o, $prefix)
	{
		$old_fields = $asset->attr('fields');

		$new_fields = Array();

		if (isset($_POST[$prefix.'_fields'])) {
			foreach ($_POST[$prefix.'_fields'] as $field_name => $field) {
				if (!isset($field['delete'])) {
					$new_fields[$field_name]['name'] = $old_fields[$field_name]['name'];
					$new_fields[$field_name]['word_logic'] = $field['word_logic'];
					$field_datasources = array_get_index($field, 'data_sources', Array());
					$new_fields[$field_name]['data_sources'] = Array();
					$new_fields[$field_name]['exclude_field'] = FALSE;
					foreach ($field_datasources as $i => $field_datasource) {
						if (!isset($field_datasource['delete'])) {
							$new_datasource = Array();
							$new_datasource['type'] = $old_fields[$field_name]['data_sources'][$i]['type'];
							if ($new_datasource['type'] != 'metadata') {
								$new_datasource['params'] = $field_datasource['params'];
							} else {
								$new_datasource['params']['assetid'] = $field_datasource['params']['metadata_field']['assetid'];
							}
							$new_fields[$field_name]['data_sources'][] = $new_datasource;
						}
					}
					if (!empty($field['new_data_source_type']) && $field['new_data_source_type'] != 'none') {
						$new_datasource = Array();
						$new_datasource['type'] = $field['new_data_source_type'];
						$new_datasource['params'] = Array();
						$new_fields[$field_name]['data_sources'][] = $new_datasource;
					}
				}
			}
		}

		if (!empty($_POST[$prefix.'_new_field_name'])) {
			$safe_name = strtolower(str_replace(' ', '_', $_POST[$prefix.'_new_field_name']));

			// NOTE: '.' chars are allowed for search field name, however they are treated as '_' for all other purposes
			// So ensure we dont have duplicate field names like 'details.name' and 'details_name', as both will refer to same name
			$check_name_wd = str_replace('.', '_', $safe_name);
			$check_name_wu = str_replace('_', '.', $safe_name);

			// If field already, don't overwrite it
			if (!isset($old_fields[$check_name_wd]) && !isset($old_fields[$check_name_wu])) {
				$new_fields[$safe_name]['name'] = $_POST[$prefix.'_new_field_name'];				
				$new_fields[$safe_name]['data_sources'] = Array();
				$new_fields[$safe_name]['exclude_field'] = FALSE;
			}
		}

		foreach ($old_fields as $field_name => $field) {
			if ($field['exclude_field']) {
				$new_fields[$field_name] = $field;
			}
		}//end foreach

		return $asset->setAttrValue('fields', $new_fields);

	}//end processFields()


	/**
	* Paints the interface for adding and configuring Exlude fields.
	*
	* @param object	&$asset	the search page asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function paintExcludeFields(&$asset, &$o, $prefix)	{

		$write_access = $asset->writeAccess('all');
		$fields = $asset->attr('fields');
		$query_types = Array(
						'none'			=> '-- '.ucfirst(translate('Add Data Source').' --'),

						'include_all'	=> ucfirst(translate('Include All')),

						'standard'		=> ucfirst(translate('Standard Asset Field')),

						'asset_attrib'	=> ucfirst(translate('Asset Attribute')),

						'metadata'		=> ucfirst(translate('Asset Metadata Field')),

					   );

		?>
		<table class ="sq-backend-table" style="border-bottom: none;">
			<tr>
				<th rowspan="2"><?php echo translate('Name'); ?></th>

				<th colspan="<?php echo $write_access ? 3 : 2; ?>"><?php echo translate('Data Sources'); ?></th>

				<th style="text-align: center;" rowspan="2"><?php echo translate('Word Logic'); ?></th>

				<?php
				if ($write_access) {
					?>
					<th style="text-align: center;" rowspan="2"><?php echo translate('Delete'); ?>?</th>

					<?php
				}
				?>
			</tr>
			<tr class="force-bottom-border">
				<th><?php echo translate('Type');?></th>

				<th class="force-right-border" <?php if (!$write_access) echo 'colspan="1"';?>><?php echo translate('Details');?></th>

				<?php
				if ($write_access) {
					?>
					<th class="force-right-border" style="text-align: center;"><?php echo translate('Delete'); ?>?</th>

					<?php
				}
				?>
			</tr>
			<?php
			foreach ($fields as $field_name => $field_data) {
				if (!$field_data['exclude_field']) continue;
				$field_datasources = $field_data['data_sources'];
				if ((!empty($field_datasources)) && ($field_datasources[0]['type'] == 'exclude')) {
					continue;
				}
				if ($write_access) $field_datasources[] = 'new';
				foreach ($field_datasources as $index => $field_datasource) {
					?>
					<tr class="force-bottom-border">
						<?php
						if ($index == 0) {
							?>
							<td rowspan="<?php echo count($field_datasources);?>"><strong><?php echo $field_data['name'] ?></strong></td>
							<?php
						}
						?>
						<td class="sq-backend-data <?php if ($field_datasource == 'new') echo 'no-right-border';?>">
							<?php
							if ($field_datasource == 'new') {
								if ($write_access) {
									combo_box($prefix.'_exclude_fields['.$field_name.'][new_data_source_type]', $query_types, FALSE,'none');
								}
							} else {
								$type = array_get_index($field_datasource, 'type');
								echo $query_types[$type];
							}
							?>
						</td>
						<td class="<?php echo $field_datasource == 'new' ? 'no-right-border' : 'force-right-border';?>">
							<?php
							if ($field_datasource != 'new') {
								switch ($type) {

									case 'asset_attrib':
										if (!isset($field_datasource['params'])) {
											$field_datasources[$index]['params'] = Array();
										}
										$search_types = $asset->attr('types');
										$search_type = array_get_index($field_datasources[$index]['params'], 'asset_type', '0');
										$type_names = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo(array_keys($search_types), 'name');
										if ($write_access) {
											$options = Array('' => '-- '.translate('Select Asset Type').' --');

											foreach (array_keys($search_types) as $this_type) {
												$options[$this_type] = $type_names[$this_type];
											}
											combo_box($prefix.'_exclude_fields['.$field_name.'][data_sources]['.$index.'][params][asset_type]', $options, FALSE, $search_type);
										} else {
											echo '<strong>'.translate('Asset Type').':</strong> ';

											echo sq_print_icon($GLOBALS['SQ_SYSTEM']->am->getAssetIconURL($search_type), 16, 16, $type_names[$search_type], $type_names[$search_type], ' align="absmiddle"');
											echo $type_names[$search_type];
										}
										// printing out attribute selection combo box
										if (!isset($field_datasource['params']['attrid']) || empty($search_type)) {
											$selected_attr = '';
										} else {
											$selected_attr = $field_datasource['params']['attrid'];
										}
										if (!empty($search_type)) {
											$asset_type_attrs = $GLOBALS['SQ_SYSTEM']->am->getAssetTypeAttributes($search_type, Array('attrid', 'name'));
											if ($write_access) {
												$type_attrs = Array('' => '-- '.translate('Select Attribute').' --');

												foreach ($asset_type_attrs as $attr_id => $attr_name) {
													$type_attrs[$attr_id] = $attr_name['name'];
												}
												combo_box($prefix.'_exclude_fields['.$field_name.'][data_sources]['.$index.'][params][attrid]', $type_attrs, FALSE, $selected_attr);
											} else if ($selected_attr != '') {
												echo '<br /><strong>'.translate('Attribute').':</strong>  '.$asset_type_attrs[$selected_attr]['name'];

											}
										}

										if (!empty($selected_attr)) {
											$search_style_options = Array(
												'exact'  => translate('Provide a single text box for exact numeric searching'),
												'range'  => translate('Provide two text boxes (upper and lower bound) for range searching'),
												'custom' => translate('Provide a drop-down list of named ranges'),
											);
											$attr_info = $GLOBALS['SQ_SYSTEM']->am->getAttributeInfo(Array($selected_attr));
											$attr_info = $attr_info[$selected_attr];

											if (($attr_info['type'] == 'int') || ($attr_info['type'] == 'float')) {
												$current_search = array_get_index($field_datasource['params'], 'numeric_search', 'exact');
												?><br/><?php
												if ($write_access) {
													radio_button($prefix.'_exclude_fields['.$field_name.'][data_sources]['.$index.'][params][numeric_search]', 'exact', ($current_search == 'exact'));
													echo $search_style_options['exact'];

													?><br/><?php

													radio_button($prefix.'_exclude_fields['.$field_name.'][data_sources]['.$index.'][params][numeric_search]', 'range', ($current_search == 'range'));
													echo $search_style_options['range'];

													?><br/><?php

													radio_button($prefix.'_exclude_fields['.$field_name.'][data_sources]['.$index.'][params][numeric_search]', 'custom', ($current_search == 'custom'));
													echo $search_style_options['custom'];

													?><br/><?php
												} else {
													?><strong><?php echo translate('Search Style:'); ?></strong> <?php

													echo $search_style_options[$current_search];
												}

												if ($current_search == 'custom') {
													$custom_ranges = array_get_index($field_datasource['params'], 'custom_num_search', Array());
													$custom_ranges[] = Array();
													?><table class="sq-backend-table" style="width:90%">
													<thead>
													<tr>
														<th class="sq-backend-table-header">Range Name</th>
														<th class="sq-backend-table-header">Minimum</th>
														<th class="sq-backend-table-header">Maximum</th>
													</tr>
													</thead>
													<tbody>
													<?php
													foreach ($custom_ranges as $key => $custom_range) {
														if ($key != max(array_keys($custom_ranges)) && (empty($custom_range) || (empty($custom_range['name']) && empty($custom_range['min']) && empty($custom_range['max'])))) {
															continue;
														}

														?><tr><td class="sq-backend-table-cell"><?php
														if ($write_access) {
															echo text_box($prefix.'_exclude_fields['.$field_name.'][data_sources]['.$index.'][params][custom_num_search]['.$key.'][name]', array_get_index($custom_range, 'name', ''), 20);
														} else {
															echo array_get_index($custom_range, 'name', '');
														}
														?></td><td class="sq-backend-table-cell"><?php
														if ($write_access) {
															echo text_box($prefix.'_exclude_fields['.$field_name.'][data_sources]['.$index.'][params][custom_num_search]['.$key.'][min]', array_get_index($custom_range, 'min', ''), 7);
														} else {
															echo array_get_index($custom_range, 'min', '');
														}
														?></td><td class="sq-backend-table-cell"><?php
														if ($write_access) {
															echo text_box($prefix.'_exclude_fields['.$field_name.'][data_sources]['.$index.'][params][custom_num_search]['.$key.'][max]', array_get_index($custom_range, 'max', ''), 7);
														} else {
															echo array_get_index($custom_range, 'max', '');
														}
														?></td></tr><?php
													}
													?></tbody></table><?php
												}//end if

											}//end if
										}//end if
										?>
										<?php
									break;

									case 'metadata':
										if (!isset($field_datasource['params'])) {
											$field_datasource['params'] = Array();
										}
										$metadata_field = array_get_index($field_datasource['params'], 'assetid', '0');
										if ($write_access) {
											asset_finder($prefix.'_exclude_fields['.$field_name.'][data_sources]['.$index.'][params][metadata_field]', $metadata_field, Array('metadata_field' => 'D'));
										} else {
											if ($metadata_field > 0) {
												$field_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($metadata_field);
												if (!is_null($metadata_field)) {
													echo get_asset_tag_line($field_asset->id);
												} else {
													echo translate('No metadata field specified');

												}
											} else {
												echo translate('No metadata field specified');

											}
										}
									break;

									case 'standard':
										if (!isset($field_datasource['params'])) {
											$field_datasources[$index]['params'] = Array();
										}
										if (isset($field_datasource['params']['field'])) {
											$selected_attr_name = $field_datasource['params']['field'];
										} else {
											$selected_attr_name = '';
										}
										if ($write_access) {
											$type_attrs = Array('' => '-- '.translate('Select Attribute').' --');

											$sm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('search_manager');
											$attributes = $sm->standard_date_fields + $sm->standard_text_fields;
											combo_box($prefix.'_exclude_fields['.$field_name.'][data_sources]['.$index.'][params][field]', $attributes, FALSE, $selected_attr_name);
										} else if ($selected_attr_name != '') {
											echo translate('Field').':  '.$selected_attr_name;

										}
									break;

									case 'include_all':
										echo '&nbsp;';
										hidden_field($prefix.'_exclude_fields['.$field_name.'][data_sources]['.$index.'][params]', '');
									break;

								}//end switch
							}//end if
							?>
						</td>
						<?php
						if ($write_access) {
							// Delete Data Source Cell
							?>
							<td class="sq-backend-data force-right-border" style="text-align: center;">
								<?php
								if ($field_datasource != 'new') {
									check_box($prefix.'_exclude_fields['.$field_name.'][data_sources]['.$index.'][delete]');
								}
								?>
							</td>
							<?php
						}
						if ($index == 0) {
							// Word Logic Cell
							?>
							<td style="text-align: center;" rowspan="<?php echo count($field_datasources);?>">
								<?php
								$word_logic_options = Array('AND' => translate('All Words'), 'OR' => translate('Any Word'));


								if ($write_access) {
									combo_box($prefix.'_exclude_fields['.$field_name.'][word_logic]', $word_logic_options, FALSE, array_get_index($field_data, 'word_logic', 'AND'), 0, 'title="'.translate('Find assets with ALL or ANY of the entered words?').'"');

								} else {
									echo array_get_index($field_data, 'word_logic', 'AND');
								}
								?>
							</td>
							<?php
						}

						if (($index == 0) && $write_access) {
							// Delete field overall cell
							?>
							<td rowspan="<?php echo count($field_datasources);?>" style="text-align: center;">
								<?php
								check_box($prefix.'_exclude_fields['.$field_name.'][delete]');
								?>
							</td>
							<?php
						}
						?>
						<?php
					}//end foreach
				}//end foreach
				?>
			</tr>

		</table>
		<?php
		if ($write_access) {
			label(translate('New Exclude Field Name').':', $prefix.'_new_exclude_field_name');

			echo '&nbsp;';
			text_box($prefix.'_new_exclude_field_name', '', 20);
		}

		return TRUE;

	}//end paintExcludeFields()


	/**
	* Processes the interface for adding and configuring Exclude fields.
	*
	* @param object	&$asset	the search page asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processExcludeFields(&$asset, &$o, $prefix)
	{
		//return true;
		$old_fields = $asset->attr('fields');

		$new_fields = Array();
		if (isset($_POST[$prefix.'_exclude_fields'])) {
			foreach ($_POST[$prefix.'_exclude_fields'] as $field_name => $field) {
				if (!isset($field['delete'])) {
					$new_fields[$field_name]['name'] = $old_fields[$field_name]['name'];
					$new_fields[$field_name]['word_logic'] = $field['word_logic'];
					$field_datasources = array_get_index($field, 'data_sources', Array());
					$new_fields[$field_name]['data_sources'] = Array();
					$new_fields[$field_name]['exclude_field'] = TRUE;
					foreach ($field_datasources as $i => $field_datasource) {
						if (!isset($field_datasource['delete'])) {
							$new_datasource = Array();
							$new_datasource['type'] = $old_fields[$field_name]['data_sources'][$i]['type'];
							if ($new_datasource['type'] != 'metadata') {
								$new_datasource['params'] = $field_datasource['params'];
							} else {
								$new_datasource['params']['assetid'] = $field_datasource['params']['metadata_field']['assetid'];
							}
							$new_fields[$field_name]['data_sources'][] = $new_datasource;
						}
					}
					if (!empty($field['new_data_source_type']) && $field['new_data_source_type'] != 'none') {
						$new_datasource = Array();
						$new_datasource['type'] = $field['new_data_source_type'];
						$new_datasource['params'] = Array();
						$new_fields[$field_name]['data_sources'][] = $new_datasource;
					}
				}
			}
		}



		if (!empty($_POST[$prefix.'_new_exclude_field_name'])) {
			$safe_name = strtolower(str_replace(' ', '_', $_POST[$prefix.'_new_exclude_field_name']));

			// NOTE: '.' chars are allowed for search field name, however they are treated as '_' for all other purposes
			// So ensure we dont have duplicate field names like 'details.name' and 'details_name', as both will refer to same name
			$check_name_wd = str_replace('.', '_', $safe_name);
			$check_name_wu = str_replace('_', '.', $safe_name);

			// If field already, don't overwrite it
			if (!isset($old_fields[$check_name_wd]) && !isset($old_fields[$check_name_wu])) {
				$new_fields[$safe_name]['name'] = $_POST[$prefix.'_new_exclude_field_name'];				
				$new_fields[$safe_name]['data_sources'] = Array();
				$new_fields[$safe_name]['exclude_field'] = TRUE;
			}
		}

		foreach ($old_fields as $field_name => $field) {
			if (!$field['exclude_field']) {
				$new_fields[$field_name] = $field;
			}
		}//end foreach

		return $asset->setAttrValue('fields', $new_fields);

	}//end processExcludeFields()


	/**
	* Paints the interface for selecting search field styles
	*
	* @param object	&$asset	the search page asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function paintStyles(&$asset, &$o, $prefix)
	{

		$write_access = $asset->writeAccess('attributes');
		$search_styles = $asset->attr('search_styles');
		$fields = $asset->attr('fields');
		foreach ($fields as $field) {
			$field_datasources = $field['data_sources'];
			$type = NULL;
			foreach ($field_datasources as $field_datasource) {
				if (isset($field_datasource['type'])) {
					$type = $field_datasource['type'];
				}
				if ($type == 'asset_attrib') {

					if (!isset($field_datasource['params']['attrid'])) {
						continue;
					}
					$attribute = $GLOBALS['SQ_SYSTEM']->am->getAttribute($field_datasource['params']['attrid']);
					if (is_null($attribute)) continue;

					// if this is a boolean or selection field, we want to make sure blank options are allowed
					if ($attribute->type() == 'boolean' || $attribute->type() == 'selection') {
						$attribute->_params['allow_empty'] = TRUE;
					}

					if (!empty($search_styles) && isset($search_styles[$attribute->id])) {
						$attribute->_edit_params = $search_styles[$attribute->id];
					}

					echo '<p>';
					$attribute->paintEditParams($prefix.'_'.$attribute->id, $write_access);
					echo '</p>';

				}//end if asset attribute
			}
		}//end foreach fields

		return $write_access;

	}//end paintStyles()


	/**
	* Processes the interface for changing search page field styles
	*
	* @param object	&$asset	the search page asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processStyles(&$asset, &$o, $prefix)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('attributes')) return FALSE;

		$search_styles = Array();

		$fields = $asset->attr('fields');

		foreach ($fields as $fieldname => $field) {
			$field_datasources = $field['data_sources'];
			$type = NULL;
			foreach ($field_datasources as $field_datasource) {
				if (isset($field_datasource['type'])) {
					$type = $field_datasource['type'];
				}
				if ($type == 'asset_attrib') {

					if (empty($field_datasource['params']['attrid'])) {
						continue;
					}
					$attribute = $GLOBALS['SQ_SYSTEM']->am->getAttribute($field_datasource['params']['attrid']);
					$search_styles[$attribute->id] = $attribute->processEditParams($prefix.'_'.$attribute->id);
				}
			}
		}

		$asset->setAttrValue('search_styles', $search_styles);

		return TRUE;

	}//end processStyles()


//--        CATEGORIES        --//


	/**
	* Paints the interface for managing the search categories
	*
	* @param object	&$asset	the search page asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintCategories(&$asset, &$o, $prefix)
	{
		?>
		<style type="text/css">
			table.category-set {
				margin-bottom: 10px;
			}
			table.category-set th.title {
				font-size: 90%;
			}
			table.category-set th.delete {
				vertical-align: middle;
				width: 15ex;
			}
			table.category-set th.delete input {
				margin: 0px 4px;
			}
			table.category-set table.sq-backend-table {
				margin-left: 15px;
				margin-top: 2px;
			}
			table.category-set table.sq-backend-table th {
				font-weight: normal;
			}
			table.category-set table.sq-backend-table td {
				white-space: nowrap;
			}
		</style>
		<script type="text/javascript"><!--
		//<![CDATA[
			function handleDefaultStatusClick(elt)
			{
				var td = elt.parentNode;
				while (td.tagName != 'TD') {
					td = td.parentNode;
				}
				var selects = td.getElementsByTagName('SELECT');
				with (selects[0].style) {
					display = (display == 'none') ? '' : 'none';
				}
			}

			function handleDefaultRootClick(elt)
			{
				var td = elt.parentNode;
				while (td.tagName != 'TD') {
					td = td.parentNode;
				}
				var divs = td.getElementsByTagName('DIV');
				with (divs[0].style) {
					display = (display == 'none') ? '' : 'none';
				}
			}
		//]]> -->
		</script>
		<?php
		$sets = $asset->attr('categories');
		$wa = $asset->writeAccess('attributes');
		foreach ($sets as $name => $set_data) {
			?>
			<table class="sq-backend-table category-set">
				<tr>
					<th class="title"><?php echo $name; ?></th>
					<th class="delete">
						<?php
						if ($wa) {
							$msg = translate('Are you sure you want to delete this category set?');

							check_box($prefix.'_category_sets_to_delete[]', $name, FALSE, "return confirm('$msg')", 'id="'.$prefix.'_category_sets_to_delete_'.$name.'"');
							label(translate('Delete'), $prefix.'_category_sets_to_delete_'.$name);

						}
						?>
					</th>
				</tr>
				<tr>
					<td colspan="2">
						<?php
						if ($wa) {
							label(translate('Unselected Text').': ', $prefix.'_category_'.$name.'_unselected_text');

							echo '&nbsp;';
							text_box($prefix.'_category_'.$name.'_unselected_text', $set_data['unselected_text']);
						} else {
							echo translate('Unselected Text');

							echo $set_data['unselected_text'];
						}
						?>
					</td>
				</tr>
				<tr>
					<td colspan="2">
						<?php
						echo translate('Options').':';

						$this->_paintCategoryOptions($asset, $prefix.'_category_'.$name.'_options', $name, $set_data['options'], $wa);
						?>
					</td>
				</tr>
			</table>
			<?php
		}//end foreach

	}//end paintCategories()


	/**
	* Paint the interface for managing a category set's category options
	*
	* @param object		&$asset		The asset whose categories are being managed
	* @param string		$prefix		The prefix for form fields
	* @param string		$set_name	The category set whose options are being managed
	* @param array		$options	The current category options
	* @param boolean	$wa			Whether we have write access to the field
	*
	* @return void
	* @access private
	*/
	function _paintCategoryOptions(&$asset, $prefix, $set_name, $options, $wa)
	{
		?>
		<table class="sq-backend-table">
			<tr>
				<th><?php echo translate('Name'); ?></th>

				<th style="width: 35ex"><?php echo translate('Statuses'); ?></th>

				<th style="width: 60ex"><?php echo translate('Root Node'); ?></th>

			<?php
			if ($wa) {
				?>
				<th style="width: 7ex"><?php echo translate('Delete'); ?></th>

				<?php
			}
			?>
			</tr>
		<?php
		foreach ($options as $code_name => $option) {
			?>
			<tr>
				<td>
					<?php
					if ($wa) {
						text_box($prefix.'_'.$code_name.'_full_name', $option['full_name']);
					} else {
						echo $option['full_name'];
					}
					?>
				</td>
				<td>
					<?php
					$customised = isset($option['statuses']);
					$current_stats = array_get_index($option, 'statuses', Array());
					if ($wa) {
						check_box($prefix.'_'.$code_name.'_statuses_default', '1', !$customised, 'handleDefaultStatusClick(this)');
						label('Use Default', $prefix.'_'.$code_name.'_statuses_default');
					} else {
						echo '<img src="'.sq_web_path('lib').'/web/images/'.($customised ? 'cross' : 'tick').'.png" />';
						echo translate('Use default');

					}
					$this->_paintStatusChooser($prefix.'_'.$code_name.'_statuses', $current_stats,  $wa, $customised);
					?>
				</td>
				<td>
					<?php
					$customised = isset($option['root_assetid']);
					$current_root = 0;
					if ($customised) {
						$link_value = 'root_'.$set_name.'_'.$code_name;
						$root_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, '', FALSE, $link_value);
						$current_root = $root_link['minorid'];
					}
					if ($wa) {
						check_box($prefix.'_'.$code_name.'_root_default', '1', !$customised, 'handleDefaultRootClick(this)');
						label('Use Default', $prefix.'_'.$code_name.'_root_default');
					} else {
						echo '<img src="'.sq_web_path('lib').'/web/images/'.($customised ? 'cross' : 'tick').'.png" />';
						echo translate('Use default');

					}
					$this->_paintAssetChooser($prefix.'_'.$code_name.'_root', $current_root,  $wa, $customised);
					?>
				</td>
			<?php
			if ($wa) {
				?>
				<td>
					<?php
					check_box($prefix.'_to_delete[]', $code_name, FALSE);
					?>
				</td>
				<?php
			}
			?>
			</tr>
			<?php
		}//end foreach
		if ($wa) {
			?>
			<tr>
				<td>
					<?php text_box($prefix.'__new__full_name', ''); ?>
				</td>
				<td>
					<?php
					check_box($prefix.'__new__statuses_default', '1', TRUE, 'handleDefaultStatusClick(this)');
					label('Use Default', $prefix.'__new__statuses_default');
					$this->_paintStatusChooser($prefix.'__new__statuses', Array(), TRUE, FALSE);
					?>
				</td>
				<td>
					<?php
					check_box($prefix.'__new__root_default', '1', TRUE, 'handleDefaultRootClick(this)');
					label('Use Default', $prefix.'__new__root_default');
					$this->_paintAssetChooser($prefix.'__new__root', '', TRUE, FALSE);
					?>
				</td>
				<td>&nbsp</td>
			</tr>
			<?php
		}
		?>
		</table>
		<?php

	}//end _paintCategoryOptions()


	/**
	* Paint the interface for choosing statuses for a category option
	*
	* @param string		$name			Name for the form field
	* @param array		$current_values	The current search statuses for the option
	* @param boolean	$editable		Whether the field is editable
	* @param boolean	$visible		Whether the field should be css-visible initially
	*
	* @return void
	* @access private
	*/
	function _paintStatusChooser($name, $current_values, $editable=TRUE, $visible=TRUE)
	{
		if (!$editable && !$visible) {
			return;
		}
		$style = $visible ? '' : 'display: none';
		echo '<br />';

		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		if ($editable) {
			require_once SQ_LIB_PATH.'/html_form/html_form.inc';
			$statuses = get_constant_values('SQ_STATUS');
			$options = Array();
			foreach ($statuses as $constant => $value) {
				$options[$value] = get_status_description($value);
			}
			combo_box($name, $options, TRUE, $current_values, '', 'style="'.$style.'"');
		} else {
			if (empty($current_values)) {
				echo translate('No statuses are selected. Only assets for which the user has sufficient read permissions will be listed.');

			} else {
				?><ul style="margin: 1px 15px;"><?php
				foreach ($current_values as $status) {
					$status_name = get_status_description($status);
					$status_colour = get_status_colour($status);
					echo '<li>';
					echo '<img src="'.sq_web_path('lib').'/web/images/blank.gif" width="10" height="10" alt="'.$status_name.'" title="'.$status_name.'" align="absmiddle" style="margin: 2px; border: 1px solid #ccc; background-color: '.$status_colour.'" /> ';
					echo $status_name;
					echo '</li>';
				}
				?></ul><?php
			}
		}

	}//end _paintStatusChooser()


	/**
	* Paint the interface for choosing a root asset for a category option
	*
	* @param string		$name			Name for the form field
	* @param int		$current_value	The current root asset id for the option
	* @param boolean	$editable		Whether the field is editable
	* @param boolean	$visible		Whether the field should be css-visible initially
	*
	* @return void
	* @access private
	*/
	function _paintAssetChooser($name, $current_value, $editable=TRUE, $visible=TRUE)
	{
		if (!$editable && !$visible) {
			return;
		}
		echo '<div style="'.($visible ? '' : 'display: none').'">';
		if ($editable) {
			asset_finder($name, $current_value);
		} else {
			echo get_asset_tag_line($current_value);
		}
		echo '</div>';

	}//end _paintAssetChooser()


	/**
	* Processes the interface for managing the search categories
	*
	* @param object	&$asset	the search page asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processCategories(&$asset, &$o, $prefix)
	{
		$cats = $asset->attr('categories');

		// Any categories to delete?
		if (!empty($_POST[$prefix.'_category_sets_to_delete'])) {
			foreach ($_POST[$prefix.'_category_sets_to_delete'] as $delete_name) {
				// delete notice links for removed category
				if (isset($cats[$delete_name]['options']) && !empty($cats[$delete_name]['options'])) {
					foreach ($cats[$delete_name]['options'] as $option_delete_name => $option_content) {
						$old_link_value = 'root_'.$delete_name.'_'.$option_delete_name;
						$root_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_NOTICE, '', FALSE, 'major', $old_link_value);

						$ok = TRUE;
						foreach ($root_links as $root_link) {
							if (!$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($root_link['linkid'])) {
								$ok = FALSE;
							}
						}

					}
				// bug fix for #3884 Search page category - unable to delete
				// if the option attribute in empty that doesnt mean we cannot
				// delete the search category for it
				} else if (isset($cats[$delete_name]['options']) && empty($cats[$delete_name]['options'])){
					$ok = TRUE;
				}
				if($ok) unset($cats[$delete_name]);
			}
		}

		// Categories to edit?
		foreach ($cats as $name => $cat_data) {
			$cat_data['unselected_text'] = $_POST[$prefix.'_category_'.$name.'_unselected_text'];
			$cat_data['options'] = $this->_processCategoryOptions($asset, $prefix.'_category_'.$name.'_options', $name, $cat_data['options']);
			$cats[$name] = $cat_data;
		}

		return $asset->setAttrValue('categories', $cats);

	}//end processCategories()


	/**
	* Process the interface where category options for a category set are managed
	*
	* Updates and creates notice links for category options' root nodes
	*
	* @param object	&$asset			The asset whose categories are being managed
	* @param string	$prefix			The prefix for form fields
	* @param string	$set_name		The name of the category set whose options are being managed
	* @param array	$old_options	The existing options for this category set
	*
	* @return array
	* @access public
	*/
	function _processCategoryOptions(&$asset, $prefix, $set_name, $old_options)
	{
		$new_options = Array();
		//delete unwanted category
		if (isset($_POST[$prefix.'_to_delete'])) {
			foreach ($_POST[$prefix.'_to_delete'] as $delete_name) {
				$old_link_value = 'root_'.$set_name.'_'.$delete_name;
				$root_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_NOTICE, '', FALSE, 'major', $old_link_value);
				// delete notice links for removed category
				$ok = TRUE;
				foreach ($root_links as $root_link) {
					if (!$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($root_link['linkid'])) {
						$ok = FALSE;
					}
				}
				if($ok) unset($old_options[$delete_name]);
			}
		}
		foreach ($old_options as $code_name => $old_option) {
			$new_option = Array();
			$new_option['full_name'] = $_POST[$prefix.'_'.$code_name.'_full_name'];
			if (empty($new_option['full_name'])) {
				// can't change the name to blank, thankyou
				$new_option['full_name'] = $old_option['full_name'];
			}

			if (empty($_POST[$prefix.'_'.$code_name.'_statuses_default']) && !empty($_POST[$prefix.'_'.$code_name.'_statuses'])) {
				$new_option['statuses'] = $_POST[$prefix.'_'.$code_name.'_statuses'];
			}

			$new_code_name = preg_replace('/[^a-zA-Z0-9 ]/', '', $new_option['full_name']);
			$new_code_name = preg_replace('/( )+/', '_', $new_code_name);
			$new_code_name = strtolower($new_code_name);

			if (empty($_POST[$prefix.'_'.$code_name.'_root_default'])) {
				$new_option['root_assetid'] = '';
				$new_root = $_POST[$prefix.'_'.$code_name.'_root']['assetid'];
				// get the existing root link
				$old_link_value = 'root_'.$set_name.'_'.$code_name;
				$new_link_value = 'root_'.$set_name.'_'.$new_code_name;
				$root_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, '', FALSE, $old_link_value);
				if (empty($root_link) || ($old_link_value != $new_link_value) || $root_link['minorid'] != $new_root) {
					// the root or required link value has changed, so update the link
					$ok = TRUE;
					$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
					$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
					// delete old root link if any found...
					if (!empty($root_link) && !$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($root_link['linkid'])) {
						$ok = FALSE;
					} else if ($new_root) {
						$root = $GLOBALS['SQ_SYSTEM']->am->getAsset($new_root);
						if (!is_null($root)) {
							if (!$asset->createLink($root, SQ_LINK_NOTICE, $new_link_value)) {
								$ok = FALSE;
								trigger_localised_error('SCH0012', sprintf(translate('Failed adding new root node "%1$s" [#%2$s]'), $root->name, $root->id), E_USER_WARNING);
							}
						}
					}
					if ($ok) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
					} else {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					}
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				}
			}//end if
			$new_options[$new_code_name] = $new_option;

		}//end foreach

		if (!empty($_POST[$prefix.'__new__full_name'])) {
			$new_option['full_name'] = $_POST[$prefix.'__new__full_name'];
			if (empty($_POST[$prefix.'__new__statuses_default'])) {
				$new_option['statuses'] = array_get_index($_POST, $prefix.'__new__statuses', Array());
			}
			$new_code_name = preg_replace('/[^a-zA-Z0-9 ]/', '', $new_option['full_name']);
			$new_code_name = preg_replace('/( )+/', '_', $new_code_name);
			$new_code_name = strtolower($new_code_name);
			$link_value = 'root_'.$set_name.'_'.$new_code_name;
			if (empty($_POST[$prefix.'__new__root_default']) && !empty($_POST[$prefix.'__new__root']['assetid'])) {
				$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
				$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
				$new_root = $_POST[$prefix.'__new__root']['assetid'];
				$root = $GLOBALS['SQ_SYSTEM']->am->getAsset($new_root);
				if (!is_null($root)) {
					if (!$asset->createLink($root, SQ_LINK_NOTICE, $link_value)) {
						trigger_localised_error('SCH0012', sprintf(translate('Failed adding new root node "%1$s" [#%2$s]'), $root->name, $root->id), E_USER_WARNING);
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					} else {
						$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
						$new_option['root_assetid'] = '';
					}
				}
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			}
			$new_options[$new_code_name] = $new_option;
		}
		return $new_options;

	}//end _processCategoryOptions()


	/**
	* Paints the interface for adding a category set
	*
	* @param object	&$asset	the search page asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintAddCategory(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess('attributes')) {
			label(translate('Name').': ', $prefix.'_new_category');

			text_box($prefix.'_new_category', '');
			return TRUE;
		}

	}//end paintAddCategory()


	/**
	* Processes the interface for adding a search category set
	*
	* @param object	&$asset	the search page asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processAddCategory(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess('attributes')) {
			if (!empty($_POST[$prefix.'_new_category'])) {
				$cats = $asset->attr('categories');
				$safe_name = preg_replace('/[^a-zA-Z0-9 ]/', '', $_POST[$prefix.'_new_category']);
				$safe_name = preg_replace('/( )+/', '_', $safe_name);
				$safe_name = strtolower($safe_name);
				if (isset($cats[$safe_name])) {
					trigger_error('New Category Set could not be added: A category set called "'.$safe_name.'" already exists.  Try another name');
					return FALSE;
				}
				$cats[$safe_name] = Array('unselected_text' => '', 'options' => Array());
				$asset->setAttrValue('categories', $cats);
				return TRUE;
			}
		}
		return FALSE;

	}//end processAddCategory()


//--        FORMS        --//


	/**
	* Paints the interface to customise the search forms
	*
	* @param object	&$asset	the search page asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintCustomiseSearchForms(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('links');

		$search_bodycopy_link = $asset->getFormatBodycopyLink('search_form');
		$sis_bodycopy_link    = $asset->getFormatBodycopyLink('sis_form'); // search in search
		$search_customised    = !empty($search_bodycopy_link) && $search_bodycopy_link['link_type'] != SQ_LINK_TYPE_3;
		$sis_customised       = !empty($sis_bodycopy_link) && $sis_bodycopy_link['link_type'] != SQ_LINK_TYPE_3;

		if (!$write_access) {
			?>
			<img src="<?php echo sq_web_path('lib'); ?>/web/images/<?php echo empty($search_customised) ? 'cross' : 'tick'; ?>.png" width="15" height="15" />
			<?php
		} else {
			check_box($prefix.'_search_form', 1, $search_customised);
		}
		label(translate('Customise search form'), $prefix.'_search_form');


		echo '<br/>';

		if (!$write_access) {
			?>
			<img src="<?php echo sq_web_path('lib'); ?>/web/images/<?php echo empty($sis_customised) ? 'cross' : 'tick'; ?>.png" width="15" height="15" />
			<?php
		} else {
			check_box($prefix.'_sis_form', 1, $sis_customised);
		}
		label(translate('Customise search within search form'), $prefix.'_sis_form');


		return $write_access;

	}//end paintCustomiseSearchForms()


	/**
	* Processes the interface to customise the search forms
	*
	* @param object	&$asset	the search page asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processCustomiseSearchForms(&$asset, &$o, $prefix)
	{
		$format_folder =& $asset->getFormatFolder();

		$forms = Array(
					translate('Default Search')	=> 'search_form',

					translate('Search in Search')				=> 'sis_form',

				 );

		$bodycopies_visible = FALSE;

		foreach ($forms as $form_name => $value) {
			if (isset($_POST[$prefix.'_'.$value])) {

				if (is_null($format_folder)) {
					// create the folder if it does not exist

					$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
					$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
					$GLOBALS['SQ_SYSTEM']->am->includeAsset('folder');
					$format_folder = new Folder();
					$copy_link = Array('asset' => &$asset, 'value' => 'format_folder', 'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);

					$format_folder->setAttrValue('name', 'Search Forms');

					if (!$format_folder->create($copy_link)) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
						trigger_localised_error('SCH0013', translate('Unable to create the format folder'), E_USER_WARNING);
						return FALSE;
					}
					$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				}

				$bodycopy =& $asset->getFormatBodycopy($value);

				if (is_null($bodycopy)) {

					// if there is no bodycopy here, then we want to create it
					$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
					$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
					$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');
					$bodycopy = new Bodycopy();
					$copy_link = Array('asset' => &$format_folder, 'value' => $value, 'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);

					$bodycopy->setAttrValue('name', $form_name);

					if (!$bodycopy->create($copy_link)) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
						trigger_localised_error('SCH0014', sprintf(translate('Unable to create the %s bodycopy'), $form_name), E_USER_WARNING);
						return FALSE;
					}
					$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				} else {
					$bodycopy_link = $asset->getFormatBodycopyLink($value);
					if ($bodycopy_link['link_type'] != SQ_LINK_TYPE_2) {
						if (!$GLOBALS['SQ_SYSTEM']->am->updateLink($bodycopy_link['linkid'], SQ_LINK_TYPE_2)) {
							trigger_localised_error('SCH0014', sprintf(translate('Unable to create the %s bodycopy'), $form_name), E_USER_WARNING);
							return FALSE;
						}
					}
				}
				$bodycopies_visible = TRUE;

			} else {
				$bodycopy_link = $asset->getFormatBodycopyLink($value);
				if (!empty($bodycopy_link['link_type']) && $bodycopy_link['link_type'] != SQ_LINK_TYPE_3) {
					if (!$GLOBALS['SQ_SYSTEM']->am->updateLink($bodycopy_link['linkid'], SQ_LINK_TYPE_3)) {
						trigger_localised_error('SCH0015', sprintf(translate('Unable to update the link type for bodycopy %s'), $form_name), E_USER_WARNING);
						return FALSE;
					}
				}
			}//end if isset
		}//end foreach


		// if none of the bodycopies are visible, the hide the folder as well
		$folder_link_type = SQ_LINK_TYPE_3;
		if ($bodycopies_visible) {
			$folder_link_type = SQ_LINK_TYPE_2;
		}

		$folder_link = $asset->getFormatFolderLink();
		if (!empty($folder_link['link_type']) && $folder_link['link_type'] != $folder_link_type) {
			if (!$GLOBALS['SQ_SYSTEM']->am->updateLink($folder_link['linkid'], $folder_link_type)) {
				trigger_localised_error('SCH0016', translate('Unable to update the link type for the format folder'), E_USER_WARNING);
				return FALSE;
			}
		}

		return FALSE;

	}//end processCustomiseSearchForms()


	/**
	* Paints the thesaurus asset selection box
	*
	* @param object	&$asset	the search page asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintThesaurusAsset(&$asset, &$o, $prefix)
	{
		$link_value = 'thesaurus';
		$root_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, 'asset', FALSE, $link_value);

		$write_access = $asset->writeAccess('links');

		if (!$write_access) {
			$root_asset = NULL;
			if (!empty($root_link)) {
				echo get_asset_tag_line($root_link['minorid']);
			} else {
				echo translate('No thesaurus asset has been selected');

			}
		} else {
			asset_finder($prefix.'_thesaurusid', (!empty($root_link)) ? $root_link['minorid'] : '', Array('thesaurus' => 'D'));
		}

		return $write_access;

	}//end paintThesaurusAsset()


	/**
	* Processes the value input from thesaurus asset selection box
	*
	* @param object	&$asset	the search page asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function processThesaurusAsset(&$asset, &$o, $prefix)
	{
		if (!isset($_POST[$prefix.'_thesaurusid']['assetid'])) {
			return FALSE;
		}
		$new_root  = $_POST[$prefix.'_thesaurusid']['assetid'];

		// getting existing root link
		$link_value = 'thesaurus';
		$root_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, '', FALSE, $link_value);

		// dont process anything if the root node has not changed
		if (!empty($root_link) && $root_link['minorid'] == $new_root) {
			return FALSE;
		}

		$ok = TRUE;

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// deleting old root link if any found...
		if (!empty($root_link) && !$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($root_link['linkid'])) {
			$ok = FALSE;
		} else if ($new_root) {
			$root = $GLOBALS['SQ_SYSTEM']->am->getAsset($new_root);

			if (!is_null($root)) {
				if (!$asset->createLink($root, SQ_LINK_NOTICE, $link_value)) {
					$ok = FALSE;
					trigger_localised_error('SCH0017', sprintf(translate('Failed adding new root node "%1$s" [#%2$s]'), $root->name, $root->id), E_USER_WARNING);
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

	}//end processThesaurusAsset()


	/**
	* Paints the auto synonym search section
	*
	* @param object	&$asset	the search page asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintAutoSynonymSearch(&$asset, &$o, $prefix)
	{
		$search_fields = array_keys($asset->attr('fields'));
		$keyed_search_fields = Array();
		foreach ($search_fields as $field) {
			$keyed_search_fields[$field] = $field;
		}

		$write_access = $asset->writeAccess('attributes');

		// No search fields present
		if (empty($search_fields)) {
			echo translate('No Search Fields have been defined for this asset. Automatic Synonym Search requires a search field to function.');

			return FALSE;
		}

		// Thesaurus asset not installed (why? it's in the core...)
		if (!$this->thesaurusInstalled($asset, $prefix)) {
			echo translate('The Thesaurus asset is not installed on the system. Automatic Synonym Search requires Thesaurus to be installed to function.');

			return FALSE;
		}

		$link_value = 'thesaurus';
		$root_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, 'asset', FALSE, $link_value);

		// No thesaurus asset set
		if (empty($root_link)) {
			echo translate('No Thesaurus asset has been set to use for synonym searching. Please select a Thesaurus asset to use first.');

			return FALSE;
		}

		$field = $asset->attr('auto_synonym_search_field');

		if ($write_access) {
			check_box($prefix.'_auto_synonym_search_enabled', 1, !empty($field), 'form.elements["'.$prefix.'_auto_synonym_search_field"].disabled = !this.checked; form.elements["'.$prefix.'_enable_multi_word_synonym_lookups"].disabled = !this.checked; return TRUE;');
			echo translate('Enable Automatic Synonym Searching');

			$o->closeField();

			$o->openField(translate('Use Search Field'));

				combo_box($prefix.'_auto_synonym_search_field', $keyed_search_fields, FALSE, $field, 0,  empty($field) ? 'disabled="disabled"' : '');
		} else {
			if (empty($field)) {
				echo translate('Automatic Synonym Search is <strong>disabled</strong>');

			} else {
				echo sprintf(translate('Automatic Synonym Search is <strong>enabled</strong>, using field: <strong>%s</strong>'), $field);

			}
		}

		return FALSE;

	}//end paintAutoSynonymSearch()


	/**
	* Processes the auto synonym search section
	*
	* @param object	&$asset	the asset being painted
	* @param object	&$o		backend outputter class
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return void
	* @access public
	*/
	function processAutoSynonymSearch(&$asset, &$o, $prefix)
	{
		// If the check box was not enabled, then store a blank string
		if (empty($_POST[$prefix.'_auto_synonym_search_enabled'])) {
			$asset->setAttrValue('auto_synonym_search_field', '');
			return TRUE;
		}

		// Otherwise
		if (!empty($_POST[$prefix.'_auto_synonym_search_field'])) {
			$asset->setAttrValue('auto_synonym_search_field', $_POST[$prefix.'_auto_synonym_search_field']);
			return TRUE;
		}

		return FALSE;

	}//end processAutoSynonymSearch()


	/**
	* Paints the "Enable Multi-Word Synonym Lookups" item
	*
	* @param object	&$asset	the search page asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintEnableMultiWordSynonymLookups(&$asset, &$o, $prefix)
	{
		$search_fields = array_keys($asset->attr('fields'));
		$keyed_search_fields = Array();
		foreach ($search_fields as $field) {
			$keyed_search_fields[$field] = $field;
		}

		$write_access = $asset->writeAccess('attributes');

		// No search fields present
		if (empty($search_fields)) {
			return FALSE;
		}

		// Thesaurus asset not installed (why? it's in the core...)
		if (!$this->thesaurusInstalled($asset, $prefix)) {
			return FALSE;
		}

		$link_value = 'thesaurus';
		$root_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, 'asset', FALSE, $link_value);

		// No thesaurus asset set
		if (empty($root_link)) {
			return FALSE;
		}

		$enable_multi_words = $asset->attr('enable_multi_word_synonym_lookups');

		$o->openField(translate('Enable Multi-Word Synonym Lookups'));

		if ($write_access) {
				check_box($prefix.'_enable_multi_word_synonym_lookups', 1, $enable_multi_words, '');
				echo translate('Enable Multi-Word Synonym Lookups');

		} else {
			echo '<img src="'.sq_web_path('lib').'/web/images/'.(empty($enable_multi_words) ? 'cross' : 'tick').'.png" />';
		}
		echo '<br />';
		echo translate('This option will iterate through the supplied search string and search the Thesaurus for groups of words. If these grouped words have synonyms in the Thesaurus then the <em>synonyms</em> will be appended to the search string.<br />For example, a search for \'fred flute player\' will result in a Thesaurus lookup of synonyms for \'fred flute\', \'flute player\' (matches \'flautist\') and \'fred flute player\'. The search string in this case will be modified on-the-fly to read \'fred flute player flautist\'.');


		return FALSE;

	}//end paintEnableMultiWordSynonymLookups()


	/**
	* Processes the "Enable Multi-Word Synonym Lookups" item
	*
	* @param object	&$asset	the asset being painted
	* @param object	&$o		backend outputter class
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return void
	* @access public
	*/
	function processEnableMultiWordSynonymLookups(&$asset, &$o, $prefix)
	{
		if (empty($_POST[$prefix.'_enable_multi_word_synonym_lookups'])) {
			$asset->setAttrValue('enable_multi_word_synonym_lookups', FALSE);
		} else {
			$asset->setAttrValue('enable_multi_word_synonym_lookups', TRUE);

		}

		return TRUE;

	}//end processEnableMultiWordSynonymLookups()


	/**
	* Return TRUE if the thesaurus asset type is installed
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param string	$prefix	the prefix for this asset
	*
	* @return boolean
	* @access public
	*/
	function thesaurusInstalled(&$asset, $prefix)
	{
		return $GLOBALS['SQ_SYSTEM']->am->installed('thesaurus');

	}//end thesaurusInstalled()


//--        SORTING        --//


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

		parent::_paintExtraSortOptions($asset, $o, $prefix);
		$wa = $asset->writeAccess('attributes');
		$o->openField(translate('Relevance'));

			$rel_enabled = FALSE;
			foreach ($asset->attr('sort_by') as $v) {
				if (array_get_index($v['params'], 'field') == 'score') {
					$rel_enabled = TRUE;
					break;
				}
			}
			if ($wa) {
				check_box($prefix.'_enable_rel', '1', $rel_enabled);
				label(translate('Enable "Sort by Relevance" option'), $prefix.'_enable_rel');

			} else {
				$filename = $rel_enabled ? 'tick' : 'cross';
				echo '<img src="'.sq_web_path('lib').'/web/images/'.$filename.'.png" /> ';
				echo translate('Enable "Sort by Relevance" option');

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
		$res = parent::_processExtraSortOptions($asset, $o, $prefix);
		$val = $asset->attr('sort_by');
		if (isset($_POST[$prefix.'_submitted'])) {
			$want_rel = !empty($_POST[$prefix.'_enable_rel']);
			$rel_index = FALSE;
			foreach ($val as $i => $v) {
				if (array_get_index($v['params'], 'field') == 'score') {
					$rel_index = $i;
					break;
				}
			}
			if (($rel_index !== FALSE) && !$want_rel) {
				unset($val[$rel_index]);
			} else if (($rel_index === FALSE) && $want_rel) {
				$val[] = Array(
							'name'		=> translate('Relevance'),

							'type'		=> '',
							'params'	=> Array('field' => 'score'),
						 );
			}
			$res = $asset->setAttrValue('sort_by', $val) || $res;
		}//end if
		return $res;

	}//end _processExtraSortOptions()


//--        SAVED SEARCH        --//


	/**
	* Paint the inteface for configuring saved search
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access private
	*/
	function paintSavedSearch(&$asset, &$o, $prefix)
	{
		// Not much extra functionality here, just need to add the search fields to the _params
		// of parameter map, then show the results
		$wa = $asset->writeAccess('attributes');
		$pmap = $asset->getAttribute('stored_query_session');
		$current_params = $pmap->_params;
		// Add in the search fields
		$fields = $asset->attr('fields');
		foreach ($fields as $fieldname => $fieldvalue) {
			$current_params['parameters'][$fieldname] = 'Search Field: '.$fieldname;
		}
		$pmap->_params = $current_params;
		if ($wa) {
			$pmap->paint($prefix);
		} else {
			$pmap->paint($prefix, TRUE);
		}

	}//end paintSavedSearch()


	/**
	* Process the interface for configuring saved search
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access private
	*/
	function processSavedSearch(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		$pmap = $asset->getAttribute('stored_query_session');
		if ($wa) {
			$pmap->process($prefix);
			$asset->setAttrValue('stored_query_session', $pmap->value);
			$asset->saveAttributes();
		}

		return TRUE;

	}//end processSavedSearch()


}//end class

?>
