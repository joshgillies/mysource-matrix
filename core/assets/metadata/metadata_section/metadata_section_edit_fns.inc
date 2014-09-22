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
* $Id: metadata_section_edit_fns.inc,v 1.58 2013/07/29 05:38:57 cupreti Exp $
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
* @version $Revision: 1.58 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Metadata_Section_Edit_Fns extends Folder_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();
		$this->static_screens['details']['force_unlock'] = FALSE;

	}//end constructor


	/**
	* Paint the interface for editing metadata fields
	*
	* @param Metadata_Section	$asset	the asset whose interface we are painting
	* @param Backend_Outputter	$o		the outputter class
	*
	* @return boolean
	* @access public
	*/
	public function paintInlineOptionInterface(Metadata_Section $asset, Backend_Outputter $o)
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

			// Section description
			$o->openField(translate('Section Description'));

				$description = $asset->attr('description');
				if ($write_access) {
					text_area($prefix.'_description', $description, 60, 5);
				} else {
					echo $description;
				}
			$o->closeField();
			// Current Restrictions
			// Show the dialog for restricting a section by asset type
			$o->openField(translate('Current Restrictions'));

				$restrictions = $asset->attr('restrict');
				if (!empty($restrictions)) {
					?>
					<table class="sq-backend-table">
						<tr>
							<td class="sq-backend-table-header"><?php echo translate('Asset Type'); ?></td>

							<td style="text-align: center;" class="sq-backend-table-header" width="70"><?php echo translate('Inherit').'?'; ?></td>

							<?php
							if ($write_access) {
							?>
							<td style="text-align: center;" class="sq-backend-table-header" width="70"><?php echo translate('Remove').'?'; ?></td>

							<?php
							}
							?>
						</tr>
						<?php
						foreach ($restrictions as $asset_type => $inheritance) {
							?>
							<tr>
								<td class="sq-backend-table-cell">
									<?php
										get_asset_type_icon($asset_type);
										echo ucfirst(str_replace('_', ' ', $asset_type));
									?>
								</td>
								<td align="center" class="sq-backend-table-cell">
									<?php
									if ($write_access) {
										check_box($prefix.'_asset_type_inherit['.$asset_type.']', '1', $inheritance);
									} else {
										echo '<img src="'.sq_web_path('lib').'/web/images/'.($inheritance ? 'tick' : 'cross').'.gif" width=" 15" height="15" />';
									}
									?>
								</td>
								<?php
								if ($write_access) {
								?>
								<td align="center" class="sq-backend-table-cell">
									<?php check_box($prefix.'_remove_asset_type['.$asset_type.']'); ?>
								</td>
								<?php
								}
								?>
							</tr>
							<?php
						}//end foreach()
					?>
					</table>

					<?php
				} else {
					echo '<em>'.translate('No Restrictions').'</em>';

				}
			$o->closeField();

			if ($write_access) {
				// New Restrictions
				$o->openField(translate('New Restriction'));

					asset_type_chooser($prefix.'_add_restriction', TRUE, Array(), NULL, FALSE, TRUE);
				$o->closeField();
			}

			$o->openField(translate('Fields'));

				?>
				<table class="sq-backend-table">
					<tr>
						<td class="sq-backend-table-header"><b><?php echo translate('Field Name'); ?></b></td>

						<td width="150" class="sq-backend-table-header"><b><?php echo translate('Edit Field'); ?></b></td>

						<?php
						if ($write_access) {
							?><td style="text-align: center;" width="70" class="sq-backend-table-header"><b><?php echo translate('Delete?'); ?></b></td><?php

						}
						?>
					</tr>
				<?php
				foreach ($current_links as $link_data) {
					$field = $GLOBALS['SQ_SYSTEM']->am->getAsset($link_data['minorid']);
					if (is_null($field)) continue;
					$edit_fns = $field->getEditFns();
					?>
					<tr>
						<td align="left" class="sq-backend-table-cell"><?php echo get_asset_tag_line($field->id);?></td>
						<td class="sq-backend-table-cell">[ <a href="<?php echo $asset_href[$field->id];?>"><?php echo translate('Edit Metadata Field'); ?></a> ]</td>

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
			$o->openField(translate('New Field Name'));


				require_once SQ_LIB_PATH.'/html_form/html_form.inc';
				text_box($prefix.'_new_field', '', 30);

			$o->closeField();

			$o->openField(translate('New Field Type'));


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
	* @param Metadata_Section	$asset	the asset whose interface we are painting
	* @param Backend_Outputter	$o		the outputter class
	*
	* @return boolean
	* @access public
	*/
	public function processInlineOptionInterface(Metadata_Section $asset, Backend_Outputter $o)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('links')) return FALSE;
		if (!$asset->writeAccess('attributes')) return FALSE;

		$prefix = $asset->getPrefix();
		$updated = FALSE;

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// handle description
		if (isset($_POST[$prefix.'_description'])) {
			$section_description = trim($_POST[$prefix.'_description']);
			$asset->setAttrValue('description', $section_description);
			$asset->saveAttributes();
		}

		// handle restrictions
		// delete
		$current_restrictions = $asset->attr('restrict');
		$changes_made = FALSE;
		if (isset($_POST[$prefix.'_remove_asset_type'])) {
			foreach($_POST[$prefix.'_remove_asset_type'] as $remove_type => $ahfortheheckofit) {
				if (array_key_exists($remove_type, $current_restrictions)) {
					unset($current_restrictions[$remove_type]);
					$changes_made = TRUE;
				}
			}

			// If something to save, save it, dammit!
			if ($changes_made) {
				$asset->setAttrValue('restrict', $current_restrictions);
				$asset->saveAttributes();
			}
		}

		// inherit type setting
		$current_restrictions = $asset->attr('restrict');
		$changes_made = FALSE;
		if (isset($_POST[$prefix.'_asset_type_inherit'])) {
			$inherit_enabled = $_POST[$prefix.'_asset_type_inherit'];
			foreach($current_restrictions as $asset_type => $inheritance) {
				if (isset($inherit_enabled[$asset_type])) {
					$current_restrictions[$asset_type] = 1;
				} else {
					$current_restrictions[$asset_type] = 0;
				}
				if ($inheritance != array_get_index($inherit_enabled, $asset_type, 0)) {
					$changes_made = TRUE;
				}

			}//end foreach

			if ($changes_made) {
				$asset->setAttrValue('restrict', $current_restrictions);
				$asset->saveAttributes();
			}
		}

		// add
		$current_restrictions = $asset->attr('restrict');
		$changes_made = FALSE;
		if (isset($_POST[$prefix.'_add_restriction']) && (isset($_POST[$prefix.'_add_restriction']['type_code']))) {
			foreach($_POST[$prefix.'_add_restriction']['type_code'] as $field => $type_code) {
				if (!empty($type_code)) {
					$inheritance = $_POST[$prefix.'_add_restriction']['inherit'][$field];
					if (!isset($current_restrictions[$type_code])) {
						$current_restrictions[$type_code] = $inheritance;
						$changes_made = TRUE;
					} else if ($current_restrictions[$type_code] !== $inheritance) {
						$current_restrictions[$type_code] = $inheritance;
						$changes_made = TRUE;
					}
				}
			}

			// If something to save, save it, dammit!
			if ($changes_made) {
				$asset->setAttrValue('restrict', $current_restrictions);
				$asset->saveAttributes();
			}
		}

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
			$new_field = new $new_field_type();

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
	* @param Metadata_Section	$asset							the asset whose interface we are painting
	* @param Backend_Outputter	$o								the outputter class
	* @param array				$values							current metadata values set for this asset
	* @param boolean			$write_access					does the current user have write access to the asset
	*															that we are setting metadata for (NOT THE SAME AS $asset)
	* @param boolean			$print_cascade_values_option	Show the cascade value option
	* @param string				$type_code						The type_code of the asset that the metadata is being printed
	* @param boolean			$type_code_prefix				Whether to use $type_code as part of the metadata field id's prefix
	*
	* @return boolean
	* @access public
	*/
	function paintInlineValueInterface(Metadata_Section $asset, Backend_Outputter $o, Array $values, $write_access, $print_cascade_values_option = TRUE, $type_code=NULL, $type_code_prefix=FALSE)
	{
		$restrictions = $asset->attr('restrict');
		$flag_match_found = FALSE;
		$res = FALSE;

		if (!empty($restrictions)) {
            foreach ($restrictions as $type_code_restricted => $inherit_it) {
                if ($inherit_it && !$flag_match_found) {
                    $asset_type_with_parents = $GLOBALS['SQ_SYSTEM']->am->getTypeAncestors($type_code);
                    foreach($asset_type_with_parents as $index => $asset_type) {
                        if (($type_code_restricted == $type_code || $asset_type ==  $type_code_restricted)&& !$flag_match_found) {
                            $flag_match_found = TRUE;
                        }
                    }
                } else if (array_key_exists($type_code, $restrictions)){
                    $flag_match_found = TRUE;
                }
            }
        }

		if (empty($restrictions) || $flag_match_found ) {
			$o->openSection($asset->name);
			if ($write_access) {
				if (isset($GLOBALS['painted_inline_value_interface'][$asset->id])) {
					// this is the second time we have painted this interface
					$o->openField(translate('Please Note'));

						echo '<span class="sq-backend-warning"><b>'.translate('The editing interface for this metadata section has already been shown above. The current metadata values are displayed below but cannot be edited. To edit these metadata values, please use the editing interface displayed above.').'</b></span>';

					$o->closeField();
					$write_access = FALSE;
				} else {
					// make sure we dont try and paint the same interface twice
					$GLOBALS['painted_inline_value_interface'][$asset->id] = TRUE;
				}
			}

			$section_description = $asset->attr('description');
			if (!empty($section_description)) {
				$o->openField('<i>'.translate('Section Description').'</i>');

					echo '<i>'.$section_description.'</i>';
				$o->closeField();
			}

			$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'metadata_field', FALSE, 'major', NULL, TRUE);
			foreach ($current_links as $link) {
				$field = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
				if (is_null($field)) continue;
				$edit_fns = $field->getEditFns();
				$type_code = $type_code_prefix ? $type_code : NULL;
				if ($edit_fns->paintInlineValueInterface($field, $o, (isset($values[$field->id])) ? $values[$field->id][0]['value'] : NULL, $write_access, $print_cascade_values_option, $type_code)) {
					$res = TRUE;
				}
			}
			$o->closeSection();
		}//end if

		return $res;

	}//end paintInlineValueInterface()


	/**
	* Process the interface for filling in metadata values
	*
	* Returns true if at least one field processed itself and got a value
	*
	* @param Metadata_Section	$asset			the asset whose interface we are painting
	* @param array				&$new_values	an array we can store the new values in
	* @param boolean			$print_cascade_values_option	Show the cascade value option
	* @param string				$type_code						The type_code of the asset that the metadata is being printed
	* @param boolean			$type_code_prefix				Whether to use $type_code as part of the metadata field id's prefix
	*
	* @return boolean
	* @access public
	*/
	function processInlineValueInterface(Metadata_Section $asset, Array &$new_values, Array &$section_cascade_value_asset_ids, $type_code=NULL, $type_code_prefix=FALSE)
	{
		$res = FALSE;
		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'metadata_field', FALSE, 'major', NULL, TRUE);
		foreach ($current_links as $link) {
			$field_cascade_value = FALSE;
			$field = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
			if (is_null($field) || !$field->attr('editable')) {
				continue;
			}
			$edit_fns = $field->getEditFns();
			$new_value = '';
			$type_code = $type_code_prefix ? $type_code : NULL;
			if ($edit_fns->processInlineValueInterface($field, $new_value, $field_cascade_value, $type_code)) {
				// the process fn returned true, meaning something was submitted
				// $new_value may be NULL, however, which signifies that we are reverting to default
				// Evaluating %asset_metadata_% keywords in metadata field will create loop
				if (strpos($new_value, '%asset_metadata_') !== FALSE) {
					trigger_localised_error('SYS0344', translate('Cannot use asset_metadata_XXX keyword for metadata field value. Replacing asset_metadata_XXX keyword by metadata_field_XXX'), E_USER_WARNING)
					$new_value = preg_replace('/%asset_metadata_([^\s]*?)%/', '%metadata_field_$1%', $new_value);
				}

				$new_values[$field->id] = Array	(
											0	=> Array (
													'name'	=> $field->name,
													'value' => $new_value,
												   ),
										  );
				if ($field_cascade_value) $section_cascade_value_asset_ids[$field->id] = 1;

				$res = TRUE;
			}
		}

		return $res;

	}//end processInlineValueInterface()


	/**
	* Check if all required fields have been completed in the values supplied
	*
	* @param Metadata_Section	$asset	the asset whose interface we are painting
	* @param array				$values	the current metadata values
	* @param string				$type_code	The type_code of the asset that the metadata is being printed
	*
	* @return boolean
	* @access public
	*/
	function requiredFieldsComplete(Metadata_Section $asset, $values, $type_code=NULL)
	{
		$inherited_restriction = Array();
		$restrictions = $asset->attr('restrict');

		foreach($restrictions as $restriction => $inheritance) {
			$inherited_types = Array();
			if ($inheritance) {
				$inherited_types = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants($restriction);
				foreach($inherited_types as $inherited_type) {
					if (!array_key_exists($inherited_type, $restrictions)) {
						$inherited_restriction = array_merge($inherited_restriction, Array($inherited_type));
					}
				}
			}
		}

		if(!empty($inherited_restriction)) {
			$inherited_restriction = array_flip($inherited_restriction);
			$restrictions = array_merge($restrictions, $inherited_restriction);
		}


		if (empty($restrictions) || (!is_null($type_code) && array_key_exists($type_code, $restrictions))) {
			$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'metadata_field', FALSE, 'major', NULL, TRUE);
			foreach ($current_links as $link) {
				$field = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
				if (is_null($field)) continue;
				$edit_fns = $field->getEditFns();
				if (!$edit_fns->requiredFieldsComplete($field, (isset($values[$field->id])) ? $values[$field->id][0]['value'] : NULL)) {
					return FALSE;
				}
			}
		}

		return TRUE;

	}//end requiredFieldsComplete()


	/**
	* Output the complete metadata tags for this section
	*
	* @param Metadata_Section	$asset			the asset whose interface we are painting
	* @param array				&$values		the current metadata values
	* @param array				&$tag_values	the metadata tag values
	* @param array				&$keywords		an array we can put keywords in that need replacing
	* @param string				$type_code		The type_code of the asset that the metadata is being printed
	*
	* @return boolean
	* @access public
	*/
	function generateMetadata(Metadata_Section $asset, Array &$values, Array &$tag_values, Array &$keywords, $type_code=NULL)
	{
		$frontend_option = $asset->attr('frontend');
		$flag_match_found = FALSE;
		// if we aren't printing this on the frontend then don't print anything

		$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();

		$restrictions = $asset->attr('restrict');
		if (!empty($restrictions)) {
			foreach ($restrictions as $type_code_restricted => $inherit_it) {
				if ($inherit_it && !$flag_match_found) {
					$asset_type_with_parents = $GLOBALS['SQ_SYSTEM']->am->getTypeAncestors($type_code);
					foreach($asset_type_with_parents as $index => $asset_type) {
						if (($type_code_restricted == $type_code || $asset_type ==  $type_code_restricted)&& !$flag_match_found) {
							$flag_match_found = TRUE;
						}
					}
				} else if (array_key_exists($type_code, $restrictions)){
					$flag_match_found = TRUE;
				}
			}
		}
		if (empty($restrictions) || $flag_match_found ) {
			// Buffer the contents of any META tags
			ob_start();

			echo '<!-- '.$mm->escapeMetadata($asset->name).' //-->'."\n";
			$section_description = $asset->attr('description');
			if ($asset->attr('desc_frontend') == TRUE && !empty($section_description)) {
				echo '<!-- '.$mm->escapeMetadata($section_description).' //-->'."\n";
			}

			$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'metadata_field', FALSE, 'major', NULL, TRUE);
			$valid_fields = Array();
			foreach ($current_links as $link) {
				$field = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
				if (is_null($field)) continue;
				$valid_fields[] = $field->name;

				$edit_fns = $field->getEditFns();
				$tag_values[$field->name] = '';


					if (!$edit_fns->generateMetadata($field, ((isset($values[$field->id])) ? $values[$field->id][0]['value'] : NULL), $tag_values[$field->name], $keywords)) {
						ob_end_clean();
						return FALSE;
					}

			}

			echo "\n";

			// If printing on front-end, print the META tags, otherwise discard them
			$contents = ob_get_clean();
			if ($frontend_option) echo $contents;
		}//end if

		return TRUE;

	}//end generateMetadata()


}//end class

?>
