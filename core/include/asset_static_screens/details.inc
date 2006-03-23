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
* $Id: details.inc,v 1.16.2.3 2006/03/23 00:20:44 sdanis Exp $
*
*/

require_once 'XML/Tree.php';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Details Static Screen Functions
*
* @author  Greg Sherwood <gsherwood@squiz.net>
* @version $Revision: 1.16.2.3 $
* @package MySource_Matrix
*/


/**
* Paints the interface for editing details
*
* @param object	Asset					&$owner		the asset whose interface we are painting
* @param object	Backend_Outputter		&$o			the outputter class
* @param object	Asset_Edit_Interface	&$ei		the edit interface
*
* @return boolean
* @access public
*/
function paintDetails(&$owner, &$o, &$ei, $keywords=null)
{
	$root = &$ei->findScreen('static_screen_details');
	if (is_null($root)) {
		return false;
	}

	$prefix = $owner->getPrefix();
	$print_commit_button = false;

	$display_status_sections = TRUE;
	$parent_asset_href = '';
	// check if the asset is linked to its parent exclusively and is dependent
	$parent_asset_link = $GLOBALS['SQ_SYSTEM']->am->getLinks($owner->id, SQ_SC_LINK_ALL, '', TRUE, 'minor', NULL, TRUE, TRUE);
	if (!empty($parent_asset_link)) {
		$display_status_sections = FALSE;
		$parent_asset_href = $GLOBALS['SQ_SYSTEM']->am->getAssetBackendHref(Array($parent_asset_link[0]['majorid'] => 'details'));
	}

	for ($i = 0; $i < count($root->children); $i++) {

		$section = &$root->children[$i];

		// only do this if we are not painting a layout
		if (is_null($keywords)) {
			// if we are in limbo and this section is not to be printed in limbo - dont print it
			if (SQ_IN_LIMBO && (!isset($section->attributes['limbo_access']) || (int)$section->attributes['limbo_access'] == 0)) continue;
		}

		if (!is_null($keywords) && isset($keywords['sections'][strtolower($section->attributes['name'])])) ob_start();

		$o->openSection(array_get_index($section->attributes, 'display_name', $section->attributes['name']));

		if (!($num_fields = count($section->children))) continue;

		for ($j = 0; $j < $num_fields; $j++) {
			$field = &$section->children[$j];

			switch (strtolower($field->attributes['code'])) {

				///// STATUS CHANGES - NOW AND IN THE FUTURE ////

				case 'status_current' :

					if ($field->children[0]->name != 'display_name') $display_name = translate('current');
					else $display_name = $field->children[0]->content;
					$o->openField($display_name);
					if (!is_null($keywords) && isset($keywords['fields']['status_current'])) ob_start();
					echo '<i>'.$owner->getStatusDescription().'</i>';

					// if there is a keyword replacement for this, add it to the replacements
					if (!is_null($keywords) && isset($keywords['fields']['status_current'])) {
						$ei->_tmp['layout_keyword_replacements'][$keywords['fields']['status_current']] = ob_get_contents();
						ob_end_flush();
					}

				break;

				case 'status_change' :

					if (!$display_status_sections) {
						$o->openField('');
							echo translate('asset_current_status_change_from_parent',$parent_asset_href[$parent_asset_link[0]['majorid']]);
						$o->closeField();
						break;
					}

					// only print this field if the status can be changed
					$statii = $owner->getAvailableStatii();
					if (empty($statii)) break;

					if ($field->children[0]->name != 'display_name') $display_name = translate('change_question');
					else $display_name = $field->children[0]->content;
					$o->openField($display_name);
						if (!is_null($keywords) && isset($keywords['fields']['status_change'])) ob_start();

						// if this asset is in workflow we do not let the change the status here
						// because they need to go off to the workflow screen
						$wfm = &$GLOBALS['SQ_SYSTEM']->getWorkflowManager();
						$running_schemas = $wfm->getSchemas($owner->id, true, true);
						if (!empty($running_schemas)) {
							$o->note(translate('asset_currently_in_workflow', $owner->getBackendHref('workflow')));
						} else {
							$available_statii = _paintStatusChange($owner, $o, $prefix);
							if (!empty($available_statii)) $print_commit_button = true;
						}
						if (!is_null($keywords) && isset($keywords['fields']['status_change'])) {
							$ei->_tmp['layout_keyword_replacements'][$keywords['fields']['status_change']] = ob_get_contents();
							ob_end_flush();
						}

					$o->closeField();

				break;

				case 'status_future_status' :

					if (!$display_status_sections) {
						$o->openField('');
							echo translate('asset_future_status_change_from_parent',$parent_asset_href[$parent_asset_link[0]['majorid']]);
						$o->closeField();
						break;
					}

					$cron_mgr = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');

					$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_future_status');
					$fs_jobs = &Cron_Job_Future_Status::getActiveJobs($owner);
					if (!empty($fs_jobs)) {
						$o->openField(translate('scheduled_future_status_change'), 'new_line');

						if (!is_null($keywords) && isset($keywords['fields']['status_future_status'])) ob_start();
						?>
						<table class="sq-backend-table">
							<tr>
								<td class="sq-backend-table-header"><?php echo translate('status'); ?></td>
								<td class="sq-backend-table-header"><?php echo translate('when'); ?></td>
								<td align="center" class="sq-backend-table-header"><?php echo translate('delete_question'); ?></td>
							</tr>
							<?php
							for ($k = 0; $k < count($fs_jobs); $k++) {
								?>
								<tr>
									<td class="sq-backend-table-cell">
										<?php echo $fs_jobs[$k]->statusName(); ?>
									</td>
									<td class="sq-backend-table-cell">
										<?php echo $fs_jobs[$k]->readableWhen(); ?>
									</td>
									<td align="center" class="sq-backend-table-cell">
									<?php
										if ($fs_jobs[$k]->canDelete()) {
											check_box($prefix.'_remove_fs[]', $fs_jobs[$k]->id);
										} else {
											echo '&nbsp;';
										}
									?>
									</td>
								</tr>
								<?php
							}//end for
							?>
						</table>
						<?php

						if (!is_null($keywords) && isset($keywords['fields']['status_future_status'])) {
							$ei->_tmp['layout_keyword_replacements'][$keywords['fields']['status_future_status']] = ob_get_contents();
							ob_end_flush();
						}

					}//end if

					if ($owner->writeAccess()) {
						$o->openField(translate('add_new_future_status'), 'new_line');
							$cron_epoc = (int)$cron_mgr->attr('epoch');
							$cron_refresh = (int)$cron_mgr->attr('refresh_time');
							if ($cron_epoc > 0 && $cron_refresh > 0) {
								require_once SQ_FUDGE_PATH.'/general/datetime.inc';
								$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_future_status');
								$fs = new Cron_Job_Future_Status();
								$edit_fns = $fs->getEditFns();

								if (!is_null($keywords) && isset($keywords['fields']['status_future_status'])) ob_start();
								$edit_fns->paintInLineBackend($fs, $o, $prefix.'_new_fs');

								$o->note(translate('cron_manager_next_run', $cron_mgr->readableRefreshTime(), readable_datetime($cron_mgr->timeOfNextRun())));

								if (!is_null($keywords) && isset($keywords['fields']['status_future_status'])) {
									// check to see if we added anything to the keyword replacements above
									if (!isset($ei->_tmp['layout_keyword_replacements'][$keywords['fields']['status_future_status']])) {
										$ei->_tmp['layout_keyword_replacements'][$keywords['fields']['status_future_status']] = '';
									}
									// append to the end of the replacement
									$ei->_tmp['layout_keyword_replacements'][$keywords['fields']['status_future_status']] .= ob_get_contents();
									ob_end_flush();
								}
								$print_commit_button = true;
							} else {
								echo translate('cron_manager_not_configured');
							}
						$o->closeField();
					}

					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($cron_mgr);

				break;

				case 'thumbnail_change' :

					if ($owner->writeAccess('links')) {

						if ($field->children[0]->name != 'display_name') $display_name = translate('change_question');
						else $display_name = $field->children[0]->content;

						$o->openField($display_name);
							if (!is_null($keywords) && isset($keywords['fields']['thumbnail_change'])) ob_start();

							$edit_fns = $owner->getEditFns();
							$edit_fns->paintNoticeLinkedAsset($owner, 'thumbnail', Array('thumbnail' => 'I'));

							if (!is_null($keywords) && isset($keywords['fields']['thumbnail_change'])) {
								$ei->_tmp['layout_keyword_replacements'][$keywords['fields']['thumbnail_change']] = ob_get_contents();
								ob_end_flush();
							}
						$o->closeField();
					}

				break;

				case 'thumbnail_current' :

					if ($field->children[0]->name != 'display_name') $display_name = translate('current');
					else $display_name = $field->children[0]->content;

					$o->openField($display_name);
						if (!is_null($keywords) && isset($keywords['fields']['thumbnail_current'])) ob_start();

						$link = $GLOBALS['SQ_SYSTEM']->am->getLink($owner->id, SQ_LINK_NOTICE, 'thumbnail', true, 'thumbnail');
						if (!empty($link)) {
							$thumbnail = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
							$edit_fns = $thumbnail->getEditFns();
							$edit_fns->printExistingFileInfo($thumbnail);

						} else {
							echo translate('no_thumbnail_attatched');
						}

						if (!is_null($keywords) && isset($keywords['fields']['thumbnail_current'])) {
							$ei->_tmp['layout_keyword_replacements'][$keywords['fields']['thumbnail_current']] = ob_get_contents();
							ob_end_flush();
						}
					$o->closeField();

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

}//end paintDetails()


/**
* Paints the interface for editing the status of an asset
*
* @param object	Asset				$owner	the asset whose interface we are painting
* @param object	Backend_Outputter	$o		the outputter class
* @param string						$prefix	prefix for form vars
*
* @return array of statii shown in the drop down
* @access public
*/
function _paintStatusChange(&$owner, &$o, $prefix)
{
	$statii = $owner->getAvailableStatii();

	if (!empty($statii)) {
		$statii = array_reverse($statii, true);
		$statii[0] = '';
		$statii = array_reverse($statii, true);
		combo_box($prefix.'_change_status', $statii, false, 0);
		echo '&nbsp;';
		check_box($prefix.'_change_status_all_children');
		echo translate('cascade_status_change');
	}

	return $statii;

}//end _paintStatusChange()


/**
* Processes the interface for editing the status of an asset
*
* @param object	Asset				$owner	the asset whose interface we are painting
* @param object	Backend_Outputter	$o		the outputter class
* @param string						$prefix	prefix for form vars
*
* @return boolean
* @access public
*/
function _processStatusChange(&$owner, &$o, $prefix)
{
	if (isset($_POST[$prefix.'_change_status']) && (int) $_POST[$prefix.'_change_status'] > 0) {
		$dependants_only = (isset($_POST[$prefix.'_change_status_all_children']) && $_POST[$prefix.'_change_status_all_children']) ? false : true;
		$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$vars = Array('assetid' => $owner->id, 'new_status' => $_POST[$prefix.'_change_status'], 'dependants_only' => $dependants_only);
		$hh->queueHipo('hipo_job_edit_status', $vars);
	}

}//end _processStatusChange()


/**
* Processes the interface for editing details
* returns true if there is a change
*
* @param object	Asset					$owner	the asset whose interface we are painting
* @param object	Backend_Outputter		$o		the outputter class
* @param object	Asset_Edit_Interface	&$ei	the edit interface
*
* @access public
* @return boolean
*/
function processDetails(&$owner, &$o, &$ei)
{
	$prefix = $owner->getPrefix();

	$is_updated = false;

	///// IMMEDIATE STATUS CHANGES ////

	// process a status change on this screen if workflow is NOT running
	$wfm = &$GLOBALS['SQ_SYSTEM']->getWorkflowManager();
	$running_schemas = $wfm->getSchemas($owner->id, true, true);
	if (empty($running_schemas)) _processStatusChange($owner, $o, $prefix);


	///// FUTURE STATUS CHANGES ////

	$cron_mgr = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
	if (!is_null($cron_mgr)) {
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_future_status');

		if (!empty($_POST[$prefix.'_remove_fs'])) {
			$fs_jobs = &Cron_Job_Future_Status::getActiveJobs($owner);
			if (!empty($fs_jobs)) {

				if ($GLOBALS['SQ_SYSTEM']->am->acquireLock($cron_mgr->id, 'links')) {
					for ($i = 0; $i < count($fs_jobs); $i++) {
						if (in_array($fs_jobs[$i]->id, $_POST[$prefix.'_remove_fs'])) {
							if ($cron_mgr->removeJob($fs_jobs[$i])) {
								$is_updated = true;
							}
						}// end if
					}// end for

					$GLOBALS['SQ_SYSTEM']->am->releaseLock($cron_mgr->id, 'links');

				} else {
					trigger_localised_error("SYS0232", E_USER_NOTICE);

				}// end if

			}// end if
		}// end if


		if ($owner->writeAccess('')) {

			$fs = new Cron_Job_Future_Status();
			$edit_fns = $fs->getEditFns();
			if ($edit_fns->processInLineBackend($fs, $o, $prefix.'_new_fs')) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
				if ($cron_mgr->addJob($fs, $GLOBALS['SQ_SYSTEM']->user)) {
					if ($GLOBALS['SQ_SYSTEM']->am->acquireLock($fs->id, 'links')) {
						if ($fs->setAssetToUpdate($owner)) {
							$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
							$is_updated = true;
							$GLOBALS['SQ_SYSTEM']->am->releaseLock($fs->id, 'links');
						} else {
							$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						}
					} else {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					}
				} else {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				}
			}// end if

		}// end if

	}//end if is null cron mgr


	///// THUMBNAIL CHANGES ////

	$edit_fns = $owner->getEditFns();
	if ($edit_fns->processNoticeLinkedAsset($owner, 'thumbnail')) {
		$is_updated = true;
	}

	return ($is_updated);

}//end processDetails()


?>