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
* $Id: tagging.inc,v 1.25 2012/08/30 01:09:20 ewang Exp $
*
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Tagging Static Screen Functions
*
* @author  Greg Sherwood <gsherwood@squiz.net>
* @version $Revision: 1.25 $
* @package MySource_Matrix
*/


/**
* Paint the interface for managing tagging values
*
* @param object	&$owner		the asset whose interface we are painting
* @param object	&$o			the outputter class
* @param object	&$ei		the edit interface
* @param object	$keywords	keywords
*
* @return boolean
* @access public
*/
function paintTagging(&$owner, &$o, &$ei, $keywords=NULL)
{// TODO TOF TEST
	$root = $ei->findScreen('static_screen_tagging');
	if (is_null($root)) return FALSE;
	$write_access = $owner->writeAccess('menu');
	$prefix = $owner->getPrefix();
	$print_commit_button = FALSE;

	foreach ($root->children() as $section) {

		// only do this if we are not painting a layout
		if (is_null($keywords)) {
			// if we are in limbo and this section is not to be printed in limbo - dont print it
			if (SQ_IN_LIMBO && (!isset($section->attributes()->limbo_access) || ((int)$section->attributes()->limbo_access) == 0)) {
				continue;
			}
		}

		$section_access = $ei->_getAccess($owner, $section, $prefix);
		if (!($section_access & SQ_EI_READ)) {
			continue;
		}

		if (!is_null($keywords) && isset($keywords['sections'][strtolower(   str_replace(' ', '_', (string)$section->attributes()->name)    )])) {
			ob_start();
		}

		$section_name = isset($section->attributes()->display_name) ? $section->attributes()->display_name : $section->attributes()->name;
		$o->openSection($section_name);

		if (!($num_fields = count($section->children()))) {
			continue;
		}

		foreach ($section->children() as $field) {

			$field_access = $ei->_getAccess($owner, $field, $prefix);
			if (!($field_access & SQ_EI_READ)) {
				continue;
			}

			switch (strtolower((string)$field->attributes()->code)) {

				case 'suggest_tags' :



					$o->openField('');
					if (!is_null($keywords) && isset($keywords['fields'][strtolower((string)$field->attributes()->code)])) {
						ob_start();
					}

					if ($write_access) {
						ob_start();
						$sm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('search_manager', TRUE);
						if (!empty($sm)) {
							ob_start();
								?>
<a href="#" onClick="Javascript: var win = window.open('<?php echo sq_web_path('lib'); ?>/tag_suggestion/tag_suggestion.php?assetid=<?php echo $owner->id; ?>&prefix=<?php echo $prefix; ?>', 'keywords', 'toolbar=0,menubar=0,location=0,status=0,scrollbars=1,resizable=1,width=580,height=320'); return FALSE;">
	<?php echo translate('Tag Suggestion') ?></a>

<?php
								$replacements_link = ob_get_contents();
							ob_end_clean();
							echo(sprintf(translate('If indexing is enabled in the Search Manager, you can generate a list of tags from the index using %s'), $replacements_link));

						}
						ob_end_flush();
					}
					if (!is_null($keywords) && isset($keywords['fields'][strtolower((string)$field->attributes()->code)])) {
						$ei->_tmp['layout_keyword_replacements'][$keywords['fields'][strtolower((string)$field->attributes()->code)]] = ob_get_contents();
						ob_end_flush();
					}
					$o->closeField();
				break;

				case 'tags' :

					$o->openField('');
					if (!is_null($keywords) && isset($keywords['fields'][strtolower((string)$field->attributes()->code)])) {
						ob_start();
					}

					$tag_manager = $GLOBALS['SQ_SYSTEM']->getTagManager();
					$current_tag_links = $tag_manager->getTagLinks($owner->id);
					ob_start();
					if ($write_access) {

						// there will be multiple tags, so start by getting a list of links to this assets to display
						$current_tag_ids = Array();
						foreach ($current_tag_links as $key => $current_tag_link) {
							$current_tag_ids[$key] = $current_tag_link['minorid'];
						}

						if ($tag_manager->isTagSourceRestricted()) {
							// restricted; check whether there's any thesaurus to choose from
							if ($tag_manager->attr('thesaurus')) {
								multiple_asset_finder($prefix.'_tags', $current_tag_ids, Array('thesaurus_term'=>'D'));
								$o->note(sprintf(translate('Note: You can only select tags from %s'), get_asset_tag_line($tag_manager->attr('thesaurus'), 'details')));

							} else {
								$o->note(translate('Note: You can only select tags from the tagging thesaurus. Please set a tagging thesaurus in the tag manager'));

							}
						} else {
							// unrestricted; just print the asset finder
							multiple_asset_finder($prefix.'_tags', $current_tag_ids, Array('thesaurus_term'=>'D'));
						}

					} else {
						foreach ($current_tag_links as $key => $current_tag_link) {
							echo get_asset_tag_line($current_tag_link['minorid']).'<br>';
						}
					}

					if (isset($field->note) && (strlen((string)$field->note) > 0)) {
						$o->note((string)$field->note);
					}
					ob_end_flush();
					if (!is_null($keywords) && isset($keywords['fields'][strtolower((string)$field->attributes()->code)])) {
						$ei->_tmp['layout_keyword_replacements'][$keywords['fields'][strtolower((string)$field->attributes()->code)]] = ob_get_contents();
						ob_end_flush();
					}
					$o->closeField();
				break;

				case 'weights' :

					$tag_manager = $GLOBALS['SQ_SYSTEM']->getTagManager();
					$current_tag_links = $tag_manager->getTagLinks($owner->id);
					$o->openField('');
					if (!is_null($keywords) && isset($keywords['fields'][strtolower((string)$field->attributes()->code)])) {
						ob_start();
					}

					ob_start();

					if ($write_access) {
						foreach ($current_tag_links as $key => $current_tag_link) {
							echo get_asset_tag_line($current_tag_link['minorid']).text_box($prefix.'_weights['.$current_tag_link['linkid'].']', $current_tag_link['value'], 4).'<br>';
						}
					} else {
						foreach ($current_tag_links as $key => $current_tag_link) {
							echo get_asset_tag_line($current_tag_link['minorid']).' '.translate('Weight').': '.$current_tag_link['value'].'<br>';

						}
					}

					if (isset($field->note) && (strlen((string)$field->note) > 0)) {
						$o->note((string)$field->note);
					}

					ob_end_flush();

					if (!is_null($keywords) && isset($keywords['fields'][strtolower((string)$field->attributes()->code)])) {
						$ei->_tmp['layout_keyword_replacements'][$keywords['fields'][strtolower((string)$field->attributes()->code)]] = ob_get_contents();
						ob_end_flush();
					}
					$o->closeField();
				break;

				case 'cascading' :
					if ($write_access) {
						$kiddies = $GLOBALS['SQ_SYSTEM']->am->getChildren($owner->id, Array(), FALSE, FALSE);
						$kiddies = array_filter($kiddies);
						if (count($kiddies)) {
							if (!is_null($keywords) && isset($keywords['fields'][strtolower((string)$field->attributes()->code)])) {
								ob_start();
							}

							$o->openSection(translate('Cascade Options'));

								$o->openField('Note');
									echo sprintf(translate('These actions will affect <strong>%1$s</strong> other assets below "<strong>%2$s</strong>"'), count($kiddies), htmlspecialchars($owner->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET));

								$o->closeField();
								$o->openField('&nbsp;');
									check_box($prefix.'_tagging[cascade_changes]', '1', TRUE);
									label(translate('Cascade tag changes'), $prefix.'_tagging[cascade_changes]');

									echo '<br/>';

										$o->note(translate('Apply all the tag changes made to this asset to all its children'));

									check_box($prefix.'_tagging[manual_cascade]');
									label(translate('Manually cascade all tags'), $prefix.'_tagging[manual_cascade]');

									echo '<br/>';

									$o->note(translate('Applies all of this asset\'s tags to all of its children'));


								$o->closeField();

							$o->closeSection();
							if (!is_null($keywords) && isset($keywords['fields'][strtolower((string)$field->attributes()->code)])) {
								$ei->_tmp['layout_keyword_replacements'][$keywords['fields'][strtolower((string)$field->attributes()->code)]] = ob_get_contents();
								ob_end_flush();
							}


						}
					}
				break;

			}//end switch

		}//end for

		$o->closeSection();

		if (!is_null($keywords) && isset($keywords['sections'][strtolower(    str_replace(' ', '_', (string)$section->attributes()->name)        )])) {
			$ei->_tmp['layout_keyword_replacements'][$keywords['sections'][strtolower(             str_replace(' ', '_', (string)$section->attributes()->name)            )]] = ob_get_contents();
			ob_end_flush();
		}
	}//end for

	return $write_access;

}//end paintTagging()


