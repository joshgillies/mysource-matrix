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
* $Id: workflow.inc,v 1.34 2005/12/14 00:27:56 lwright Exp $
*
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once DIRNAME(__FILE__).'/details.inc';

/**
* Workflow Static Screen Functions
*
* @author  Greg Sherwood <gsherwood@squiz.net>
* @version $Revision: 1.34 $
* @package MySource_Matrix
*/


/**
* Paints the interface for viewing current workflows (used for popups etc)
*
* @param object	Asset					$owner		the asset whose interface we are painting
* @param object	Backend_Outputter		$o			the outputter class
* @param object	Asset_Edit_Interface	&$ei		the edit interface
*
* @access public
* @return boolean
*/
function paintInlineWorkflow(&$owner, &$o, &$ei)
{
	$prefix = $owner->getPrefix();

	$wfm = &$GLOBALS['SQ_SYSTEM']->getWorkflowManager();
	$schemas = $wfm->getSchemas($owner->id);

	if (empty($schemas)) return false;

	$o->openSection(translate('workflow_schemas'));

	$direct = Array();
	foreach ($schemas as $schemaid => $granted) {
		if (!isset($direct[$granted])) {
			$direct[$granted] = Array();
		}
		$direct[$granted][] = $schemaid;
	}
	ksort($direct);
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
					$schema_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($schemaid), 'workflow_schema', false);
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

	return true;

}//end paintInlineWorkflow()


