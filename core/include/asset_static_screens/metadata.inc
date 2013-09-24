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
* $Id: metadata.inc,v 1.56.4.1 2013/09/24 00:58:07 ewang Exp $
*
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* MetaData Static Screen Functions
*
* @author  Greg Sherwood <gsherwood@squiz.net>
* @version $Revision: 1.56.4.1 $
* @package MySource_Matrix
*/


/**
* Paint the interface for managing metadata values
*
* @param object		&$owner			the asset whose interface we are painting
* @param object		&$o			the outputter class
* @param object		&$ei			the edit interface
* @param boolean	$keywords		the keywords to replace
* @param Array		$layout_options		the Simple Edit layout options
*
* @return boolean
* @access public
*/
function paintMetadata(&$owner, &$o, &$ei, $keywords=NULL, Array $layout_options=Array())
{
	$root = $ei->findScreen('static_screen_metadata');
	if (is_null($root)) return FALSE;

	$prefix = $owner->getPrefix();
	$print_commit_button = FALSE;

	$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
	$schemas = $mm->getSchemas($owner->id, TRUE, NULL, TRUE);
	$metadata_warnings = Array();
	$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();

	// If we are in Simple Edit, obtain the "Show Cascade Metadata field" option value so we can determine
	// whether or not to print the checkbox
	$print_cascade_values_option = TRUE;

	if ((!SQ_IN_BACKEND) && (isset($layout_options['show_cascade_metadata_field']))) {
		$print_cascade_values_option = $layout_options['show_cascade_metadata_field'];
	}

	foreach ($root->section as $section) {
		// do this if keywords are set
		if (is_null($keywords)) {
			// if we are in limbo and this section is not to be printed in limbo - dont print it
			if (SQ_IN_LIMBO && (!isset($section->attributes()->limbo_access) || (int)$section->attributes()->limbo_access == 0)) {
				continue;
			}
		}

		$section_access = $ei->_getAccess($owner, $section, $prefix);
		if (!($section_access & SQ_EI_READ)) {
			continue;
		}

		if (!is_null($keywords) && isset($keywords['sections'][strtolower(str_replace(' ', '_', (string) $section->attributes()->name))])) {
			ob_start();
		}

		// TODO: Fix this section->attributes() call here, don't use array get index.
		$o->openSection(array_get_index($section->attributes, 'display_name', (string) $section->attributes()->name));

		if (!($num_fields = count($section->children()))) {
			continue;
		}

		foreach ($section->field as $field) {
			$field_access = $ei->_getAccess($owner, $field, $prefix);
			if (!($field_access & SQ_EI_READ)) {
				continue;
			}

			switch (strtolower((string) $field->attributes()->code)) {
				///// CURRENT METADATA /////

				case 'metadata_current' :

					$o->openField('');
						if (!is_null($keywords) && isset($keywords['fields']['metadata_current'])) {
							ob_start();
						}

						if ($contextid === 0) {
							$metadata_basename = 'metadata.php';
						} else {
							$metadata_basename = 'metadata.'.$contextid.'.php';
						}
						$metadata_default_name = 'metadata.php';
						$metadata = NULL;

						if (empty($schemas)) {
							echo translate('no_metadata_schemas_applied');
						} else if (file_exists($owner->data_path.'/'.$metadata_basename)) {
							ob_start();
								require($owner->data_path.'/'.$metadata_basename);
								$metadata = ob_get_contents();
							ob_end_clean();
						} else if (file_exists($owner->data_path.'/'.$metadata_default_name)) {
							// No generated metadata for this context yet. We can assume
							// that until it is generated, it's using the same defaults
							ob_start();
								require($owner->data_path.'/'.$metadata_default_name);
								$metadata = ob_get_contents();
							ob_end_clean();
						}

						// If metadata was filled, clean it and print it
						// for the user's edification
						if ($metadata !== NULL) {
							echo nl2br(htmlspecialchars(trim($metadata)));
						}

						// if there is a keyword replacement for this, add it to the replacements
						if (!is_null($keywords) && isset($keywords['fields']['metadata_current'])) {
							$ei->_tmp['layout_keyword_replacements'][$keywords['fields']['metadata_current']] = ob_get_contents();
							ob_end_flush();
						}
					$o->closeField();

				break;

				///// METADATA INSTRUCTIONS /////

				case 'metadata_instructions' :

					$o->openField('&nbsp;');
						if (!is_null($keywords) && isset($keywords['fields']['metadata_instructions'])) {
							ob_start();
						}

						// construct example keywords link
						$replacements_link = '<a href="'.sq_web_path('data').'/asset_types/metadata_field/popups/keywords.php?assetid='.$owner->id.'&amp;all=0" onclick="var win = window.open(this.href, \'keywords\', \'toolbar=0,menubar=0,location=0,status=0,scrollbars=1,resizable=1,width=580,height=320\'); return false;">'.translate('click_here').'</a>';
						echo '<p>'.translate('metadata_keyword_replacements_usage', $replacements_link).'</p>';

						$sm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('search_manager', TRUE);
						if (!empty($sm) && $sm->attr('indexing')) {
						?>
							<?php
							$replacements_link = '<a href="'.sq_web_path('data').'/asset_types/metadata_field/popups/keyword_extraction.php?assetid='.$owner->id.'" onclick="var win = window.open(this.href, \'keywords\', \'toolbar=0,menubar=0,location=0,status=0,scrollbars=1,resizable=1,width=580,height=320\'); return false;">'.translate('keyword_extraction').'</a>';
							echo '<p>'.translate('metadata_indexed_keywords', $replacements_link).'</p>';

						}

						// paint the instructions for 'Use Default'
						echo '<p>'.translate('metadata_explain_use_default').'</p>';

						// paint the instructions for 'Cascade value'
						echo '<p>'.translate('metadata_explain_cascade_value').'</p>';

						// paint required field
						echo '<p> '.translate('denotes_required_field', '<span class="sq-backend-warning">*</span>').'</p>';

						// if there is a keyword replacement for this, add it to the replacements
						if (!is_null($keywords) && isset($keywords['fields']['metadata_instructions'])) {
							$ei->_tmp['layout_keyword_replacements'][$keywords['fields']['metadata_instructions']] = ob_get_contents();
							ob_end_flush();
						}
					$o->closeField();

				break;

				///// METADATA EDITING INTERFACE /////

				case 'metadata_values' :
					if (empty($schemas)) {
						$o->openField('&nbsp;');
							if (!is_null($keywords) && isset($keywords['fields']['metadata_values'])) {
								ob_start();
							}

							echo translate('must_apply_one_or_more_schemas');

							// if there is a keyword replacement for this, add it to the replacements
							if (!is_null($keywords) && isset($keywords['fields']['metadata_values'])) {
								$ei->_tmp['layout_keyword_replacements'][$keywords['fields']['metadata_values']] = ob_get_contents();
								ob_end_flush();
							}
						$o->closeField();
					} else {
						$write_access = $owner->writeAccess('metadata');
						$print_commit_button = FALSE;

						if (!is_null($keywords) && isset($keywords['fields']['metadata_values'])) {
							ob_start();
						}

						$values = $mm->getMetadata($owner->id);
						foreach ($schemas as $schemaid) {
							$schema = $GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
							if (is_null($schema)) continue;
							$edit_fns = $schema->getEditFns();

							// Paint only if user is not in limbo or if keywords for metadata fields are set
							if ((!is_null($keywords) && isset($keywords['fields']['metadata_values'])) || !SQ_IN_LIMBO) {
								if ($edit_fns->paintInlineValueInterface($schema, $o, $values, $write_access, $print_cascade_values_option, $owner->type())) {
									$print_commit_button = TRUE;
								}
							}
							else {
								$print_commit_button = TRUE;
							}
						}

						// if there is a keyword replacement for this, add it to the replacements
						if (!is_null($keywords) && isset($keywords['fields']['metadata_values'])) {
							// Wrap this keyword's contents in table tags because it is printing out the contents of nested sections,
							// but the top level section (opened earlier in this file) is not printed along with it.
							$ei->_tmp['layout_keyword_replacements'][$keywords['fields']['metadata_values']] = '<table>'.ob_get_contents().'</table>';
							ob_end_flush();
						}
					}

				break;

			}//end switch

			$note = (string) $field->note;
			if (!empty($note)) {
				$o->note($note);
			}//end if

		}//end for
		$o->closeSection();

		if (!is_null($keywords) && isset($keywords['sections'][strtolower(str_replace(' ', '_', $section->attributes()->name))])) {
			$ei->_tmp['layout_keyword_replacements'][$keywords['sections'][strtolower(str_replace(' ', '_', $section->attributes()->name))]] = ob_get_contents();
			ob_end_flush();
		}

	}//end for
	// prints a sq backend warning message if we have same metadata field names
	$field_names = Array();
	$duplicate_field_names = Array();
	$field_ids = $mm->getMetadataFields($schemas);
	if (!empty($field_ids)) {
		foreach ($field_ids as $field_assetid => $field_info) {
			$field = $GLOBALS['SQ_SYSTEM']->am->getAsset($field_assetid);
			if (!isset($field_names[$field->attr('name')])) {
				$field_names[$field->attr('name')] = 1;
			} else {
				$duplicate_field_names[$field->attr('name')] = $field->attr('name');
			}
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($field);
		}
	}

	if (!empty($duplicate_field_names)) {
		$o->openSection(translate('warning'));
			$o->openField('&nbsp;');
			// paint required field
			ob_start();
			?><ul><?php
			foreach ($duplicate_field_names as $name) {
				?><li><?php echo $name ?></li><?php
			}
			?></ul><span class="sq-backend-warning"><?php echo translate('duplicate_metadata_fields'); ?></span>
			<?php
			ob_end_flush();
			$o->closeField();
		$o->closeSection();
	}

	// $metadata_warnings is provided by the metadata cache file
	if (!empty($metadata_warnings['keyword_circular_references'])) {
		$o->openSection(translate('warning'));
			$o->openField('&nbsp;');
			// paint required field
			ob_start();
			?><ul><?php
			foreach ($metadata_warnings['keyword_circular_references'] as $name) {
				?><li><?php echo $name ?></li><?php
			}
			?></ul><span class="sq-backend-warning"><?php echo translate('circular_reference_metadata_field_keywords'); ?></span>
			<?php
			ob_end_flush();
			$o->closeField();
		$o->closeSection();
	}

	// set the keyword replacements for any numbered field or section keywords
	if (!is_null($keywords) && isset($keywords['fields'])) {

		// get the schemas that apply to this asset
		$schemas = $mm->getSchemas($owner->id, TRUE);
		$write_access = $owner->writeAccess('metadata');

		foreach ($keywords['fields'] as $field_assetid => $keyword) {

			// is this actually a metadata field or what?!
			if ($mm->isMetadataFieldAssetid($field_assetid)) {
				// get the parent schema of this metadata field
				$schemaid = $GLOBALS['SQ_SYSTEM']->am->getParents($field_assetid, 'metadata_schema');

				// Only display the field if its schema is applied to the asset,
				// or if the asset is not yet created
				if (empty($owner->id) || (array_intersect($schemas, array_keys($schemaid)) != Array())) {
					$value = $mm->getMetadataInterfaceValueByAssetid($owner->id, $field_assetid);

					$field = $GLOBALS['SQ_SYSTEM']->am->getAsset($field_assetid);
					$edit_fns = $field->getEditFns();

					ob_start();
					$edit_fns->paintValueInterface($field, $o, $value, $write_access, FALSE, $print_cascade_values_option, $owner->type());
					$ei->_tmp['layout_keyword_replacements'][$keywords['fields'][$field_assetid]] = ob_get_contents();
					ob_end_flush();
				}
			}
			// if this is section keyword replacement
			$matches = Array();
			if (preg_match('/section_(\d+)_(name|description|values)/', $field_assetid, $matches)) {
				// get the parent of this metadata section
				$section_id = $matches[1];
				$keyword_suffix = $matches[2];
				$schemaid = $GLOBALS['SQ_SYSTEM']->am->getParents($section_id, 'metadata_schema');

				// Only display the section if its schema is applied to the asset,
				// or if the asset is not yet created
				if (empty($owner->id) || (array_intersect($schemas, array_keys($schemaid)) != Array())) {
					$section = $GLOBALS['SQ_SYSTEM']->am->getAsset($section_id);

					if ($keyword_suffix == 'values') {
						ob_start();
						$values = $mm->getMetadata($owner->id);
						$edit_fns = $section->getEditFns();
						$edit_fns->paintInlineValueInterface($section, $o, $values, $write_access, $print_cascade_values_option, $owner->type());
						$ei->_tmp['layout_keyword_replacements'][$keywords['fields'][$field_assetid]] = ob_get_contents();
						ob_end_flush();
					} else {
						ob_start();
						echo $section->attr($keyword_suffix);
						$ei->_tmp['layout_keyword_replacements'][$keywords['fields'][$field_assetid]] = ob_get_contents();
						ob_end_flush();
					}
				}//end if
			}//end if
		}//end foreach
	}//end if

	return $print_commit_button;

}//end paintMetadata()