/**
* Paint the interface for viewing Tags (used in popups etc)
*
* @param object	&$owner	the asset whose interface we are painting
* @param object	&$o		the outputter class
* @param object	&$ei	the edit interface
*
* @return boolean
* @access public
*/
function paintInlineTagging(&$owner, &$o, &$ei)
{
	$prefix = $owner->getPrefix();

	$tag_manager = $GLOBALS['SQ_SYSTEM']->getTagManager();
	$current_tag_links = $tag_manager->getTagLinks($owner->id);

	if (empty($current_tag_links)) return FALSE;

		$o->openSection(translate('Content Tags'));

		$o->openField('&nbsp;');
		?>
		<table class="sq-backend-table">
		<tr>
				<td class="sq-backend-table-header">
					<?php echo translate('Content Tag'); ?>

				</td>
				<td class="sq-backend-table-header" align="right">
					<?php echo translate('Weight'); ?>

				</td>
			</tr>

			<?php
			foreach ($current_tag_links as $key => $current_tag_link) {
				?>
					<tr>
						<td class="sq-backend-table-cell">
						<?php
						echo get_asset_tag_line($current_tag_link['minorid']);
						?>
						</td>
						<td class="sq-backend-table-cell" align="right">
						<?php
						echo $current_tag_link['value'];
						?>
						</td>
					</tr>
				<?php
			}
		?></table><?php
		$o->closeField();
	$o->closeSection();

	return TRUE;

}//end paintInlineTagging()


