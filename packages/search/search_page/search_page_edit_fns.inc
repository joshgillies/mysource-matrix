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
* $Id: search_page_edit_fns.inc,v 1.29 2004/09/14 01:48:21 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_CORE_PACKAGE_PATH.'/page/page_edit_fns.inc';

/**
* Search_Page_Edit_Fns
*
*     Edit functions for search page
*
* @author  Dominic Wong <dwong@squiz.net>
* @version $Revision: 1.29 $ - 1.0
* @package MySource_Matrix_Packages
* @subpackage search
*/
class Search_Page_Edit_Fns extends Page_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Search_Page_Edit_Fns()
	{
		$this->Page_Edit_Fns();
		$this->static_screens['details']['lock_type'] = 'content';
		$this->static_screens['details']['force_unlock'] = false;

	}//end Search_Page_Edit_Fns()


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

		if ($read_only) $write_access = false; 
		else $write_access = $asset->writeAccess('links');

		if (!$write_access) {
			$root_asset = null;
			if (!empty($root_link)) $root_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($root_link['minorid']);
			if (!is_null($root_asset)) echo $root_asset->name . ' (Id: #' . $root_asset->id . ')';
			else echo 'No root node has been selected';
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
	function processRootAsset(&$asset, &$o, $prefix, $category='')
	{
		if (!isset($_POST[$prefix.'_rootid']['assetid'])) return false;
		$new_root  = $_POST[$prefix.'_rootid']['assetid'];

		// getting existing root link
		$link_value = 'root';
		if (!empty($category)) $link_value .= '_'.$category;
		$root_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, '', false, $link_value);

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
				if (!$asset->createLink($root, SQ_LINK_NOTICE, $link_value)) {
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

	}//end processRootAsset()


	/**
	* Paint the interface for selecting searched statuses
	*
	* @param object Asset				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	* @param string						$category	the category we are painting for
	*												(if blank we paint for the whole search page)
	* @param boolean					$read_only	force read only view
	*
	* @return boolean
	* @access public
	*/
	function paintStatus(&$asset, &$o, $prefix, $category='', $read_only=false)
	{
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		if ($read_only) $write_access = false; 
		else $write_access = $asset->writeAccess('links');

		if (empty($category)) {
			$current = $asset->attr('search_status');
		} else {
			$categories = $asset->attr('categories');
			$current = $categories[$category]['search_status'];
		}

		if ($write_access) {
			require_once SQ_LIB_PATH.'/html_form/html_form.inc';
			$statuses = get_constant_values('SQ_STATUS');

			$options = Array();
			foreach ($statuses as $constant => $value) $options[$value] = get_status_description($value);

			combo_box($prefix.'_statuses', $options, true, $current);
		} else {
			if (empty($current)) {
				echo 'No statuses are selected. Only LIVE assets will be searched.';
			} else {
				echo '<ul>';
				foreach ($current as $status) echo '<li>'.get_status_description($status).'</li>';
				echo '</ul>';
			}
		}
		
		return $write_access;

	}//end paintStatus()


	/**
	* Processes the interface for selecting searched statuses
	*
	* @param object Asset				&$asset	the asset to which we belong
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processStatus(&$asset, &$o, $prefix)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('attributes')) return false;

		$statuses = Array();
		if (isset($_POST[$prefix.'_statuses'])) {
			foreach ($_POST[$prefix.'_statuses'] as $status) {
				$statuses[] = $status;
			}
			$statuses = array_unique($statuses);
		}

		return $asset->setAttrValue('search_status', $statuses);

	}//end processStatus()


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
		$write_access = $asset->writeAccess('attributes');
		$fields = $asset->attr('fields');

		$query_types = Array(
			'include_all'  => 'Include all',
			'exclude'      => 'Exclude',
			'asset_attrib' => 'Asset attribute',
			'asset_date'   => 'Asset date',
			'metadata'     => 'Metadata'
		);

		$i = 0;
		
		$o->openSection('Standard Search Fields');
		$o->openField('');
		?>
		<table class="sq-backend-table">
			<tr>
				<td class="sq-backend-table-header" style="width: 200px;">Name</td>
				<td class="sq-backend-table-header">Type</td>
				<?php
				if ($write_access) {
					?><td class="sq-backend-table-header" style="width: 200px;">Delete ?</td><?php
				}
				?>
			</tr>
			<?php
			foreach ($fields as $field_name => $field_data) {
				if ($field_data['type'] != 'include_all' && $field_data['type'] != 'exclude') continue;
				?>
				<tr>
					<td class="sq-backend-data" style="padding: 4px;">
						<?php
							if ($write_access) text_box($prefix.'_fields['.$i.'][name]', $field_name);
							else echo $field_name;
						?>
					</td>
					<td class="sq-backend-data"><?php echo $query_types[$field_data['type']] ?></td>
					<?php
					if ($write_access) {
						?>
						<td class="sq-backend-data">
							<?php
								hidden_field($prefix.'_fields['.$i.'][type]', $field_data['type']);
								check_box($prefix.'_deleted_fields['.$i.']')
							?>
						</td>
						<?php
					}
					?>
				</tr>
				<?php
				$i++;
			}//end foreach field
			?>
		</table>
		<?php
	
		$o->closeField();
		$o->closeSection();
		$o->openSection('Attribute Search Fields');
		$o->openField('');

		?>
		<table class="sq-backend-table">
			<tr>
				<td class="sq-backend-table-header" style="width: 200px;">Name</td>
				<td class="sq-backend-table-header">Asset Type</td>
				<td class="sq-backend-table-header">Attribute</td>
				<?php
				if ($write_access) {
					?><td class="sq-backend-table-header" style="width: 200px;">Delete ?</td><?php
				}
				?>
			</tr>
			<?php
			foreach ($fields as $field_name => $field_data) {
				if ($field_data['type'] != 'asset_attrib') continue;
				?>
				<tr>
					<td class="sq-backend-data" style="padding: 4px;">
						<?php
							if ($write_access) text_box($prefix.'_fields['.$i.'][name]', $field_name);
							else echo $field_name;
						?>
					</td>
					<td class="sq-backend-data">
						<?php
						if (!isset($field_data['params'])) $field_data['params'] = Array();
						$search_types = $asset->attr('search_types');
						$search_type = array_get_index($field_data['params'], 'asset_type', '0');
		
						if (!empty($search_types)) {
							if ($write_access) {
								$options = Array('' => '- type -');
								foreach ($search_types as $type) $options[$type] = $type;
								combo_box($prefix.'_fields['.$i.'][params][asset_type]', $options, false, $search_type);
							} else {
								echo $search_type;
							}
						}
						?>
					</td>
					<td>
						<?php
						// printing out attribute selection combo box
						if (!isset($field_data['params']['attrid']) || empty($search_type)) {
							$selected_attr = '';
						} else {
							$selected_attr = $field_data['params']['attrid'];
						}
		
						if (!empty($search_type)) {
							
							$asset_type_attrs = $GLOBALS['SQ_SYSTEM']->am->getAssetTypeAttributes($search_type, Array('attributeid', 'name'));
		
							if ($write_access) {
								$type_attrs = Array('' => '- attribute -');
								foreach ($asset_type_attrs as $attr_id => $attr_name) {
									$type_attrs[$attr_id] = $attr_name;
								}
								combo_box($prefix.'_fields['.$i.'][params][attrid]', $type_attrs, false, $selected_attr);
							} else {
								echo $asset_type_attrs[$selected_attr];
							}
						}
						?>
					</td>
					<?php
					if ($write_access) {
						?>
						<td class="sq-backend-data">
							<?php
								hidden_field($prefix.'_fields['.$i.'][type]', $field_data['type']);
								check_box($prefix.'_deleted_fields['.$i.']')
							?>
						</td>
						<?php
					}
					?>
				</tr>
				<?php
				$i++;
			}//end foreach fields
			?>
		</table>
		<?php


		$o->closeField();
		$o->closeSection();
		$o->openSection('Asset Date Search Fields');
		$o->openField('');

		?>
		<table class="sq-backend-table">
			<tr>
				<td class="sq-backend-table-header" style="width: 200px;">Name</td>
				<td class="sq-backend-table-header">Date Type</td>
				<?php
				if ($write_access) {
					?><td class="sq-backend-table-header" style="width: 200px;">Delete ?</td><?php
				}
				?>
			</tr>
			<?php
			foreach ($fields as $field_name => $field_data) {
				if ($field_data['type'] != 'asset_date') continue;
				?>
				<tr>
					<td class="sq-backend-data" style="padding: 4px;">
						<?php
							if ($write_access) text_box($prefix.'_fields['.$i.'][name]', $field_name);
							else echo $field_name;
						?>
					</td>
					<td class="sq-backend-data">
						<?php
						if (!isset($field_data['params'])) $field_data['params'] = Array();
						$date_types = Array(''          => '-- select date --',
											'created'   => 'Created',
											'updated'   => 'Updated',
											'published' => 'Published',
											);
						$date_type = array_get_index($field_data['params'], 'date_type', '');
		
						if ($write_access) {
							combo_box($prefix.'_fields['.$i.'][params][date_type]', $date_types, false, $date_type);
						} else {
							echo $date_types[$date_type];
						}
						?>
					</td>
					<?php
					if ($write_access) {
						?>
						<td class="sq-backend-data">
							<?php
								hidden_field($prefix.'_fields['.$i.'][type]', $field_data['type']);
								check_box($prefix.'_deleted_fields['.$i.']')
							?>
						</td>
						<?php
					}
					?>
				</tr>
				<?php
				$i++;
			}//end foreach fields
			?>
		</table>
		<?php
	
		$o->closeField();
		$o->closeSection();
		$o->openSection('Metadata Search Fields');
		$o->openField('');

		?>
		<table class="sq-backend-table">
			<tr>
				<td class="sq-backend-table-header" style="width: 200px;">Name</td>
				<td class="sq-backend-table-header">Metadata Field</td>
				<?php
				if ($write_access) {
					?><td class="sq-backend-table-header" style="width: 200px;">Delete ?</td><?php
				}
				?>
			</tr>
			<?php
			foreach ($fields as $field_name => $field_data) {
				if ($field_data['type'] != 'metadata') continue;
				?>
				<tr>
					<td class="sq-backend-data" style="padding: 4px;">
						<?php
							if ($write_access) text_box($prefix.'_fields['.$i.'][name]', $field_name);
							else echo $field_name;
						?>
					</td>
					<td class="sq-backend-data">
						<?php
						if (!isset($field_data['params'])) $field_data['params'] = Array();
						$metadata_field = array_get_index($field_data['params'], 'assetid', '0');

						if ($write_access) {
							asset_finder($prefix.'_fields['.$i.'][params][metadata_field]', $metadata_field, Array('metadata_field' => 'D'));
						} else {
							if ($metadata_field > 0) {
								$field_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($metadata_field);
								if (!is_null($metadata_field)) echo $field_asset->name.' #'.$field_asset->id;
								else echo 'No metadata field specified';
							} else {
								echo 'No metadata field specified';
							}
						}
						?>
					</td>
					<?php
					if ($write_access) {
						?>
						<td class="sq-backend-data">
							<?php
								hidden_field($prefix.'_fields['.$i.'][type]', $field_data['type']);
								check_box($prefix.'_deleted_fields['.$i.']')
							?>
						</td>
						<?php
					}
					?>
				</tr>
				<?php
				$i++;
			}//end foreach fields
			?>
		</table>
		<br />
		<?php

		$o->closeField();
		$o->closeSection();
		if ($asset->writeAccess('attributes')) {
			$o->closeField();
			$i++;
			$o->openField('Add Field');
				echo 'Name: ';
				text_box($prefix.'_fields['.$i.'][name]', '', 15);
				echo ' Type: ';
				combo_box($prefix.'_fields['.$i.'][type]', $query_types, false, 'include_all');
			// field closed by asset_edit_interface
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
		if (!$asset->writeAccess('attributes')) return false;

		$fields_post = array_get_index($_POST, $prefix.'_fields', Array());
		$deleted_fields = array_get_index($_POST, $prefix.'_deleted_fields', Array());

		$fields = Array();
		if (!empty($fields_post)) {
			foreach ($fields_post as $i => $field_data) {
				if (isset($deleted_fields[$i])) continue;          // remove deleted fields
				if ($field_data['name'] == '') continue;           // remove fields with no name
				if (isset($fields[$field_data['name']])) continue; // ensure no duplicate names
				
				// metadata needs to rework the params array
				if ($field_data['type'] == 'metadata') {
					$fieldid = 0;
					if (isset($field_data['params'])) {
						$fieldid = array_get_index($field_data['params']['metadata_field'], 'assetid', '');
					}
					$field_data['params'] = Array('assetid' => $fieldid);
				}

				$fields[$field_data['name']] = $field_data;
			}
		}

		return $asset->setAttrValue('fields', $fields);

	}//end processFields()


	/**
	* Paints the interface for adding and configuring searchable asset types
	*
	* @param object Search_Page			&$asset	the search page asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*
	*/
	function paintTypes(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$search_types = $asset->attr('search_types');

		if (!empty($search_types)) {

				?>
				<table class="sq-backend-table">
					<tr>
						<td class="sq-backend-table-header">Searchable Asset Type</td>
						<?php
						if ($write_access) {
							?><td class="sq-backend-table-header">Delete ?</td><?php
						}
						?>
					</tr>
				<?php

				foreach ($search_types as $type) {
					?>
					<tr>
						<td class="sq-backend-table-cell">
							<?php echo $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type, 'name'); ?>
						</td>
						<?php
						if ($write_access) {
							?>
							<td class="sq-backend-table-cell">
								<?php check_box($prefix.'_delete_type[]', $type); ?>
							</td>
							<?php
						}
						?>
					</tr>
					<?php
				}

				?></table><?php

		} else {
			echo 'There are currently no searchable asset types';
		}

		// selection field for searchable asset types
		if ($write_access) {
			$o->openField('Add asset type');

				$asset_types = array_values(array_diff($GLOBALS['SQ_SYSTEM']->am->getTypeList(), $search_types));
				$asset_type_list = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($asset_types, 'name');
				asort($asset_type_list);

				// displaying combo boxes...
				if ($asset->writeAccess('attributes')) {
					if (!empty($asset_type_list)){
						combo_box($prefix.'_new_type', Array('' => '- Select asset type -') + $asset_type_list, false, 'none');
					}
				}

			$o->closeField();
		}

		return true;

	}//end paintTypes


	/**
	* Processes the interface for adding and configuring searchable asset types.
	*
	* @param object Search_Page			&$asset	the search page asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*
	*/
	function processTypes(&$asset, &$o, $prefix)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('attributes')) return false;

		$search_types = $asset->attr('search_types');

		$update = false;

		// deleting checked items
		if (isset($_POST[$prefix.'_delete_type'])) {
			$search_types = array_values(array_diff($search_types, $_POST[$prefix.'_delete_type']));
			$update = true;
		}

		// now get the new submitted values (if any) and add them to array
		if (!empty($_POST[$prefix.'_new_type']) && $_POST[$prefix.'_new_type'] != '') {
			$new_type = trim($_POST[$prefix.'_new_type']);
			if (!in_array($new_type, $search_types)) {
				$search_types[] = $new_type;
				$update = true;
			}
		}

		// updating asset attribute value
		if ($update) {
			sort($search_types);
			$asset->setAttrValue('search_types', $search_types);
		}

		return true;

	}//end processTypes()


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

			if ($field['type'] == 'asset_attrib') {
	
				if (!isset($field['params']['attrid'])) continue;
				$attribute = &$GLOBALS['SQ_SYSTEM']->am->getAttribute($field['params']['attrid']);
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
			if ($field['type'] == 'asset_attrib') {
				if (empty($field['params']['attrid'])) continue;
				$attribute = &$GLOBALS['SQ_SYSTEM']->am->getAttribute($field['params']['attrid']);
				$search_styles[$attribute->id] = $attribute->processEditParams($prefix.'_'.$attribute->id);
			}
		}

		$asset->setAttrValue('search_styles', $search_styles);

		return true;

	}//end processTypes()
	
	
	/**
	* Paints the interface for selecting a search category to customise
	*
	* @param object Search_Page			&$asset	the search page asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*
	*/
	function paintChooseCategory(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('menu');
		$categories = $asset->attr('categories');

		if (isset($_REQUEST[$prefix.'_select_category'])) {
			$selected = $_REQUEST[$prefix.'_select_category'];
		} else {
			$selected = (isset($_REQUEST[$prefix.'_category'])) ? $_REQUEST[$prefix.'_category'] : '';
		}

		$options = Array();
		if ($write_access) $options[''] = '-- add new category --';
		else $options[''] = '-- select a category --';

		foreach ($categories as $name => $data) {
			if (empty($name)) continue;
			$options[$name] = $data['name'];
		}
		combo_box($prefix.'_select_category', $options, false, $selected);
		$o->addFormActionGetVar($prefix.'_category', $selected, true);
		submit_button($prefix.'_go', 'Go');

		if (isset($_REQUEST[$prefix.'_select_category']) && !empty($_REQUEST[$prefix.'_select_category'])) {
			echo '<br/><br/><b>Currently customising '.$options[$_REQUEST[$prefix.'_select_category']].'</b>';
		}

		return false;

	}//end paintChooseCategory()
	
	
	/**
	* Processes the interface for selecting a search category to customise
	*
	* @param object Search_Page			&$asset	the search page asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*
	*/
	function processChooseCategory(&$asset, &$o, $prefix)
	{
		return false;

	}//end processChooseCategory()
	
	
	/**
	* Paints the interface for editing a search category
	*
	* @param object Search_Page			&$asset	the search page asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*
	*/
	function paintCustomiseCategory(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('menu');

		if (isset($_REQUEST[$prefix.'_select_category'])) {
			$selected = $_REQUEST[$prefix.'_select_category'];
		} else {
			$selected = (isset($_REQUEST[$prefix.'_category'])) ? $_REQUEST[$prefix.'_category'] : '';
		}

		if (!empty($selected)) {

			$categories = $asset->attr('categories');

			// field opened by asset_edit_interface
				if ($write_access) {
					text_box($prefix.'_category_name', $categories[$selected]['name'], 50);
				} else {
					echo $categories[$selected]['name'];
				}
			$o->closeField();
			
			if ($write_access) {
				$o->openField('Delete ?');
					check_box($prefix.'_delete_category');
					echo 'Delete this category';
				$o->closeField();
			}
			
			$o->openSection('Category Specific Search Settings');
				$o->openField('Root Asset');
					$using_default = !isset($categories[$selected]['root_assetid']);
					if ($using_default) echo 'Currently using default root node <b>';
					$category = ($using_default) ? '' : $selected;
					$this->paintRootAsset($asset, $o, $prefix, $category, $using_default);
					if ($using_default) echo '</b>';
					
					if ($write_access) {
						check_box($prefix.'_default_root_assetid', '1', $using_default);
						echo ' Use default';
					}
				$o->closeField();
				
				$o->openField('Search Statuses');
					$using_default = !isset($categories[$selected]['search_status']);
					if ($write_access) {
						check_box($prefix.'_default_search_status', '1', $using_default);
						echo ' Use default<br />';
					}
					if ($using_default) echo 'Currently using default stauses of';
					$category = ($using_default) ? '' : $selected;
					$this->paintStatus($asset, $o, $prefix, $category, $using_default);
				$o->closeField();
			$o->closeSection();

		} else {
			if ($write_access) {
				text_box($prefix.'_category_name', '', 50);
			} else {
				echo 'Please select a search category above to customise';
			}

		}//end if !empty()
		
		return $write_access;

	}//end paintCustomiseCategory()
	
	
	/**
	* Processes the interface for editing a search category
	*
	* @param object Search_Page			&$asset	the search page asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*
	*/
	function processCustomiseCategory(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('menu')) return false;

		$categories = $asset->attr('categories');

		if (isset($_REQUEST[$prefix.'_category'])) {
			$category = $_REQUEST[$prefix.'_category'];
			
			if (!empty($category)) {
			
				// delete the category ?
				if (isset($_POST[$prefix.'_delete_category']) && $_POST[$prefix.'_delete_category']) {
					if (isset($categories[$category])) {
						// delete the root node link
						$_POST[$prefix.'_rootid']['assetid'] = '';
						$this->processRootAsset($asset, $o, $prefix, $category);
						unset($categories[$category]);
						$_REQUEST[$prefix.'_select_category'] = '';
					}
					
				} else {
				
					// change the name of the category ?
					if (isset($_POST[$prefix.'_category_name'])) {
						$new_name = $_POST[$prefix.'_category_name'];
						if ($new_name != $categories[$category]['name']) {
							$old_key = str_replace(' ', '_', strtolower($categories[$category]['name']));
							$new_key = str_replace(' ', '_', strtolower($new_name));
							if ($old_key != $new_key) {
								$categories[$new_key] = $categories[$old_key];
								unset($categories[$old_key]);
								
								// change the root asset link
								$root_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, '', false, 'root_'.$category);
								if (!empty($root_link)) {
									$GLOBALS['SQ_SYSTEM']->am->updateLink($root_link['linkid'], null, 'root_'.$new_key, $root_link['sort_order']);
								}
								$category = $new_key;
								$_REQUEST[$prefix.'_select_category'] = $new_key;
							}
						}//end if name changed
						$categories[$category]['name'] = $new_name;
					}//end if category name set
					
					$defaults = Array('root_assetid', 'search_status');
					foreach ($defaults as $default) {
						$using_default = false;
						if (!isset($_POST[$prefix.'_default_'.$default])) {
							if (!isset($categories[$category][$default])) {
								if ($default == 'root_assetid') {
									$categories[$category][$default] = '';
								} else {
									$categories[$category][$default] = $asset->attr($default);
								}
								$using_default = true;
							}
						} else {
							$using_default = true;
							if (isset($categories[$category][$default])) unset($categories[$category][$default]);
							if ($default == 'root_assetid') {
								$_POST[$prefix.'_rootid']['assetid'] = '';
								$this->processRootAsset($asset, $o, $prefix, $category);
							}
						}
		
						if (!$using_default) {
							$new_value = '';
							switch ($default) {
								case 'root_assetid' :
									$this->processRootAsset($asset, $o, $prefix, $category);
								break;
			
								case 'search_status' :
									$new_value = Array();
									if (isset($_POST[$prefix.'_statuses'])) {
										foreach ($_POST[$prefix.'_statuses'] as $status) {
											$new_value[] = $status;
										}
										$new_value = array_unique($new_value);
									}
								break;
							}//end switch
			
							$categories[$category][$default] = $new_value;
						}
					}
	
					if (isset($categories[$category]) && empty($categories[$category])) unset($categories[$category]);
					if (isset($categories[$category]['name']) && empty($categories[$category]['name'])) unset($categories[$category]);

				}//end if delete category

			} else {
				if (isset($_POST[$prefix.'_category_name'])) {
					$new_name = trim($_POST[$prefix.'_category_name']);
					if (!empty($new_name)) {
						$new_key = str_replace(' ', '_', strtolower($new_name));
						$categories[$new_key] = Array('name' => $new_name);
						$_REQUEST[$prefix.'_select_category'] = $new_key;
					}
				}
			}//end if category is not empty
			
			if (isset($categories[''])) unset($categories['']);
			return $asset->setAttrValue('categories', $categories);
		}
		return false;

	}//end processCustomiseCategory()


	
	
	
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

		$search_bodycopy_link = &$asset->getFormatBodycopyLink('search_form');
		$sis_bodycopy_link    = &$asset->getFormatBodycopyLink('sis_form'); //search in search
		$search_customised    = !empty($search_bodycopy_link) && $search_bodycopy_link['link_type'] != SQ_LINK_TYPE_3;
		$sis_customised       = !empty($sis_bodycopy_link) && $sis_bodycopy_link['link_type'] != SQ_LINK_TYPE_3;
		
		?>
			<table>
				<tr>
					<td>Customise search form</td>
					<td>
						<?php 
							if (!$write_access) {
								echo ($search_customised) ? 'Yes' : 'No';
							} else {
								check_box($prefix.'_search_form', 1, $search_customised);
							}
						?>
					</td>
				</tr>
				<tr>
					<td>Customise search within search form</td>
					<td>
						<?php 
							if (!$write_access) {
								echo ($sis_customised) ? 'Yes' : 'No';
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
			'Default Search'   => 'search_form',
			'Search In Search' => 'sis_form'
		);

		$bodycopies_visible = false;

		foreach ($forms as $form_name => $value) {
			if (isset($_POST[$prefix.'_'.$value])) {
				
				if (is_null($format_folder)) {
					// create the folder if it does not exist
		
					$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
					$GLOBALS['SQ_SYSTEM']->am->includeAsset('folder');
					$format_folder = new Folder();
					$copy_link = Array('asset' => &$asset, 'value' => 'format_folder', 'link_type' => SQ_LINK_TYPE_2, 'dependant' => 1, 'exclusive' => 1);
		
					$format_folder->setAttrValue('name', 'Search Forms');
		
					if (!$format_folder->create($copy_link)) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						trigger_error('Could not create the format folder', E_USER_WARNING);
						return false;
					}
					$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
				}
				
				$bodycopy = &$asset->getFormatBodycopy($value);

				if (is_null($bodycopy)) {

					// if there is no bodycopy here, then we want to create it
					$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
					$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');
					$bodycopy = new Bodycopy();
					$copy_link = Array('asset' => &$format_folder, 'value' => $value, 'link_type' => SQ_LINK_TYPE_2, 'dependant' => 1, 'exclusive' => 1);

					$bodycopy->setAttrValue('name', $form_name);

					if (!$bodycopy->create($copy_link)) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						trigger_error('Could not create the '.$form_name.' bodycopy', E_USER_WARNING);
						return false;
					}
					$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
				} else {
					$bodycopy_link = $asset->getFormatBodycopyLink($value);
					if ($bodycopy_link['link_type'] != SQ_LINK_TYPE_2) {
						if (!$GLOBALS['SQ_SYSTEM']->am->updateLink($bodycopy_link['linkid'], SQ_LINK_TYPE_2)) {
							trigger_error('Could not update the link type for bodycopy '.$form_name, E_USER_WARNING);
							return false;
						}
					}
				}
				$bodycopies_visible = true;

			} else {
				$bodycopy_link = $asset->getFormatBodycopyLink($value);
				if (!empty($bodycopy_link['link_type']) && $bodycopy_link['link_type'] != SQ_LINK_TYPE_3) {
					if (!$GLOBALS['SQ_SYSTEM']->am->updateLink($bodycopy_link['linkid'], SQ_LINK_TYPE_3)) {
						trigger_error('Could not update the link type for bodycopy '.$form_name, E_USER_WARNING);
						return false;
					}
				}
			}//end if isset
		}//end foreach


		// if none of the bodycopies are visible, the hide the folder as well
		$folder_link_type = SQ_LINK_TYPE_3;
		if ($bodycopies_visible) $folder_link_type = SQ_LINK_TYPE_2;
		
		$folder_link = $asset->getFormatFolderLink();
		if (!empty($folder_link['link_type']) && $folder_link['link_type'] != $folder_link_type) {
			if (!$GLOBALS['SQ_SYSTEM']->am->updateLink($folder_link['linkid'], $folder_link_type)) {
				trigger_error('Could not update the link type for the format folder', E_USER_WARNING);
				return false;
			}
		}

		return false;

	}//end processCustomiseSearchForms()


}//end class

?>