/**
* Paints the interface for managing workflow
*
* @param object	Asset					$owner		the asset whose interface we are painting
* @param object	Backend_Outputter		$o			the outputter class
* @param object	Asset_Edit_Interface	&$ei		the edit interface
*
* @access public
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
function paintWorkflow(&$owner, &$o, &$ei, $keywords=null)
{
	$root = &$ei->findScreen('static_screen_workflow');
	if (is_null($root)) return false;

	$prefix = $owner->getPrefix();
	$print_commit_button = false;

	$wfm = &$GLOBALS['SQ_SYSTEM']->getWorkflowManager();
	$schemas = $wfm->getSchemas($owner->id);
	$running_schemas = $wfm->getSchemas($owner->id, true, true);
	$admin_access = $owner->adminAccess('workflow');

	//we only want to paint messages and commit buttons if the user can publish, so we must
	//use workflow manager to see who these users are.
	$publishers = $wfm->whoCanPublish($owner->id);
	$current_userid = $GLOBALS['SQ_SYSTEM']->currentUserId();
	$can_publish = in_array($current_userid, $publishers);

	for ($i = 0; $i < count($root->children); $i++) {

		$section = &$root->children[$i];

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
			$field = &$section->children[$j];

			switch (strtolower($field->attributes['code'])) {

				///// WORKFLOW APPROVAL/REJECTION ////

				case 'workflow_status_change' :

					//shouldn't be able to changes status, so don't show anything
					if ((!$can_publish) && (!$admin_access)) {
						continue;
					}
					if (!empty($running_schemas)) {

						$o->openField(translate('change_status'));
							if (!is_null($keywords) && isset($keywords['fields']['workflow_status_change'])) {
								ob_start();
							}
							$available_statii = _paintStatusChange($owner, $o, $prefix);
							?><span class="sq-backend-fineprint"><?php
							if (!empty($available_statii)) {
								echo '<br/>'.translate('change_to_approve_or_reject');
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

					//shouldn't be able to changes status, so don't show anything
					if ((!$can_publish) && (!$admin_access)) {
						continue;
					}
					if (!empty($running_schemas)) {

						$o->openField(translate('log_message'));
							if (!is_null($keywords) && isset($keywords['fields']['workflow_log_message'])) {
								ob_start();
							}

							text_area($prefix.'_workflow_new_log_message', '', 60, 6);
							echo '<br/>';

							if ($wfm->requiresComment($owner->id, $GLOBALS['SQ_SYSTEM']->user->id)) {
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

					//shouldn't be able to changes status, so don't show anything
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
							normal_button($prefix.'_workflow_status_update', translate('commit_button'), $button_js);

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

						$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
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
									<td class="sq-backend-table-header" colspan="3" align="middle">
										<a class="sq-backend-table-header" href="<?php echo $owner->getBackendHref('logs'); ?>&sq_backend_log_type=asset.workflow"><?php echo translate('view_all_workflow_logs'); ?></a>
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

							$schema_title = translate('running_workflow').' - '.$workflow['schema_name'];
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
									echo easy_time_total(time() - $workflow['started'], true);
								$o->closeField();
								$o->closeSection();

								foreach ($workflow['steps'] as $stepid => $step_data) {
									$section_title = "Step $stepid - ".$step_data['step_name'];
									if (!$workflow['current_step'] || $workflow['current_step'] > $stepid) {
										$section_title .= ' <span style="color:#008800">['.translate('completed').']</span>';
									} else {
										$section_title .= ' <span style="color:#880000">['.translate('not_completed').']</span>';
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
													echo easy_time_total($step_data['completed'] - $step_data['started'], true);
												} else {
													echo easy_time_total(time() - $step_data['started'], true);
												}
												echo '</i>)';
											$o->closeField();
										}

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

											$has_conds = false;
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
												$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($pub);
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

												if (is_a($asset, 'user')) {
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
															if (is_a($asset, 'user')) {
																echo ((isset($cond_data['published_by'][$asset->id])) ? '<span style="color:#008800"><b>'.translate('yes').'</b></span>' : '&nbsp;');
															} else {
																if ($cond_logic_num) {
																	$user_count = $cond_logic_num;
																} else {
																	$user_count = count($GLOBALS['SQ_SYSTEM']->am->getChildren($asset->id, Array('user'), false));
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
												$has_conds = true;
											}

											if (!$has_conds) {
												ob_end_clean();
												echo translate('none');
											} else {
												ob_end_flush();
												?></table><?php
											}

										$o->closeField();

									$o->closeSection();

								}// end foreach

							$o->closeSection();

							// if there is a keyword replacement for this, add it to the replacements
							if (!is_null($keywords) && isset($keywords['fields']['workflow_report'])) {
								$ei->_tmp['layout_keyword_replacements'][$keywords['fields']['workflow_report']] = ob_get_contents();
								ob_end_flush();
							}

						}// end foreach

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
						$mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager();
						if ($mm->allowsMetadata($owner->id)) {
							$m_complete = $mm->requiredFieldsComplete($owner->id);
							if (!$m_complete) {
								$o->openField('<span class="sq-backend-warning"><b>'.strtolower(translate('warning')).'</b></span>');
									if (!is_null($keywords) && isset($keywords['fields']['workflow_metadata_warning'])) {
										ob_start();
									}

									?>
									<p class="sq-backend-data">
										<span class="sq-backend-warning">
											<b><?php echo translate('metadata_in_approval_requires_fields_completion', $owner->name); ?></b>
										</span>
										<?php echo translate('metadata_fields_only_completed_by_administrator', $owner->name, '<a href="'.$owner->getBackendHref('metadata').'">'.translate('metadata_screen').'</a>'); ?>
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

					$direct = Array();
					foreach ($schemas as $schemaid => $granted) {
						if (!isset($direct[$granted])) {
							$direct[$granted] = Array();
						}
						$direct[$granted][] = $schemaid;
					}
					ksort($direct);

					$o->openField('');
						if (!is_null($keywords) && isset($keywords['fields']['workflow_schemas'])) {
							ob_start();
						}

						if (empty($direct)) {
							echo translate('no_workflow_schemas');
						} else {
							foreach ($direct as $granted => $schemaids) {
							?>
								<table class="sq-backend-table">
									<tr>
										<td class="sq-backend-table-header">
											<span style="color: <?php echo (($granted == '0') ? 'red' : 'green'); ?>"><?php echo (($granted == '0') ? translate('denied') : translate('applied')) ?></span>
										</td>
										<?php
										if ($admin_access && $GLOBALS['SQ_SYSTEM']->wm->isWorkflowComplete($owner->id)) {
											?><td align="center" width="100" class="sq-backend-table-header" style="font-weight: bold;"><?php echo translate('delete_question'); ?></td><?php
										}
										?>
									</tr>
								<?php
								foreach ($schemaids as $schemaid) {
									if ($schemaid) {
										$schema_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($schemaid), 'workflow_schema', false);
										if (empty($schema_info)) {
											continue;
										}
									}
								?>
									<tr>
										<td class="sq-backend-table-cell">
										<?php
										echo get_asset_tag_line($schemaid);
										?>
										</td>
										<?php
										if ($admin_access && $GLOBALS['SQ_SYSTEM']->wm->isWorkflowComplete($owner->id)) {
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
								<br>
							<?php
							}//end foreach inherited

						}// end if

						// if there is a keyword replacement for this, add it to the replacements
						if (!is_null($keywords) && isset($keywords['fields']['workflow_schemas'])) {
							$ei->_tmp['layout_keyword_replacements'][$keywords['fields']['workflow_schemas']] = ob_get_contents();
							ob_end_flush();
						}

					$o->closeField();

				break;

				///// ADD NEW WORKFLOW SCHEMA ////

				case 'workflow_add_schemas' :

					if ($admin_access && empty($running_schemas)) {
						$o->openField('');
							if (!is_null($keywords) && isset($keywords['fields']['workflow_add_schemas'])) {
								ob_start();
							}

							$o->openField('New ?', 'new_line');
								combo_box($prefix.'_workflow[new][granted]', Array('' => '', '1' => translate('apply'), '0' => translate('deny')), false, '');
								echo ' '.translate('workflow_schema').' ';
								asset_finder($prefix.'_workflow[new][schemaid]', 0, Array('workflow_schema' => 'D'), 'sq_sidenav', false, 'null', Array('clear'));
							$o->closeField();

							if (is_null($keywords) || isset($keywords['fields']['workflow_add_schemas'])) {
								$print_commit_button = true;
							}

							// if there is a keyword replacement for this, add it to the replacements
							if (!is_null($keywords) && isset($keywords['fields']['workflow_add_schemas'])) {
								$ei->_tmp['layout_keyword_replacements'][$keywords['fields']['workflow_add_schemas']] = ob_get_contents();
								ob_end_flush();
							}

						$o->closeField();
					}

				break;

				///// CASACADE OPTIONS ////

				case 'workflow_cascade_options' :

					$o->openField('');
						if (!is_null($keywords) && isset($keywords['fields']['workflow_cascade_options'])) {
							ob_start();
						}

						if ($admin_access && empty($running_schemas)) {
							$kiddies = $GLOBALS['SQ_SYSTEM']->am->getChildren($owner->id);
							if (count($kiddies)) {
								echo translate('action_affects_assets_below_this', count($kiddies), $owner->name).'<br /><br />';

								check_box($prefix.'_workflow[cascade_changes]', '1', true);
								echo ' '.translate('cascade_schema_changes');
								echo '<br/>';
								check_box($prefix.'_workflow[manual_cascade]');
								echo ' '.translate('manually_cascade_schema');

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

}//end paintWorkflow()


/**
* Processes the interface for managing workflow
* returns true if there is a change
*
* @param object	Asset					$owner	the asset whose interface we are painting
* @param object	Backend_Outputter		$o		the outputter class
* @param object	Asset_Edit_Interface	&$ei	the edit interface
*
* @return boolean
* @access public
*/
function processWorkflow(&$owner, &$o, &$ei)
{
	$updated = false;
	$prefix = $owner->getPrefix();

	// process a status change on this screen if workflow is running
	$wfm = &$GLOBALS['SQ_SYSTEM']->getWorkflowManager();
	$running_schemas = $wfm->getSchemas($owner->id, true, true);

	// has a message been entered?
	$message_body = '';
	if (isset($_POST[$prefix.'_workflow_new_log_message'])) {
		$message_body = trim($_POST[$prefix.'_workflow_new_log_message']);
	}

	// are comments required?
	$require_comment = $wfm->requiresComment($owner->id, $GLOBALS['SQ_SYSTEM']->user->id);

	// process the status, but only if the required comment conditions are met
	if (!empty($running_schemas) && !($require_comment && empty($message_body))) {
		_processStatusChange($owner, $o, $prefix);
		$updated = true;
	}

	// process any messages that may have been submitted
	if (!empty($message_body)) {
		$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
		$msg_reps = Array(
						'message'	=> $message_body,
					);
		$message = $ms->newMessage(Array(), 'asset.workflow.userlog', $msg_reps);
		$message->parameters['assetid'] = $owner->id;
		$message->parameters['version'] = substr($owner->version, 0, strrpos($owner->version, '.'));
		$message->send();
		$updated = true;
	}

	// if we dont have admin access, dont process anything
	if (!$owner->adminAccess('workflow')) return $updated;

	if (!isset($_POST[$prefix.'_workflow'])) {
		return $updated;
	}

	$wfm = &$GLOBALS['SQ_SYSTEM']->getWorkflowManager();

	// are we cascading these schema changes to our children?
	$cascade_changes = false;
	if (isset($_POST[$prefix.'_workflow']['cascade_changes']) && $_POST[$prefix.'_workflow']['cascade_changes']) {
		$cascade_changes = true;
	}

	$schema_vars = Array();
	$post_data = $_POST[$prefix.'_workflow'];

	// apply a schema
	if (!empty($post_data['new']['schemaid']['assetid'])) {
		if (!isset($post_data['new']['granted']) || $post_data['new']['granted'] == '') {
			trigger_localised_error('SYS0268', E_USER_NOTICE, $post_data['new']['schemaid']['assetid']);
		} else {
			if ($cascade_changes) {
				// cascading access of some kind, so queue it up for the HIPO
				$schema_vars[] = Array(
									'granted'			=> (int)$post_data['new']['granted'],
									'schemaid'			=> (int)$post_data['new']['schemaid']['assetid'],
									'previous_access'	=> null,
								 );
			} else {
				// lets do this ourselves
				if (!$wfm->setSchema($owner->id, (int)$post_data['new']['schemaid']['assetid'], (int)$post_data['new']['granted'])) {
					return $updated;
				}
			}
		}
	}

	// revoke [deny] schema (apply or deny can be revoked)
	if (!empty($post_data['delete'])) {
		foreach ($post_data['delete'] as $schemaid => $on) {
			$schemaid = (int)$schemaid;
			if ($cascade_changes) {
				$schemas = $wfm->getSchemas($owner->id);
				$current_access = $schemas[$schemaid];
				$schema_vars[] = Array(
									'granted'			=> -1,
									'schemaid'			=> $schemaid,
									'previous_access'	=> $current_access,
								 );
			} else {
				if (!$wfm->deleteSchema($owner->id, $schemaid)) {
					return $updated;
				}
			}
		}
	}

	if (isset($_POST[$prefix.'_workflow']['manual_cascade']) && $_POST[$prefix.'_workflow']['manual_cascade']) {
		$schemas = $wfm->getSchemas($owner->id);
		foreach ($schemas as $schemaid => $granted) {
			$schema_vars[] = Array(
								'granted'			=> $granted,
								'schemaid'			=> $schemaid,
								'previous_access'	=> null,
							 );
		}
	}


	if (!empty($schema_vars)) {
		$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$vars = Array('assets' => Array($owner->id => Array('type_code' => $owner->type())), 'schema_changes' => $schema_vars);
		$hh->queueHipo('hipo_job_edit_workflow_schemas', $vars);
	}

	return true;

}//end processWorkflow()


?>
