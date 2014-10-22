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
* $Id: roles.inc,v 1.16 2012/08/30 01:09:20 ewang Exp $
*
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Roles Static Screen Functions
*
* @author  Nathan de Vries <ndvries@squiz.net>
* @version $Revision: 1.16 $
* @package MySource_Matrix
*/


/**
* Paints the interface for viewing current roles (used for popups etc)
*
* @param object	&$owner	the asset whose interface we are painting
* @param object	&$o		the outputter class
* @param object	&$ei	the edit interface
*
* @return boolean
* @access public
*/
function paintInlineRoles(&$owner, &$o, &$ei)
{
	$prefix = $owner->getPrefix();

	$roles = $GLOBALS['SQ_SYSTEM']->am->getRole($owner->id);

	if (empty($roles)) return FALSE;

	$o->openSection(translate('Roles'));

	$o->openField('&nbsp;');
	?>
	<table class="sq-backend-table">
		<td class="sq-backend-table-header"><?php echo translate('Roles'); ?></td>

		<td class="sq-backend-table-header"><?php echo translate('Users'); ?></td>

		<td class="sq-backend-table-header"><?php echo translate('Global?'); ?></td>

		<?php
		foreach ($roles as $roleid => $userids) {
			$global_role = FALSE;
			if (in_array(0, $userids)) {
				$userids = array_diff($userids, Array(0));
				$global_role = TRUE;
			}

			$userid_count = count($userids);
			?>
			<tr>
				<td class="sq-backend-table-cell-alt" rowspan="<?php echo $userid_count; ?>">
					<?php echo get_asset_tag_line($roleid); ?>
				</td>
				<td class="sq-backend-table-cell-alt">
					<?php
					if (!empty($userids)) {
						echo get_asset_tag_line(array_shift($userids));
					}
					?>
				</td>
				<td class="sq-backend-table-cell-alt" rowspan="<?php echo $userid_count; ?>">
					<img src="<?php echo sq_web_path('lib'); ?>/web/images/<?php echo ($global_role) ? 'tick' : 'cross'; ?>.gif" width="15" height="15" />
				</td>
			</tr>
			<?php
			foreach ($userids as $userid) {
				?>
				<tr>
					<td class="sq-backend-table-cell-alt">
						<?php echo get_asset_tag_line($userid); ?>
					</td>
				</tr>
				<?php
			}
		}//end foreach
		?>
		</table><?php
		$o->closeField();
	$o->closeSection();

	return TRUE;

}//end paintInlineRoles()


