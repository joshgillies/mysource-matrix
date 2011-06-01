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
* $Id: details.inc,v 1.43 2008/01/21 03:04:24 hnguyen Exp $
*
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Details Static Screen Functions
*
* @author  Greg Sherwood <gsherwood@squiz.net>
* @version $Revision: 1.43 $
* @package MySource_Matrix
*/


/**
* Paints the interface for editing details
*
* @param object	&$owner		the asset whose interface we are painting
* @param object	&$o			the outputter class
* @param object	&$ei		the edit interface
* @param array	$keywords	the list of keywords
*
* @return boolean
* @access public
*/
function paintDetails(&$owner, &$o, &$ei, $keywords=NULL)
{
	// no static details screen fields relevant to an uncreated asset
	if (empty($owner->id)) return;

	$root = $ei->findScreen('static_screen_details');
	if (is_null($root)) return FALSE;

	$prefix = $owner->getPrefix();
	$print_commit_button = FALSE;

	$display_status_sections = TRUE;
	$parent_asset_href = '';
	// check if the asset is linked to its parent exclusively and is dependent
	$parent_asset_link = $GLOBALS['SQ_SYSTEM']->am->getLinks($owner->id, SQ_SC_LINK_ALL, '', TRUE, 'minor', NULL, TRUE, TRUE);
	if (!empty($parent_asset_link)) {
		$display_status_sections = FALSE;
		$parent_asset_href = $GLOBALS['SQ_SYSTEM']->am->getAssetBackendHref(Array($parent_asset_link[0]['majorid'] => 'details'));
	}

	foreach ($root->section as $section) {

		// only do this if we are not painting a layout
		if (is_null($keywords)) {
			// if we are in limbo and this section is not to be printed in limbo - dont print it
			if (SQ_IN_LIMBO && (!isset($section->attributes()->limbo_access) || (int)$section->attributes()->limbo_access == 0)) {
				continue;
			}
		}

		if (!is_null($keywords) && isset($keywords['sections'][strtolower(str_replace(' ', '_', $section->attributes()->name))])) {
			ob_start();
		}

		if (!$ei->_getAccess($owner, $section, $prefix)) {
			continue;
		}

		//
		$safe_trash_job_link = $GLOBALS['SQ_SYSTEM']->am->getLink($owner->id, SQ_LINK_NOTICE, 'cron_job_attempt_safe_trash', TRUE, NULL, 'minor');

		$avoiding_safe_trash_section = ($section->attributes()->name == 'Safe Trash Status') && !$safe_trash_job_link;
		if (!$avoiding_safe_trash_section) {
			$o->openSection(array_get_index($section->attributes(), 'display_name', $section->attributes()->name));
		}

		if (!($num_fields = count($section->children()))) {
			continue;
		}

		foreach ($section->field as $field) {

			switch (strtolower($field->attributes()->code)) {

				case 'safe_trash' :

					if (!$avoiding_safe_trash_section) {
						$safe_trash_enabled = $GLOBALS['SQ_SYSTEM']->getUserPrefs('user', 'SQ_USER_SAFE_TYPE3_TRASH');
						$safe_trash_errors = $GLOBALS['SQ_SYSTEM']->am->canSafeTrashAsset($owner->id);
						$o->openRaw();
							if ($safe_trash_enabled) {
								if (empty($safe_trash_errors)) {
								?>
								<div><?php echo translate('safe_trash_line_common'); ?><br/>
								<?php echo translate('safe_trash_line_unblocked'); ?></div>
								<?php
								} else {
								?>
								<div class="sq-backend-warning"><strong><?php echo translate('safe_trash_line_common'); ?><br/>
								<?php echo translate('safe_trash_line_still_blocked'); ?></strong></div>

								<ul>
								<?php
								if (!empty($safe_trash_errors['status'])) {
								?>
								<li><?php echo translate('safe_trash_line_blocked_status'); ?></li>
								<?php
								}
								?>
								<?php
								if (!empty($safe_trash_errors['links'])) {
								?>
								<li><?php echo translate('safe_trash_line_blocked_links', $owner->getBackendHref('linking')); ?></li>
								<?php
								}
								?>
								<?php
								if (!empty($safe_trash_errors['children'])) {
								?>
								<li><?php echo translate('safe_trash_line_blocked_children'); ?></li>
								<?php
								}
								?>
								</ul></div>
								<?php
								}
							} else {
								?>
								<div><?php echo translate('safe_trash_line_common'); ?><br/>
								<?php echo translate('safe_trash_line_disabled'); ?></div>
								<?php
							}

						$o->closeRaw();
					}

				break;

				///// STATUS CHANGES - NOW AND IN THE FUTURE ////

				case 'status_current' :

					if (!empty($field->display_name)) {
						$display_name = translate('current');
					} else {
						// If we dont have a display name node then we dont have anything to say
						$display_name = '';
					}
					$o->openField($display_name);
					if (!is_null($keywords) && isset($keywords['fields']['status_current'])) {
						ob_start();
					}
					echo get_asset_status_icon($owner->status);
					echo ' <i>'.$owner->getStatusDescription().'</i>';

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

					if (!empty($field->display_name)) {
						$display_name = translate('change_question');
					} else {
						// If we dont have a display name node then we dont have anything to say
						$display_name = '';
					}
					$o->openField($display_name);
						if (!is_null($keywords) && isset($keywords['fields']['status_change'])) {
							ob_start();
						}

						// if this asset is in workflow we do not let the change the status here
						// because they need to go off to the workflow screen
						$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
						$running_schemas = $wfm->getSchemas($owner->id, TRUE, TRUE);
						if (!empty($running_schemas)) {
							$o->note(translate('asset_currently_in_workflow', $owner->getBackendHref('workflow')));
						} else {
							_paintStatusChange($owner, $o, $prefix, $statii);
							$print_commit_button = TRUE;
							$status = $owner->getStatus();
							$note = $status->getStatusChangeMessage();
							if ($note) {
								$o->note($note);
							}
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

					// this makes assumptions about how to get the future status jobs for a particular asset, which is the domain of the cron manager.
					// The problem is that when we ask the future status job to get a list of jobs it, in turn, asks the cron manager to get a list of
					// jobs, which is turn asks the asset manager to get a list of links under the cron manager that are future status jobs.
					// In a system with a large number of future status jobs, the details screen will become very slow to load.
					$fs_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($owner->id, SQ_LINK_NOTICE, 'cron_job_future_status', TRUE, 'minor');

					if (!empty($fs_links)) {
						$o->openField(translate('scheduled_future_status_change'), 'new_line');

						if (!is_null($keywords) && isset($keywords['fields']['status_future_status'])) {
							ob_start();
						}
						?>
						<table class="sq-backend-table">
							<tr>
								<td class="sq-backend-table-header"><?php echo translate('status'); ?></td>
								<td class="sq-backend-table-header"><?php echo translate('when'); ?></td>
								<td align="center" class="sq-backend-table-header" style="text-align: center;"><?php echo translate('cascade_question'); ?></td>
								<td align="center" class="sq-backend-table-header" style="text-align: center;"><?php echo translate('delete_question'); ?></td>
							</tr>
							<?php
							for ($k = 0; $k < count($fs_links); $k++) {
								$fs_job = $GLOBALS['SQ_SYSTEM']->am->getAsset($fs_links[$k]['majorid'], 'cron_job_future_status');
								?>
								<tr>
									<td class="sq-backend-table-cell">
										<?php echo $fs_job->statusName(); ?>
									</td>
									<td class="sq-backend-table-cell">
										<?php echo $fs_job->readableWhen(); ?>
									</td>
									<td align="center" class="sq-backend-table-cell">
									<?php
										if ($fs_job->attr('dependants_only')) {
											echo translate('no');
										} else {
											echo translate('yes');
										}
									?>
									</td>
									<td align="center" class="sq-backend-table-cell">
									<?php
										if ($fs_job->canDelete()) {
											check_box($prefix.'_remove_fs[]', $fs_job->id);
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
							$cron_mgr = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
							$cron_epoc = (int)$cron_mgr->attr('epoch');
							$cron_refresh = (int)$cron_mgr->attr('refresh_time');
							if ($cron_epoc > 0 && $cron_refresh > 0) {
								require_once SQ_FUDGE_PATH.'/general/datetime.inc';
								$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_future_status');
								$fs = new Cron_Job_Future_Status();
								$edit_fns = $fs->getEditFns();

								if (!is_null($keywords) && isset($keywords['fields']['status_future_status'])) {
									ob_start();
								}
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
								$print_commit_button = TRUE;
							} else {
								echo translate('cron_manager_not_configured');
							}
						$o->closeField();
						$GLOBALS['SQ_SYSTEM']->am->forgetAsset($cron_mgr);
					}//end if

				break;

				case 'thumbnail_change' :

					if ($owner->writeAccess('links')) {

						if (!empty($field->display_name)) {
							$display_name = translate('change_question');
						} else {
							$display_name = '';
						}

						$o->openField($display_name);
							if (!is_null($keywords) && isset($keywords['fields']['thumbnail_change'])) {
								ob_start();
							}

							$edit_fns = $owner->getEditFns();
							$edit_fns->paintNoticeLinkedAsset($owner, 'thumbnail', Array('image' => 'D'));

							if (!is_null($keywords) && isset($keywords['fields']['thumbnail_change'])) {
								$ei->_tmp['layout_keyword_replacements'][$keywords['fields']['thumbnail_change']] = ob_get_contents();
								ob_end_flush();
							}
						$o->closeField();
					}

				break;

				case 'thumbnail_current' :

					if (!empty($field->display_name)) {
						$display_name = translate('current');
					} else {
						$display_name = '';
					}

					$o->openField($display_name);
						if (!is_null($keywords) && isset($keywords['fields']['thumbnail_current'])) {
							ob_start();
						}

						$link = $GLOBALS['SQ_SYSTEM']->am->getLink($owner->id, SQ_LINK_NOTICE, 'image', FALSE, 'thumbnail');
						if (!empty($link)) {
							$thumbnail = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
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

			$note = (string) $field->note;
			if (!empty($note)) {
				$o->note($note);
			}

		}//end for

		if (!$avoiding_safe_trash_section) {
			$o->closeSection();
		}

		if (!is_null($keywords) && isset($keywords['sections'][strtolower(str_replace(' ', '_', $section->attributes()->name))])) {
			$ei->_tmp['layout_keyword_replacements'][$keywords['sections'][strtolower(str_replace(' ', '_', $section->attributes()->name))]] = ob_get_contents();
			ob_end_flush();
		}

	}//end for

	return $print_commit_button;

}//end paintDetails()


/**
* Paints the interface for editing the status of an asset
*
* It returns the array of of statii shown in the drop down
*
* @param object	&$owner	the asset whose interface we are painting
* @param object	&$o		the outputter class
* @param string	$prefix	prefix for form vars
*
* @return array
* @access public
*/
function _paintStatusChange(&$owner, &$o, $prefix, $statii, $default_status=0)
{
	if (!empty($statii)) {
		$statii = array_reverse($statii, TRUE);
		$statii[0] = '';
		$statii = array_reverse($statii, TRUE);
		combo_box($prefix.'_change_status', $statii, FALSE, $default_status);
		echo '&nbsp;';
		check_box($prefix.'_change_status_all_children');
		label(translate('cascade_status_change'), $prefix.'_change_status_all_children');
	}

}//end _paintStatusChange()


/**
* Processes the interface for editing the status of an asset
*
* @param object	&$owner	the asset whose interface we are painting
* @param object	&$o		the outputter class
* @param string	$prefix	prefix for form vars
*
* @return boolean
* @access public
*/
function _processStatusChange(&$owner, &$o, $prefix)
{
	if (isset($_POST[$prefix.'_change_status'])) {
		$dependants_only = (isset($_POST[$prefix.'_change_status_all_children']) && $_POST[$prefix.'_change_status_all_children']) ? FALSE : TRUE;
		if ($_POST[$prefix.'_change_status'] > 0 || !$dependants_only) {
			$new_status = 0;
			if ($_POST[$prefix.'_change_status'] == 0 && !$dependants_only) {
				$new_status = $owner->status;
			} else if ($_POST[$prefix.'_change_status'] > 0) {
				$new_status = $_POST[$prefix.'_change_status'];
			}

			$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$vars = Array('assetid' => $owner->id, 'new_status' => $new_status, 'dependants_only' => $dependants_only);
			$hh->queueHipo('hipo_job_edit_status', $vars);
		}
	}

}//end _processStatusChange()


/**
* Processes the interface for editing details
* returns TRUE if there is a change
*
* @param object	&$owner	the asset whose interface we are painting
* @param object	&$o		the outputter class
* @param object	&$ei	the edit interface
*
* @return boolean
* @access public
*/
function processDetails(&$owner, &$o, &$ei)
{
	$prefix = $owner->getPrefix();

	$is_updated = FALSE;

	///// IMMEDIATE STATUS CHANGES ////

	// process a status change on this screen if workflow is NOT running
	$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
	$running_schemas = $wfm->getSchemas($owner->id, TRUE, TRUE);
	if (empty($running_schemas)) {
		_processStatusChange($owner, $o, $prefix);
	}

	///// FUTURE STATUS CHANGES ////

	$cron_mgr = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
	if (!is_null($cron_mgr)) {
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_future_status');

		if (!empty($_POST[$prefix.'_remove_fs'])) {
			$fs_jobs = Cron_Job_Future_Status::getActiveJobs($owner);
			if (!empty($fs_jobs)) {

				if ($GLOBALS['SQ_SYSTEM']->am->acquireLock($cron_mgr->id, 'links')) {
					for ($i = 0; $i < count($fs_jobs); $i++) {
						if (in_array($fs_jobs[$i]->id, $_POST[$prefix.'_remove_fs'])) {
							if ($cron_mgr->removeJob($fs_jobs[$i])) {
								$is_updated = TRUE;
							}
						}// end if
					}// end for

					$GLOBALS['SQ_SYSTEM']->am->releaseLock($cron_mgr->id, 'links');

				} else {
					trigger_localised_error('SYS0232', E_USER_NOTICE);

				}// end if

			}// end if
		}// end if


		if ($owner->writeAccess('')) {
			$fs = new Cron_Job_Future_Status();
			$edit_fns = $fs->getEditFns();
			if ($fs->setAssetToUpdate($owner) && $edit_fns->processInLineBackend($fs, $o, $prefix.'_new_fs')) {
				$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
				$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
				if ($cron_mgr->addJob($fs, $GLOBALS['SQ_SYSTEM']->user)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
					$is_updated = TRUE;
				} else {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				}
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			}

		}// end if

	}//end if is NULL cron mgr

	///// THUMBNAIL CHANGES ////

	$edit_fns = $owner->getEditFns();
	if ($edit_fns->processNoticeLinkedAsset($owner, 'thumbnail')) {
		$is_updated = TRUE;
	}

	return ($is_updated);

}//end processDetails()
?>
