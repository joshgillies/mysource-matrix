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
* $Id: permissions.inc,v 1.27.2.1 2006/07/18 05:19:27 lwright Exp $
*
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Permissions Static Screen Functions
*
* @author  Greg Sherwood <gsherwood@squiz.net>
* @version $Revision: 1.27.2.1 $
* @package MySource_Matrix
*/


/**
* Paint the the access for permissions on a particular asset (used in popups etx)
*
* @param object	&$owner	the asset whose interface we are painting
* @param object	&$o		the outputter class
* @param object	&$ei	the edit interface
*
* @return boolean
* @access public
*/
function paintInlinePermissions(&$owner, &$o, &$ei)
{
	$prefix = $owner->getPrefix();
	$perms = Array('Read' => SQ_PERMISSION_READ, 'Write' => SQ_PERMISSION_WRITE, 'Administrator' => SQ_PERMISSION_ADMIN);

	$direct = Array();

	foreach ($perms as $title => $perm) {
		$permissions = $GLOBALS['SQ_SYSTEM']->am->getPermission($owner->id, $perm, NULL, FALSE, FALSE, TRUE, TRUE);

		foreach ($permissions as $userid => $granted) {
			if (!isset($direct[$perm][$granted])) {
				$direct[$perm][$granted] = Array();
			}
			$direct[$perm][$granted][] = $userid;
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
							<span style="color: <?php echo (($granted == '0') ? 'red' : 'green'); ?>; font-weight:bold;"><?php echo (($granted == '0') ? translate('denied') : translate('granted')) ?></span>
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
*
* @return boolean
* @access public
*/
function paintPermissions(&$owner, &$o, &$ei)
{
	$prefix = $owner->getPrefix();
	$perms = Array('read' => SQ_PERMISSION_READ, 'write' => SQ_PERMISSION_WRITE, 'admin' => SQ_PERMISSION_ADMIN);

	$admin_access = $owner->adminAccess('permissions');
	$public_userid = $GLOBALS['SQ_SYSTEM']->am->getSystemAssetid('public_user');

	foreach ($perms as $title => $perm) {
		$o->openSection(translate('permission_section', translate($title)));

		$permissions = $GLOBALS['SQ_SYSTEM']->am->getPermission($owner->id, $perm, NULL, FALSE, FALSE, TRUE, TRUE);

		$direct = Array();
		foreach ($permissions as $userid => $granted) {
			if (!isset($direct[$granted])) {
				$direct[$granted] = Array();
			}
			$direct[$granted][] = $userid;
		}
		ksort($direct);

		if (!empty($direct)) {
			$o->openField('&nbsp;');
			foreach ($direct as $granted => $userids) {
			?>
				<table class="sq-backend-table">
					<tr>
						<td class="sq-backend-table-header">
							<span style="color: <?php echo (($granted == '0') ? 'red' : 'green'); ?>"><?php echo (($granted == '0') ? translate('denied') : translate('granted')) ?></span>
						</td>
						<?php
						if ($admin_access) {
							?><td align="center" width="100" class="sq-backend-table-header" style="font-weight: bold;"><?php echo translate('delete_question'); ?></td><?php
						}
						?>
					</tr>
				<?php

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
			$o->openRaw();
			echo translate('no_access_permissions_set', strtolower($title));
			$o->closeRaw();
		}//end if

		if ($admin_access) {
			$o->openField('New ?', 'new_line');
				// if Public Access isn't set
				if (!in_array($public_userid, array_get_index($direct, 0, Array())) && !in_array($public_userid, array_get_index($direct, 1, Array()))) {
					combo_box($prefix.'_permissions['.$perm.'][public_access][granted]', Array('' => '', '1' => translate('grant'), '0' => translate('deny')), FALSE, '');
					echo ' '.translate('public_permission').'<br/>';
				}

				combo_box($prefix.'_permissions['.$perm.'][new][granted]', Array('' => '', '1' => translate('grant'), '0' => translate('deny')), FALSE, '');
				echo ' '.translate('specific_permission').' ';
				asset_finder($prefix.'_permissions['.$perm.'][new][userid]', 0, Array('user' => 'D', 'user_group' => 'D'));
			$o->closeField();
		}//end if

		$o->closeSection();
	}//end foreach $perms

	if ($admin_access) {
		$kiddies = $GLOBALS['SQ_SYSTEM']->am->getChildren($owner->id);
		if (count($kiddies)) {
			$o->openSection(translate('cascade_permission_changes'));
				$o->openField(translate('note'));
					$o->note(translate('cascade_permissions_note', $owner->name, count($kiddies)));
				$o->closeField();
				$o->openField('&nbsp;');
					check_box($prefix.'_permissions[cascade_changes]', '1', TRUE);
					label(translate('cascade_permissions'), $prefix.'_permissions[cascade_changes]');
				$o->closeField();
			$o->closeSection();

			$o->openSection(translate('manually_cascade_permissions'));
				$o->openField(translate('note'));
					$o->note(translate('manually_cascade_permissions_note', $owner->name, count($kiddies)));
				$o->closeField();
				$o->openField('&nbsp;');
					check_box($prefix.'_permissions[manual_cascade]');
					label(translate('manually_cascade_permissions_option'), $prefix.'_permissions[manual_cascade]');
				$o->closeField();
			$o->closeSection();
		}
	}

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
		$GLOBALS['SQ_SYSTEM']->paintLogin(translate('login'), translate('cannot_access_asset', $owner->name));
		exit();
	}

	// if we dont have the permissions lock, dont process anything
	if (!$owner->adminAccess('permissions')) return FALSE;

	$permission_vars = Array();
	$prefix = $owner->getPrefix();
	if (!isset($_POST[$prefix.'_permissions'])) {
		return FALSE;
	}

	$cascade_changes = !empty($_POST[$prefix.'_permissions']['cascade_changes']);

	$perms = Array('Administrator' => SQ_PERMISSION_ADMIN, 'Write' => SQ_PERMISSION_WRITE, 'Read' => SQ_PERMISSION_READ);
	$public_userid = $GLOBALS['SQ_SYSTEM']->am->getSystemAssetid('public_user');

	foreach ($perms as $title => $perm) {
		// we need to check for write access each time because
		// we may actually be revoking write access and then
		// trying to put it back
		if (!$owner->adminAccess('permissions')) continue;

		$post_data = $_POST[$prefix.'_permissions'][$perm];

		// grant public access
		if (isset($post_data['public_access']['granted']) && $post_data['public_access']['granted'] != '') {
			$permission_vars[$perm][] = Array(
											'granted'			=> (int)$post_data['public_access']['granted'],
											'userid'			=> $public_userid,
											'previous_access'	=> NULL,
										);
		}

		// grant access to a user
		if (!empty($post_data['new']['userid']['assetid'])) {
			if (!isset($post_data['new']['granted']) || $post_data['new']['granted'] == '') {
				trigger_localised_error('SYS0269', E_USER_NOTICE, $post_data['new']['userid']['assetid']);
			} else {
					$permission_vars[$perm][] = Array(
													'granted'			=> (int)$post_data['new']['granted'],
													'userid'			=> $post_data['new']['userid']['assetid'],
													'previous_access'	=> NULL,
												);
			}
		}

		// revoke [delete] access (grant or deny can be revoked)
		if (!empty($post_data['delete'])) {
			foreach ($post_data['delete'] as $userid => $on) {
				$userid = rawurldecode($userid);
				$current = $GLOBALS['SQ_SYSTEM']->am->getPermission($owner->id, $perm, NULL, FALSE, FALSE, TRUE, TRUE);
				$current_access = $current[$userid];
				$permission_vars[$perm][] = Array(
												'granted'			=> -1,
												'userid'			=> $userid,
												'previous_access'	=> $current_access,
											);
			}
		}

		if (!empty($_POST[$prefix.'_permissions']['manual_cascade'])) {
			$current = $GLOBALS['SQ_SYSTEM']->am->getPermission($owner->id, $perm, NULL, FALSE, FALSE, TRUE, TRUE);
			foreach ($current as $userid => $granted) {
				$permission_vars[$perm][] = Array(
												'granted'			=> $granted,
												'userid'			=> $userid,
												'previous_access'	=> NULL,
											);
			}
		}

	}//end foreach perms

	if (!empty($permission_vars)) {
		$hh =& $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$vars = Array(
					'assets'				=> Array($owner->id => Array('type_code' => $owner->type())),
					'permission_changes'	=> $permission_vars,
				);
		if (!$cascade_changes) $vars['dependants_only'] = 1;
		$hh->queueHipo('hipo_job_edit_permissions', $vars);
	}

	return TRUE;

}//end processPermissions()


?>