/**
* Paint the interface for managing roles
*
* @param object		&$owner		the asset whose interface we are painting
* @param object		&$o			the outputter class
* @param object		&$ei		the edit interface
* @param array		$keywords	the keywords to replace
*
* @return boolean
* @access public
*/
function paintRoles(&$owner, &$o, &$ei, $keywords=NULL)
{
	$root = $ei->findScreen('static_screen_roles');
	if (is_null($root)) return FALSE;

	$prefix = $owner->getPrefix();
	$admin_access = $owner->adminAccess('roles');

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

		$o->openSection(translate(array_get_index($section->attributes(), 'display_name', $section->attributes()->name)));

		if (!($num_fields = count($section->children()))) {
			continue;
		}

		if (SQ_CONF_ENABLE_ROLES_PERM_SYSTEM  == '0' && SQ_CONF_ENABLE_ROLES_WF_SYSTEM == '0') {
			$o->openField('');
			ob_start();
			echo translate('The Roles System is disabled');

			ob_end_flush();
			$o->closeField();
			$o->closeSection();

			return $admin_access;
		}

		foreach ($section->field as $field) {

			$field_access = $ei->_getAccess($owner, $field, $prefix);
			if (!($field_access & SQ_EI_READ)) {
				continue;
			}

			$field_name = '';
			if (!empty($field->display_name)) {
				$field_name = (string) $field->display_name;
			}

			$o->openField(translate($field_name));
			if (!is_null($keywords) && isset($keywords['fields'][strtolower($field->attributes()->code)])) {
					ob_start();
			}
				switch (strtolower($field->attributes()->code)) {
					case 'roles_current' :
						$roles = $GLOBALS['SQ_SYSTEM']->am->getRole($owner->id);
						if (empty($roles)) {
							echo translate('No roles have been assigned');

						} else {
							?>
							<table class="sq-backend-table">
							<td class="sq-backend-table-header"><?php echo translate('Roles'); ?></td>

							<td class="sq-backend-table-header"><?php echo translate('Users'); ?></td>

							<?php
							if (SQ_CONF_ENABLE_GLOBAL_ROLES == '1') {
							?>
							<td class="sq-backend-table-header"><?php echo translate('Global?'); ?></td>

							<?php
							}

							foreach ($roles as $roleid => $ids) {
								$userids = Array();
								foreach ($ids as $row => $userid) {
									if ($GLOBALS['SQ_SYSTEM']->am->getAssetInfo($userid, Array('user', 'user_group'), FALSE) || $userid == 0) {
										$userids[] = $userid;
									}
								}
								$global_role = FALSE;
								if (in_array(0, $userids)) {
									$userids = array_diff($userids, Array(0));
									$global_role = TRUE;
								}
								if ($admin_access) {
									?>
									<tr>
										<td class="sq-backend-table-cell-alt">
											<?php echo asset_finder($prefix.'_role_'.$roleid, $roleid, Array('role' => 'I')); ?>
										</td>
										<td class="sq-backend-table-cell-alt">
											<?php echo multiple_asset_finder($prefix.'_userids_'.$roleid, $userids, Array('user' => 'D', 'user_group' => 'D')); ?>
										</td>
									<?php
									if (SQ_CONF_ENABLE_GLOBAL_ROLES == '1') {
									?>
										<td class="sq-backend-table-cell-alt">
											<?php echo check_box($prefix.'_globally_assign_'.$roleid, 1, $global_role); ?>
										</td>
									<?php
									}
									?>
									</tr>
									<?php
								} else {
									$userid_count = count($userids);
									?>
									<tr>
										<td class="sq-backend-table-cell-alt" rowspan="<?php echo $userid_count; ?>">
											<?php echo get_asset_tag_line($roleid); ?>
										</td>
										<td class="sq-backend-table-cell-alt">
											<?php
											if (!empty($userids)) {
												echo get_asset_tag_line(array_shift($userids));
											}
											?>
										</td>
									<?php
									if (SQ_CONF_ENABLE_GLOBAL_ROLES == '1') {
									?>
										<td class="sq-backend-table-cell-alt" rowspan="<?php echo $userid_count; ?>">
											<img src="<?php echo sq_web_path('lib'); ?>/web/images/<?php echo ($global_role) ? 'tick' : 'cross'; ?>.gif" width="15" height="15" />
										</td>
									<?php
									}
									?>
									</tr>
									<?php
									foreach ($userids as $userid) {
										?>
										<tr>
											<td class="sq-backend-table-cell-alt">
												<?php echo get_asset_tag_line($userid); ?>
											</td>
										</tr>
										<?php
									}
								}
							}//end foreach
							?>
							</table>
							<?php
						}//end else
					break;
					case 'roles_add' :
						echo '<p>'.translate('Select a role').'<br />';

						echo asset_finder($prefix.'_role_new', 0, Array('role' => 'I')).'</p>';
						echo '<p>'.translate('Select the users who will perform the role on this asset').'<br />';

						echo multiple_asset_finder($prefix.'_userids_new', Array(), Array('user' => 'D', 'user_group' => 'D')).'</p>';
						if (SQ_CONF_ENABLE_GLOBAL_ROLES == '1') {
							echo check_box($prefix.'_globally_assign_new').translate('Allow globally assigned users to perform this role?');

						}
					break;
				}//end switch

				if (!empty($field->note)) {
					$o->note(translate((string) $field->note));
				}

			if (!is_null($keywords) && isset($keywords['fields'][strtolower((string)$field->attributes()->code)])) {
				$ei->_tmp['layout_keyword_replacements'][$keywords['fields'][strtolower($field->attributes()->code)]] = ob_get_contents();
				ob_end_flush();
			}
			$o->closeField();

		}//end for

		$o->closeSection();

		if ($admin_access) {
			$kiddies = $GLOBALS['SQ_SYSTEM']->am->getChildren($owner->id);
			if (count($kiddies)) {
				$o->openSection(translate('Cascade Changes to Role Assignments?'));

					$o->openField(translate('Note'));

						$o->note(sprintf(translate('If you choose to cascade the role assignment changes you make for <strong>"%1$s"</strong>, the changes will also be made on all assets that fall under <strong>"%1$s"</strong> in the various hierarchies in which it exists. This will affect <strong>%2$s</strong> other assets.'), htmlspecialchars($owner->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET), count($kiddies)));

					$o->closeField();
					$o->openField('&nbsp;');
						check_box($prefix.'_cascade_roles', TRUE, TRUE);
						label(translate('Yes, cascade role assignment changes'), $prefix.'_roles[cascade_changes]');

					$o->closeField();
				$o->closeSection();

				$o->openSection(translate('Manually Cascade Changes to Role Assignments?'));

					$o->openField(translate('Note'));

						$o->note(sprintf(translate('Manually cascading role assignments will try to set the current roles of "%1$s" on all assets that fall under "%1$s" in the various hierarchies in which it exists. This will affect %2$s other assets.<br/><i>Any additional roles set on affected assets will <b>not</b> be removed during a manual cascade. Any roles set on affected assets that conflict with ones set on "%1$s" will also <b>not</b> be changed.</i><br/><b>NB. If you select to manually cascade, any role changes you make will also cascade even if you do not select them to. Use the option on its own for more predictable results.</b>'), htmlspecialchars($owner->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET), count($kiddies)));

					$o->closeField();
					$o->openField('&nbsp;');
						check_box($prefix.'_manually_cascade_roles', TRUE);
						label(translate('Yes, manually cascade role assignment changes'), $prefix.'_roles[manual_cascade]');

					$o->closeField();
				$o->closeSection();
			}
		}

		if (!is_null($keywords) && isset($keywords['sections'][strtolower(str_replace(' ', '_', $section->attributes()->name))])) {
			$ei->_tmp['layout_keyword_replacements'][$keywords['sections'][strtolower(str_replace(' ', '_', $section->attributes()->name))]] = ob_get_contents();
			ob_end_flush();
		}

	}//end for

	return $admin_access;

}//end paintRoles()


