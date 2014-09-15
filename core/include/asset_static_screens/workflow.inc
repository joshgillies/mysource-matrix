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
* $Id: workflow.inc,v 1.64 2013/02/26 01:40:53 cupreti Exp $
*
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once DIRNAME(__FILE__).'/details.inc';

/**
* Workflow Static Screen Functions
*
* @author  Greg Sherwood <gsherwood@squiz.net>
* @version $Revision: 1.64 $
* @package MySource_Matrix
*/


/**
* Paints the interface for viewing current workflows (used for popups etc)
*
* @param object	$owner	the asset whose interface we are painting
* @param object	$o		the outputter class
* @param object	$ei		the edit interface
*
* @return boolean
* @access public
*/
function paintInlineWorkflow(Asset $owner, Backend_Outputter $o, Asset_Edit_Interface $ei)
{
	$prefix = $owner->getPrefix();

	$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
	$schemas = $wfm->getSchemas($owner->id, NULL, FALSE, TRUE);

	if (empty($schemas)) return FALSE;

	$direct = Array();
	foreach ($schemas as $schemaid => $schema_info) {
		$granted = $schema_info['granted'];
		if (!isset($direct[$granted])) {
			$direct[$granted] = Array();
		}
		$direct[$granted][] = $schemaid;
	}
	ksort($direct);

	$o->openSection(translate('workflow_schemas'));
	$o->openField('&nbsp;');
	?>
		<table class="sq-backend-table">
			<tr>
				<td class="sq-backend-table-header">
					<?php echo translate('workflow_schema'); ?>
				</td>
				<td class="sq-backend-table-header" align="right">
					<?php translate('access'); ?>
				</td>
			</tr>
	<?php

	if (!empty($direct)) {
		foreach ($direct as $granted => $schemaids) {
			foreach ($schemaids as $schemaid) {
				if ($schemaid) {
					$schema_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($schemaid), 'workflow_schema', FALSE);
					if (empty($schema_info)) continue;
				}
			?>
				<tr>
					<td class="sq-backend-table-cell">
					<?php
					echo get_asset_tag_line($schemaid);
					?>
					</td>
					<td class="sq-backend-table-cell" align="right">
						<span style="color: <?php echo (($granted == '0') ? 'red' : 'green'); ?>; font-weight: bold;"><?php echo (($granted == '0') ? translate('denied') : translate('applied')) ?></span>
					</td>
				</tr>
			<?php
			}// end foreach data
		}//end foreach inherited

	}// end if
		?></table><?php
		$o->closeField();
	$o->closeSection();

	return TRUE;

}//end paintInlineWorkflow()


