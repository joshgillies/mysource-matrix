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
* $Id: search_page_edit_fns.inc,v 1.26 2004/08/09 10:22:58 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_CORE_PACKAGE_PATH.'/page/page_edit_fns.inc';

/**
* Search_Page_Edit_Fns
*
*     Edit functions for search page
*
* @author  Dominic Wong <dwong@squiz.net>
* @version $Revision: 1.26 $ - 1.0
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

	}//end Search_Page_Edit_Fns()


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
	function paintRootAsset(&$asset, &$o, $prefix)
	{
		$root_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, 'asset', false, 'root');
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

	}//end paintRootAsset()


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
	function processRootAsset(&$asset, &$o, $prefix)
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

	}//end processRootAsset()


	/**
	* Paint the interface for selecting searched statuses
	*
	* @param object Asset				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintStatus(&$asset, &$o, $prefix)
	{
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		$write_access = $asset->writeAccess('attributes');

		$current = $asset->attr('search_status');
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
		} else {
			$statuses = Array();
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
		$fields = $asset->attr('fields');
		if (is_scalar($fields))
			$fields = unserialize($fields);

		$query_types = Array(
			'include_all'	=> 'Include All',
			'asset_attrib'	=> 'Asset attribute', // added for testing new features
			'exclude'		=> 'Exclude',
			'metadata'		=> 'Metadata'
		);

		$i = 0;
		?>
		<table class="sq-backend-table">
			<tr>
				<td class="sq-backend-table-header">Name</td>
				<td class="sq-backend-table-header">Type</td>
				<td class="sq-backend-table-header">Parameters</td>
				<?php
				if ($asset->writeAccess('attributes')) {
					echo "<td class=\"sq-backend-table-header\">Delete?</td>";
				}
				?>
			</tr>
			<?php
			foreach (array_values($fields) as $field_name => $field_data) {
				echo "<tr>";
				if ($asset->writeAccess('attributes')) {
					?>
					<tr>
						<td class="sq-backend-data"><?php text_box($prefix.'_fields[' .$i. '][name]', $field_data['name'])?></td>
						<td class="sq-backend-data"><?php combo_box($prefix.'_fields[' .$i .'][type]', $query_types, false, $field_data['type'])?></td>
						<td class="sq-backend-data"><?php $this->paintFieldParameters($asset, $prefix."_fields[$i]", $field_data)?></td>
						<td class="sq-backend-data"><?php check_box($prefix.'_deleted_fields['. $i. ']') ?></td>
					</tr>
					<?php
				} else {
					?>
					<tr>
						<td class="sq-backend-data"><?php echo $field_data['name'] ?></td>
						<td class="sq-backend-data"><?php echo $query_types[$field_data['type']] ?></td>
						<td class="sq-backend-data"><?php $this->paintFieldParameters($asset, $prefix."_fields[$i]", $field_data)?></td>
					</tr>
					<?php
				}
				echo "</li>";
				$i++;
			}
			?>
		</table>
		<br />
		<?php

		if ($asset->writeAccess('attributes')) {
			$i++;
			?>Add field (name): <?php
			text_box($prefix."_fields[$i][name]", '', 15);
			?><br />Query type: <?php
			echo combo_box($prefix.'_fields['. $i .'][type]', $query_types, false, 'include_all');
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
		if (!$asset->writeAccess('attributes')) {
			return false;
		}
		$changed = false;
		$fields = $asset->attr('fields');

		if (is_scalar($fields))
			$fields = unserialize($fields);

		$fields_post =& $_POST[$prefix.'_fields'];

		$deleted_fields =& $_POST[$prefix.'_deleted_fields'];

		$fields = Array();
		//return $asset->setAttrValue('fields', $fields);

		if(isset($fields_post) && count($fields_post) > 0) {
			foreach ($fields_post as $i => $field_data) {
				if (isset($deleted_fields[$i]))
					continue;
				if($field_data['name'] == '')
					continue;
				if (isset($fields[$field_data['name']]))
					continue;

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
					<td class="sq-backend-table-cell"><?php echo $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type, 'name'); ?></td>
					<?php
					if ($write_access) {
						?><td class="sq-backend-table-cell"><?php check_box($prefix.'_delete_type[]', $type); ?></td><?php
					}
					?>
				</tr>
				<?php

				} // end foreach select_options

				?></table><?php

		} else {
			echo 'There are currently no searchable asset types';
		} // end if empty

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

		// 1). Deleting checked items
		if (isset($_POST[$prefix.'_delete_type'])) {
			$search_types = array_values(array_diff($search_types, $_POST[$prefix.'_delete_type']));
			$update = true;
		}

		// 2). now get the new submitted values (if any) and add them to array
		if (!empty($_POST[$prefix.'_new_type']) && $_POST[$prefix.'_new_type'] != '') {
			$new_type = trim($_POST[$prefix.'_new_type']);
			if (!in_array($new_type, $search_types)) {
				$search_types[] = $new_type;
				$update = true;
			}
		}

		// 3). Updating asset attribute value
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
		$search_styles = $asset->attr('search_styles'); // current settings with attr names
		$fields = $asset->attr('fields');

		foreach ($fields as $field) {

		  if ($field['type'] == 'asset_attrib' && isset($field['params']['type_attr'])) {

			// [0] - name, [1] - type, [2] - id
			$attr_parts = explode("__", $field['params']['type_attr']);
			if (empty($attr_parts[1]) || empty($attr_parts[2])) continue;

			require_once SQ_ATTRIBUTES_PATH.'/'.$attr_parts[1].'/'.$attr_parts[1].'.inc';
			$attr_class = 'Asset_Attribute_'.$attr_parts[1]; // attr name
			$attribute = new $attr_class($attr_parts[2], null); // attr id

			if (!empty($search_styles) && isset($search_styles[$attribute->name])) {
				$attribute->_edit_params = $search_styles[$attribute->name];
			}

			$attribute->paintEditParams($prefix, $write_access);
		  }
		}

		return true;

	}//end paintStyles


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
		//if (!$asset->writeAccess('attributes')) return false;

		$search_styles = Array(); // assembling the new styles array

		$fields = $asset->attr('fields');

		foreach ($fields as $fieldname => $field) {
		  if ($field['type'] == 'asset_attrib') {

			if (empty($field['params']['type_attr'])) continue;

			// [0] - name, [1] - type, [2] - id
			$attr_parts = explode("__", $field['params']['type_attr']);
			require_once SQ_ATTRIBUTES_PATH.'/'.$attr_parts[1].'/'.$attr_parts[1].'.inc';
			$attr_class = 'Asset_Attribute_'.$attr_parts[1]; // attr name
			$attribute = new $attr_class($attr_parts[2], null); // attr id

			// processing attrs
			$result = $attribute->processEditParams($prefix);

			// add result to the array which will be saved as $search_styles
			$search_styles = array_merge($search_styles, $result);
		  }
		}

		$asset->setAttrValue('search_styles', $search_styles);

		return true;

	}//end processTypes()


	/**
	* Paints the interface for the fields' parameters.
	*
	* @param object Search_Page			&$asset			the search page asset
	* @param string						$field_prefix	The field prefix
	* @param string						$field_data		field data passed from the caller
	*
	* @return boolean
	* @access public
	*
	*/
	function paintFieldParameters(&$asset, $field_prefix, $field_data)
	{
		switch ($field_data['type']) {

			case 'metadata':
				if (!isset($field_data['params']['metadata_field']['assetid'])) {
					$metadata_field = 0;
				} else {
					$metadata_field = $field_data['params']['metadata_field']['assetid'];
				}
				if ($asset->writeAccess('attributes')) {
					asset_finder($field_prefix. '[params][metadata_field]', $metadata_field, Array('metadata_field' => 'D'));
				} else {
					if ($metadata_field > 0) {
						$field_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($metadata_field);
						echo $field_asset->name . ' #'. $field_asset->id;
					} else {
						echo 'No metadata field specified.';
					}
				}
			break;

			case 'asset_attrib':
				// printing out attribute selection combo box
				if (!isset($field_data['params']['searchtype'])) {
					$search_type = '';
				} else {
					$search_type = $field_data['params']['searchtype'];
				}

				// getting array of searchable asset types from page attribute
				$search_types = $asset->attr('search_types');

				// getting the values for available types list
				$avail_asset_types = & $GLOBALS['SQ_SYSTEM']->am->getTypeList();
				$asset_types = Array();
				foreach ($avail_asset_types as $type_key => $type_val) {
					if (in_array($type_val, $search_types)) { // adding only searchable attributes
						$asset_types[$type_val] = $type_val; // creating assoc array out of normal one
					}
				}

				// displaying combo boxes...
				if ($asset->writeAccess('attributes')) {

					// not printing the combo box unless there is something to show
					if (!empty($asset_types)){
						combo_box($field_prefix.'[params][searchtype]', $asset_types, false, $search_type);
					}

					// printing out attribute selection combo box
					if (!isset($field_data['params']['type_attr']) || $search_type == '') {
						$selected_attr = '';
					} else {
						$selected_attr = $field_data['params']['type_attr'];
					}

					// not printing the combo box unless there is something to show
					if ($search_type != '') {

						$asset_type_attrs = & $GLOBALS['SQ_SYSTEM']->am->getAssetTypeAttributes($search_type, Array('attributeid', 'name', 'type'));
						#pre_echo($type_attrs);

						echo " <b>Attr:</b> ";
						$type_attrs = Array();
						$type_attrs[''] = "- attribute -";
						foreach ($asset_type_attrs as $attr_id => $attr_params) {
							// we could use $attr_type to check if it's wanted here
							$attr_name = $attr_params['name'];
							$attr_type = $attr_params['type'];
							$attr_ind = $attr_name . "__" . $attr_type . "__" . $attr_id;
							$type_attrs[$attr_ind] = $attr_name;
						}

						combo_box($field_prefix.'[params][type_attr]', $type_attrs, false, $selected_attr);
					}

				} else {
					if ($search_type != '') {
						echo $search_type;
					} else {
						echo 'No asset type selected';
					}
				}
			break;

			case 'include_all':
			case 'exclude':
			default:
			break;
		}

	}//end paintFieldParameters()


}//end class

?>
