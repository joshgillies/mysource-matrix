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
* $Id: metadata.inc,v 1.22.2.2 2006/09/21 22:54:03 colivar Exp $
*
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* MetaData Static Screen Functions
*
* @author  Greg Sherwood <gsherwood@squiz.net>
* @version $Revision: 1.22.2.2 $
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
		echo '<tr><td>';
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

					$instructions = Array();

					$o->openField('&nbsp;');
						if (!is_null($keywords) && isset($keywords['fields']['metadata_instructions'])) {
							ob_start();
						}

						// construct example keywords link
						ob_start();
							?><a href="#" onClick="Javascript: var win = window.open('<?php echo sq_web_path('data'); ?>/asset_types/metadata_field/popups/keywords.php?assetid=<?php echo $owner->id; ?>&all=0', 'keywords', 'toolbar=0,menubar=0,location=0,status=0,scrollbars=1,resizable=1,width=580,height=520'); return false;"><?php echo translate('click_here'); ?></a><?php
							$replacements_link = ob_get_contents();
						ob_end_clean();

						// $replacements_link = '<a href="#" onClick="Javascript: var win = window.open(\''.sq_web_path('data').'/asset_types/metadata_field/popups/keywords.html\', \'keywords\', \'toolbar=0,menubar=0,location=0,status=0,scrollbars=1,resizable=1,width=580,height=520\'); return false;">'.translate('click_here').'</a>';
						$instructions[] = translate('metadata_keyword_replacements_usage', $replacements_link);

						$sm =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('search_manager', TRUE);
						if (!empty($sm)) {
						?>
							<?php
							ob_start();
								?><a href="#" onClick="Javascript: var win = window.open('<?php echo sq_web_path('data'); ?>/asset_types/metadata_field/popups/keyword_extraction.php?assetid=<?php echo $owner->id; ?>', 'keywords', 'toolbar=0,menubar=0,location=0,status=0,scrollbars=1,resizable=1,width=580,height=320'); return false;"><?php echo translate('keyword_extraction'); ?></a><?php
								$replacements_link = ob_get_contents();
							ob_end_clean();
							// $replacements_link = '<a href="#" onClick="Javascript: var win = window.open(\''.sq_web_path('data').'/asset_types/metadata_field/popups/keyword_extraction.php?assetid='.$owner->id.'\', \'keywords\', \'toolbar=0,menubar=0,location=0,status=0,scrollbars=1,resizable=1,width=580,height=320\'); return false;">'.translate('keyword_extraction').'</a>';
							$instructions[] = translate('metadata_indexed_keywords', $replacements_link);

						}

						// paint the instructions for 'Use Default'
						$instructions[] = translate('metadata_explain_use_default');

						// paint required field
						ob_start();
						?><span style="color:#FF0000"> * </span><?php echo translate('denotes_required_field'); ?><?php
						$instructions[] = ob_get_contents();
						ob_end_clean();

						echo implode('<br/><br/>', $instructions);

						// if there is a keyword replacement for this, add it to the replacements
						if (!is_null($keywords) && isset($keywords['fields']['metadata_instructions'])) {
							$ei->_tmp['layout_keyword_replacements'][$keywords['fields']['metadata_instructions']] = ob_get_contents();
							ob_end_flush();
						}
					$o->closeField();

					// we close the section so each of the metadata schemas printed below us have their own
					// section headings based on their schema names - makes the interface cleaner
					// $o->closeSection(); this line was commented to fix bug #1665

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
							$ei->_tmp['layout_keyword_replacements'][$keywords['fields']['metadata_values']] = ob_get_contents();
							ob_end_flush();
						}
					}

				break;

			}//end switch

			if ($field->children[1]->name == 'note' && $field->children[1]->content) {
				$o->note($field->children[1]->content);
			}

		}//end for

		echo '</td></tr>';
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
