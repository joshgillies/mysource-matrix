<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: metadata_section_edit_fns.inc,v 1.40 2007/03/05 22:22:53 hnguyen Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/folder/folder_edit_fns.inc';

/**
* Metadata_Section_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.40 $
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
		$this->static_screens['details']['force_unlock'] = FALSE;

	}//end constructor


	/**
	* Paint the interface for editing metadata fields
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	*
	* @return boolean
	* @access public
	*/
	function paintInlineOptionInterface(&$asset, &$o)
	{
		$write_access = $asset->writeAccess('links');
		$prefix = $asset->getPrefix();

		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'metadata_field', FALSE, 'major', NULL, TRUE);
		if (!empty($current_links)) {
			$assetids = Array();
			foreach ($current_links as $link) {
				$assetids[$link['minorid']] = 'details';
			}
			$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_keys($assetids));
			$asset_href = $GLOBALS['SQ_SYSTEM']->am->getAssetBackendHref($assetids);

			$o->openField(translate('fields'));
				?>
				<table class="sq-backend-table">
					<tr>
						<td class="sq-backend-table-header"><b><?php echo translate('field_name'); ?></b></td>
						<td align="center" width="150" class="sq-backend-table-header"><b><?php echo translate('edit_fields'); ?></b></td>
						<?php
						if ($write_access) {
							?><td align="center" width="70" class="sq-backend-table-header"><b><?php echo translate('delete_question'); ?></b></td><?php
						}
						?>
					</tr>
				<?php
				foreach ($current_links as $link_data) {
					$field =& $GLOBALS['SQ_SYSTEM']->am->getAsset($link_data['minorid']);
					if (is_null($field)) continue;
					$edit_fns = $field->getEditFns();
					?>
					<tr>
						<td align="left" class="sq-backend-table-cell"><?php echo get_asset_tag_line($field->id);?></td>
						<td align="center" class="sq-backend-table-cell">[ <a href="<?php echo $asset_href[$field->id];?>"><?php echo translate('metadata_edit_fields'); ?></a> ]</td>
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
		}//end if

		if ($write_access) {
			$o->openField(translate('new_field_name'));

				require_once SQ_LIB_PATH.'/html_form/html_form.inc';
				text_box($prefix.'_new_field', '', 20);

			$o->closeField();

			$o->openField(translate('new_field_type'));

				// now getting available types list
				$descendant_types = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants('metadata_field');

				$contents = Array();
				foreach ($descendant_types as  $type) {
					// $te[2] - type of the field, which we need
					$te = explode('_', $type, 3);
					$contents[$type] = ucwords(str_replace('_', ' ', $te[2])); // capitalizing the first char
				}

				combo_box($prefix.'_field_type', $contents, FALSE, 'metadata_field_text');

			$o->closeField();
		}

		return TRUE;

	}//end paintInlineOptionInterface()


	/**
	* Process the interface for editing metadata fields
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	*
	* @return boolean
	* @access public
	*/
	function processInlineOptionInterface(&$asset, &$o)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('links')) return FALSE;

		$prefix = $asset->getPrefix();
		$updated = FALSE;

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// delete any fields that need deleting
		if (isset($_POST[$prefix.'_delete_fields'])) {
			$delete_fields = $_POST[$prefix.'_delete_fields'];
			foreach ($delete_fields as $linkid => $on) {
				if ($asset->deleteLink($linkid)) $updated = TRUE;
			}
		}

		// create a new field?
		if (isset($_POST[$prefix.'_new_field']) && trim($_POST[$prefix.'_new_field']) != '') {
			$new_field_name = trim($_POST[$prefix.'_new_field']);
			$new_field_type = trim($_POST[$prefix.'_field_type']);

			$GLOBALS['SQ_SYSTEM']->am->includeAsset($new_field_type);
			$new_field =& new $new_field_type();

			$section_link = Array('asset' => &$asset, 'link_type' => SQ_LINK_TYPE_2, 'sort_order' => -1, 'is_dependant' => 1, 'is_exclusive' => 0);
			$new_field->setAttrValue('name', $new_field_name);
			if ($new_field->create($section_link)) $updated = TRUE;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return $updated;

		return TRUE;

	}//end processInlineOptionInterface()


	/**
	* Paint the interface for filling in metadata values
	*
	* @param object		&$asset			the asset whose interface we are painting
	* @param object		&$o				the outputter class
	* @param array		$values			current metadata values set for this asset
	* @param boolean	$write_access	does the current user have write access to the asset
	*									that we are setting metadata for (NOT THE SAME AS $asset)
	*
	* @return boolean
	* @access public
	*/
	function paintInlineValueInterface(&$asset, &$o, $values, $write_access)
	{
		$o->openSection($asset->name);
		if ($write_access) {
			if (isset($GLOBALS['painted_inline_value_interface'][$asset->id])) {
				// this is the second time we have painted this interface
				$o->openField(translate('please_note'));
					echo '<span class="sq-backend-warning"><b>'.translate('metadata_use_editing_interface').'</b></span>';
				$o->closeField();
				$write_access = FALSE;
			} else {
				// make sure we dont try and paint the same interface twice
				$GLOBALS['painted_inline_value_interface'][$asset->id] = TRUE;
			}
		}

		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'metadata_field', FALSE, 'major', NULL, TRUE);
		foreach ($current_links as $link) {
			$field =& $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
			if (is_null($field)) continue;
			$edit_fns = $field->getEditFns();
			if (!$edit_fns->paintInlineValueInterface($field, $o, (isset($values[$field->id])) ? $values[$field->id]['value'] : NULL, $write_access)) {
				return FALSE;
			}
		}
		$o->closeSection();

		return TRUE;

	}//end paintInlineValueInterface()


	/**
	* Process the interface for filling in metadata values
	*
	* Returns true if at least one field processed itself and got a value
	*
	* @param object	&$asset			the asset whose interface we are painting
	* @param array	&$new_values	an array we can store the new values in
	*
	* @return boolean
	* @access public
	*/
	function processInlineValueInterface(&$asset, &$new_values)
	{
		$res = FALSE;
		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'metadata_field', FALSE, 'major', NULL, TRUE);
		foreach ($current_links as $link) {
			$field =& $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
			if (is_null($field) || !$field->attr('editable')) {
				continue;
			}
			$edit_fns = $field->getEditFns();
			$new_value = '';
			if ($edit_fns->processInlineValueInterface($field, $new_value)) {
				// the process fn returned true, meaning something was submitted
				// $new_value may be NULL, however, which signifies that we are reverting to default
				$new_values[$field->id] = Array('name' => $field->name, 'value' => $new_value);
				$res = TRUE;
			}
		}

		return $res;

	}//end processInlineValueInterface()


	/**
	* Check if all required fields have been completed in the values supplied
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param array	$values	the current metadata values
	*
	* @return boolean
	* @access public
	*/
	function requiredFieldsComplete(&$asset, $values)
	{
		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'metadata_field', FALSE, 'major', NULL, TRUE);
		foreach ($current_links as $link) {
			$field =& $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
			if (is_null($field)) continue;
			$edit_fns = $field->getEditFns();
			if (!$edit_fns->requiredFieldsComplete($field, (isset($values[$field->id])) ? $values[$field->id]['value'] : NULL)) {
				return FALSE;
			}
		}

		return TRUE;

	}//end requiredFieldsComplete()


	/**
	* Output the complete metadata tags for this section
	*
	* @param object	&$asset			the asset whose interface we are painting
	* @param array	&$values		the current metadata values
	* @param array	&$tag_values	the metadata tag values
	* @param array	&$keywords		an array we can put keywords in that need replacing
	*
	* @return boolean
	* @access public
	*/
	function generateMetadata(&$asset, &$values, &$tag_values, &$keywords)
	{
		$frontend_option = $asset->attr('frontend');
		// if we aren't printing this on the frontend then don't print anything

		$mm =& $GLOBALS['SQ_SYSTEM']->mm;

		// Buffer the contents of any META tags
		ob_start();

		echo '<!-- '.$mm->escapeMetadata($asset->name).' //-->'."\n";

		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'metadata_field', FALSE, 'major', NULL, TRUE);
		$valid_fields = Array();
		foreach ($current_links as $link) {
			$field =& $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
			if (is_null($field)) continue;
			$valid_fields[] = $field->name;

			$edit_fns = $field->getEditFns();
			$tag_values[$field->name] = '';


				if (!$edit_fns->generateMetadata($field, ((isset($values[$field->id])) ? $values[$field->id]['value'] : NULL), $tag_values[$field->name], $keywords)) {
					ob_end_clean();
					return FALSE;
				}

		}

		echo "\n";

		// If printing on front-end, print the META tags, otherwise discard them
		$contents = ob_get_clean();
		if ($frontend_option) echo $contents;

		return TRUE;

	}//end generateMetadata()


}//end class

?>