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
* $Id: metadata.inc,v 1.7 2004/08/27 01:12:00 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* MetaData Static Screen Functions
*
* @author  Greg Sherwood <gsherwood@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/


/**
* Paint the interface for managing metadata values
*
* @param object	Asset					$owner		the asset whose interface we are painting
* @param object	Backend_Outputter		$o			the outputter class
* @param object	Asset_Edit_Interface	&$ei		the edit interface
*
* @access public
* @return boolean
*/
function paintMetadata(&$owner, &$o, &$ei, $keywords=null)
{
	if (!($file = $ei->_findFile('static_screen_metadata'))) return false;

	$prefix = $owner->getPrefix();
	$print_commit_button = false;

	$mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager();
	$schemas = $mm->getSchemas($owner->id, true);

	$input = new XML_Tree($file);
	$root  = &$input->getTreeFromFile();
	if (PEAR::isError($root)) {
		trigger_error('Unable to Read File : '.$file."\n".$root->getMessage()."\n".$root->getUserInfo(), E_USER_WARNING);
		return false;
	}

	for ($i = 0; $i < count($root->children); $i++) {

		$section = &$root->children[$i];

		// only do this if we are not painting a layout
		if (is_null($keywords)) {
			// if we are in limbo and this section is not to be printed in limbo - dont print it
			if (SQ_IN_LIMBO && (!isset($section->attributes['limbo_access']) || (int)$section->attributes['limbo_access'] == 0)) continue;
		}

		$section_access = $ei->_getAccess($owner, $section);
		if (!($section_access & SQ_EI_READ)) continue;

		if (!is_null($keywords) && isset($keywords['sections'][strtolower($section->attributes['name'])])) ob_start();

		$o->openSection($section->attributes['name']);

		if (!($num_fields = count($section->children))) continue;

		for ($j = 0; $j < $num_fields; $j++) {
			$field = &$section->children[$j];

			switch (strtolower($field->attributes['code'])) {

				///// CURRENT METADATA ////

				case 'metadata_current' :

					$o->openField('');
						if (!is_null($keywords) && isset($keywords['fields']['metadata_current'])) ob_start();

						if (empty($schemas)) {
							echo 'There are no metadata schemas applied to this asset.';
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

				///// METADATA INSTRUCTIONS ////

				case 'metadata_instructions' :

					$o->openField('&nbsp;');
						if (!is_null($keywords) && isset($keywords['fields']['metadata_instructions'])) ob_start();

						?>
						You can use keyword replacements in place of values for metadata fields on this page.<br/>
						For a complete list of available keyword replacements <a href="#" onClick="Javascript: var win = window.open('<?php echo sq_web_path('data')?>/asset_types/metadata_field/popups/keywords.html', 'keywords', 'toolbar=0,menubar=0,location=0,status=0,scrollbars=1,resizable=1,width=580,height=520'); return false;">click here</a>
						<?php
						$sm =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('search_manager', true);
						if (!empty($sm)) {
						?>
							<br/><br/>
							If indexing is enabled in the Search Manager, you can generate a list of keywords from the index using
							<a href="#" onClick="Javascript: var win = window.open('<?php echo sq_web_path('data')?>/asset_types/metadata_field/popups/keyword_extraction.php?assetid=<?php echo $owner->id ?>', 'keywords', 'toolbar=0,menubar=0,location=0,status=0,scrollbars=1,resizable=1,width=580,height=320'); return false;">keyword extraction</a>.
						<?php
						}
						?>
						<br/><br/>
						<span style="color:#FF0000"> * </span>denotes a required field
						<?php

						// if there is a keyword replacement for this, add it to the replacements
						if (!is_null($keywords) && isset($keywords['fields']['metadata_instructions'])) {
							$ei->_tmp['layout_keyword_replacements'][$keywords['fields']['metadata_instructions']] = ob_get_contents();
							ob_end_flush();
						}
					$o->closeField();

					// we close the section so each of the metadata schemas printed below us have their own
					// section headings based on their schema names - makes the interface cleaner
					$o->closeSection();

				break;

				///// METADATA EDITING INTERFACE ////

				case 'metadata_values' :

					if (empty($schemas)) {
						$o->openField('&nbsp;');
							if (!is_null($keywords) && isset($keywords['fields']['metadata_values'])) ob_start();

							echo 'You must apply one or more schemas before entering metadata values.';

							// if there is a keyword replacement for this, add it to the replacements
							if (!is_null($keywords) && isset($keywords['fields']['metadata_values'])) {
								$ei->_tmp['layout_keyword_replacements'][$keywords['fields']['metadata_values']] = ob_get_contents();
								ob_end_flush();
							}
						$o->closeField();
					} else {
						$write_access = $owner->writeAccess('metadata');
						$print_commit_button = $write_access;

						if (!is_null($keywords) && isset($keywords['fields']['metadata_values'])) ob_start();

						foreach ($schemas as $schemaid) {
							$schema = &$GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
							if (is_null($schema)) continue;
							$values = $mm->getSchemaMetadata($owner->id, $schema->id);
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

		$o->closeSection();

		if (!is_null($keywords) && isset($keywords['sections'][strtolower($section->attributes['name'])])) {
			$ei->_tmp['layout_keyword_replacements'][$keywords['sections'][strtolower($section->attributes['name'])]] = ob_get_contents();
			ob_end_flush();
		}

	}//end for

	return $print_commit_button;

}//end paintMetadata()


/**
* Process the interface for managing metadata values
*
* @param object	Asset					$owner	the asset whose interface we are painting
* @param object	Backend_Outputter		$o		the outputter class
* @param object	Asset_Edit_Interface	&$ei	the edit interface
*
* @access public
* @return boolean TRUE is there is a change, false on error or no change
*/
function processMetadata(&$owner, &$o, $ei)
{
	// if we dont have write access, dont process anything
	if (!$owner->writeAccess('metadata')) return false;

	$mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager();

	$schemas = $mm->getSchemas($owner->id, true);

	if (!empty($schemas)) {
		$new_values = Array();
		foreach ($schemas as $schemaid) {
			$schema = &$GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
			$edit_fns = $schema->getEditFns();
			$edit_fns->processInlineValueInterface($schema, $new_values);
		}
		if (!$mm->setMetadata($owner->id, $new_values)) return false;
	}
	return $mm->generateContentFile($owner->id);

}//end processMetadata()


?>