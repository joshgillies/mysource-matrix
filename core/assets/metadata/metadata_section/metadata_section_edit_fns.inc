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
* $Id: metadata_section_edit_fns.inc,v 1.16 2004/02/24 07:37:35 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_CORE_PACKAGE_PATH.'/folder/folder_edit_fns.inc';

/**
* Metadata_Section_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Metadata_Section_Edit_Fns extends Folder_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Metadata_Section_Edit_Fns()
	{
		$this->Folder_Edit_Fns();
		$this->static_screens['details']['force_unlock'] = false;

	}//end constructor


	/**
	* Paint the interface for editing metadata fields
	*
	* @param object Metadata_Section	&$asset			the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o				the outputter class
	*
	* @return boolean
	* @access public
	*/
	function paintInlineOptionInterface(&$asset, &$o)
	{
		$write_access = $asset->writeAccess('links');
		$prefix = $asset->getPrefix();

		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'metadata_field', false, 'major', null, true, true);
		if (!empty($current_links)) {
			$assetids = Array();
			foreach ($current_links as $link) $assetids[$link['minorid']] = 'details';
			$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_keys($assetids));
			$asset_href = $GLOBALS['SQ_SYSTEM']->am->getAssetBackendHref($assetids);

			$o->openField('Fields');
				?>
				<table class="sq-backend-table">
					<tr>
						<td class="sq-backend-table-header"><b>Field Name</b></td>
						<td align="center" width="150" class="sq-backend-table-header"><b>Edit Field</b></td>
						<?php
						if ($write_access) {
							?><td align="center" width="70" class="sq-backend-table-header"><b>Delete ?</b></td><?php
						}
						?>
					</tr>
				<?php
				foreach ($current_links as $link_data) {
					$field = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link_data['minorid']);
					if (is_null($field)) continue;
					$edit_fns = $field->getEditFns();
					?>
					<tr>
						<td align="left" class="sq-backend-table-cell"><?php echo $field->name;?></td>
						<td align="center" class="sq-backend-table-cell">[ <a href="<?php echo $asset_href[$field->id];?>">Go To Edit Interface</a> ]</td>
					<?php
					if ($write_access) {
						?><td align="center" class="sq-backend-table-cell"><?php
						check_box($prefix.'_delete_fields['.$link_data['linkid'].']')
						?></td><?php
					}
					echo '</tr>';
				}
				?></table><?php
			$o->closeField();
		}

		if ($write_access) {
			$o->openField('New Field Name');

				require_once SQ_LIB_PATH.'/html_form/html_form.inc';
				text_box($prefix.'_new_field', '', 20);

			$o->closeField();

			$o->openField('New Field Type');

				// now getting available types list
				$descendant_types = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants('metadata_field');
				
				$contents = Array();
				foreach ($descendant_types as  $type) {
					$contents[$type] = $type;
				}

				combo_box($prefix.'_field_type', $contents, false, 'metadata_field_text');

			$o->closeField();
		}

		return true;

	}//end paintInlineOptionInterface()


	/**
	* Process the interface for editing metadata fields
	*
	* @param object Metadata_Section	&$asset			the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o				the outputter class
	*
	* @return boolean
	* @access public
	*/
	function processInlineOptionInterface(&$asset, &$o)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('links')) return false;

		$prefix = $asset->getPrefix();
		$updated = false;

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// delete any fields that need deleting
		if (isset($_POST[$prefix.'_delete_fields'])) {
			$delete_fields = $_POST[$prefix.'_delete_fields'];
			foreach ($delete_fields as $linkid => $on) {
				if ($asset->deleteLink($linkid)) $updated = true;
			}
		}

		// create a new field?
		if (isset($_POST[$prefix.'_new_field']) && trim($_POST[$prefix.'_new_field']) != '') {
			$new_field_name = trim($_POST[$prefix.'_new_field']);
			$new_field_type = trim($_POST[$prefix.'_field_type']);

			$GLOBALS['SQ_SYSTEM']->am->includeAsset($new_field_type); 

			if ($new_field_type == 'metadata_field_text') { // add more type-checks here

				$new_field =& new Metadata_Field_Text();

			} elseif ($new_field_type == 'metadata_field_select') {

				$new_field =& new Metadata_Field_Select();
			
			} elseif ($new_field_type == 'metadata_field_date') {

				$new_field =& new Metadata_Field_Date();

			} else {
				$new_field =& new Metadata_Field_Text(); // default is standard Metadata Field Text
			}
			
			$section_link = Array('asset' => &$asset, 'link_type' => SQ_LINK_TYPE_2, 'sort_order' => -1, 'dependant' => 1, 'exclusive' => 1);
			$new_field->setAttrValue('name', $new_field_name);
			if ($new_field->create($section_link)) $updated = true;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		return $updated;

		return true;

	}//end processInlineOptionInterface()


	/**
	* Paint the interface for filling in metadata values
	*
	* @param object Metadata_Section	&$asset			the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o				the outputter class
	* @param array						$values			current metadata values set for this asset
	* @param boolean					$write_access	does the current user have write access to the asset
	*													that we are setting metadata for (NOT THE SAME AS $asset)
	*
	* @return boolean
	* @access public
	*/
	function paintInlineValueInterface(&$asset, &$o, $values, $write_access)
	{
		$o->openSection($asset->name);
		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'metadata_field', false, 'major', null, true);
		foreach ($current_links as $link) {
			$field = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
			if (is_null($field)) continue;
			$edit_fns = $field->getEditFns();
			if (!isset($values['fields'][$field->name])) $values['fields'][$field->name] = Array();
			if (!$edit_fns->paintInlineValueInterface($field, $o, $values['fields'][$field->name], $write_access)) return false;
		}
		$o->closeSection();

		return true;

	}//end paintInlineValueInterface()


	/**
	* Process the interface for filling in metadata values
	*
	* @param object Metadata_Section	&$asset			the asset whose interface we are painting
	* @param array						&$new_values	an array we can store the new values in
	*
	* @return boolean
	* @access public
	*/
	function processInlineValueInterface(&$asset, &$new_values)
	{
		$new_values[$asset->name] = Array();
		$new_values[$asset->name]['fields'] = Array();
		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'metadata_field', false, 'major', null, true);
		foreach ($current_links as $link) {
			$field = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
			if (is_null($field)) continue;
			$edit_fns = $field->getEditFns();
			if (!$edit_fns->processInlineValueInterface($field, $new_values[$asset->name]['fields'])) return false;
		}

		return true;

	}//end processInlineValueInterface()


	/**
	* Check if all required fields have been completed in the values supplied
	*
	* @param object Metadata_Section	&$asset		the asset whose metadata we are checking
	* @param array						$values		the current metadata values
	*
	* @return boolean
	* @access public
	*/
	function requiredFieldsComplete(&$asset, $values)
	{
		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'metadata_field', false, 'major', null, true);
		foreach ($current_links as $link) {
			$field = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
			if (is_null($field)) continue;
			$edit_fns = $field->getEditFns();
			if (!isset($values['fields'][$field->name])) $values['fields'][$field->name] = Array();
			if (!$edit_fns->requiredFieldsComplete($field, $values['fields'][$field->name])) return false;
		}

		return true;

	}//end requiredFieldsComplete()


	/**
	* Output the complete metadata tags for this section
	*
	* @param object Metadata_Section	&$asset			the asset whose interface we are painting
	* @param array						&$values		the current metadata values
	* @param array						&$tag_values	the metadata tag values
	* @param array						&$keywords		an array we can put keywords in that need replacing
	*
	* @return boolean
	* @access public
	*/
	function generateMetadata(&$asset, &$values, &$tag_values, &$keywords)
	{
		echo "<!-- $asset->name //-->\n";

		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'metadata_field', false, 'major', null, true);
		$valid_fields = Array();
		foreach ($current_links as $link) {
			$field = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
			if (is_null($field)) continue;
			$valid_fields[] = $field->name;

			$edit_fns = $field->getEditFns();
			if (!isset($values['fields'][$field->name])) $values['fields'][$field->name] = Array();
			$tag_values[$field->name] = '';
			if (!$edit_fns->generateMetadata($field, $values['fields'][$field->name], $tag_values[$field->name], $keywords)) return false;
		}
		echo "\n";

		// remove any invalid fields from the values array
		$fields = $values['fields'];
		foreach ($fields as $field_name => $field_data) {
			if (!in_array($field_name, $valid_fields)) unset($values['fields'][$field_name]);
		}

		return true;

	}//end generateMetadata()


	/**
	* Generate an array of current values for the passed metadata field names
	*
	* @param object Asset				&$asset		the asset whose field values we are gettings
	* @param object Metadata_Section	&$section	the asset whose interface we are painting
	* @param array						$values		the current metadata values
	* @param array						&$fields	an array of field names that need replacing
	*
	* @access public
	* @return boolean
	*/
	function getMetadataFieldValues(&$asset, &$section, $values, &$fields)
	{
		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($section->id, SQ_LINK_TYPE_2, 'metadata_field', false, 'major', null, true);
		foreach ($current_links as $link) {
			$field = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
			if (is_null($field)) continue;
			if (!isset($fields[$field->name])) continue;
			$edit_fns = $field->getEditFns();
			if (!isset($values['fields'][$field->name])) $values['fields'][$field->name] = Array();
			if (!$edit_fns->getMetadataFieldValues($asset, $field, $values['fields'][$field->name], $fields)) return false;
		}

		return true;

	}//end getMetadataFieldValues()


}//end class

?>