/**
* Process the interface for managing asset tag values
*
* Returns TRUE is there is a change, FALSE on error or no change
*
* @param object	&$owner	the asset whose interface we are painting
* @param object	&$o		the outputter class
* @param object	$ei		the edit interface
*
* @return boolean
* @access public
*/
function processTagging(&$owner, &$o, $ei)
{
	// if we dont have write access, dont process anything
	if (!$owner->writeAccess('menu')) return FALSE;
	$prefix = $owner->getPrefix();

	$tag_manager = $GLOBALS['SQ_SYSTEM']->getTagManager();
	$submitted_tags = array_get_index($_REQUEST, $prefix.'_tags', Array());
	// are we cascading these tag changes to our children? If we are doing a manual cascade, that is handled later, after changes have been made to the parent asset.
	$cascade_changes = FALSE;
	if (isset($_POST[$prefix.'_tagging']['cascade_changes']) && $_POST[$prefix.'_tagging']['cascade_changes'] ) {

		$cascade_changes = TRUE;
	}


	// are we manually cascading?
	$manual_cascade = FALSE;
	if (isset($_POST[$prefix.'_tagging']['manual_cascade']) && $_POST[$prefix.'_tagging']['manual_cascade'] ) {
		$manual_cascade = TRUE;
		$cascade_changes = FALSE;
	}


	$tag_vars = Array();
	// Build a 'to do' list for the hippo job with additions, deletions and weight changes
	foreach ($submitted_tags as $key => $submitted_tag) {
		$submitted_tag_ids[$key] = $submitted_tag['assetid'];
	}

	$current_tag_links = $tag_manager->getTagLinks($owner->id);
	$current_tag_ids = Array();
	$current_weights = Array();

	foreach ($current_tag_links as $key => $current_tag_link) {
		$current_tag_ids[$key] = $current_tag_link['minorid'];
		$current_weights[$current_tag_link['linkid']] = $current_tag_link['value'];
	}


	// Insert any new tags
	$new_tag_ids = array_diff($submitted_tag_ids, $current_tag_ids);
	foreach ($new_tag_ids as $one_tag_id) {

		if (empty($one_tag_id)) continue;

		if ($cascade_changes) {
				// cascading creation, so queue it up for the HIPO
				$tag_vars[] = Array(
								'tag_id'	=> $one_tag_id,
								'action'	=> 'add',
								'weight'	=> '1',
							  );
		} else {
			$tag_manager->setTag($owner->id, $one_tag_id, '1');
		}
	}


	$submitted_weights = array_get_index($_REQUEST, $prefix.'_weights', Array());
	foreach ($submitted_weights as $linkid => $weight) {
		if (empty($weight)) $submitted_weights[$linkid] = 1;
	}

	$modified_weights = array_diff_assoc($submitted_weights, $current_weights);
	foreach ($modified_weights as $linkid => $weight) {
		if ($cascade_changes) {
			$link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($linkid);
			// cascading add, so queue it up for the HIPO
			$tag_vars[] = Array(
							'tag_id'	=> $link['minorid'],
							'action'	=> 'add',
							'weight'	=> $weight,
						  );
		} else {
			$tag_manager->setTagWeight($linkid, $weight);
		}
	}


	// delete any tags that haven't been resubmitted
	$deleted_tag_ids = array_diff($current_tag_ids, $submitted_tag_ids);
	foreach ($deleted_tag_ids as $one_tag_id) {
		if (empty($one_tag_id)) continue;

		if ($cascade_changes) {
			// cascading delete, so queue it up for the HIPO
			$tag_vars[] = Array(
							'tag_id'	=> $one_tag_id,
							'action'	=> 'delete',
							'weight'	=> NULL,
						  );
		} else {
			$tag_manager->deleteTag($owner->id, $one_tag_id);
		}
	}


	// to perform a manual cascade, we make any changes necessary to the parent asset, then build a list of actions required to add all the tags of the parent to any children
	if ($manual_cascade) {
		$current_tag_links = $tag_manager->getTagLinks($owner->id);

		foreach ($current_tag_links as $link) {
			$tag_vars[] = Array(
							'tag_id'	=> $link['minorid'],
							'action'	=> 'add',
							'weight'	=> $link['value'],
						  );
		}
	}


	if (!empty($tag_vars)) {
		$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$vars = Array(
					'assets'		=> Array(
										$owner->id	=> Array(
														'type_code'	=> $owner->type(),
													   ),
									   ),
					'tag_changes'	=> $tag_vars,
				);

		$hh->queueHipo('hipo_job_edit_tags', $vars);
	}

	// no need to call saveAttributes
	return FALSE;

}//end processTagging()


?>
