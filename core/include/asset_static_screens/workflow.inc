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
* $Id: workflow.inc,v 1.1 2003/12/02 04:43:25 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once DIRNAME(__FILE__).'/details.inc';



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

	$o->openSection('Workflow Schemas');

	$direct = Array();
	foreach($schemas as $schemaid => $access) {
		if (!isset($direct[$access])) $direct[$access] = Array();
		$direct[$access][] = $schemaid;
	}
	ksort($direct);
	$o->openField('&nbsp;');
	?>
		<table class="sq-backend-table">
			<tr>
				<td class="sq-backend-table-header">
					Work Flow Schema
				</td>
				<td class="sq-backend-table-header" align="right">
					Access
				</td>
			</tr>
	<?php

	if (!empty($direct)) {
		foreach ($direct as $access => $schemaids) {
			foreach($schemaids as $schemaid) {
				if ($schemaid) {
					$schema = &$GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
					if (!$schema->id || !is_a($schema, 'workflow_schema')) continue;
				}
			?>
				<tr>
					<td class="sq-backend-table-cell">
					<?php
					echo $schema->name.' ['.$schema->type().' : '.$schemaid.']';
					?>
					</td>
					<td class="sq-backend-table-cell" align="right">
						<span style="color: <?php echo (($access == '0') ? 'red' : 'green'); ?>; font-weight: bold;"><?php echo (($access == '0') ? 'Denied' : 'Applied') ?></span>
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
function paintWorkflow(&$owner, &$o, &$ei)
{
	$prefix = $owner->getPrefix();

	$wfm = &$GLOBALS['SQ_SYSTEM']->getWorkflowManager();
	$schemas = $wfm->getSchemas($owner->id);
	$running_schemas = $wfm->getSchemas($owner->id, true, true);

	// print any currently running workflows
	if (!empty($running_schemas)) {
		// cant edit workflow when workflow is currently running
		$admin_access = false;

		$o->openSection('Workflow for this asset is currently in progress');
			$o->openField('Change Status?');
				$available_statii = _paintStatusChange($owner, $o, $prefix);
				?><span class="sq-backend-fineprint"><?php
				if (!empty($available_statii)) {
					echo '<br/>Change the status to approve or reject the changes made to this asset';
				} else {
					echo '<i>You cannot approve or reject changes at this time</i>';
				}
				?></span><?php
			$o->closeField();
			if (!empty($available_statii)) {
				$o->openField('Log Message');
					text_area($prefix.'_workflow_new_log_message', '', 60, 6);
					echo '&nbsp;';
					normal_button($prefix.'_workflow_status_update', 'Commit', 'submit_form()');
					?>
					<br/>You can enter a reason for your decision in the box above so that other approvers know why you have accepted or rejected the changes.
					<br/><a href="<?php echo $owner->getBackendHref('logs'); ?>&sq_backend_log_type=asset.workflow&sq_workflow_log_version=<?php echo substr($owner->version, 0, strrpos($owner->version, '.'));?>">Click here to view the workflow message log</a> - a list of system generated workflow messages as well as any comments made by other approvers.
					<?php
				$o->closeField();
			}

		$o->closeSection();

		$schema_workflows = $wfm->getSchemaWorkflows($owner->id);
		foreach ($schema_workflows as $schemaid => $workflow) {
			if (!in_array($schemaid, $running_schemas)) continue;

			$schema_title = "Running Workflow - ".$workflow['schema_name'];
			if (isset($workflow['complete']) && $workflow['complete']) {
				$schema_title .= ' <span style="color:#008800">[Completed]</span>';
			} else {
				$schema_title .= ' <span style="color:#880000">[Not Completed]</span>';
			}
			$o->openSection($schema_title);

			$o->openField('Workflow Started');
				echo $GLOBALS['SQ_SYSTEM']->datetime($workflow['started']);
			$o->closeField();
			$o->openField('Time Running');
				require_once SQ_FUDGE_PATH.'/general/datetime.inc';
				echo easy_time_total(time() - $workflow['started'], true);
			$o->closeField();

			foreach ($workflow['steps'] as $stepid => $step_data) {
				$section_title = "Step $stepid - ".$step_data['step_name'];
				if (!$workflow['current_step'] || $workflow['current_step'] > $stepid) {
					$section_title .= ' <span style="color:#008800">[Completed]</span>';
				} else {
					$section_title .= ' <span style="color:#880000">[Not Completed]</span>';
				}

				$o->openSection($section_title);

				if ($step_data['started']) {
					$o->openField('Step Duration');
						echo $GLOBALS['SQ_SYSTEM']->datetime($step_data['started']);
						echo ' - ';
					if ($step_data['completed']) echo $GLOBALS['SQ_SYSTEM']->datetime($step_data['completed']);
					else echo '??';
					echo ' (<i>';
					require_once SQ_FUDGE_PATH.'/general/datetime.inc';
					if ($step_data['completed']) {
						echo easy_time_total($step_data['completed'] - $step_data['started'], true);
					} else {
						echo easy_time_total(time() - $step_data['started'], true);
					}
					echo '</i>)';
				}

				$o->openField('Step Logic');
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
				if ($step_logic_num) echo " $step_logic_num condition".(($step_logic_num > 1) ? 's' : '')." must be met</i>";

				$o->openField('Conditions');

				$has_conds = false;
				ob_start();
				?>
				<table class="sq-backend-table">
					<tr>
						<td class="sq-backend-table-header">&nbsp;</td>
						<td class="sq-backend-table-header" width="270"><b>Group Logic</b></td>
						<td align="center" width="100" class="sq-backend-table-header"><b>Complete ?</b></td>
					</tr>
				<?php
				foreach ($step_data['conditions'] as $pub => $cond_data) {
					$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($pub);
					if (!$asset->id) continue;
					?>
					<tr>
						<td class="sq-backend-table-cell"><?php echo '&nbsp;'.$asset->name.' ['.$asset->type()." : $pub]";?></td>
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
						if ($cond_logic_num) echo " $cond_logic_num group member".(($cond_logic_num > 1) ? 's' : '')." must approve</i>";
					}

					?></td>

						<td align="center" class="sq-backend-table-cell">
						<?php
							if (isset($cond_data['complete']) && $cond_data['complete']) {
								echo '<span style="color:#008800"><b>Yes</b></span>';
							} else {
								// do some extra checking
								if (is_a($asset, 'user')) {
									echo ((isset($cond_data['published_by'][$asset->id])) ? '<span style="color:#008800"><b>Yes</b></span>' : '&nbsp;');
								} else {
									if ($cond_logic_num) $user_count = $cond_logic_num;
									else $user_count = count($GLOBALS['SQ_SYSTEM']->am->getChildren($asset->id, Array('user'), false));

									echo '<span style="color:#';
									if (count($cond_data['published_by']) >= $user_count) echo '008800';
									else echo '880000';
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
					echo 'None';
				} else {
					ob_end_flush();
					?></table><?php
				}

				$o->closeSection();

			}// end foreach

			$o->closeSection();

		}// end foreach

	} else {
		// there is no running workflow so admin access can be set
		$admin_access = $owner->adminAccess('workflow');
	}

	// lets show some workflow warnings about metadata if we need to
	if ($owner->status & SQ_SC_STATUS_PENDING) {
		$mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager();
		$m_complete = $mm->requiredFieldsComplete($owner->id);
		if (!$m_complete) {
			$o->openSection('Metadata Warning');
				$o->openField('&nbsp;');
					?>
					<p class="sq-backend-data"><span style="color:#880000"><b>WARNING:</b></span> "<?php echo $owner->name; ?>" is currently in an approval process but some required metadata fields are not yet compelete. These fields must be completed before "<?php echo $owner->name; ?>" can become 'Approved' to go live.</p>
					<p class="sq-backend-data">These metadata fields can be completed by any editor or administrator of "<?php echo $owner->name; ?>" by going to the <a href="<?php echo $owner->getBackendHref('metadata'); ?>">Metadata screen</a>.</p>
					<?php
				$o->closeField();
			$o->closeSection();
		}
	}

	$o->openSection('Workflow Schemas');

	$direct = Array();
	foreach($schemas as $schemaid => $access) {
		if (!isset($direct[$access])) $direct[$access] = Array();
		$direct[$access][] = $schemaid;
	}
	ksort($direct);

	if (!empty($direct)) {
		$o->openField('&nbsp;');
		foreach ($direct as $access => $schemaids) {
		?>
			<table class="sq-backend-table">
				<tr>
					<td class="sq-backend-table-header">
						<span style="color: <?php echo (($access == '0') ? 'red' : 'green'); ?>"><?php echo (($access == '0') ? 'Denied' : 'Applied') ?></span>
					</td>
					<?php
					if ($admin_access) {
						?><td align="center" width="100" class="sq-backend-table-header" style="font-weight: bold;">Delete ?</td><?php
					}
					?>
				</tr>
			<?php
			foreach($schemaids as $schemaid) {
				if ($schemaid) {
					$schema = &$GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
					if (!$schema->id || !is_a($schema, 'workflow_schema')) continue;
				}
			?>
				<tr>
					<td class="sq-backend-table-cell">
					<?php
					echo $schema->name.' ['.$schema->type().' : '.$schemaid.']';
					?>
					</td>
					<?php
					if ($admin_access) {
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

	if ($admin_access) {
		$o->openField('New ?', 'new_line');
			combo_box($prefix.'_workflow[new][access]', Array('' => '', '1' => 'Apply', '0' => 'Deny'), false, '');
			echo ' Workflow schema ';
			asset_finder($prefix.'_workflow[new][schemaid]', 0, Array('workflow_schema' => 'D'));
		$o->closeField();
	}

	$o->closeSection();

	if ($admin_access) {
		$kiddies = $GLOBALS['SQ_SYSTEM']->am->getChildren($owner->id);
		if (count($kiddies)) {
			$o->openSection('Cascade Options');
				$o->openField('Note');
					echo 'These actions will affect '.count($kiddies).' other assets below "'.$owner->name.'"';
				$o->closeField();
				$o->openField('&nbsp;');
					check_box($prefix.'_workflow[cascade_changes]', '1', true);
					echo ' Cascade schema changes';
					echo '<br/>';
					check_box($prefix.'_workflow[manual_cascade]');
					echo ' Manually cascade schemas';
				$o->closeField();
			$o->closeSection();
		}
	}

	return $admin_access;

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
	if (!empty($running_schemas)) {
		_processStatusChange($owner, $o, $prefix);
		$updated = true;
	}

	// first we need to process any messages that may have been submitted
	if (isset($_POST[$prefix.'_workflow_new_log_message'])) {
		$message_body = trim($_POST[$prefix.'_workflow_new_log_message']);
		if (!empty($message_body)) {
			$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
			$message = $ms->newMessage(Array(), 'Workflow User Log', $message_body, 'asset.workflow.userlog');
			$message->parameters['assetid'] = $owner->id;
			$message->parameters['version'] = substr($owner->version, 0, strrpos($owner->version, '.'));
			$message->send();

			$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($GLOBALS['SQ_SYSTEM']->currentUserid());
			$log_body = 'Workflow log message by "'.$user->name.'" added successfully to asset "'.$owner->name.'"';
			$log = $ms->newMessage(Array(), 'User Log Message Added', $log_body, 'backend');
			$log->parameters['assetid'] = $owner->id;
			$log->send();
			$updated = true;
		}
	}

	// from here on in, if we dont have admin access - piss off
	if ($owner->id && !$owner->adminAccess('')) {
		if (!$updated) {
			$GLOBALS['SQ_SYSTEM']->paintLogin('Login', 'You do not have permission to edit the workflow schemas for <i>'.$owner->name.'</i>');
			exit();
		}
		return $updated;
	}

	// if we dont have admin access, dont process anything
	if (!$owner->adminAccess('workflow')) return $updated;

	if (!isset($_POST[$prefix.'_workflow'])) return $updated;

	$wfm = &$GLOBALS['SQ_SYSTEM']->getWorkflowManager();

	// are we cascading these schema changes to our children?
	$cascade_changes = false;
	if (isset($_POST[$prefix.'_workflow']['cascade_changes']) && $_POST[$prefix.'_workflow']['cascade_changes']) {
		$cascade_changes = true;
	}

	$schema_vars = Array();
	$post_data = $_POST[$prefix.'_workflow'];

	// apply a schema
	if (!empty($post_data['new']['schemaid'])) {
		if (!isset($post_data['new']['access']) || $post_data['new']['access'] == '') {
			trigger_error('You need to select the type of access in order to apply schema #'.$post_data['new']['schemaid'], E_USER_NOTICE);
		} else {
			if ($cascade_changes) {
				// cascading access of some kind, so queue it up for the HIPO
				$schema_vars[] = Array('access'          => (int)$post_data['new']['access'],
									   'schemaid'        => (int)$post_data['new']['schemaid'],
									   'previous_access' => null,
									   );
			} else {
				// lets do this ourselves
				if (!$wfm->setSchema($owner->id, (int)$post_data['new']['schemaid'], (int)$post_data['new']['access'])) return $updated;
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
				$schema_vars[] = Array('access'          => -1,
									   'schemaid'        => $schemaid,
									   'previous_access' => $current_access,
									   );
			} else {
				if (!$wfm->deleteSchema($owner->id, $schemaid)) return $updated;
			}
		}
	}

	if (isset($_POST[$prefix.'_workflow']['manual_cascade']) && $_POST[$prefix.'_workflow']['manual_cascade']) {
		$schemas = $wfm->getSchemas($owner->id);
		foreach ($schemas as $schemaid => $access) {
			$schema_vars[] = Array('access'          => $access,
								   'schemaid'        => $schemaid,
								   'previous_access' => null,
								   );
		}
	}


	if (!empty($schema_vars)) {
		$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$vars = Array('assetid' => $owner->id, 'schema_changes' => $schema_vars);
		$hh->queueHipo('hipo_job_edit_workflow_schemas', $vars);
	}

	return true;

}//end processWorkflow()



?>