/**
* Paint the interface for managing metadata values
*
* @param object	$owner		the asset whose interface we are painting
* @param object	$o			the outputter class
* @param object	$ei			the edit interface
* @param array	$keywords	the list of the keywords
*
* @return boolean
* @access public
*/
function paintWorkflow(Asset $owner, Backend_Outputter $o, Asset_Edit_Interface $ei, $keywords=NULL)
{
	$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
	$schemas = $wfm->getSchemas($owner->id);
	$running_schemas = $wfm->getSchemas($owner->id, TRUE, TRUE);
	$admin_access = $owner->adminAccess('workflow');

	// we only want to paint messages and commit buttons if the user can publish, so we must
	// use workflow manager to see who these users are.
	$publishers = $wfm->whoCanPublish($owner->id);
	$current_userid = $GLOBALS['SQ_SYSTEM']->currentUserId();
	$can_publish = in_array($current_userid, $publishers);

	$email_link_require_comment = FALSE;	// Flag to display text when email link is used to approve or reject the workflow which requires comment.
	$wf_email_link_updated = FALSE;			// Flag to display text when the asset has been accepted or rejected.
	$no_access_to_edit = FALSE;				// Flag to display text when the user does not have permission to edit the asset.

	$available_statii = $owner->getAvailableStatii();

	if (isset($_REQUEST['asset_version'])) {
		$link_asset_version = $_REQUEST['asset_version'];
		$link_asset_version_wo_micro = substr($link_asset_version, 0, strrpos($link_asset_version, '.'));
		$owner_version_wo_micro = substr($owner->version, 0, strrpos($owner->version, '.'));
	}//end if

	if (isset($_REQUEST['workflow_link_action'])) {
		if (isset($_REQUEST['asset_version']) && version_compare($link_asset_version_wo_micro,$owner_version_wo_micro )== 0 ) {
			if ($can_publish) {
				ksort($available_statii);
				$new_status = NULL;
				if ($_REQUEST['workflow_link_action'] == 'approve') {
					foreach ($available_statii as $status_val => $desc) {
						if ($owner->status <= $status_val) {
						$new_status = $status_val;
						break;
						}//end if
					}//end foreach
				} else if ($_REQUEST['workflow_link_action'] == 'reject') {
					foreach ($available_statii as $status_val => $desc) {
						if ($owner->status >= $status_val) {
						$new_status = $status_val;
						break;
						}//end if
					}//end foreach
				}

				if (!$wfm->requiresComment($owner->id, $GLOBALS['SQ_SYSTEM']->user->id)) {
					$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
					$vars	= Array (
								'assetid'			=> $owner->id,
								'dependants_only'	=> TRUE,
							  );
					if ($new_status != NULL) {
						$vars['new_status'] = $new_status;
						$hh->freestyleHipo('hipo_job_edit_status', $vars);
					}//end if
					$wf_email_link_updated = TRUE;
				} else {
					$email_link_require_comment = TRUE;
					$default_selection = $_REQUEST['workflow_link_action'];
				}
			} else {
				$no_access_to_edit = TRUE;
			}//end else if
		} else {
			trigger_localised_error('CORE0280', E_USER_WARNING);
		}

	}//end if

	// Get the running schema again now things has been changed.
	$running_schemas = $wfm->getSchemas($owner->id, TRUE, TRUE);

	$root = $ei->findScreen('static_screen_workflow');
	if (is_null($root)) return FALSE;

	$prefix = $owner->getPrefix();
	$print_commit_button = FALSE;

	foreach ($root->section as $section) {

		// only do this if we are not painting a layout
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

		if (!is_null($keywords) && isset($keywords['sections'][strtolower(str_replace(' ', '_', $section->attributes()->name))])) {
			ob_start();
		}

		$o->openSection(array_get_index($section->attributes(), 'display_name', $section->attributes()->name));

		if (!($num_fields = count($section->children()))) {
			continue;
		}

		foreach ($section->field as $field) {

			switch (strtolower($field->attributes()->code)) {

				///// WORKFLOW APPROVAL/REJECTION ////

				case 'workflow_status_change' :

					// shouldn't be able to changes status, so don't show anything
					if ((!$can_publish) && (!$admin_access)) {
						if ($no_access_to_edit) {
							trigger_localised_error('CORE0281', E_USER_WARNING, $_REQUEST['workflow_link_action']. ($_REQUEST['workflow_link_action'] == 'approve' ? 'd' : 'ed'));
						}//end if
						continue;
					}

					if ($wf_email_link_updated) {
						$o->openField('');
						ob_start();
						echo '<b class="sq-backend-warning">['.sprintf(translate('workflow_email_link_updated'), ($_REQUEST['workflow_link_action']. ($_REQUEST['workflow_link_action'] == 'approve' ? 'd' : 'ed'))).']</b>';
						ob_end_flush();
						$o->closeField();
					}//end if

					if (!empty($running_schemas)) {
						if ($email_link_require_comment) {
							$o->openField('');
							ob_start();
									echo '<b class="sq-backend-warning">['.translate('email_link_require_comment').']</b>';
							ob_end_flush();
							$o->closeField();
						}
						$o->openField(translate('change_status'));
							if (!is_null($keywords) && isset($keywords['fields']['workflow_status_change'])) {
								ob_start();

							}
							$available_statii = $owner->getAvailableStatii();
							_paintStatusChange($owner, $o, $prefix, $available_statii, $email_link_require_comment ? $new_status : 0);
							?><span class="sq-backend-fineprint"><?php
							if (!empty($available_statii)) {
								echo '<br />'.translate('change_to_approve_or_reject');
							} else {
								echo translate('cannot_approve_or_reject');
							}
							?></span><?php

							// if there is a keyword replacement for this, add it to the replacements
							if (!is_null($keywords) && isset($keywords['fields']['workflow_status_change'])) {
								$ei->_tmp['layout_keyword_replacements'][$keywords['fields']['workflow_status_change']] = ob_get_contents();
								ob_end_flush();
							}
						$o->closeField();
					}

				break;

				case 'workflow_log_message' :

					// shouldn't be able to changes status, so don't show anything
					if ((!$can_publish) && (!$admin_access)) {
						continue;
					}
					if (!empty($running_schemas)) {

						$o->openField(translate('log_message'));
							if (!is_null($keywords) && isset($keywords['fields']['workflow_log_message'])) {
								ob_start();
							}

							$requires_comment = $wfm->requiresComment($owner->id, $GLOBALS['SQ_SYSTEM']->user->id);

							// If userlog required, add a required class. Note that we
							// also have to provide sq-form-field because that is
							// overridden otherwise.
							$extras = '';
							if ($requires_comment === TRUE) {
							    $extras = ' class="sq-form-field sq-requires-userlog"';
							}

							text_area($prefix.'_workflow_new_log_message', '', 60, 6, 0, $extras);
							echo '<br/>';

							if ($requires_comment === TRUE) {
								echo translate('workflow_must_enter_reason_for_decision');
							} else {
								echo translate('workflow_may_enter_reason_for_decision');
							}

							// if there is a keyword replacement for this, add it to the replacements
							if (!is_null($keywords) && isset($keywords['fields']['workflow_log_message'])) {
								$ei->_tmp['layout_keyword_replacements'][$keywords['fields']['workflow_log_message']] = ob_get_contents();
								ob_end_flush();
							}
						$o->closeField();
					}

				break;

				case 'workflow_status_commit' :

					// shouldn't be able to changes status, so don't show anything
					if ((!$can_publish) && (!$admin_access)) {
						continue;
					}
					if (!empty($running_schemas)) {

						$o->openField('');
							if (!is_null($keywords) && isset($keywords['fields']['workflow_status_commit'])) {
								ob_start();
							}
							$require_comment = $wfm->requiresComment($owner->id, $GLOBALS['SQ_SYSTEM']->user->id);
							$button_js = 'submit_form()';
							if ($require_comment) {
								$button_js = "elt = document.getElementById('".$prefix."_workflow_new_log_message'); if (elt && elt.value.replace(".'/^\s+|\s+$/g'.", '').length == 0) alert(js_translate('must_enter_comment_error')); else submit_form();";
							}
							normal_button($prefix.'_workflow_status_update', SQ_CONF_COMMIT_BUTTON_TEXT, $button_js);

							// if there is a keyword replacement for this, add it to the replacements
							if (!is_null($keywords) && isset($keywords['fields']['workflow_status_commit'])) {
								$ei->_tmp['layout_keyword_replacements'][$keywords['fields']['workflow_status_commit']] = ob_get_contents();
								ob_end_flush();
							}
						$o->closeField();
					}

				break;

				case 'workflow_logs' :

					$o->openField(translate('workflow_logs'));
						if (!is_null($keywords) && isset($keywords['fields']['workflow_logs'])) {
							ob_start();
						}

						$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
						$logs = $ms->getLogs('asset.workflow.log.*', Array('assetid' => $owner->id));

						$user_logs = $ms->getLogs('asset.workflow.userlog', Array('assetid' => $owner->id));
						usort($logs, Array('Messaging_Service', 'sortMessages'));
						usort($user_logs, Array('Messaging_Service', 'sortMessages'));

						if (!empty($user_logs) || !empty($logs)) {
							?>
							<table class="sq-backend-table">
								<tr>
									<td class="sq-backend-table-header" nowrap width="120"><?php echo translate('log_time'); ?></td>
									<td class="sq-backend-table-header" nowrap width="120"><?php echo translate('logged_by'); ?></td>
									<td class="sq-backend-table-header" width="100%"><?php echo translate('message'); ?></td>
								</tr>
								<?php
								if (!empty($user_logs)) {
									?>
									<tr>
										<td class="sq-backend-table-header" colspan="3"><?php echo translate('most_recent_user_log_message'); ?></td>
									</tr>
									<?php
									for ($log_num = 0; $log_num < 4; $log_num++) {
										if (!isset($user_logs[$log_num])) break;
										$log_data = $user_logs[$log_num];
										?>
										<tr>
											<td class="sq-backend-table-cell" nowrap><?php echo ts_iso8601($log_data['sent']); ?></td>
											<td class="sq-backend-table-cell" nowrap><?php echo $log_data['from_name']; ?></td>
											<td class="sq-backend-table-cell" width="100%"><?php echo $log_data['body']; ?></td>
										</tr>
										<?php
									}
								}//end if there are user workflow logs

								if (!empty($logs)) {
									?>
									<tr>
										<td class="sq-backend-table-header" colspan="3"><?php echo translate('most_recent_system_workflow_message'); ?></td>
									</tr>
									<?php
									for ($log_num = 0; $log_num < 4; $log_num++) {
										if (!isset($logs[$log_num])) break;
										$log_data = $logs[$log_num];
										?>
										<tr>
											<td class="sq-backend-table-cell" nowrap><?php echo ts_iso8601($log_data['sent']); ?></td>
											<td class="sq-backend-table-cell" nowrap><?php echo $log_data['from_name']; ?></td>
											<td class="sq-backend-table-cell" width="100%"><b><?php echo $log_data['subject']; ?></b><br/><?php echo $log_data['body']; ?></td>
										</tr>
										<?php
									}
								}//end if there are system workflow logs
								?>
								<tr>
									<td colspan="3" align="left">
										<a href="<?php echo $owner->getBackendHref('logs'); ?>&sq_backend_log_type=asset.workflow"><?php echo translate('view_all_workflow_logs'); ?></a>
									</td>
								</tr>
							</table>
							<?php

						}//end if there are workflow logs

						// if there is a keyword replacement for this, add it to the replacements
						if (!is_null($keywords) && isset($keywords['fields']['workflow_logs'])) {
							$ei->_tmp['layout_keyword_replacements'][$keywords['fields']['workflow_logs']] = ob_get_contents();
							ob_end_flush();
						}
					$o->closeField();

				break;

				///// WORKFLOW REPORT ////

				case 'workflow_report' :

					if (!empty($running_schemas)) {

						$o->closeSection();

						if (!is_null($keywords) && isset($keywords['fields']['workflow_report'])) {
							ob_start();
						}

						$schema_workflows = $wfm->getSchemaWorkflows($owner->id);
						foreach ($schema_workflows as $schemaid => $workflow) {

							if (!in_array($schemaid, $running_schemas)) continue;

							if (!isset($workflow['stream_name'])) $workflow['stream_name'] = translate('default_stream'); //Pre 3-26 systems
							$schema_title = translate('running_workflow').' - '.$workflow['schema_name'].' - '.$workflow['stream_name'];
							if (isset($workflow['complete']) && $workflow['complete']) {
								$schema_title .= ' <span style="color:#008800">['.translate('completed').']</span>';
							} else {
								$schema_title .= ' <span style="color:#880000">['.translate('not_completed').']</span>';
							}

							$o->openSection($schema_title);

								$o->openSection(translate('workflow_summary'));

								$o->openField(translate('workflow_started'));
									echo $GLOBALS['SQ_SYSTEM']->datetime($workflow['started']);
								$o->closeField();

								$o->openField(translate('time_running'));
									require_once SQ_FUDGE_PATH.'/general/datetime.inc';
									echo easy_time_total(time() - $workflow['started'], TRUE);
								$o->closeField();
								$o->closeSection();

								foreach ($workflow['steps'] as $stepid => $step_data) {
									$section_title = "Step $stepid - ".$step_data['step_name'];

									if ($step_data['expired']) {
										$section_title .= ' <span style="color:#FF9900">['.translate('escalated').']</span>';
									} else {
										if ($step_data['completed']) {
											$section_title .= ' <span style="color:#008800">['.translate('completed').']</span>';
										} else {
											$section_title .= ' <span style="color:#880000">['.translate('not_completed').']</span>';
										}
									}
									$o->openSection($section_title);

										if ($step_data['started']) {
											$o->openField(translate('step_duration'));
												echo $GLOBALS['SQ_SYSTEM']->datetime($step_data['started']);
												echo ' - ';
												if ($step_data['completed']) {
													echo $GLOBALS['SQ_SYSTEM']->datetime($step_data['completed']);
												} else {
													echo '??';
												}
												echo ' (<i>';
												require_once SQ_FUDGE_PATH.'/general/datetime.inc';
												if ($step_data['completed']) {
													echo easy_time_total($step_data['completed'] - $step_data['started'], TRUE);
												} else {
													echo easy_time_total(time() - $step_data['started'], TRUE);
												}
												echo '</i>)';
											$o->closeField();
											if ($step_data['expiry_time'] > 0) {
												$o->openField(translate('expiry_time'));
													$expiry_time = $step_data['started'] + $step_data['expiry_time'];
													echo $GLOBALS['SQ_SYSTEM']->datetime($expiry_time);
													echo ' (<i> ';
													echo easy_time_total($expiry_time - time(), TRUE);
													echo ' </i>)';
												$o->closeField();
											}
											$o->openField(translate('automatically_escalate'));
												if ($step_data['escalate']) {
													echo translate('yes');
												} else {
													echo translate('no');
												}
											$o->closeField();
										}//end if
										$o->openField(translate('step_logic'));
											if (isset($wfm->_valid_cond_logic[$step_data['logic']])) {
												// a special type of logic statement
												$step_logic = $step_data['logic'];
												$step_logic_num = '';
											} else {
												if ($step_data['logic'] != 'all') {
													list($step_logic, $step_logic_num) = explode(' ', $step_data['logic']);
												} else {
													$step_logic = 'all';
													$step_logic_num = '';
												}
											}

											echo '<i>'.$wfm->_valid_step_logic[$step_logic];
											if ($step_logic_num) {
												echo " $step_logic_num condition".(($step_logic_num > 1) ? 's' : '').' must be met</i>';
											}
										$o->closeField();

										$o->openField(translate('conditions'));

											$has_conds = FALSE;
											ob_start();
											?>
											<table class="sq-backend-table">
												<tr>
													<td class="sq-backend-table-header">&nbsp;</td>
													<td class="sq-backend-table-header" width="270"><b><?php echo translate('group_logic'); ?></b></td>
													<td align="center" width="100" class="sq-backend-table-header"><b><?php echo translate('complete_question'); ?></b></td>
												</tr>
											<?php
											foreach ($step_data['conditions'] as $pub => $cond_data) {
												?>
												<tr>
													<td class="sq-backend-table-cell"><?php echo get_asset_tag_line($pub); ?></td>
													<td class="sq-backend-table-cell">
												<?php
												if (isset($wfm->_valid_cond_logic[$cond_data['logic']])) {
													// a special type of logic statement
													$cond_logic = $cond_data['logic'];
													$cond_logic_num = '';
												} else {
													if (!empty($cond_data['logic']) && $cond_data['logic'] != 'all') {
														list($cond_logic, $cond_logic_num) = explode(' ', $cond_data['logic']);
													} else {
														$cond_logic = 'all';
														$cond_logic_num = '';
													}
												}

												if (!$GLOBALS['SQ_SYSTEM']->am->getAssetInfo($pub, Array('user', 'user_group'), FALSE)) {
													$asset = NULL;
												} else {
													$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($pub);
												}
												if (is_null($asset) || $asset instanceof User) {
													echo '&nbsp;';
												} else {
													echo '<i>'.$wfm->_valid_cond_logic[$cond_logic];
													if ($cond_logic_num) {
														echo " $cond_logic_num group member".(($cond_logic_num > 1) ? 's' : '').' must approve</i>';
													}
												}

												?></td>

													<td align="center" class="sq-backend-table-cell">
													<?php
														if (isset($cond_data['complete']) && $cond_data['complete']) {
															echo '<span style="color:#008800"><b>'.translate('yes').'</b></span>';
														} else {
															// do some extra checking
															if (is_null($asset) || $asset instanceof User) {
																echo ((isset($cond_data['published_by'][$pub])) ? '<span style="color:#008800"><b>'.translate('yes').'</b></span>' : '&nbsp;');
															} else {
																if ($cond_logic_num) {
																	$user_count = $cond_logic_num;
																} else {
																	$user_count = count($GLOBALS['SQ_SYSTEM']->am->getChildren($asset->id, Array('user'), FALSE));
																}

																echo '<span style="color:#';
																if (count($cond_data['published_by']) >= $user_count) {
																	echo '008800';
																} else {
																	echo '880000';
																}
																echo '"><b>';
																echo count($cond_data['published_by']).' of '.$user_count;
																echo '</b></span>';
															}
														}
													?>
													</td>
												</tr>

												<?php
												$has_conds = TRUE;
											}//end foreach

											if (!$has_conds) {
												ob_end_clean();
												echo translate('none');
											} else {
												ob_end_flush();
												?></table><?php
											}

										$o->closeField();

										// escalation steps
										if ($step_data['expired']) {
											$o->openField('Escalation Steps');
											$esc_address = Array();
											foreach ($step_data['escalation_steps'] as $esc_stepid => $esc_step_data) {
												$esc_step_address = Array($stepid);
												$esc_step_address[] = $esc_stepid;

												_paintWorkflowStep($owner, $o, $ei, $workflow, $esc_step_address);
											}
											$o->closeField();
										} //end if expiry automatic
									$o->closeSection();
								}//end foreach step
							$o->closeSection();
						}//end foreach

						// if there is a keyword replacement for this, add it to the replacements
						if (!is_null($keywords) && isset($keywords['fields']['workflow_report'])) {
							$ei->_tmp['layout_keyword_replacements'][$keywords['fields']['workflow_report']] = ob_get_contents();
							ob_end_flush();
						}

						$o->openSection();

					} else {
						$o->openField(translate('workflow_status'));
							if (!is_null($keywords) && isset($keywords['fields']['workflow_report'])) {
								ob_start();
							}

							echo translate('workflow_not_running');

							// if there is a keyword replacement for this, add it to the replacements
							if (!is_null($keywords) && isset($keywords['fields']['workflow_report'])) {
								$ei->_tmp['layout_keyword_replacements'][$keywords['fields']['workflow_report']] = ob_get_contents();
								ob_end_flush();
							}
						$o->closeField();
					}

				break;

				///// METADATA WARNING ////

				case 'workflow_metadata_warning' :

					// lets show some workflow warnings about metadata if we need to
					if ($owner->status & SQ_SC_STATUS_PENDING) {
						$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
						if ($mm->allowsMetadata($owner->id)) {
							$m_complete = $mm->requiredFieldsComplete($owner->id);
							if (!$m_complete) {
								$o->openField('<span class="sq-backend-warning"><b>'.ucfirst(strtolower(translate('warning'))).'</b></span>');
									if (!is_null($keywords) && isset($keywords['fields']['workflow_metadata_warning'])) {
										ob_start();
									}

									?>
									<p class="sq-backend-data">
										<span class="sq-backend-warning">
											<b><?php echo sprintf(translate('metadata_in_approval_requires_fields_completion'), htmlspecialchars($owner->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET)); ?></b>
										</span>
										<?php echo sprintf(translate('metadata_fields_only_completed_by_administrator'), htmlspecialchars($owner->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET), '<a href="'.$owner->getBackendHref('metadata').'">'.translate('metadata_screen').'</a>'); ?>
									</p>
									<?php

									// if there is a keyword replacement for this, add it to the replacements
									if (!is_null($keywords) && isset($keywords['fields']['workflow_metadata_warning'])) {
										$ei->_tmp['layout_keyword_replacements'][$keywords['fields']['workflow_metadata_warning']] = ob_get_contents();
										ob_end_flush();
									}
								$o->closeField();
							}
						}
					}

				break;

				///// CURRENT WORKFLOW SCHEMAS ////

				case 'workflow_schemas' :

					$schemas = $wfm->getAssetSchemaInfo($owner->id);

					$direct = Array();
					foreach ($schemas as $schemaid => $schema_info) {
						$granted = $schema_info['granted'];
						if (!isset($direct[$granted])) {
							$direct[$granted] = Array();
						}
						$direct[$granted][$schemaid] = $schema_info;
					}
					ksort($direct);

					$o->openSection('Existing Schemas');
						$o->openRaw();
							if (!is_null($keywords) && isset($keywords['fields']['workflow_schemas'])) {
								ob_start();
							}

							if (empty($direct)) {
								echo '<p><em>'.translate('no_workflow_schemas').'</em></p>';
							} else {
								foreach ($direct as $granted => $schema_list) {
								?>
									<table class="sq-backend-table">
										<tr>
											<th class="sq-backend-table-header">
												<span style="color: <?php echo (($granted == '0') ? 'red' : 'green'); ?>"><?php echo (($granted == '0') ? translate('denied') : translate('applied')) ?> <?php echo translate('schemas');?></span>
											</th>
											<th style="text-align: center;" width="100" class="sq-backend-table-header">
												<?php echo translate('cascade_question'); ?>
											</th>
											<?php
											if ($admin_access && $wfm->isWorkflowComplete($owner->id)) {
												?><th style="text-align: center;" width="100" class="sq-backend-table-header"><?php echo translate('delete_question'); ?></th><?php
											}
											?>
										</tr>
									<?php
									foreach ($schema_list as $schemaid => $schema_detail) {
										if ($schemaid) {
											$schema_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($schemaid), 'workflow_schema', FALSE);
											if (empty($schema_info)) continue;
										}
									?>
										<tr>
											<td class="sq-backend-table-cell">
											<?php
											echo get_asset_tag_line($schemaid);
											?>
											</td>
											<td align="center" width="100" class="sq-backend-table-cell">
											<?php if ($admin_access && $wfm->isWorkflowComplete($owner->id)) {
												check_box($prefix.'_workflow[cascades]['.$schemaid.']', 1, $schema_detail['cascades']);
											} else {
												?><img src="<?php echo sq_web_path('lib'); ?>/web/images/<?php echo ($schema_detail['cascades'] ? 'tick' : 'cross'); ?>.gif" width="15" height="15" /><?php
											}
											?>
											</td>
											<?php
											if ($admin_access && $wfm->isWorkflowComplete($owner->id)) {
												?><td align="center" width="100" class="sq-backend-table-cell"><?php
												check_box($prefix.'_workflow[delete]['.$schemaid.']');
												?></td><?php
											}
											?>
										</tr>
									<?php
									}// end foreach data
								?>
									</table>
								<?php
								}//end foreach inherited

							}//end else

							// if there is a keyword replacement for this, add it to the replacements
							if (!is_null($keywords) && isset($keywords['fields']['workflow_schemas'])) {
								$ei->_tmp['layout_keyword_replacements'][$keywords['fields']['workflow_schemas']] = ob_get_contents();
								ob_end_flush();
							}

						$o->closeRaw();
						if (!empty($direct)) {
							$o->sectionNote('The "Cascade?" option determines whether new children of this asset will automatically have the schema cascaded to them. If this setting is changed, existing non-dependant children will NOT be affected. If you want to apply a schema to existing children where they have not been before, please do a Manual Cascade after toggling this setting on.');
						}
					$o->closeSection();

				break;

				///// ADD NEW WORKFLOW SCHEMA ////

				case 'workflow_add_schemas' :

					if ($admin_access && empty($running_schemas)) {
						$o->openSection('Add New Schema');

							if (!is_null($keywords) && isset($keywords['fields']['workflow_add_schemas'])) {
								ob_start();
							}

							$o->openField('Select Schema', '');
								echo '<span class="sq-inline-fields-wrapper bottom-margin">';
									combo_box($prefix.'_workflow[new][granted]', Array('' => '', '1' => translate('apply'), '0' => translate('deny')), FALSE, '');
									echo ' '.translate('workflow_schema').' ';
									asset_finder($prefix.'_workflow[new][schemaid]', 0, Array('workflow_schema' => 'D'), 'sq_sidenav', FALSE, 'null', Array('clear'));
								echo '</span>';
							$o->closeField();

							if (is_null($keywords) || isset($keywords['fields']['workflow_add_schemas'])) {
								$print_commit_button = TRUE;
							}

							// if there is a keyword replacement for this, add it to the replacements
							if (!is_null($keywords) && isset($keywords['fields']['workflow_add_schemas'])) {
								$ei->_tmp['layout_keyword_replacements'][$keywords['fields']['workflow_add_schemas']] = ob_get_contents();
								ob_end_flush();
							}

							$o->openField(translate('cascade_question'), '');
								check_box($prefix.'_workflow[new][cascades]', 1, TRUE);
								label(translate('cascade_schema_changes_to_new_children'), $prefix.'_workflow[new][cascades]');
								$o->note('If unchecked, the schema will be applied or denied only to dependants of this asset, regardless of the Cascade Changes settings below. It will also not apply the workflow schema to any asset that becomes a new child of this asset (through creating a new asset, or moving an existing one, or creating a new link from one). This setting can be toggled after being applied.');
							$o->closeField();

						$o->closeSection();
					}

				break;

				///// CASACADE OPTIONS ////

				case 'workflow_cascade_options' :

					$o->openField(translate('cascade_options'));
						if (!is_null($keywords) && isset($keywords['fields']['workflow_cascade_options'])) {
							ob_start();
						}

						if ($admin_access && empty($running_schemas)) {
							$kiddies = $GLOBALS['SQ_SYSTEM']->am->getChildren($owner->id);
							if (count($kiddies)) {

								echo '<p><strong>'.translate('note').':</strong> '.sprintf(translate('action_affects_assets_below_this'), count($kiddies), htmlspecialchars($owner->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET)).'.</p>';

								check_box($prefix.'_workflow[cascade_changes]', '1', TRUE);
								echo ' ';
								label(translate('cascade_schema_changes'), $prefix.'_workflow[cascade_changes]');
								echo '<br/>';
								check_box($prefix.'_workflow[manual_cascade]');
								echo ' ';
								label(translate('manually_cascade_schema'), $prefix.'_workflow[manual_cascade]');

							} else {
								echo translate('no_children_to_cascade_workflow');
							}
						} else if ($admin_access) {
							echo translate('cannot_cascade_while_workflow_running');
						} else {
							echo translate('no_access_to_cascade_workflow');
						}

						// if there is a keyword replacement for this, add it to the replacements
						if (!is_null($keywords) && isset($keywords['fields']['workflow_cascade_options'])) {
							$ei->_tmp['layout_keyword_replacements'][$keywords['fields']['workflow_cascade_options']] = ob_get_contents();
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

		$o->closeSection();

		if (!is_null($keywords) && isset($keywords['sections'][strtolower(str_replace(' ', '_', $section->attributes()->name))])) {
			$ei->_tmp['layout_keyword_replacements'][$keywords['sections'][strtolower(str_replace(' ', '_', $section->attributes()->name))]] = ob_get_contents();
			ob_end_flush();
		}

	}//end for

	return $print_commit_button;

}//end paintWorkflow()


/**
* Processes the interface for managing workflow
* returns TRUE if there is a change
*
* @param object	$owner	the asset whose interface we are painting
* @param object	$o		the outputter class
* @param object	$ei		the edit interface
*
* @return boolean
* @access public
*/
function processWorkflow(Asset $owner, Backend_Outputter $o, Asset_Edit_Interface $ei)
{
	$updated = FALSE;
	$prefix = $owner->getPrefix();

	// process a status change on this screen if workflow is running
	$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
	$running_schemas = $wfm->getSchemas($owner->id, TRUE, TRUE);

	// has a message been entered?
	$message_body = '';
	if (isset($_REQUEST[$prefix.'_workflow_new_log_message'])) {
		$message_body = trim($_REQUEST[$prefix.'_workflow_new_log_message']);
	}

	// are comments required?
	$require_comment = $wfm->requiresComment($owner->id, $GLOBALS['SQ_SYSTEM']->user->id);

	// process any messages that may have been submitted
	if (!empty($message_body)) {
		$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
		$msg_reps = Array(
						'message'	=> $message_body,
					);
		$message = $ms->newMessage(Array(), 'asset.workflow.userlog', $msg_reps);
		$message->parameters['assetid'] = $owner->id;
		$message->parameters['version'] = substr($owner->version, 0, strrpos($owner->version, '.'));
		$message->send();
		$updated = TRUE;
	}

	// process the status, but only if the required comment conditions are met
	if (!empty($running_schemas) && !($require_comment && empty($message_body))) {
		_processStatusChange($owner, $o, $prefix);
		$updated = TRUE;
	}

	// if we dont have admin access, dont process anything
	if (!$owner->adminAccess('workflow')) return $updated;

	if (!isset($_POST[$prefix.'_workflow'])) {
		return $updated;
	}

	$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();

	// are we cascading these schema changes to our children?
	$cascade_changes = FALSE;
	if (isset($_POST[$prefix.'_workflow']['cascade_changes']) && $_POST[$prefix.'_workflow']['cascade_changes']) {
		$cascade_changes = TRUE;
	}

	$schema_vars = Array();
	$post_data = $_POST[$prefix.'_workflow'];
	$depandants = $GLOBALS['SQ_SYSTEM']->am->getDependantChildren($owner->id);
	$has_dependants = !empty($depandants);

	// apply a schema
	if (!empty($post_data['new']['schemaid']['assetid'])) {
		if (!isset($post_data['new']['granted']) || $post_data['new']['granted'] == '') {
			trigger_localised_error('SYS0268', E_USER_NOTICE, $post_data['new']['schemaid']['assetid']);
		} else {

			// Cascade changes to lower assets if "cascade" is set
			// UNLESS there are dependents
			if (($cascade_changes && isset($post_data['new']['cascades'])) || $has_dependants) {
				// cascading access of some kind, so queue it up for the HIPO
				$schema_vars[] = Array(
									'assetids'			=> Array($owner->id),
									'granted'			=> (int)$post_data['new']['granted'],
									'schemaid'			=> (int)$post_data['new']['schemaid']['assetid'],
									'cascades'			=> (int)isset($post_data['new']['cascades']) ? 1 : 0,
									'previous_access'	=> NULL,
									'dependants_only'	=> !($cascade_changes && isset($post_data['new']['cascades'])) && $has_dependants,
								 );
			} else {
				// lets do this ourselves
				if (!$wfm->setSchema($owner->id, (int)$post_data['new']['schemaid']['assetid'], (int)$post_data['new']['granted'], (isset($post_data['new']['cascades']) ? 1 : 0))) {
					return $updated;
				}
			}
		}
	}

	// revoke [deny] schema (apply or deny can be revoked)
	if (!empty($post_data['delete'])) {
		foreach ($post_data['delete'] as $schemaid => $on) {
			$schemaid = (int)$schemaid;
			$schema_info = $wfm->getAssetSchemaInfo($owner->id, $schemaid);
			if (($cascade_changes && $schema_info['cascades']) || $has_dependants) {
				$current_access = $schema_info['granted'];
				$schema_vars[] = Array(
									'assetids'			=> Array($owner->id),
									'granted'			=> -1,
									'schemaid'			=> $schemaid,
									'cascades'			=> $cascade_changes,
									'previous_access'	=> $current_access,
									'dependants_only'	=> !($cascade_changes && $schema_info['cascades']) && $has_dependants,
								 );
			} else {
				if (!$wfm->deleteSchema($owner->id, $schemaid)) {
					return $updated;
				}
			}
		}
	}

	// Set cascades here
	$schemas = $wfm->getAssetSchemaInfo($owner->id);
	foreach ($schemas as $schemaid => $schema_info) {
		$cascades = isset($post_data['cascades'][$schemaid]);
		if ((boolean)$cascades != (boolean)$schema_info['cascades']) {
			if ($has_dependants) {
				$current_access = $schema_info['granted'];
				$schema_vars[] = Array(
									'assetids'			=> Array($owner->id),
									'granted'			=> $current_access,
									'schemaid'			=> $schemaid,
									'cascades'			=> $cascades,
									'previous_access'	=> NULL,
									'dependants_only'	=> 1,
								 );
			} else {
				if (!$wfm->setSchema($owner->id, (int)$schemaid, (int)$schema_info['granted'], (int)$cascades)) {
					return FALSE;
				}
			}
		}
	}

	if (isset($_POST[$prefix.'_workflow']['manual_cascade']) && $_POST[$prefix.'_workflow']['manual_cascade']) {
		$schemas = $wfm->getAssetSchemaInfo($owner->id, NULL, TRUE);
		foreach ($schemas as $schemaid => $schema_info) {
			$schema_vars[] = Array(
								'assetids'			=> Array($owner->id),
								'granted'			=> $schema_info['granted'],
								'cascades'			=> TRUE,
								'schemaid'			=> $schemaid,
								'previous_access'	=> NULL,
							 );
		}
	}


	if (!empty($schema_vars)) {
		$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$vars = Array(
					'schema_changes'	=> $schema_vars,
				);
		$hh->queueHipo('hipo_job_edit_workflow_schemas', $vars);
	}

	return TRUE;

}//end processWorkflow()


/**
* Called to paint escalation steps recursively
*
* @param object	$owner			the asset whose interface we are painting
* @param object	$o				the outputter class
* @param object	$ei			the edit interface
* @param array	&$workflow		the workflow status array
* @param array	$step_address	the address of the current step within the workflow array
*
* @return boolean
* @access public
*/
function _paintWorkflowStep(Asset $owner, Backend_Outputter $o, Asset_Edit_Interface $ei, array &$workflow, array $step_address)
{
	$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
	$step_data =& $wfm->getCurrentStep($workflow, $step_address); // need by ref
	$section_title = 'Step '.implode('.',$step_address).' - '.$step_data['step_name'];

	if ($step_data['expired']) {
		$section_title .= ' <span style="color:#FF9900">['.translate('escalated').']</span>';
	} else {
		if ($step_data['completed']) {
			$section_title .= ' <span style="color:#008800">['.translate('completed').']</span>';
		} else {
			$section_title .= ' <span style="color:#880000">['.translate('not_completed').']</span>';
		}
	}

	$o->openSection($section_title);
		if ($step_data['started']) {
			$o->openField(translate('step_duration'));
				echo $GLOBALS['SQ_SYSTEM']->datetime($step_data['started']);
				echo ' - ';
				if ($step_data['completed']) {
					echo $GLOBALS['SQ_SYSTEM']->datetime($step_data['completed']);
				} else {
					echo '??';
				}
				echo ' (<i>';
				require_once SQ_FUDGE_PATH.'/general/datetime.inc';
				if ($step_data['completed']) {
					echo easy_time_total($step_data['completed'] - $step_data['started'], TRUE);
				} else {
					echo easy_time_total(time() - $step_data['started'], TRUE);
				}
				echo '</i>)';
			$o->closeField();
			if ($step_data['expiry_time'] > 0) {
				$o->openField(translate('expiry_time'));
					$expiry_time = $step_data['started'] + $step_data['expiry_time'];
					echo $GLOBALS['SQ_SYSTEM']->datetime($expiry_time);
					echo ' (<i> ';
					echo easy_time_total($expiry_time - time(), TRUE);
					echo ' </i>)';

				$o->closeField();
				$o->openField(translate('automatically_escalate'));
					if ($step_data['escalate']) {
						echo translate('yes');
					} else {
						echo translate('no');
					}
				$o->closeField();
			}
		}//end if started


		$o->openField(translate('step_logic'));
			if (isset($wfm->_valid_cond_logic[$step_data['logic']])) {
				// a special type of logic statement
				$step_logic = $step_data['logic'];
				$step_logic_num = '';
			} else {
				if ($step_data['logic'] != 'all') {
					list($step_logic, $step_logic_num) = explode(' ', $step_data['logic']);
				} else {
					$step_logic = 'all';
					$step_logic_num = '';
				}
			}

			echo '<i>'.$wfm->_valid_step_logic[$step_logic];
			if ($step_logic_num) {
				echo " $step_logic_num condition".(($step_logic_num > 1) ? 's' : '').' must be met</i>';
			}
		$o->closeField();

		$o->openField(translate('conditions'));

			$has_conds = FALSE;
			ob_start();
			?>
			<table class="sq-backend-table">
				<tr>
					<td class="sq-backend-table-header">&nbsp;</td>
					<td class="sq-backend-table-header" width="270"><b><?php echo translate('group_logic'); ?></b></td>
					<td align="center" width="100" class="sq-backend-table-header"><b><?php echo translate('complete_question'); ?></b></td>
				</tr>
			<?php
			foreach ($step_data['conditions'] as $pub => $cond_data) {
				$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($pub);
				if (!$asset->id) continue;
				?>
				<tr>
					<td class="sq-backend-table-cell"><?php echo get_asset_tag_line($pub); ?></td>
					<td class="sq-backend-table-cell">
				<?php
				if (isset($wfm->_valid_cond_logic[$cond_data['logic']])) {
					// a special type of logic statement
					$cond_logic = $cond_data['logic'];
					$cond_logic_num = '';
				} else {
					if ($cond_data['logic'] != 'all') {
						list($cond_logic, $cond_logic_num) = explode(' ', $cond_data['logic']);
					} else {
						$cond_logic = 'all';
						$cond_logic_num = '';
					}
				}

				if ($asset instanceof User) {
					echo '&nbsp;';
				} else {
					echo '<i>'.$wfm->_valid_cond_logic[$cond_logic];
					if ($cond_logic_num) {
						echo " $cond_logic_num group member".(($cond_logic_num > 1) ? 's' : '').' must approve</i>';
					}
				}

				?></td>

					<td align="center" class="sq-backend-table-cell">
					<?php
						if (isset($cond_data['complete']) && $cond_data['complete']) {
							echo '<span style="color:#008800"><b>'.translate('yes').'</b></span>';
						} else {
							// do some extra checking
							if ($asset instanceof User) {
								echo ((isset($cond_data['published_by'][$asset->id])) ? '<span style="color:#008800"><b>'.translate('yes').'</b></span>' : '&nbsp;');
							} else {
								if ($cond_logic_num) {
									$user_count = $cond_logic_num;
								} else {
									$user_count = count($GLOBALS['SQ_SYSTEM']->am->getChildren($asset->id, Array('user'), FALSE));
								}

								echo '<span style="color:#';
								if (count($cond_data['published_by']) >= $user_count) {
									echo '008800';
								} else {
									echo '880000';
								}
								echo '"><b>';
								echo count($cond_data['published_by']).' of '.$user_count;
								echo '</b></span>';
							}
						}
					?>
					</td>
				</tr>

				<?php
				$has_conds = TRUE;
			}//end foreach

			if (!$has_conds) {
				ob_end_clean();
				echo translate('none');
			} else {
				ob_end_flush();
				?></table><?php
			}

		$o->closeField();

		// escalation steps
		if ($step_data['expired']) {
			$o->openField('Escalation Steps');
			foreach ($step_data['escalation_steps'] as $esc_stepid => $esc_step) {
				$esc_step_address = $step_address;
				$esc_step_address[] = $esc_stepid;
				_paintWorkflowStep($owner, $o, $ei, $workflow, $esc_step_address);
			}
			$o->closeField();
		}
	$o->closeSection();

}//end _paintWorkflowStep()


?>
