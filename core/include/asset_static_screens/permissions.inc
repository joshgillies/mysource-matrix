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
* $Id: permissions.inc,v 1.5 2004/06/17 00:50:32 lwright Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Permissions Static Screen Functions
*
* @author  Greg Sherwood <gsherwood@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/


/**
* Paint the the access for permissions on a particular asset (used in popups etx)
*
* @param object	Asset					$owner	the asset whose interface we are painting
* @param object	Backend_Outputter		$o		the outputter class
* @param object	Asset_Edit_Interface	&$ei		the edit interface
*
* @access public
* @return boolean
*/
function paintInlinePermissions(&$owner, &$o, &$ei)
{
	$prefix = $owner->getPrefix();
	$perms = Array('Read' => SQ_PERMISSION_READ, 'Write' => SQ_PERMISSION_WRITE, 'Administrator' => SQ_PERMISSION_ADMIN);

	if (empty($perms)) return false;

	$o->openSection('Permissions');
		$o->openField('&nbsp;');
		?>
			<table class="sq-backend-table">
				<tr>
					<td class="sq-backend-table-header">
					  <?php echo 'User'; ?>
					</td>
					<td class="sq-backend-table-header">
					  <?php echo 'Permission'; ?>
					</td>
					<td class="sq-backend-table-header" align="right">
					  <?php echo 'Access'; ?>
					</td>
				</tr>
		<?php

	foreach ($perms as $title => $perm) {

		$permissions = $GLOBALS['SQ_SYSTEM']->am->getPermission($owner->id, $perm, null, false, false, true);

		$direct = Array();
		foreach($permissions as $userid => $access) {
			if (!isset($direct[$access])) $direct[$access] = Array();
			$direct[$access][] = $userid;
		}
		ksort($direct);
		if (empty($direct)) continue;

		if (!empty($direct)) {
			foreach ($direct as $access => $userids) {
				foreach($userids as $userid) {
					if ($userid) {
						$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($userid);
						if (!$user->id || (!is_a($user, 'user') && !is_a($user, 'user_group'))) continue;
					}
					?>
					<tr>
						<td class="sq-backend-table-cell">
					<?php
						echo (($userid) ? $user->name.' ['.$user->type().' : '.$userid.']' : 'Public Access');
					?>
						</td>
						<td class="sq-backend-table-cell">
							<?php echo $title.' Access'?>
						</td>
						<td class="sq-backend-table-cell" align="right">
							<span style="color: <?php echo (($access == '0') ? 'red' : 'green'); ?>; font-weight:bold;"><?php echo (($access == '0') ? 'Denied' : 'Granted') ?></span>
						</td>
					</tr>
				<?php
				}// end foreach data
			}//end foreach inherited
		}// end if
	}// end foreach $perms
	?>
	</table>
	<?php
		$o->closeField();
	$o->closeSection();

	return true;

}//end paintInlinePermissions()