/**
* Process the the roles on a particular asset
*
* @param object	&$owner	the asset whose interface we are painting
* @param object	&$o		the outputter class
* @param object	&$ei	the edit interface
*
* @return boolean
* @access public
*/
function processRoles(&$owner, &$o, &$ei)
{
	if (!$owner->adminAccess('roles')) return FALSE;
	$roles = $GLOBALS['SQ_SYSTEM']->am->getRole($owner->id);
	$current_roles = Array();
	foreach ($roles as $roleid => $ids) {
		foreach ($ids as $row => $userid) {
			$current_roles[$roleid][] = $userid;
		}
	}
	$new_roles = array_get_index($_REQUEST, $owner->getPrefix().'_roles', Array());

	$prefix = $owner->getPrefix();

	$role_vars = Array();
	foreach ($current_roles as $roleid => $userids) {
		$currently_global = FALSE;
		if (in_array(0, $userids)) {
			$userids = array_diff($userids, Array(0));
			$currently_global = TRUE;
		}

		$tmp = array_get_index($_REQUEST, $prefix.'_role_'.$roleid, Array());
		$roleid_new = array_get_index($tmp, 'assetid', 0);
		$tmp = array_get_index($_REQUEST, $prefix.'_userids_'.$roleid, Array());

		$userids_new = Array();
		$userids_keep = Array();

		foreach ($tmp as $tmp_values) {
			if ($tmp_values['assetid']) {
				if (!in_array($tmp_values['assetid'], $userids)) {
					$userids_new[] = $tmp_values['assetid'];
				} else {
					$userids_keep[] = $tmp_values['assetid'];
				}
			}
		}

		if (!$roleid_new) {
			$role_vars['delete'][$roleid] = $userids;
		} else if ($roleid_new != $roleid) {
			$role_vars['delete'][$roleid] = $userids;
			$role_vars['add'][$roleid_new] = array_merge($userids_new, $userids_keep);
		} else {
			$roles_diff = array_diff($userids, $userids_keep);
			if (!empty($roles_diff)) {
				$role_vars['delete'][$roleid] = $roles_diff;
			}
			if (!empty($userids_new)) {
				$role_vars['add'][$roleid] = $userids_new;
			}
		}

		$globally_assigned = array_get_index($_REQUEST, $prefix.'_globally_assign_'.$roleid, FALSE);
		if (!$globally_assigned && $currently_global) {
			$role_vars['delete'][$roleid][] = 0;
		} else if ($globally_assigned && !$currently_global) {
			$role_vars['add'][$roleid][] = 0;
		}
	}//end foreach

	$roleid_new = array_get_index($_REQUEST, $prefix.'_role_new', FALSE);
	$globally_assign_new = array_get_index($_REQUEST, $prefix.'_globally_assign_new', FALSE);
	if ($roleid_new['assetid']) {
		$userids_new = array_get_index($_REQUEST, $prefix.'_userids_new', Array());
		foreach ($userids_new as $userid_new) {
			if ($userid_new['assetid']) {
				$role_vars['add'][$roleid_new['assetid']][] = $userid_new['assetid'];
			}
		}
		if ($globally_assign_new) {
			$role_vars['add'][$roleid_new['assetid']][] = 0;
		}
	}

	$cascade = array_get_index($_REQUEST, $prefix.'_cascade_roles', FALSE);
	$manually_cascade = array_get_index($_REQUEST, $prefix.'_manually_cascade_roles', FALSE);

	if ($manually_cascade) {
		foreach ($current_roles as $roleid => $userids) {
			if (isset($role_vars['add'][$roleid])) {
				$role_vars['add'][$roleid] += $userids;
			} else {
				$role_vars['add'][$roleid] = $userids;
			}
		}
	}

	if (empty($role_vars['add']) && empty($role_vars['delete']) && !$manually_cascade) {
		return;
	}

	if (!empty($role_vars)) {
		$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$vars = Array(
					'assets'		=> Array(
										$owner->id	=> Array(
														'type_code'	=> $owner->type(),
													   ),
									   ),
					'role_changes'	=> $role_vars,
				);

		if (!$cascade) $vars['dependants_only'] = 1;

		$hh->queueHipo('hipo_job_edit_roles', $vars);
	}

	return TRUE;

}//end processRoles()


?>
