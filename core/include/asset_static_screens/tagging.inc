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
* $Id: tagging.inc,v 1.10.2.1 2006/03/14 17:00:20 gnoel Exp $
*
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Tagging Static Screen Functions
*
* @author  Greg Sherwood <gsherwood@squiz.net>
* @version $Revision: 1.10.2.1 $
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
{
	$root =& $ei->findScreen('static_screen_tagging');
	if (is_null($root)) return FALSE;
	$write_access = $owner->writeAccess('links');
	$prefix = $owner->getPrefix();
	$print_commit_button = FALSE;


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

				case 'suggest_tags' :

					$o->openField('');

					if ($write_access) {
						ob_start();
						$sm =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('search_manager', TRUE);
						if (!empty($sm)) {
							ob_start();
								?>
<a href="#" onClick="Javascript: var win = window.open('<?php echo sq_web_path('lib'); ?>/tag_suggestion/tag_suggestion.php?assetid=<?php echo $owner->id; ?>&prefix=<?php echo $prefix; ?>', 'keywords', 'toolbar=0,menubar=0,location=0,status=0,scrollbars=1,resizable=1,width=580,height=320'); return FALSE;">
	<?php echo translate('tag_suggestion') ?></a>
<?php
								$replacements_link = ob_get_contents();
							ob_end_clean();
							echo(translate('tagging_suggest_indexed', $replacements_link));
						}
						ob_end_flush();
					}
					$o->closeField();
				break;

				case 'tags' :

					$o->openField('');
					$tag_manager =& $GLOBALS['SQ_SYSTEM']->getTagManager();
					$current_tag_links = $tag_manager->getTagLinks($owner->id);
					ob_start();
					if ($write_access) {

						// there will be multiple tags, so start by getting a list of links to this assets to display
						$current_tag_ids = Array();
						foreach ($current_tag_links as $key => $current_tag_link) {
							$current_tag_ids[$key] = $current_tag_link['minorid'];
						}
						multiple_asset_finder($prefix.'_tags', $current_tag_ids, Array('thesaurus_term'=>'D'));
					} else {
						foreach ($current_tag_links as $key => $current_tag_link) {
							echo get_asset_tag_line($current_tag_link['minorid']).'<br>';
						}
					}

					if (isset($field->children[1]) && $field->children[1]->name == 'note' && $field->children[1]->content) {
						$o->note($field->children[1]->content);
					}
					ob_end_flush();
					$o->closeField();
				break;

				case 'weights' :

					$tag_manager =& $GLOBALS['SQ_SYSTEM']->getTagManager();
					$current_tag_links = $tag_manager->getTagLinks($owner->id);
					$o->openField('');
					ob_start();

					if ($write_access) {
						foreach ($current_tag_links as $key => $current_tag_link) {
							echo get_asset_tag_line($current_tag_link['minorid']).text_box($prefix.'_weights['.$current_tag_link['linkid'].']', $current_tag_link['value'], 4).'<br>';
						}
					} else {
						foreach ($current_tag_links as $key => $current_tag_link) {
							echo get_asset_tag_line($current_tag_link['minorid']).' '.translate('weight').': '.$current_tag_link['value'].'<br>';
						}
					}

					if (isset($field->children[1]) && $field->children[1]->name == 'note' && $field->children[1]->content) {
						$o->note($field->children[1]->content);
					}

					ob_end_flush();
					$o->closeField();
				break;

				case 'cascading' :
					if ($write_access) {
						$kiddies = $GLOBALS['SQ_SYSTEM']->am->getChildren($owner->id, Array(), FALSE, FALSE);
						$kiddies = array_filter($kiddies);
						if (count($kiddies)) {
							$o->openSection(translate('cascade_options'));
								$o->openField('Note');
									echo translate('action_affects_assets_below_this', count($kiddies), $owner->name);
								$o->closeField();
								$o->openField('&nbsp;');
									check_box($prefix.'_tagging[cascade_changes]', '1', TRUE);
									echo(translate('cascade_tag_changes'));
									echo '<br/>';

										$o->note(translate('tagging_cascade_changes_note'));
									check_box($prefix.'_tagging[manual_cascade]');
									echo(translate('manually_cascade_all_tags'));
									echo '<br/>';

									$o->note(translate('tagging_manual_cascade_note'));

								$o->closeField();

							$o->closeSection();

						}
					}
				break;

			}//end switch

		}//end for

		$o->closeSection();

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

	$tag_manager =& $GLOBALS['SQ_SYSTEM']->getTagManager();
	$current_tag_links = $tag_manager->getTagLinks($owner->id);

	if (empty($current_tag_links)) return FALSE;

		$o->openSection(translate('content_tags'));
		$o->openField('&nbsp;');
		?>
		<table class="sq-backend-table">
		<tr>
				<td class="sq-backend-table-header">
					<?php echo translate('content_tag'); ?>
				</td>
				<td class="sq-backend-table-header" align="right">
					<?php echo translate('weight'); ?>
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
	// if we dont have admin access, go away
	if ($owner->id && !$owner->adminAccess('')) {
		$GLOBALS['SQ_SYSTEM']->paintLogin(translate('login'), translate('cannot_access_asset', $owner->name));
		exit();
	}
	// if we dont have write access, dont process anything
	if (!$owner->writeAccess('all')) return FALSE;
	$prefix = $owner->getPrefix();

	$tag_manager =& $GLOBALS['SQ_SYSTEM']->getTagManager();

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
		$hh =& $GLOBALS['SQ_SYSTEM']->getHipoHerder();
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
