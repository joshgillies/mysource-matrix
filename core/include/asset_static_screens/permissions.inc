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
* $Id: permissions.inc,v 1.44 2012/08/30 01:09:20 ewang Exp $
*
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Permissions Static Screen Functions
*
* @author  Greg Sherwood <gsherwood@squiz.net>
* @version $Revision: 1.44 $
* @package MySource_Matrix
*/


/**
* Paint the the access for permissions on a particular asset (used in popups etx)
*
* @param object	&$owner	the asset whose interface we are painting
* @param object	&$o				the outputter class
* @param object	&$ei			the edit interface
*
* @return boolean
* @access public
*/
function paintInlinePermissions(Asset $owner, Backend_Outputter $o, Asset_Edit_Interface $ei, $cascades=TRUE)
{
	$prefix = $owner->getPrefix();
	$perms = Array('Read' => SQ_PERMISSION_READ, 'Write' => SQ_PERMISSION_WRITE, 'Administrator' => SQ_PERMISSION_ADMIN);

	$direct = Array();

	foreach ($perms as $title => $perm) {
		$permissions = $GLOBALS['SQ_SYSTEM']->am->getAssetPermissionByCascade($owner->id, $perm, NULL, $cascades);

		foreach ($permissions as $userid => $perm_info) {
			$granted = $perm_info['granted'];
			if (!isset($direct[$perm][$granted])) {
				$direct[$perm][$granted] = Array();
			}
			$direct[$perm][$granted][] = $perm_info['userid'];
		}

		if (!empty($direct[$perm])) ksort($direct[$perm]);
	}

	// No permissions set at all for this asset?
	if (empty($direct)) return FALSE;

	$o->openSection(translate('permissions'));
		$o->openField('&nbsp;');
		?>
			<table class="sq-backend-table">
				<tr>
					<td class="sq-backend-table-header">
					  <?php echo translate('user'); ?>
					</td>
					<td class="sq-backend-table-header">
					  <?php echo translate('permissions'); ?>
					</td>
					<td class="sq-backend-table-header" align="right">
					  <?php echo translate('access'); ?>
					</td>
				</tr>
		<?php

	// Print out the permissions, then
	foreach ($perms as $title => $perm) {
		if (!empty($direct[$perm])) {
			foreach ($direct[$perm] as $granted => $userids) {
				foreach ($userids as $userid) {
					if ($userid) {
						$user_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($userid), Array('user', 'user_group'), FALSE);
						if (empty($user_info)) continue;
					}
					?>
					<tr>
						<td class="sq-backend-table-cell">
					<?php
						echo get_asset_tag_line($userid);
					?>
						</td>
						<td class="sq-backend-table-cell">
							<?php echo $title.' Permission'?>
						</td>
						<td class="sq-backend-table-cell" align="right">
							<span class="sq-txt-highlight-<?php echo (($granted == '0') ? 'red' : 'green'); ?>"><?php echo (($granted == '0') ? translate('denied') : translate('granted')) ?></span>
						</td>
					</tr>
				<?php
				}// end foreach
			}//end foreach
		}// end if
	}//end foreach

	?>
	</table>
	<?php
		$o->closeField();
	$o->closeSection();

	return TRUE;

}//end paintInlinePermissions()


