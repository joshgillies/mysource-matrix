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
* $Id: metadata.inc,v 1.30 2006/12/14 22:00:09 tbarrett Exp $
*
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* MetaData Static Screen Functions
*
* @author  Greg Sherwood <gsherwood@squiz.net>
* @version $Revision: 1.30 $
* @package MySource_Matrix
*/


/**
* Paint the interface for managing metadata values
*
* @param object		&$owner		the asset whose interface we are painting
* @param object		&$o			the outputter class
* @param object		&$ei		the edit interface
* @param boolean	$keywords	the keywords to replace
*
* @return boolean
* @access public
*/
function paintMetadata(&$owner, &$o, &$ei, $keywords=NULL)
{
	$root =& $ei->findScreen('static_screen_metadata');
	if (is_null($root)) return FALSE;

	$prefix = $owner->getPrefix();
	$print_commit_button = FALSE;

	$mm =& $GLOBALS['SQ_SYSTEM']->getMetadataManager();
	$schemas = $mm->getSchemas($owner->id, TRUE);

	$metadata_warnings = Array();

	for ($i = 0; $i < count($root->children); $i++) {

		$section =& $root->children[$i];

		// only do this if we are not painting a layout
		if (is_null($keywords)) {
			// if we are in limbo and this section is not to be printed in limbo - dont print it
			if (SQ_IN_LIMBO && (!isset($section->attributes['limbo_access']) || (int)$section->attributes['limbo_access'] == 0)) {
				continue;
			}
		}

		$section_access = $ei->_getAccess($owner, $section, $prefix);
		if (!($section_access & SQ_EI_READ)) {
			continue;
		}

		if (!is_null($keywords) && isset($keywords['sections'][strtolower($section->attributes['name'])])) {
			ob_start();
		}

		$o->openSection(array_get_index($section->attributes, 'display_name', $section->attributes['name']));
		if (!($num_fields = count($section->children))) {
			continue;
		}

		for ($j = 0; $j < $num_fields; $j++) {
			$field =& $section->children[$j];
			$field_access = $ei->_getAccess($owner, $field, $prefix);
			if (!($field_access & SQ_EI_READ)) {
				continue;
			}

			switch (strtolower($field->attributes['code'])) {

				///// CURRENT METADATA /////

				case 'metadata_current' :

					$o->openField('');
						if (!is_null($keywords) && isset($keywords['fields']['metadata_current'])) {
							ob_start();
						}

						if (empty($schemas)) {
							echo translate('no_metadata_schemas_applied');
						} else if (file_exists($owner->data_path.'/metadata.php')) {
							ob_start();
								require($owner->data_path.'/metadata.php');
								$metadata = ob_get_contents();
							ob_end_clean();
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

						$sm =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('search_manager', TRUE);
						if (!empty($sm) && $sm->attr('indexing')) {
						?>
							<?php
							$replacements_link = '<a href="'.sq_web_path('data').'/asset_types/metadata_field/popups/keyword_extraction.php?assetid='.$owner->id.'" onclick="var win = window.open(this.href, \'keywords\', \'toolbar=0,menubar=0,location=0,status=0,scrollbars=1,resizable=1,width=580,height=320\'); return false;">'.translate('keyword_extraction').'</a>';
							echo '<p>'.translate('metadata_indexed_keywords', $replacements_link).'</p>';

						}

						// paint the instructions for 'Use Default'
						echo '<p>'.translate('metadata_explain_use_default').'</p>';

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
						$print_commit_button = $write_access;

						if (!is_null($keywords) && isset($keywords['fields']['metadata_values'])) {
							ob_start();
						}

						$values = $mm->getMetadata($owner->id);
						foreach ($schemas as $schemaid) {
							$schema =& $GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
							if (is_null($schema)) continue;
							$edit_fns = $schema->getEditFns();
							$edit_fns->paintInlineValueInterface($schema, $o, $values, $write_access);
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

			if ($field->children[1]->name == 'note' && $field->children[1]->content) {
				$o->note($field->children[1]->content);
			}

		}//end for
		$o->closeSection();

		if (!is_null($keywords) && isset($keywords['sections'][strtolower($section->attributes['name'])])) {
			$ei->_tmp['layout_keyword_replacements'][$keywords['sections'][strtolower($section->attributes['name'])]] = ob_get_contents();
			ob_end_flush();
		}

	}//end for
	// prints a sq backend warning message if we have same metadata field names
	$field_names = Array();
	$duplicate_field_names = Array();
	$field_ids = $mm->getMetadataFields($schemas);
	if (!empty($field_ids)) {
		foreach ($field_ids as $field_id => $field_type) {
			$field =& $GLOBALS['SQ_SYSTEM']->am->getAsset($field_id);
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

	// set the keyword replacements for any numbered field keywords
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

					$field =& $GLOBALS['SQ_SYSTEM']->am->getAsset($field_assetid);
					$edit_fns = $field->getEditFns();

					ob_start();
					$edit_fns->paintValueInterface($field, $o, $value, $write_access, FALSE);
					$ei->_tmp['layout_keyword_replacements'][$keywords['fields'][$field_assetid]] = ob_get_contents();
					ob_end_flush();
				}
			}
		}
	}

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

	$mm =& $GLOBALS['SQ_SYSTEM']->getMetadataManager();

	$schemas = $mm->getSchemas($owner->id, TRUE);

	if (!empty($schemas)) {
		$new_values = Array();
		foreach ($schemas as $schemaid) {
			$schema =& $GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
			$edit_fns = $schema->getEditFns();
			$edit_fns->processInlineValueInterface($schema, $new_values);
		}
		if (!$mm->setMetadata($owner->id, $new_values)) {
			return FALSE;
		}
	}
	$mm->generateContentFile($owner->id);
	return FALSE; // no need to call saveAttributes

}//end processMetadata()


?>
