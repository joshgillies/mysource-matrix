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
* $Id: search_page_edit_fns.inc,v 1.8.2.2 2004/02/21 12:43:54 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_PACKAGES_PATH.'/cms/page/page_edit_fns.inc';

/**
* Search_Page_Edit_Fns
*
*     Edit functions for search page
*
* @author  Dominic Wong <dwong@squiz.net>
* @version $Revision: 1.8.2.2 $ - 1.0
* @package MySource_Matrix_Packages
* @subpackage search
*/
class Search_Page_Edit_Fns extends Page_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Search_Page_Edit_fns()
	{
		$this->Page_Edit_Fns();

	}//end constructor


	/**
	* Paints the interface for choosing the root asset.
	*
	* @param object Search_Page			&$asset	the search page asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return void
	* @access public
	* 
	*/
	function paintRootAsset(&$asset, &$o, $prefix) 
	{
		$name = $prefix."_root_asset";
		$am =& $GLOBALS['SQ_SYSTEM']->am;

		$assetid = $asset->attr('root_assetid');
		if ($asset->writeAccess('attributes')) {
			asset_finder($name, $assetid);
		} else {
			if ($assetid > 0) {
				$rootAsset =& $am->getAsset($assetid);
				echo "<a href=\"".$rootAsset->getBackendHref()."\">".$rootAsset->name."</a>";
			} else {
				echo "No root asset specified.";
			}
		}

	}//end paintRootAsset()


	/**
	* Processes the interface for choosing the root asset.
	*
	* @param object Search_Page			&$asset	the search page asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	* 
	*/
	function processRootAsset(&$asset, &$o, $prefix)
	{
		$name = $prefix."_root_asset";
		$assetid = null;

		if (isset($_REQUEST[$name])) {
			$assetid = $_REQUEST[$name]['assetid'];
		}
		return $asset->setAttrValue('root_assetid', $assetid);

	}//end processRootAsset()


	/**
	* Paints the interface for restricting the asset types returned.
	*
	* @param object Search_Page			&$asset	the search page asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return void
	* @access public
	* 
	*/
	function paintAssetTypeRestriction(&$asset, &$o, $prefix)
	{
		?>Implement me!<?php

	}//end paintAssetTypeRestriction()


	/**
	* Processes the interface for restricting the asset types returned.
	*
	* @param object Search_Page			&$asset	the search page asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	* 
	*/
	function processAssetTypeRestriction(&$asset, &$o, $prefix)
	{

	}//end processAssetTypeRestriction()


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

		$queryTypes = Array(
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
			foreach (array_values($fields) as $fieldName => $fieldData) {
				echo "<tr>";
				if ($asset->writeAccess('attributes')) {
					?>
					<tr>
						<td class="sq-backend-data"><?php text_box($prefix.'_fields[' .$i. '][name]', $fieldData['name'])?></td>
						<td class="sq-backend-data"><?php combo_box($prefix.'_fields[' .$i .'][type]', $queryTypes, false, $fieldData['type'])?></td>
						<td class="sq-backend-data"><?php $this->paintFieldParameters($asset, $prefix."_fields[$i]", $fieldData)?></td>
						<td class="sq-backend-data"><?php check_box($prefix.'_deleted_fields['. $i. ']') ?></td>
					</tr>
					<?php
				} else {
					?>
					<tr>
						<td class="sq-backend-data"><?php echo $fieldData['name'] ?></td>
						<td class="sq-backend-data"><?php echo $queryTypes[$fieldData['type']] ?></td>
						<td class="sq-backend-data"><?php $this->paintFieldParameters($asset, $prefix."_fields[$i]", $fieldData)?></td>
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
			echo combo_box($prefix.'_fields['. $i .'][type]', $queryTypes, false, 'include_all');
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
			foreach ($fields_post as $i => $fieldData) {
				if (isset($deleted_fields[$i]))
					continue;
				if($fieldData['name'] == '')
					continue;
				if (isset($fields[$fieldData['name']]))
					continue;

				$fields[$fieldData['name']] = $fieldData;
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
					<td class="sq-backend-table-cell"><?php echo $type; ?></td>
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

				$system_types = & $GLOBALS['SQ_SYSTEM']->am->getTypeList();
				$systemTypes = Array();
				$systemTypes['none'] = '- Select asset type -'; // default value

				foreach ($system_types as $sys_key => $sys_val) {
					if (!in_array($sys_val, $search_types)) {
						$systemTypes[$sys_val] = $sys_val; // adding only if it's not already there
					}
				}

				// displaying combo boxes...
				if ($asset->writeAccess('attributes')) {
					if (!empty($systemTypes)){
						combo_box($prefix.'_new_type', $systemTypes, false, 'none'); 
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
			foreach ($_POST[$prefix.'_delete_type'] as $type_key) {
				unset($search_types[$type_key]);
			}
			$update = true;
		}

		// 2). now get the new submitted values (if any) and add them to array

		if (!empty($_POST[$prefix.'_new_type']) && $_POST[$prefix.'_new_type'] != 'none') {
			$new_type  = trim($_POST[$prefix.'_new_type']);
			$search_types[$new_type] = $new_type;
			$update = true;
		}
			
		// 3). Updating asset attribute value
		if ($update) {
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
		  if ($field['type'] == 'asset_attrib') {

			// [0] - name, [1] - type, [2] - id
			$attr_parts = explode("__", $field['params']['type_attr']);
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
	* @param string						$fieldPrefix	The field prefix.
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	* 
	*/
	function paintFieldParameters(&$asset, $fieldPrefix, $fieldData)
	{
		switch ($fieldData['type']) {
	
			case 'metadata':
				if (!isset($fieldData['params']['metadata_field']['assetid'])) {
					$metadata_field = 0;
				} else {
					$metadata_field = $fieldData['params']['metadata_field']['assetid'];
				}
				if ($asset->writeAccess('attributes')) {
					asset_finder($fieldPrefix. '[params][metadata_field]', $metadata_field, Array('metadata_field' => 'D'));
				} else {
					if ($metadata_field > 0) {
						$fieldAsset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($metadata_field);
						echo $fieldAsset->name . ' #'. $fieldAsset->id;
					} else {
						echo 'No metadata field specified.';
					}
				}
			break;
			
			case 'asset_attrib': 
				// printing out attribute selection combo box
				if (!isset($fieldData['params']['searchtype'])) {
					$search_type = '';
				} else {
					$search_type = $fieldData['params']['searchtype'];
				}

				// getting array of searchable asset types from page attribute
				$search_types = $asset->attr('search_types');
				
				// getting the values for available types list
				$avail_asset_types = & $GLOBALS['SQ_SYSTEM']->am->getTypeList();
				$assetTypes = Array();
				foreach ($avail_asset_types as $type_key => $type_val) {
					if (in_array($type_val, $search_types)) { // adding only searchable attributes
						$assetTypes[$type_val] = $type_val; // creating assoc array out of normal one
					}
				}
				
				// displaying combo boxes...
				if ($asset->writeAccess('attributes')) {

					// not printing the combo box unless there is something to show
					if (!empty($assetTypes)){
						combo_box($fieldPrefix.'[params][searchtype]', $assetTypes, false, $search_type); 
					}

					// printing out attribute selection combo box
					if (!isset($fieldData['params']['type_attr']) || $search_type == '') {
						$selected_attr = '';
					} else {
						$selected_attr = $fieldData['params']['type_attr'];
					}

					// not printing the combo box unless there is something to show
					if ($search_type != '') {

					$type_attrs = & $GLOBALS['SQ_SYSTEM']->am->getAssetTypeAttributes($search_type, Array('attributeid', 'name', 'type'));

						echo " <b>Attr:</b> ";
						$typeAttrs = Array();
						$typeAttrs[''] = "- attribute -";
						foreach ($type_attrs as $attr_id => $attr_params) {
							// we could use $attr_type to check if it's wanted here
							$attr_name = $attr_params['name'];
							$attr_type = $attr_params['type'];
							$attr_ind = $attr_name . "__" . $attr_type . "__" . $attr_id;
							$typeAttrs[$attr_ind] = $attr_name;
						}

						combo_box($fieldPrefix.'[params][type_attr]', $typeAttrs, false, $selected_attr); 
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