/**
* Paint the interface for managing permissions
*
* @param object	&$owner	the asset whose interface we are painting
* @param object	&$o		the outputter class
* @param object	&$ei	the edit interface
* @param array	$keywords the keywords to replace
*
* @return boolean
* @access public
*/
function paintPermissions(&$owner, &$o, &$ei, $keywords=NULL)
{
	$prefix = $owner->getPrefix();
	$perms = Array('read' => SQ_PERMISSION_READ, 'write' => SQ_PERMISSION_WRITE, 'admin' => SQ_PERMISSION_ADMIN);

	$admin_access = $owner->adminAccess('permissions');
	$public_userid = $GLOBALS['SQ_SYSTEM']->am->getSystemAssetid('public_user');

	
	foreach ($perms as $title => $perm) {
		$o->openSection(translate('permission_section', translate($title)));

		$permissions = $GLOBALS['SQ_SYSTEM']->am->getAssetPermissionByCascade($owner->id, $perm);

		$direct = Array();
		foreach ($permissions as $perm_info) {
			$granted = $perm_info['granted'];
			$userid = $perm_info['userid'];

			if (!isset($direct[$granted])) {
				$direct[$granted] = Array();
			}
			$direct[$granted][$userid] = $perm_info;
		}
		ksort($direct);

		if ($admin_access) {
			$o->openField(translate('current'));
		} else {
			$o->openRaw();
		}

		if (!empty($direct)) {
			foreach ($direct as $granted => $perm_details) {
			?>
				<table class="sq-backend-table">
					<tr>
						<th class="sq-backend-table-header">
							<span class="sq-txt-highlight-<?php echo (($granted == '0') ? 'red' : 'green'); ?>"><?php echo (($granted == '0') ? translate('denied') : translate('granted')) ?></span>
						</th>
						<th align="center" width="100" class="sq-backend-table-header" style="font-weight: bold;"><?php echo translate('cascade_question'); ?></th>
						<?php
						if ($admin_access) {
							?><th align="center" width="100" class="sq-backend-table-header" style="font-weight: bold;"><?php echo translate('delete_question'); ?></th><?php
						}
						?>
					</tr>
				<?php

				foreach ($perm_details as $userid => $perm_detail) {
					?>
					<tr>
						<td class="sq-backend-table-cell">
							<?php
							echo get_asset_tag_line($userid);
							?>
						</td>
						<td align="center" width="100" class="sq-backend-table-cell">
						<?php
						if ($admin_access) {
							check_box($prefix.'_permissions['.$perm.'][cascades]['.rawurlencode($userid).']', 1, $perm_detail['cascades']);
						} else {
							?><img src="<?php echo sq_web_path('lib'); ?>/web/images/<?php echo ($perm_detail['cascades'] ? 'tick' : 'cross'); ?>.gif" width="15" height="15" /><?php
						}
							?>
						</td>
						<?php
						if ($admin_access) {
							?><td align="center" width="100" class="sq-backend-table-cell"><?php
							check_box($prefix.'_permissions['.$perm.'][delete]['.rawurlencode($userid).']');
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

		} else {
			echo translate('no_access_permissions_set', strtolower($title));
		}//end if

		if ($admin_access) {
			$o->closeField();
		} else {
			$o->closeRaw();
		}

		if ($admin_access) {
			$o->openField(translate('new'));
				?>
				<table cellspacing="0">
				<?php
				// if Public Access isn't set
				if (!in_array($public_userid, array_get_index($direct, 0, Array())) && !in_array($public_userid, array_get_index($direct, 1, Array()))) {
					?>
					<tr>
						<td colspan="2">
							<?php
							combo_box($prefix.'_permissions['.$perm.'][public_access][granted]', Array('' => '', '1' => translate('grant'), '0' => translate('deny')), FALSE, '');
							echo ' '.translate('public_permission');
							?>
						</td>
					</tr>
					<?php
				}
				?>
					<tr>
						<td valign="top">
							<?php
							combo_box($prefix.'_permissions['.$perm.'][new][granted]', Array('' => '', '1' => translate('grant'), '0' => translate('deny')), FALSE, '');
							echo ' '.translate('specific_permission');
							?>
							&nbsp;
						</td>
						<td><?php multiple_asset_finder($prefix.'_permissions['.$perm.'][new][userid]', Array(), Array('user' => 'D', 'user_group' => 'D')); ?></td>
					</tr>
				</table>
				<?php
			$o->closeField();
			$o->openField(translate('cascade_question'), 'new_line');
				check_box($prefix.'_permissions['.$perm.'][new][cascades]', 1, TRUE);
				?> Automatically cascade this permission to new children<?php
				//$o->note('If unchecked, the new permission(s) above will be applied or denied only to dependants of this asset, regardless of the Cascade Changes settings below. It will also not apply the workflow to any asset that becomes a new child of this asset (through creating a new asset, or moving an existing one, or creating a new link from one). This setting can be toggled after being applied.');
			$o->closeField();
		}//end if

		$o->closeSection();
	}//end foreach $perms

	if ($admin_access) {
		$kiddies = $GLOBALS['SQ_SYSTEM']->am->getChildren($owner->id);
		if (count($kiddies)) {
			$o->openSection(translate('cascade_permission_changes'));
				$o->openField(translate('note'));
					$o->note(translate('cascade_permissions_note', htmlspecialchars($owner->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET), count($kiddies)));
				$o->closeField();
				$o->openField('&nbsp;');
					check_box($prefix.'_permissions[cascade_changes]', '1', TRUE);
					label(translate('cascade_permissions'), $prefix.'_permissions[cascade_changes]');
				$o->closeField();
			$o->closeSection();

			$o->openSection(translate('manually_cascade_permissions'));
				$o->openField(translate('note'));
					$o->note(translate('manually_cascade_permissions_note', htmlspecialchars($owner->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET), count($kiddies)));
				$o->closeField();
				$o->openField('&nbsp;');
					check_box($prefix.'_permissions[manual_cascade]');
					label(translate('manually_cascade_permissions_option'), $prefix.'_permissions[manual_cascade]');
				$o->closeField();
			$o->closeSection();
		}
	}
	
	//FUTURE PERMISSION (SIMILAR TO FUTURE STATUS)
	$o->openSection(translate('future_permission'));
	
	$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_future_permission');
	$fp_jobs = Cron_Job_Future_Permission::getActiveJobs($owner);
	if (!empty($fp_jobs)) {
		$o->openField(translate('scheduled_future_permission_change'), 'new_line');
		?>
		<table class="sq-backend-table">
			<tr>
				<td class="sq-backend-table-header"><?php echo translate('action'); ?></td>
				<td class="sq-backend-table-header"><?php echo translate('permission'); ?></td>
				<td class="sq-backend-table-header">
					<span class="sq-txt-highlight-green"><?php echo translate('granted'); ?></span>
				</td>
				<td class="sq-backend-table-header">
					<span class="sq-txt-highlight-red"><?php echo translate('denied'); ?></span>
				</td>
				<td class="sq-backend-table-header"><?php echo translate('when'); ?></td>
				<td align="center" class="sq-backend-table-header" style="text-align: center;"><?php echo translate('cascade_question'); ?></td>
				<td align="center" class="sq-backend-table-header" style="text-align: center;"><?php echo translate('update_all_question'); ?></td>
				<?php
				if ($admin_access) {
				?>
				<td align="center" class="sq-backend-table-header" style="text-align: center;"><?php echo translate('delete_question'); ?></td>
				<?php
				}
				?>
			</tr>
			<?php
			//sort the job by date
			$sorted_fp_jobs = _sortCronJobsByDate($fp_jobs);
			//print the active jobs
			foreach ($sorted_fp_jobs as $fp_job) {
				?>
				<tr>
					<td class="sq-backend-table-cell">
						<?php echo $fp_job->actionName(); ?>
					</td>
					<td class="sq-backend-table-cell">
						<?php echo $fp_job->permissionName(); ?>
					</td>
					<td class="sq-backend-table-cell">
						<?php
							 $granted_userids = $fp_job->getGrantedUsers(TRUE);
							 foreach ($granted_userids as $userid) {
							 	echo get_asset_tag_line($userid);
							 	echo '<br />';
							 }
						?>
					</td>
					<td class="sq-backend-table-cell">
						<?php
							 $denied_userids = $fp_job->getGrantedUsers(FALSE);
							 foreach ($denied_userids as $userid) {
							 	echo get_asset_tag_line($userid);
							 	echo '<br />';
							 }
						?>
					</td>
					<td class="sq-backend-table-cell">
						<?php echo $fp_job->readableWhen(); ?>
					</td>
					<td align="center" class="sq-backend-table-cell">
					<?php
						if ($fp_job->attr('cascade_to_new')) {
							echo translate('yes');
						} else {
							echo translate('no');
						}
					?>
					</td>
					<td align="center" class="sq-backend-table-cell">
					<?php
						if ($fp_job->attr('dependants_only')) {
							echo translate('no');
						} else {
							echo translate('yes');
						}
						
						if ($admin_access) {
					?>
					</td>
					<td align="center" class="sq-backend-table-cell">
					<?php
						if ($fp_job->canDelete()) {
							check_box($prefix.'_remove_fp[]', $fp_job->id);
						} else {
							echo '&nbsp;';
						}
						}
					?>
					</td>
				</tr>
				<?php
			}//end for
			?>
		</table>
		<?php
		
		$o->closeField();
	} else if (!$admin_access){
		$o->openRaw();
		echo translate('no_future_permissions_set');
		$o->closeRaw();
	}//end if

	if ($admin_access) {
		$o->openField(translate('add_new_future_permission'), 'new_line');
			$cron_mgr = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
			$cron_epoc = (int)$cron_mgr->attr('epoch');
			$cron_refresh = (int)$cron_mgr->attr('refresh_time');
			if ($cron_epoc > 0 && $cron_refresh > 0) {
				require_once SQ_FUDGE_PATH.'/general/datetime.inc';
				$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_future_permission');
				$fp = new Cron_Job_Future_Permission();
				$edit_fns = $fp->getEditFns();
				$edit_fns->paintInLineBackend($fp, $o, $prefix.'_new_fp');
				echo '<br /><br />';

				$o->note(translate('cron_manager_next_run', $cron_mgr->readableRefreshTime(), readable_datetime($cron_mgr->timeOfNextRun())));

				$print_commit_button = TRUE;
			} else {
				echo translate('cron_manager_not_configured');
			}
		$o->closeField();
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($cron_mgr);
	}//end if
	
	$o->closeSection();
	//END FUTURE PERMISSION SECTION

	return $admin_access;

}//end paintPermissions()


/**
* Process the interface for managing permissions
*
* @param object	&$owner	the asset whose interface we are painting
* @param object	&$o		the outputter class
* @param object	&$ei	the edit interface
*
* @return boolean
* @access public
*/
function processPermissions(&$owner, &$o, &$ei)
{
	// if we dont have admin access, go away
	if ($owner->id && !$owner->adminAccess('')) {
		$GLOBALS['SQ_SYSTEM']->paintLogin(translate('login'), translate('cannot_access_asset', htmlspecialchars($owner->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET)));
		exit();
	}

	$admin_access = $owner->adminAccess('permissions');
	// if we dont have the permissions lock, dont process anything
	if (!$admin_access) return FALSE;

	$permission_vars = Array();
	$prefix = $owner->getPrefix();
	if (!isset($_POST[$prefix.'_permissions'])) {
		return FALSE;
	}

	$cascade_changes = !empty($_POST[$prefix.'_permissions']['cascade_changes']);

	$perms = Array('Administrator' => SQ_PERMISSION_ADMIN, 'Write' => SQ_PERMISSION_WRITE, 'Read' => SQ_PERMISSION_READ);
	$public_userid = $GLOBALS['SQ_SYSTEM']->am->getSystemAssetid('public_user');

	//PROCESS FUTURE PERMISSION
	$cron_mgr = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
	if (!is_null($cron_mgr)) {
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_future_permission');

		if (!empty($_POST[$prefix.'_remove_fp'])) {
			$fp_jobs = Cron_Job_Future_Permission::getActiveJobs($owner);
			if (!empty($fp_jobs)) {

				if ($GLOBALS['SQ_SYSTEM']->am->acquireLock($cron_mgr->id, 'links')) {
					for ($i = 0; $i < count($fp_jobs); $i++) {
						if (in_array($fp_jobs[$i]->id, $_POST[$prefix.'_remove_fp'])) {
							$cron_mgr->removeJob($fp_jobs[$i]);
						}// end if
					}// end for

					$GLOBALS['SQ_SYSTEM']->am->releaseLock($cron_mgr->id, 'links');

				} else {
					trigger_localised_error('SYS0232', E_USER_NOTICE);

				}// end if

			}// end if
		}// end if


		if ($admin_access) {
			$fp = new Cron_Job_Future_Permission();
			$edit_fns = $fp->getEditFns();
			if ($fp->setAssetToUpdate($owner) && $edit_fns->processInLineBackend($fp, $o, $prefix.'_new_fp')) {
				$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
				$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
				if ($cron_mgr->addJob($fp, $GLOBALS['SQ_SYSTEM']->user)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
				} else {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				}
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			}

		}// end if

	}//end if is NULL cron mgr
	
	//END PROCESS FUTURE PERMISSION
	
	foreach ($perms as $title => $perm) {
		// we need to check for write access each time because
		// we may actually be revoking write access and then
		// trying to put it back
		if (!$owner->adminAccess('permissions')) continue;

		$post_data = $_POST[$prefix.'_permissions'][$perm];

		// grant public access
		if (isset($post_data['public_access']['granted']) && $post_data['public_access']['granted'] != '') {
			$permission_vars[] = Array(
											'permission'		=> $perm,
											'assetids'			=> Array($owner->id),
											'granted'			=> (int)$post_data['public_access']['granted'],
											'userid'			=> $public_userid,
											'cascades'			=> (int)isset($post_data['new']['cascades']),
											'previous_access'	=> NULL,
											'dependants_only'	=> !($cascade_changes && isset($post_data['new']['cascades'])),
										);
		}

		// grant access to users
		if (!empty($post_data['new'])) {
			foreach ($post_data['new']['userid'] as $entry) {
				if (!empty($entry['assetid'])) {
					if (!isset($post_data['new']['granted']) || $post_data['new']['granted'] == '') {
						trigger_localised_error('SYS0269', E_USER_NOTICE);
						break;
					} else {
						$permission_vars[] = Array(
														'permission'		=> $perm,
														'assetids'			=> Array($owner->id),
														'granted'			=> (int)$post_data['new']['granted'],
														'userid'			=> $entry['assetid'],
														'cascades'			=> (int)isset($post_data['new']['cascades']),
														'previous_access'	=> NULL,
														'dependants_only'	=> !($cascade_changes && isset($post_data['new']['cascades'])),
													);
					}
				}
			}
		}

		// revoke [delete] access (grant or deny can be revoked)
		if (!empty($post_data['delete'])) {
			foreach ($post_data['delete'] as $userid => $on) {
				$userid = rawurldecode($userid);
				$current_access = $GLOBALS['SQ_SYSTEM']->am->getAssetPermissionByCascade($owner->id, $perm, $userid);
				$current_access = $current_access[0];
				$permission_vars[] = Array(
												'permission'		=> $perm,
												'assetids'			=> Array($owner->id),
												'granted'			=> -1,
												'userid'			=> $userid,
												'previous_access'	=> $current_access['granted'],
												'dependants_only'	=> !($cascade_changes && $current_access['cascades']),
											);
			}
		}

		// Change of cascade status
		$perm_infos = $GLOBALS['SQ_SYSTEM']->am->getAssetPermissionByCascade($owner->id, $perm);
		foreach ($perm_infos as $perm_info) {
			$userid   = $perm_info['userid'];
			$cascades = isset($post_data['cascades'][rawurlencode($userid)]);
			if ((boolean)$cascades != (boolean)$perm_info['cascades']) {
				// Cascade changes are only made to dependants
				$permission_vars[] = Array(
									'permission'		=> $perm,
									'assetids'			=> Array($owner->id),
									'granted'			=> $perm_info['granted'],
									'userid'			=> $userid,
									'previous_access'	=> NULL,
									'cascades'			=> $cascades,
									'dependants_only'	=> 1,
								 );
			}
		}

		if (!empty($_POST[$prefix.'_permissions']['manual_cascade'])) {
			$current = $GLOBALS['SQ_SYSTEM']->am->getAssetPermissionByCascade($owner->id, $perm, NULL, TRUE);
			foreach ($current as $perm_info) {
				$permission_vars[] = Array(
												'permission'		=> $perm,
												'granted'			=> $perm_info['granted'],
												'userid'			=> $perm_info['userid'],
												'assetids'			=> Array($owner->id),
												'previous_access'	=> NULL,
												'cascades'			=> TRUE,
											);
			}
		}

	}//end foreach perms

	if (!empty($permission_vars)) {
		$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$vars = Array(
					'permission_changes'	=> $permission_vars,
					'permission_screen_assetid'	=> $owner->id,
				);
		$hh->queueHipo('hipo_job_edit_permissions', $vars);
	}

	return TRUE;

}//end processPermissions()


/**
 * Sort the array of cron jobs by their 'when' attributes
 *
 * @param array $cron_jobs	the array of cron jobs that needs to be sorted
 * @return array			the sorted cron jobs array
 */
function _sortCronJobsByDate($cron_jobs) {
	//create an array that keeps assetid and date of the cron jobs
	$assetid_date_arr = Array();
	foreach ($cron_jobs as $cron_job) {
		$assetid_date_arr[$cron_job->id] = $cron_job->attr('when');
	}
	//sort the array by date but keep the keys associated with them
	asort($assetid_date_arr);
	
	//the sorted cron jobs array
	$sorted_cron_jobs = Array();
	foreach ($assetid_date_arr as $assetid => $date) {
		//get the cron job that has the $assetid
		$matched = NULL;
		foreach ($cron_jobs as $cron_job) {
			if ($assetid == $cron_job->id) {
				$matched = $cron_job;
				break;
			}
		}
		$sorted_cron_jobs[] = $matched;
	}
	
	return $sorted_cron_jobs;
	
}//end _sortCronJobByDate()


?>