/**
* Process the interface for managing metadata values
*
* @param object	&$owner	the asset whose interface we are painting
* @param object	&$o		the outputter class
* @param object	$ei		the edit interface
*
* @return boolean
* @access public
*/
function processMetadata(&$owner, &$o, $ei)
{
	// if we dont have write access, dont process anything
	if (!$owner->writeAccess('metadata')) return FALSE;

	$cascade_values_asset_ids = Array();

	$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
	$schemas = $mm->getSchemas($owner->id, TRUE);
	$save = FALSE;
	if (!empty($schemas)) {
		$new_values = Array();
		foreach ($schemas as $schemaid) {
			$schema = $GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
			$edit_fns = $schema->getEditFns();
			if ($edit_fns->processInlineValueInterface($schema, $new_values, $cascade_values_asset_ids, $owner->type())) {
				$save = TRUE;
			}
		}
	}
	if ($save) {
 		// broadcast update event for all fields to refresh search index
 		// we are updating most fields here anyway (except non-editable fields), but those non-editable fields might contain keywords, so reindex them all together
 		$mm->setMetadata($owner->id, $new_values, NULL, TRUE, TRUE);
		
		// All context metadata files need to be regenerated if there is a non-contextable field changed
		$regen_all_contexts = FALSE;
		foreach ($new_values as $fieldid => $field_data) {
			$field = $GLOBALS['SQ_SYSTEM']->am->getAsset($fieldid);
			if (!$field) continue;
			
			if ((boolean)$field->attr('is_contextable') === FALSE || $field instanceof Metadata_Field_Select) {
				$regen_all_contexts = TRUE;
				break;
			}
		}
		
		if ($regen_all_contexts) {
			$all_contexts = $GLOBALS['SQ_SYSTEM']->getAllContexts();
			foreach ($all_contexts as $contextid => $context_data) {
				$mm->generateContentFile($owner->id, FALSE, $contextid);
			}
		} else {
			$mm->generateContentFile($owner->id);
		}

		// If the user has opted to cascade field values, we will handle these now
		if (!empty($cascade_values_asset_ids)) {
			$metadata_values_vars = Array();
			foreach($cascade_values_asset_ids as $asset_id => $val) {
				$field_value = Array($asset_id => $new_values[$asset_id]);
				$metadata_values_vars[$asset_id] = Array('field_asset_id' => $asset_id, 'field_value' => $field_value);
			}

			// Time to feed the HIPO
			$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$vars = Array(
					'assets'						=> Array($owner->id => Array('type_code' => $owner->type())),
					'metadata_field_value_changes'	=> $metadata_values_vars,
					'dependants_only'				=> 1,
					'contextid'						=> $GLOBALS['SQ_SYSTEM']->getContextId(),
				);

			$hh->queueHipo('hipo_job_edit_metadata_field_values', $vars);

		}
	}
	return FALSE; // no need to call saveAttributes

}//end processMetadata()


?>
