<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: search_page_edit_fns.inc,v 1.69.4.1 2006/07/23 22:56:42 rong Exp $
*
*/

require_once SQ_PACKAGES_PATH.'/cms/listing_engine/listing_engine_edit_fns.inc';

/**
* Search_Page_Edit_Fns
*
*     Edit functions for search page
*
* @author  Dominic Wong <dwong@squiz.net>
* @version $Revision: 1.69.4.1 $
* @package MySource_Matrix_Packages
* @subpackage search
*/
class Search_Page_Edit_Fns extends Listing_Engine_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Search_Page_Edit_Fns()
	{
		$this->Listing_Engine_Edit_Fns();
		$this->static_screens['details']['lock_type'] = 'content';
		$this->static_screens['details']['force_unlock'] = false;

	}//end constructor


	/**
	* Paints the root node selection box
	*
	* @param object	Asset				&$rootid	asset being painted
	* @param object Backend_Outputter	&$o			backend outputter
	* @param string						$prefix		prefix for the html doc element name
	* @param string						$category	the category we are painting for
	*												(if blank we paint for the whole search page)
	* @param boolean					$read_only	force read only view
	*
	* @return boolean
	* @access public
	*/
	function paintRootAsset(&$asset, &$o, $prefix, $category='', $read_only=false)
	{
		$link_value = 'root';
		if (!empty($category)) $link_value .= '_'.$category;
		$root_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, 'asset', false, $link_value);

		if ($read_only) {
			$write_access = false;
		} else {
			$write_access = $asset->writeAccess('links');
		}

		if (!$write_access) {
			$root_asset = null;
			if (!empty($root_link)) {
				$root_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($root_link['minorid']);
			}
			if (!is_null($root_asset)) {
				echo $root_asset->name.' (Id: #'.$root_asset->id.')';
			} else {
				echo translate('sch_no_root_node_selected');
			}
		} else {
			asset_finder($prefix.'_rootid', (!empty($root_link)) ? $root_link['minorid'] : '');
		}

		return $write_access;

	}//end paintRootAsset()


	/**
	* Processes the value input from root node selection box
	*
	* @param object	Asset				&$rootid	asset being painted
	* @param object Backend_Outputter	&$o			backend outputter
	* @param string						$prefix		prefix for the html doc element name
	* @param string						$category	the category we are processing for
	*												(if blank we process the root node for the page)
	*
	* @return void
	* @access public
	*/
	function processRootAsset(&$asset, &$o, $prefix)
	{
		if (!isset($_POST[$prefix.'_rootid']['assetid'])) {
			return false;
		}
		$new_root  = $_POST[$prefix.'_rootid']['assetid'];

		// getting existing root link
		$link_value = 'root';
		$root_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, '', false, $link_value);

		// dont process anything if the root node has not changed
		if (!empty($root_link) && $root_link['minorid'] == $new_root) {
			return false;
		}

		$ok = true;

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// deleting old root link if any found...
		if (!empty($root_link) && !$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($root_link['linkid'])) {
			$ok = false;
		} else if ($new_root) {
			$root = &$GLOBALS['SQ_SYSTEM']->am->getAsset($new_root);
			if (!is_null($root)) {
				if (!$asset->createLink($root, SQ_LINK_NOTICE, $link_value)) {
					$ok = false;
					trigger_localised_error('SCH0012', E_USER_WARNING, $root->name, $root->id);
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
	* @param object Search_Page			&$asset	the search page asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*
	*/
	function paintFields(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('all');
		$fields = $asset->attr('fields');
		$query_types = Array(
						'none'			=> '-- '.ucfirst(translate('sch_page_add_data_source').' --'),
						'include_all'	=> ucfirst(translate('include_all')),
						'standard'		=> ucfirst(translate('sch_page_datatype_standard')),
						'asset_attrib'	=> ucfirst(translate('sch_page_datatype_attribute')),
						'metadata'		=> ucfirst(translate('sch_page_datatype_metadata')),
					   );

		?>
		<table class ="sq-backend-table" border="1" style="margin-bottom: 7px">
			<tr>
				<th rowspan="2"><?php echo translate('name'); ?></th>
				<th colspan="<?php echo $write_access ? 3 : 2; ?>"><?php echo translate('sch_page_data_sources'); ?></th>
				<th rowspan="2"><?php echo translate('sch_page_word_logic'); ?></th>
				<?php
				if ($write_access) {
					?>
					<th rowspan="2"><?php echo translate('delete'); ?></th>
					<?php
				}
				?>
			</tr>
			<tr>
				<th><?php echo translate('type');?></th>
				<th <?php if (!$write_access) echo 'colspan=2';?>><?php echo translate('details');?> </th>
				<?php
				if ($write_access) {
					?>
					<th><?php echo translate('delete'); ?></th>
					<?php
				}
				?>
			</tr>
			<?php
			foreach ($fields as $field_name => $field_data) {
				$field_datasources = $field_data['data_sources'];
				if ((!empty($field_datasources)) && ($field_datasources[0]['type'] == 'exclude')) {
					continue;
				}
				if ($write_access) $field_datasources[] = 'new';
				foreach ($field_datasources as $index => $field_datasource) {
					?>
					<tr>
						<?php
						if ($index == 0) {
							?>
							<td rowspan="<?php echo count($field_datasources);?>"> <?php echo $field_data['name'] ?></td>
							<?php
						}
						?>
						<td class="sq-backend-data" style="padding: 4px;">
							<?php
							if ($field_datasource == 'new') {
								if ($write_access) {
									combo_box($prefix.'_fields['.$field_name.'][new_data_source_type]', $query_types, false,'none');
								}
							} else {
								$type = array_get_index($field_datasource, 'type');
								echo $query_types[$type];
							}
							?>
						</td>
						<td>
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
											$options = Array('' => '-- '.translate('select_asset_type').' --');
											foreach (array_keys($search_types) as $this_type) {
												$options[$this_type] = $type_names[$this_type];
											}
											combo_box($prefix.'_fields['.$field_name.'][data_sources]['.$index.'][params][asset_type]', $options, false, $search_type);
										} else {
											echo '<strong>'.translate('asset_type').':</strong> ';
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
												$type_attrs = Array('' => '-- '.translate('select_attribute').' --');
												foreach ($asset_type_attrs as $attr_id => $attr_name) {
													$type_attrs[$attr_id] = $attr_name;
												}
												combo_box($prefix.'_fields['.$field_name.'][data_sources]['.$index.'][params][attrid]', $type_attrs, false, $selected_attr);
											} else if ($selected_attr != '') {
												echo '<br /><strong>'.translate('attribute').':</strong>  '.$asset_type_attrs[$selected_attr];
											}
										}

										if (!empty($selected_attr)) {
											$attr_info = $GLOBALS['SQ_SYSTEM']->am->getAttributeInfo(Array($selected_attr));
											$attr_info = $attr_info[$selected_attr];

											if (($attr_info['type'] == 'int') || ($attr_info['type'] == 'float')) {
												$current_search = array_get_index($field_datasource['params'], 'numeric_search', 'exact');
												?><br/><?php
												if ($write_access) {
													radio_button($prefix.'_fields['.$field_name.'][data_sources]['.$index.'][params][numeric_search]', 'exact', ($current_search == 'exact'));
													echo translate('sch_page_numeric_search_exact');
													?><br/><?php

													radio_button($prefix.'_fields['.$field_name.'][data_sources]['.$index.'][params][numeric_search]', 'range', ($current_search == 'range'));
													echo translate('sch_page_numeric_search_range');
													?><br/><?php

													radio_button($prefix.'_fields['.$field_name.'][data_sources]['.$index.'][params][numeric_search]', 'custom', ($current_search == 'custom'));
													echo translate('sch_page_numeric_search_custom');
													?><br/><?php
												} else {
													?><strong><?php echo translate('sch_page_numeric_search_style'); ?></strong> <?php
													echo translate('sch_page_numeric_search_'.$current_search);
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
													foreach($custom_ranges as $key => $custom_range) {
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
												}

											}
										}
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
												$field_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($metadata_field);
												if (!is_null($metadata_field)) {
													echo get_asset_tag_line($field_asset->id);
												} else {
													echo translate('sch_page_no_metadata_field_specified');
												}
											} else {
												echo translate('sch_page_no_metadata_field_specified');
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
											$type_attrs = Array('' => '-- '.translate('select_attribute').' --');
											$sm =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('search_manager');
											$attributes = $sm->standard_date_fields + $sm->standard_text_fields;
											combo_box($prefix.'_fields['.$field_name.'][data_sources]['.$index.'][params][field]', $attributes, false, $selected_attr_name);
										} else if ($selected_attr_name != '') {
											echo translate('sch_page_field').':  '.$selected_attr_name;
										}
										break;

									case 'include_all':
										echo '&nbsp;';
										hidden_field($prefix.'_fields['.$field_name.'][data_sources]['.$index.'][params]', '');
										break;

								}//end switch
							}
							?>
						</td>
						<?php
						if ($write_access) {
							// Delete Data Source Cell
							?>
							<td class="sq-backend-data">
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
							<td rowspan="<?php echo count($field_datasources);?>">
								<?php
								$word_logic_options = Array('AND' => translate('sch_page_all_words'), 'OR' => translate('sch_page_any_word'));
								if ($write_access) {
									combo_box($prefix.'_fields['.$field_name.'][word_logic]', $word_logic_options, false, array_get_index($field_data, 'word_logic', 'AND'), 0, 'title="'.translate('sch_page_word_logic_exp').'"');
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
							<td rowspan="<?php echo count($field_datasources);?>">
								<?php
								check_box($prefix.'_fields['.$field_name.'][delete]');
								?>
							</td>
							<?php
						}
						?>
						<?php
					}
				}
				?>
			</tr>

		</table>
		<?php
		if ($write_access) {
			label(translate('sch_page_new_search_field'), $prefix.'_new_field_name');
			echo '&nbsp;';
			text_box($prefix.'_new_field_name', '', 15);
		}
		return true;

	}//end paintFields()


	/**
	* Processes the interface for adding and configuring search fields.
	*
	* @param object Search_Page			&$asset	the search page asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*
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
			$new_fields[$safe_name]['name'] = $_POST[$prefix.'_new_field_name'];
			$new_fields[$safe_name]['data_sources'] = Array();
		}

		foreach ($old_fields as $field_name => $field) {
			if ((!empty($field['data_sources'])) && ($field['data_sources'][0]['type'] == 'exclude')) {
				$new_fields[$field_name] = $field;
			}
		}
		return $asset->setAttrValue('fields', $new_fields);

	}//end processFields()


	/**
	* Paints the interface for adding and configuring Exlude fields.
	*
	* @param object Search_Page			&$asset	the search page asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*
	*/
	function paintExcludeFields(&$asset, &$o, $prefix)
	{
		$fields = $asset->attr('fields');
		$write_access = $asset->writeAccess('all');
		?>
		<table class="sq-backend-table" style="margin-bottom: 7px">
			<tr>
				<th <?php if (!$write_access) echo 'colspan=2';?>><?php echo translate('name');?></th>
				<?php
				if ($write_access) {
					?>
					<th><?php echo translate('delete');?></th>
					<?php
				}
				?>
			</tr>
				<?php
				foreach ($fields as $field_name => $field_data) {
					$field_datasources = $field_data['data_sources'];
					if ((!empty($field_datasources)) && ($field_datasources[0]['type'] == 'exclude')) {
						?>
						<tr>
							<td><?php echo $field_data['name'];?></td>
							<?php
							if ($write_access) {
								?>
								<td class="sq-backend-data">
									<?php
									check_box($prefix.'_delete_exclude_fields['.$field_name.']');
									?>
								</td>
								<?php
							}
							?>
						</tr>

						<?php
					}
				}
				?>
		</table>
		<?php
		if ($write_access) {
			label(translate('sch_page_new_exclude_field'), $prefix.'_new_exclude_field');
			echo '&nbsp;';
			text_box($prefix.'_new_exclude_field', '', 15);
		}

	}//end paintExcludeFields()


	/**
	* Processes the interface for adding and configuring Exclude fields.
	*
	* @param object Search_Page			&$asset	the search page asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*
	*/
	function processExcludeFields(&$asset, &$o, $prefix)
	{
		$fields = $asset->attr('fields');
		if (isset($_POST[$prefix.'_delete_exclude_fields'])) {
			foreach ($_POST[$prefix.'_delete_exclude_fields'] as $field_name => $field) {
				unset($fields[$field_name]);
			}
		}
		if (!empty($_POST[$prefix.'_new_exclude_field'])) {
			$safe_name = strtolower(str_replace(' ', '_', $_POST[$prefix.'_new_exclude_field']));
			$fields[$safe_name]['name'] = $_POST[$prefix.'_new_exclude_field'];
			$fields[$safe_name]['data_sources'][0]['type'] = 'exclude';
		}
		return $asset->setAttrValue('fields', $fields);

	}//end processExcludeFields()


	/**
	* Paints the interface for selecting search field styles
	*
	* @param object Search_Page			&$asset	the search page asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*
	*/
	function paintStyles(&$asset, &$o, $prefix)
	{

		$write_access = $asset->writeAccess('attributes');
		$search_styles = $asset->attr('search_styles');
		$fields = $asset->attr('fields');
		foreach ($fields as $field) {
			$field_datasources = $field['data_sources'];
			$type = null;
			foreach ($field_datasources as $field_datasource) {
				if (isset($field_datasource['type'])) {
					$type = $field_datasource['type'];
				}
				if ($type == 'asset_attrib') {

					if (!isset($field_datasource['params']['attrid'])) {
						continue;
					}
					$attribute = &$GLOBALS['SQ_SYSTEM']->am->getAttribute($field_datasource['params']['attrid']);
					if (is_null($attribute)) continue;

					// if this is a boolean or selection field, we want to make sure blank options are allowed
					if ($attribute->type() == 'boolean' || $attribute->type() == 'selection') {
						$attribute->_params['allow_empty'] = true;
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
	* @param object Search_Page			&$asset	the search page asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*
	*/
	function processStyles(&$asset, &$o, $prefix)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('attributes')) return false;

		$search_styles = Array();

		$fields = $asset->attr('fields');

		foreach ($fields as $fieldname => $field) {
			$field_datasources = $field['data_sources'];
			$type = null;
			foreach ($field_datasources as $field_datasource) {
				if(isset($field_datasource['type']))
					$type = $field_datasource['type'];
				if ($type == 'asset_attrib') {

					if (empty($field_datasource['params']['attrid'])) continue;
					$attribute = &$GLOBALS['SQ_SYSTEM']->am->getAttribute($field_datasource['params']['attrid']);
					$search_styles[$attribute->id] = $attribute->processEditParams($prefix.'_'.$attribute->id);
				}
			}
		}

		$asset->setAttrValue('search_styles', $search_styles);

		return true;

	}//end processStyles()


//--        CATEGORIES        --//


	/**
	* Paints the interface for managing the search categories
	*
	* @param object Search_Page			&$asset	the search page asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
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
							$msg = translate('search_page_confirm_cat_delete');
							check_box($prefix.'_category_sets_to_delete[]', $name, false, "return confirm('$msg')", 'id="'.$prefix.'_category_sets_to_delete_'.$name.'"');
							label(translate('delete'), $prefix.'_category_sets_to_delete_'.$name);
						}
						?>
					</th>
				</tr>
				<tr>
					<td colspan="2">
						<?php
						if ($wa) {
							label(translate('search_cat_unselected_text').': ', $prefix.'_category_'.$name.'_unselected_text');
							echo '&nbsp;';
							text_box($prefix.'_category_'.$name.'_unselected_text', $set_data['unselected_text']);
						} else {
							echo translate('search_cat_unselected_text');
							echo $set_data['unselected_text'];
						}
						?>
					</td>
				</tr>
				<tr>
					<td colspan="2">
						<?php
						echo translate('options').':';
						$this->_paintCategoryOptions($asset, $prefix.'_category_'.$name.'_options', $name, $set_data['options'], $wa);
						?>
					</td>
				</tr>
			</table>
			<?php
		}

	}//end paintCategories()


	/**
	* Paint the interface for managing a category set's category options
	*
	* @param object		$asset		The asset whose categories are being managed
	* @param string		$prefix		The prefix for form fields
	* @param string		$set_name	The category set whose options are being managed
	* @param array		$options	The current category options
	* @param boolean	$wa			Whether we have write access to the field
	*
	* @access private
	* @return void
	*/
	function _paintCategoryOptions(&$asset, $prefix, $set_name, $options, $wa)
	{
		?>
		<table class="sq-backend-table">
			<tr>
				<th><?php translate('name'); ?></th>
				<th style="width: 35ex"><?php echo translate('statuses'); ?></th>
				<th style="width: 60ex"><?php echo translate('root_node'); ?></th>
			<?php
			if ($wa) {
				?>
				<th style="width: 7ex"><?php echo translate('delete'); ?></th>
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
						echo '<img src="'.sq_web_path('lib').'/web/images/'.($customised ? 'cross' : 'tick').'.gif" />';
						echo translate('use_default');
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
						$root_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, '', false, $link_value);
						$current_root = $root_link['minorid'];
					}
					if ($wa) {
						check_box($prefix.'_'.$code_name.'_root_default', '1', !$customised, 'handleDefaultRootClick(this)');
						label('Use Default', $prefix.'_'.$code_name.'_root_default');
					} else {
						echo '<img src="'.sq_web_path('lib').'/web/images/'.($customised ? 'cross' : 'tick').'.gif" />';
						echo translate('use_default');
					}
					$this->_paintAssetChooser($prefix.'_'.$code_name.'_root', $current_root,  $wa, $customised);
					?>
				</td>
			<?php
			if ($wa) {
				?>
				<td>
					<?php
					check_box($prefix.'_to_delete[]', $code_name, false);
					?>
				</td>
				<?php
			}
			?>
			</tr>
			<?php
		}
		if ($wa) {
			?>
			<tr>
				<td>
					<?php text_box($prefix.'__new__full_name', ''); ?>
				</td>
				<td>
					<?php
					check_box($prefix.'__new__statuses_default', '1', true, 'handleDefaultStatusClick(this)');
					label('Use Default', $prefix.'__new__statuses_default');
					$this->_paintStatusChooser($prefix.'__new__statuses', Array(), true, false);
					?>
				</td>
				<td>
					<?php
					check_box($prefix.'__new__root_default', '1', true, 'handleDefaultRootClick(this)');
					label('Use Default', $prefix.'__new__root_default');
					$this->_paintAssetChooser($prefix.'__new__root', '', true, false);
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
	* @access private
	* @return void
	*/
	function _paintStatusChooser($name, $current_values, $editable=true, $visible=true)
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
			combo_box($name, $options, true, $current_values, '', 'style="'.$style.'"');
		} else {
			if (empty($current_values)) {
				echo translate('sch_page_no_status_selected');
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
	* @param string	$name			Name for the form field
	* @param int	$current_value	The current root asset id for the option
	* @param boolean	$editable		Whether the field is editable
	* @param boolean	$visible		Whether the field should be css-visible initially
	*
	* @access private
	* @return void
	*/
	function _paintAssetChooser($name, $current_value, $editable=true, $visible=true)
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
	* @param object Search_Page			&$asset	the search page asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
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
				unset($cats[$delete_name]);
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
	* @param object	$asset			The asset whose categories are being managed
	* @param string	$prefix			The prefix for form fields
	* @param string	$set_name		The name of the category set whose options are being managed
	* @param array	$old_options	The existing options for this category set
	*
	* @access public
	* @return array	New options for the specified category
	*/
	function _processCategoryOptions(&$asset, $prefix, $set_name, $old_options)
	{
		$new_options = Array();
		if (isset($_POST[$prefix.'_to_delete'])) {
			foreach ($_POST[$prefix.'_to_delete'] as $delete_name) {
				unset($old_options[$delete_name]);
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
				$root_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, '', false, $old_link_value);
				if (empty($root_link) || ($old_link_value != $new_link_value) || $root_link['minorid'] != $new_root) {
					// the root or required link value has changed, so update the link
					$ok = true;
					$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
					$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
					// delete old root link if any found...
					if (!empty($root_link) && !$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($root_link['linkid'])) {
						$ok = false;
					} else if ($new_root) {
						$root = &$GLOBALS['SQ_SYSTEM']->am->getAsset($new_root);
						if (!is_null($root)) {
							if (!$asset->createLink($root, SQ_LINK_NOTICE, $new_link_value)) {
								$ok = false;
								trigger_localised_error('SCH0012', E_USER_WARNING, $root->name, $root->id);
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
			}
			$new_options[$new_code_name] = $new_option;

		}

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
				if (!is_null($root =& $GLOBALS['SQ_SYSTEM']->am->getAsset($new_root))) {
					if (!$asset->createLink($root, SQ_LINK_NOTICE, $link_value)) {
						trigger_localised_error('SCH0012', E_USER_WARNING, $root->name, $root->id);
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
	* @param object Search_Page			&$asset	the search page asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintAddCategory(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess('attributes')) {
			label(translate('name').': ', $prefix.'_new_category');
			text_box($prefix.'_new_category', '');
			return true;
		}

	}//end paintAddCategory()


	/**
	* Processes the interface for adding a search category set
	*
	* @param object Search_Page			&$asset	the search page asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
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
					return false;
				}
				$cats[$safe_name] = Array('unselected_text' => '', 'options' => Array());
				$asset->setAttrValue('categories', $cats);
				return true;
			}
		}
		return false;

	}//end processAddCategory()


//--        FORMS        --//


	/**
	* Paints the interface to customise the search forms
	*
	* @param object Search_Page			&$asset	the search page asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintCustomiseSearchForms(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('links');

		$search_bodycopy_link = $asset->getFormatBodycopyLink('search_form');
		$sis_bodycopy_link    = $asset->getFormatBodycopyLink('sis_form'); //search in search
		$search_customised    = !empty($search_bodycopy_link) && $search_bodycopy_link['link_type'] != SQ_LINK_TYPE_3;
		$sis_customised       = !empty($sis_bodycopy_link) && $sis_bodycopy_link['link_type'] != SQ_LINK_TYPE_3;

		?>
			<table>
				<tr>
					<td><?php echo translate('sch_page_customise_search_form') ?></td>
					<td>
						<?php
							if (!$write_access) {
								echo ($search_customised) ? translate('yes') : translate('no');
							} else {
								check_box($prefix.'_search_form', 1, $search_customised);
							}
						?>
					</td>
				</tr>
				<tr>
					<td><?php echo translate('sch_page_customise_sis_form') ?></td>
					<td>
						<?php
							if (!$write_access) {
								echo ($sis_customised) ? translate('yes') : translate('no');
							} else {
								check_box($prefix.'_sis_form', 1, $sis_customised);
							}
						?>
					</td>
				</tr>
			</table>
		<?php

		return $write_access;

	}//end paintCustomiseSearchForms()


	/**
	* Processes the interface to customise the search forms
	*
	* @param object Search_Page			&$asset	the search page asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processCustomiseSearchForms(&$asset, &$o, $prefix)
	{
		$format_folder = &$asset->getFormatFolder();

		$forms = Array(
					translate('sch_page_default_search')	=> 'search_form',
					translate('sch_page_sis')				=> 'sis_form',
				 );

		$bodycopies_visible = false;

		foreach ($forms as $form_name => $value) {
			if (isset($_POST[$prefix.'_'.$value])) {

				if (is_null($format_folder)) {
					// create the folder if it does not exist

					$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
					$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
					$GLOBALS['SQ_SYSTEM']->am->includeAsset('folder');
					$format_folder =& new Folder();
					$copy_link = Array('asset' => &$asset, 'value' => 'format_folder', 'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);

					$format_folder->setAttrValue('name', 'Search Forms');

					if (!$format_folder->create($copy_link)) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
						trigger_localised_error('SCH0013', E_USER_WARNING);
						return false;
					}
					$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				}

				$bodycopy = &$asset->getFormatBodycopy($value);

				if (is_null($bodycopy)) {

					// if there is no bodycopy here, then we want to create it
					$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
					$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
					$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');
					$bodycopy =& new Bodycopy();
					$copy_link = Array('asset' => &$format_folder, 'value' => $value, 'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);

					$bodycopy->setAttrValue('name', $form_name);

					if (!$bodycopy->create($copy_link)) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
						trigger_localised_error('SCH0014', E_USER_WARNING, $form_name);
						return false;
					}
					$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				} else {
					$bodycopy_link = $asset->getFormatBodycopyLink($value);
					if ($bodycopy_link['link_type'] != SQ_LINK_TYPE_2) {
						if (!$GLOBALS['SQ_SYSTEM']->am->updateLink($bodycopy_link['linkid'], SQ_LINK_TYPE_2)) {
							trigger_localised_error('SCH0014', E_USER_WARNING, $form_name);
							return false;
						}
					}
				}
				$bodycopies_visible = true;

			} else {
				$bodycopy_link = $asset->getFormatBodycopyLink($value);
				if (!empty($bodycopy_link['link_type']) && $bodycopy_link['link_type'] != SQ_LINK_TYPE_3) {
					if (!$GLOBALS['SQ_SYSTEM']->am->updateLink($bodycopy_link['linkid'], SQ_LINK_TYPE_3)) {
						trigger_localised_error('SCH0015', E_USER_WARNING, $form_name);
						return false;
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
				trigger_localised_error('SCH0016', E_USER_WARNING);
				return false;
			}
		}

		return false;

	}//end processCustomiseSearchForms()


	/**
	* Paints the thesaurus asset selection box
	*
	* @param object	Asset				&$rootid	asset being painted
	* @param object Backend_Outputter	&$o			backend outputter
	* @param string						$prefix		prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function paintThesaurusAsset(&$asset, &$o, $prefix)
	{
		$link_value = 'thesaurus';
		$root_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, 'asset', false, $link_value);

		$write_access = $asset->writeAccess('links');

		if (!$write_access) {
			$root_asset = null;
			if (!empty($root_link)) {
				echo get_asset_tag_line($root_link['minorid']);
			} else {
				echo translate('sch_page_no_thesaurus_asset_selected');
			}
		} else {
			asset_finder($prefix.'_thesaurusid', (!empty($root_link)) ? $root_link['minorid'] : '', Array('thesaurus' => 'D'));
		}

		return $write_access;

	}//end paintThesaurusAsset()


	/**
	* Processes the value input from thesaurus asset selection box
	*
	* @param object	Asset				&$rootid	asset being painted
	* @param object Backend_Outputter	&$o			backend outputter
	* @param string						$prefix		prefix for the html doc element name
	*
	* @return void
	* @access public
	*/
	function processThesaurusAsset(&$asset, &$o, $prefix)
	{
		if (!isset($_POST[$prefix.'_thesaurusid']['assetid'])) {
			return false;
		}
		$new_root  = $_POST[$prefix.'_thesaurusid']['assetid'];

		// getting existing root link
		$link_value = 'thesaurus';
		$root_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, '', false, $link_value);

		// dont process anything if the root node has not changed
		if (!empty($root_link) && $root_link['minorid'] == $new_root) {
			return false;
		}

		$ok = true;

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// deleting old root link if any found...
		if (!empty($root_link) && !$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($root_link['linkid'])) {
			$ok = false;
		} else if ($new_root) {
			$root = &$GLOBALS['SQ_SYSTEM']->am->getAsset($new_root);

			if (!is_null($root)) {
				if (!$asset->createLink($root, SQ_LINK_NOTICE, $link_value)) {
					$ok = false;
					trigger_localised_error('SCH0017', E_USER_WARNING, $root->name, $root->id);
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
	* Get the possible 'sort by' options
	*
	* @return Array(string => string)
	* @access protected
	*/
	function _getSortByOptions(&$asset)
	{
		$options = parent::_getSortByOptions($asset);
		$options['score'] = translate('sch_page_relevance');
		return $options;

	}//end _getSortByOptions()


	/**
	* Return true if the thesaurus asset type is installed
	*
	* @param object Asset        $asset  the asset whose interface we are painting
	* @param string                        $prefix        the prefix for this asset
	*
	* @return boolean
	* @access public
	*/
	function thesaurusInstalled(&$asset, $prefix)
	{
		return $GLOBALS['SQ_SYSTEM']->am->installed('thesaurus');

	}//end thesaurusInstalled()


//--        PROXY SEARCHING        --//


	/**
	* This needs to be overriden because when a type is removed from the search
	* page the type format bodycopy is removed. We need to be able to have type
	* formats for the children of all our proxy asset types.
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
		$proxy_settings = $asset->attr('proxy_types');

		if (isset($_POST[$prefix.'_types'])) {
			$existing_types = $asset->attr('types');

			$proxy_child_types = Array();
			foreach ($proxy_settings as $proxy_type => $child_types) {
				if (!empty($child_types)) {
					$proxy_child_types = array_merge($proxy_child_types, array_keys($child_types));
					$existing_types = array_merge($existing_types, $child_types);
				}
			}

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
					if (in_array($type, $formats) && !in_array($type, $proxy_child_types)) {
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
	* Gets a list of the available asset types that can be use for type formats.
	* The reason it's used is so that assets (such as Search_Page) don't need to
	* override the whole paint/process functions if they've got special asset
	* type logic (Eg. proxy asset types)
	*
	* @param object Asset				&$asset		the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o			the outputter class
	* @param string						$prefix		prefix for the form elements
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
	* Paints the list of current proxy search asset types (when one of these
	* asset returns a search result, the children are listed rather than the
	* proxy itself)
	*
	* @param object	Asset				&$rootid	asset being painted
	* @param object Backend_Outputter	&$o			backend outputter
	* @param string						$prefix		prefix for the html doc element name
	*
	* @return void
	* @access public
	*/
	function paintProxyTypes(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$current_types = $asset->attr('proxy_types');

		$am =& $GLOBALS['SQ_SYSTEM']->am;

		$o->openField(translate('search_page_proxy_asset_types'));
			if (empty($current_types)) {
				echo translate('search_page_no_types_defined');
			} else {
				?>
					<table class="sq-backend-table">
						<tr>
							<td class="sq-backend-table-header" style="width: 200px;"><?php echo translate('search_page_proxy_type_code') ?></td>
							<td class="sq-backend-table-header"><?php echo translate('search_page_proxy_child_type_codes') ?></td>
							<?php
								if ($write_access) {
									?><td class="sq-backend-table-header" style="width: 20px;"><?php echo translate('delete') ?></td><?php
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
											asset_type_chooser($prefix.'_'.$current_type.'_children', true, $options, null, true, true);
										} else {
											$type_names = $am->getTypeInfo(array_keys($type_settings), 'name');
											?><ul style="margin: 1px 15px"><?php
											foreach($type_settings as $type_code => $inherits) {
												echo '<li>';
												echo get_asset_type_icon($type_code);
												echo $type_names[$type_code];
												if ($inherits) {
													echo ' ('.strtolower(translate('cms_listing_inherits_child_types')).')';
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
							}
						?>
					</table>
				<?php
			}
		$o->closeField();

		$search_types = array_keys($asset->attr('types'));
		if ($write_access && !empty($search_types)) {
			$o->openField(translate('search_page_proxy_type_code_new'));
				$types = array_diff($search_types, array_keys($current_types));

				if (empty($types)) {
					echo translate('search_page_proxy_all_types_selected');
				} else {
					echo '&nbsp;';
					$options = Array('' => '');
					$options += $am->getTypeInfo($types, 'name');
					combo_box($prefix.'_new_type', $options, false, count($types));
				}
			$o->closeField();
		}

		return $write_access;

	}//end paintProxyTypes()


	/**
	*
	* @param object	Asset				&$rootid	asset being painted
	* @param object Backend_Outputter	&$o			backend outputter
	* @param string						$prefix		prefix for the html doc element name
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
					$current_types[$current_type][$type_code] = (empty($children['inherit'][$index])) ? false : true;
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


}//end class

?>