/**
* Paint the interface for managing permissions
*
* @param object	Asset					$owner		the asset whose interface we are painting
* @param object	Backend_Outputter		$o			the outputter class
* @param object	Asset_Edit_Interface	&$ei		the edit interface
*
* @access public
* @return boolean
*/
function paintPermissions(&$owner, &$o, &$ei)
{
	$prefix = $owner->getPrefix();
	$perms = Array('Read' => SQ_PERMISSION_READ, 'Write' => SQ_PERMISSION_WRITE, 'Administrator' => SQ_PERMISSION_ADMIN);

	$admin_access = $owner->adminAccess('permissions');

	foreach ($perms as $title => $perm) {
		$o->openSection($title.' Access');

		$permissions = $GLOBALS['SQ_SYSTEM']->am->getPermission($owner->id, $perm, null, false, false, true);

		$direct = Array();
		foreach($permissions as $userid => $access) {
			if (!isset($direct[$access])) $direct[$access] = Array();
			$direct[$access][] = $userid;
		}
		ksort($direct);

		if (!empty($direct)) {
			$o->openField('&nbsp;');
			foreach ($direct as $access => $userids) {
			?>
				<table class="sq-backend-table">
					<tr>
						<td class="sq-backend-table-header">
							<span style="color: <?php echo (($access == '0') ? 'red' : 'green'); ?>"><?php echo (($access == '0') ? 'Denied' : 'Granted') ?></span>
						</td>
						<?php
						if ($admin_access) {
							?><td align="center" width="100" class="sq-backend-table-header" style="font-weight: bold;">Delete ?</td><?php
						}
						?>
					</tr>
				<?php
				$user_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($userids, 'user', false);

				foreach($userids as $userid) {
					$valid_user = true;
					if ($userid) {

						if (!isset($user_info[$userid])) {
							$valid_user = false;
						} else {
							$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($userid);
							if (!$user->id || (!is_a($user, 'user') && !is_a($user, 'user_group'))) continue;
						}

					}
				?>
					<tr>
						<td class="sq-backend-table-cell">
					<?php
						if (!$valid_user) {
							echo ('<span class="sq-backend-warning"><b>Deleted user or user group</b> ['.$userid.']</span>');
						} else {
							echo (($userid) ? $user->name.' ['.$user->type().' : '.$userid.']' : 'Public Access');
						}
					?>
						</td>
					<?php
						if ($admin_access) {
							?><td align="center" width="100" class="sq-backend-table-cell"><?php
							check_box($prefix.'_permissions['.$perm.'][delete]['.$userid.']');
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
				// if Public Access isn't set
				if (!isset($direct['0'])) {
					combo_box($prefix.'_permissions['.$perm.'][public_access][access]', Array('' => '', '1' => 'Grant', '0' => 'Deny'), false, '');
					echo ' Public Access<br/>';
				}

				combo_box($prefix.'_permissions['.$perm.'][new][access]', Array('' => '', '1' => 'Grant', '0' => 'Deny'), false, '');
				echo ' Specific Access ';
				asset_finder($prefix.'_permissions['.$perm.'][new][userid]', 0, Array('user' => 'D', 'user_group' => 'D'));
			$o->closeField();
		}// end if

		$o->closeSection();
	}// end foreach $perms

	if ($admin_access) {
		$kiddies = $GLOBALS['SQ_SYSTEM']->am->getChildren($owner->id);
		if (count($kiddies)) {
			$o->openSection('Cascade Permission Changes?');
				$o->openField('Note');
					?>
					If you choose to cascade the changes to permissions you make on "<?php echo $owner->name; ?>", the changes will also be made on all assets that fall under "<?php echo $owner->name; ?>" in the various hierarchies in which it exists. This will affect <?php echo count($kiddies); ?> other assets.
					<?php
				$o->closeField();
				$o->openField('&nbsp;');
					check_box($prefix.'_permissions[cascade_changes]', '1', true);
					echo ' Yes, cascade permission changes';
				$o->closeField();
			$o->closeSection();

			$o->openSection('Manually Cascade Permissions?');
				$o->openField('Note');
					?>
					Manually cascading permissions will try to set the current permissions of "<?php echo $owner->name; ?>" on all assets that fall under "<?php echo $owner->name; ?>" in the various hierarchies in which it exists. This will affect <?php echo count($kiddies); ?> other assets.<br/>
					<i>Any additional permissions set on affected assets will <b>not</b> be removed during a manual cascade. Any permissions set on affected assets that conflict with ones set on "<?php echo $owner->name; ?>" will also <b>not</b> be changed.</i><br/>
					<b>NB. If you select to manually cascade, any permission changes you make will also cascade even if you do not select them to. Use the option on its own for more predicatable results.</b>
					<?php
				$o->closeField();
				$o->openField('&nbsp;');
					check_box($prefix.'_permissions[manual_cascade]');
					echo ' Manually cascade permissions';
				$o->closeField();
			$o->closeSection();
		}
	}

	return $admin_access;

}//end paintPermissions()


/**
* Process the interface for managing permissions
*
* @param object	Asset					$owner	the asset whose interface we are painting
* @param object	Backend_Outputter		$o		the outputter class
* @param object	Asset_Edit_Interface	&$ei	the edit interface
*
* @access public
* @return boolean TRUE is there is a change, false on error or no change
*/
function processPermissions(&$owner, &$o, &$ei)
{
	// if we dont have admin access, piss off
	if ($owner->id && !$owner->adminAccess('')) {
		$GLOBALS['SQ_SYSTEM']->paintLogin('Login', 'You do not have permission to edit the permissions for <i>'.$owner->name.'</i>');
		exit();
	}

	// if we dont have admin access, dont process anything
	if (!$owner->adminAccess('permissions')) return false;

	$permission_vars = Array();
	$prefix = $owner->getPrefix();
	if (!isset($_POST[$prefix.'_permissions'])) return false;

	// are we cascading these permission changes to our children?
	$cascade_changes = !empty($_POST[$prefix.'_permissions']['cascade_changes']);

	$perms = Array('Administrator' => SQ_PERMISSION_ADMIN, 'Write' => SQ_PERMISSION_WRITE, 'Read' => SQ_PERMISSION_READ);
	foreach ($perms as $title => $perm) {
		// we need to check for write access each time because
		// we may actually be revoking write access and then
		// trying to put it back
		if (!$owner->adminAccess('permissions')) continue;

		$post_data = $_POST[$prefix.'_permissions'][$perm];

		// grant public access
		if (isset($post_data['public_access']['access']) && $post_data['public_access']['access'] != '') {
			if ($cascade_changes) {
				// cascading public access of some kind, so queue it up for the HIPO
				$permission_vars[$perm][] = Array('access'          => (int)$post_data['public_access']['access'],
												  'userid'          => 0,
												  'previous_access' => null,
												  );
			} else {
				// lets do this ourselves
				if (!$GLOBALS['SQ_SYSTEM']->am->setPermission($owner->id, 0, $perm, $post_data['public_access']['access'])) return false;
			}
		}

		// grant access to a user
		if (!empty($post_data['new']['userid']['assetid'])) {
			if (!isset($post_data['new']['access']) || $post_data['new']['access'] == '') {
				trigger_error('You need to select the type of access in order to set a permission for Asset #'.$post_data['new']['userid']['assetid'], E_USER_NOTICE);
			} else {
				if ($cascade_changes) {
					// cascading access of some kind, so queue it up for the HIPO
					$permission_vars[$perm][] = Array('access'          => (int)$post_data['new']['access'],
													  'userid'          => $post_data['new']['userid']['assetid'],
													  'previous_access' => null,
													  );
				} else {
					// lets do this ourselves
					if (!$GLOBALS['SQ_SYSTEM']->am->setPermission($owner->id, $post_data['new']['userid']['assetid'], $perm, $post_data['new']['access'])) return false;

				}
			}
		}

		// revoke [delete] access (grant or deny can be revoked)
		if (!empty($post_data['delete'])) {
			foreach ($post_data['delete'] as $userid => $on) {
				if ($cascade_changes) {
					$current = $GLOBALS['SQ_SYSTEM']->am->getPermission($owner->id, $perm, null, false, false, true);
					$current_access = $current[$userid];
					$permission_vars[$perm][] = Array('access'          => -1,
													  'userid'          => $userid,
													  'previous_access' => $current_access,
													  );
				} else {
					if (!$GLOBALS['SQ_SYSTEM']->am->deletePermission($owner->id, $userid, $perm)) return false;
				}
			}
		}

		if (!empty($_POST[$prefix.'_permissions']['manual_cascade'])) {
			$current = $GLOBALS['SQ_SYSTEM']->am->getPermission($owner->id, $perm, null, false, false, true);
			foreach ($current as $userid => $access) {
				$permission_vars[$perm][] = Array('access'          => $access,
												  'userid'          => $userid,
												  'previous_access' => null,
												  );
			}
		}

	}//end foreach perms

	if (!empty($permission_vars)) {
		$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$vars = Array('assetid' => $owner->id, 'permission_changes' => $permission_vars);
		$hh->queueHipo('hipo_job_edit_permissions', $vars);
	}

	return true;

}//end processPermissions()


?>
