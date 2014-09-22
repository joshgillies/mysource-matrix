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
* $Id: user_group_edit_fns.inc,v 1.50 2012/12/06 00:05:39 cupreti Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/folder/folder_edit_fns.inc';

/**
* User_Group_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.50 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class User_Group_Edit_Fns extends Folder_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function User_Group_Edit_Fns()
	{
		$this->Folder_Edit_Fns();

	}//end constructor


	/**
	* Get an array of the current security restrictions for the user group
	*
	* @param object	&$asset	the user group asset
	*
	* @return array
	* @access public
	*/
	function getCurrentRestrictions(&$asset)
	{
		$db = MatrixDAL::getDb();

		$sql = 'SELECT type_code, screen, section
				FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_edit_access ';
		$where = 'userid = :userid';
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);

		try {
			$query	= MatrixDAL::preparePdoQuery($sql.$where);
			MatrixDAL::bindValueToPdo($query, 'userid', $asset->id);

			$result	= MatrixDAL::executePdoGroupedAssoc($query);
		} catch (Exception $e) {
			throw new Exception('Unable to get current restriction information for userid #'.$asset->id.' due to the following database error:'.$e->getMessage());
		}//end try catch

		return $result;

	}//end getCurrentRestrictions()


	/**
	* Determine if any security restrictions apply to this user group
	*
	* This function will only return FALSE if access has been granted for all screens
	* on all asset types (the global unrestricted flag)
	*
	* @param object	&$asset	the user group asset
	* @param string	$prefix	the prefix for this asset
	*
	* @return boolean
	* @access public
	*/
	function isRestricted(&$asset, $prefix)
	{
		$current = $this->getCurrentRestrictions($asset);
		return !empty($current);

	}//end isRestricted()


	/**
	* Prints the interface for editing type restrictions
	*
	* @param object	&$asset	the user group asset
	* @param object	&$o		the backend outputter object
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintTypeRestrictions(&$asset, &$o, $prefix)
	{
		$current = $this->getCurrentRestrictions($asset);
		$admin_access = $asset->adminAccess('security');

		if (empty($current) && !$admin_access) {
			$o->openField('');
				$o->note(translate('Unrestricted access has been granted. No security restrictions apply on this user group.'));

			$o->closeField();
		}

		foreach ($current as $type_code => $type_data) {
			if ($type_code == '0') continue;

			$o->openSection($type_code);
				$o->openField('');
					?>
					<table class="sq-backend-table">
						<tr>
							<td class="sq-backend-table-header" style="width: 75%;"><?php echo translate('Screen Name'); ?></td>

							<?php if ($admin_access) {
							?><td class="sq-backend-table-header" style="width: 25%;"><?php echo translate('Remove?'); ?></td><?php

							}
						?></tr>
					<?php
					$granted_screens = Array();
					foreach ($type_data as $screen_data) {
						$granted_screens[] = $screen_data['screen'];
						if ($screen_data['screen'] == '0') {
							$screen_name = '<b>'.translate('All Screens').'</b>';

						} else {
							$screen_name = ucwords(str_replace('_', '', $screen_data['screen']));
						}
						?>
						<tr>
							<td class="sq-backend-table-cell"><?php echo $screen_name; ?></td>
							<?php if ($admin_access) {
								?><td class="sq-backend-table-cell">
								<?php check_box($prefix.'_remove_type_screen['.$type_code.']['.$screen_data['screen'].']'); ?>
							</td>
							<?php
							}
							?>
						</tr>
						<?php
					}
					?>
					</table>

					<?php
					if ($admin_access) {
						if (!in_array('0', $granted_screens)) {
							$ei = new Asset_Edit_Interface($type_code);

							$GLOBALS['SQ_SYSTEM']->am->includeAsset($type_code);
							$asset = new $type_code();
							$ef = $asset->getEditFns();

							$screens = Array(' ' => ''); // first option is blank
							$screens['0'] = '-- '.translate('All Screens').' --'; // second option is 'All Screens'

							foreach (array_keys($ef->static_screens) as $screen) {
								if (in_array($screen, $granted_screens)) continue;
								$screens[$screen] = ucfirst($screen);
							}
							foreach (array_keys($ei->getScreens()) as $screen) {
								if (in_array($screen, $granted_screens)) continue;
								$screens[$screen] = ucfirst($screen);
							}

							echo '<br />'.translate('Allow access to the following screen (any screen not specified will be restricted):').' ';

							combo_box($prefix.'_add_type_screen['.$type_code.']', $screens, FALSE, Array());
						} else {
							echo '<br />'.translate('You have granted access to all screens so you cannot grant access to any more. Remove the "All Screens" option to further customise this asset type');

							if (count($current[$type_code]) == 1) {
								echo '<br />'.translate('If you delete the "All Screens" option, the "Details" screen will be added as a default. You can choose to remove this later.');

							}
						}
					}

				$o->closeField();
			$o->closeSection();
		}//end foreach type code

		// allow them to restrict access to asset types
		if ($admin_access) {
			$uc_types = Array();
			$types = $GLOBALS['SQ_SYSTEM']->am->getTypeList();
			asort($types);
			foreach ($types as $type) {
				if (isset($current[$type])) continue;
				$uc_types[$type] = ucwords(str_replace('_',' ', $type));
			}

			$o->openSection(translate('Add new type code to restrict'));

				$o->openField('');
					combo_box($prefix.'_restrict_types', $uc_types, TRUE, '', 6);
				$o->closeField();
			$o->closeSection();
		}

		return $admin_access;

	}//end paintTypeRestrictions()


	/**
	* Processes the interface for editing type restrictions
	*
	* @param object	&$asset	the user group asset
	* @param object	&$o		the Backend Outputter object
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processTypeRestrictions(&$asset, &$o, $prefix)
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$db = MatrixDAL::getDb();
		$current = $this->getCurrentRestrictions($asset);

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$bind_vars	= Array (
							'userid'	=> $asset->id,
					  );

		  //////////////////////////////////////
		 //  ADD ACCESS FOR NEW ASSET TYPES  //
		//////////////////////////////////////
		if (isset($_POST[$prefix.'_restrict_types'])) {
			$new_types = $_POST[$prefix.'_restrict_types'];


			foreach ($new_types as $type) {
				if (isset($current[$type])) continue;

				$bind_vars['type_code']	= $type;
				$bind_vars['screen']	= 'details';

				try {
					$result = MatrixDAL::executeQuery('user_group', 'addScreenSectionRestrictionForUser', $bind_vars);
				} catch (Exception $e) {
					throw new Exception('Unable to add access for new asset type due to the following database error:'.$e->getMessage());
				}//end try catch
			}//end foreach

		}//end if


		  ////////////////////////////////
		 //  REMOVE ACCESS TO SCREENS  //
		////////////////////////////////
		if (isset($_POST[$prefix.'_remove_type_screen'])) {
			$doomed_screens = $_POST[$prefix.'_remove_type_screen'];
			foreach ($doomed_screens as $type_code => $screen_data) {
				// Here we set the type code for the bind variable array.
				$bind_vars['type_code']	= $type_code;
				$bind_vars['screen']	= 'details';

				foreach ($screen_data as $doomed_screen => $on) {

					if ($doomed_screen == '0' && count($current[$type_code]) == 1) {
						// deleting the All Screens option without having a fall back
						// option to stop this whole type being deleted
						// so we'll add one
						try {
							$result = MatrixDAL::executeQuery('user_group', 'addScreenSectionRestrictionForUser', $bind_vars);
						} catch (Exception $e) {
							throw new Exception('Unable to add access for userid #'.$asset->id.' due to the following database error:'.$e->getMessage());
						}//end try catch
					}
					// note that we need to put quote around the doomed screen to ensure
					// it is treated like a string and thus does not blow away all entries
					// for the type code from the DB

					$bind_vars['screen'] = $doomed_screen;

					try {
						$result = MatrixDAL::executeQuery('user_group', 'deleteAccessRestrictionForUser', $bind_vars);
					} catch (Exception $e) {
						throw new Exception('Unable to delete access for userid #'.$asset->id.' due to the following database error:'.$e->getMessage());
					}//end try catch

				}//end foreach doomed screen
			}//end foreach type code
		}//end if


		  ///////////////////////////////
		 //  GRANT ACCESS TO SCREENS  //
		///////////////////////////////
		if (isset($_POST[$prefix.'_add_type_screen'])) {
			$new_screens = $_POST[$prefix.'_add_type_screen'];

			foreach ($new_screens as $type_code => $new_screen) {
				if (trim($new_screen) == '') continue;
				// Here we set the type code for the bind variable array.
				$bind_vars['type_code']	= $type_code;
				$bind_vars['screen']	= $new_screen;

				try {
					$result = MatrixDAL::executeQuery('user_group', 'addScreenSectionRestrictionForUser', $bind_vars);
				} catch (Exception $e) {
					throw new Exception('Unable to add access for new asset type due to the following database error:'.$e->getMessage());
				}//end try catch

			}//end foreach
		}//end if

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return TRUE;

	}//end processTypeRestrictions()


	/**
	* Prints the interface for customising prefenences
	*
	* @param object	&$asset	the user group asset
	* @param object	&$o		the Backend Outputter object
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintPreferences(&$asset, &$o, $prefix)
	{
		$write_access = $asset->adminAccess('preferences');

		// Disable OPCache so that we can get the updated preferences files
		if (!empty($_POST['process_form']) && ini_get('opcache.enable')) {
			ini_set('opcache.enable', '0');
		}

		$pref_file_path = $asset->data_path.'/.preferences.inc';
		if (is_file($pref_file_path)) {
			include $pref_file_path;
			$custom_preferences = $preferences;
		} else {
			$custom_preferences = Array();
		}

		include SQ_DATA_PATH.'/private/conf/preferences.inc';

		foreach ($preferences as $type_code => $pref_vars) {
			$o->openSection($GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code, 'name').' Preferences');
				// load up the asset (for locale) and prefs to display
				$GLOBALS['SQ_SYSTEM']->am->includeAsset($type_code);
				$path = SQ_SYSTEM_ROOT.'/'.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code, 'dir').'/'.$type_code.'_prefs.inc';
				require_once $path;
				$pref_name = $type_code.'_Prefs';
				$prefs = new $pref_name($pref_file_path);

				foreach ($pref_vars as $var_name => $var_data) {
					if(!isset($var_data['name'])) continue;
					$o->openField(translate($var_data['name']));
						if (isset($custom_preferences[$type_code][$var_name])) {
							echo '<p>'.translate('This preference is currently customised. The global preference setting is currently being overridden for users in this user group.');

							if ($write_access) {
								echo '<br />';
								check_box($prefix.'_revert['.$type_code.']['.$var_name.']');
								echo ' <label for="'.$prefix.'_revert['.$type_code.']['.$var_name.']">'.translate('Revert to global preference').'</label>';

							}
							echo '</p>';
							$prefs->paintBackend($o, $write_access, $var_name);
						} else {
							echo '<p>'.translate('This preference is currently not customised. The global preference setting is currently being used for this user group.');

							if ($write_access) {
								echo '<br />';
								check_box($prefix.'_customise['.$type_code.']['.$var_name.']');
								echo ' <label for="'.$prefix.'_customise['.$type_code.']['.$var_name.']">'.translate('Customise this preference').'</label>';

							}
							echo '</p>';
						}
					$o->closeField();
				}
			$o->closeSection();
		}

		return $write_access;

	}//end paintPreferences()


	/**
	* Processes the interface for customising preferences
	*
	* @param object	&$asset	the user group asset
	* @param object	&$o		the Backend Outputter object
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processPreferences(&$asset, &$o, $prefix)
	{
		if (!$asset->adminAccess('preferences')) return FALSE;

		$new_preferences = Array();

		if (isset($_POST[$prefix.'_customise'])) {
			// customising global preferences
			include SQ_DATA_PATH.'/private/conf/preferences.inc';
			foreach ($_POST[$prefix.'_customise'] as $type_code => $var_data) {
				foreach ($var_data as $var_name => $on) {
					if (isset($preferences[$type_code][$var_name])) {
						$new_preferences[$type_code][$var_name] = $preferences[$type_code][$var_name];
					}
				}
			}
		}

		$pref_file_path = $asset->data_path.'/.preferences.inc';
		if (is_file($pref_file_path)) {
			include $pref_file_path;
			foreach ($preferences as $type_code => $pref_vars) {
				$path = SQ_SYSTEM_ROOT.'/'.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code, 'dir').'/'.$type_code.'_prefs.inc';
				require_once $path;
				$pref_name = $type_code.'_Prefs';
				$prefs = new $pref_name();
				foreach ($pref_vars as $var_name => $var_data) {
					if (!isset($_POST[$prefix.'_revert'][$type_code][$var_name])) {
						if ($prefs->processBackend($o, TRUE, $var_name)) {
							$new_preferences[$type_code][$var_name] = $prefs->pref_vars[$var_name];
						}
					}
				}
			}
		}

		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		$str = '<'.'?php $preferences = '.var_export($new_preferences, TRUE).'; ?'.'>';
		if (!is_dir($asset->data_path) && !create_directory($asset->data_path)) {
			return FALSE;
		}
		if (!string_to_file($str, $asset->data_path.'/.preferences.inc')) {
			return FALSE;
		}

		return TRUE;

	}//end processPreferences()


	/**
	* Paints a table with a list of roles
	*
	* @param string		$prefix			Prefix of the calling edit function
	* @param string		$pag_id			The id of the page
	* @param array		$roles			Roles to be printed
	* @param boolean	$write_access	True to show checkboxes for re-assigning
	* @param boolean	$include_userid	True to include an "Assigned To" column
	*
	* @return NULL
	* @access private
	*/
	function _printRoleTable($prefix, $pag_id, $roles, $write_access=FALSE, $include_userid=FALSE)
	{
		if (empty($roles)) return;

		?>
		<table class="sq-backend-table">
			<tr>
				<?php
					if ($include_userid) {
						?><th class="sq-backend-table-header"><?php echo translate('User/Group'); ?></td><?php

					}
				?>
				<th class="sq-backend-table-header"><?php echo translate('Role'); ?></td>

				<th class="sq-backend-table-header">Asset for which the role is to be performed</td>
				<?php
					if ($write_access) {
						?><th class="sq-backend-table-header"><?php echo translate('Reassign?'); ?></td><?php

					}
				?>
			</tr>
		<?php

		$num_roles = 0;
		$num_printed  = 0;
		$roles_pointer = 0;
		$num_per_page = 10;

		$start_printing_at = array_get_index($_REQUEST, $prefix.$pag_id.'_start_at', 0);
		$role_tag_lines = Array();
		$user_tag_lines = Array();
		$asset_tag_lines = Array();

		foreach ($roles as $roleid => $userids) {
			foreach ($userids as $userid => $assetids) {
				$num_roles += count($assetids);
				foreach ($assetids as $assetid) {
					if ($roles_pointer < $start_printing_at) {
						$roles_pointer++;
						continue;
					}
					if ($roles_pointer >= $start_printing_at + $num_per_page) {
						break 2;
					}

					if (!isset($asset_tag_lines[$assetid])) {
						$asset_tag_lines[$assetid] = get_asset_tag_line($assetid);
					}
					if (!isset($role_tag_lines[$roleid])) {
						$role_tag_lines[$roleid] = get_asset_tag_line($roleid);
					}

				?>
					<tr>
						<?php
							if ($include_userid) {
								if (!isset($user_tag_lines[$userid])) {
									$user_tag_lines[$userid] = get_asset_tag_line($userid);
								}
								?><td class="sq-backend-table-cell"><?php echo $user_tag_lines[$userid]; ?></td><?php
							}
						?>
						<td class="sq-backend-table-cell"><?php echo $role_tag_lines[$roleid] ?></td>
						<td class="sq-backend-table-cell"><?php echo get_asset_tag_line($assetid); ?></td>
						<?php
							if ($write_access) {
								?><td class="sq-backend-table-cell"><?php echo check_box($prefix.'_reassign[]', $assetid); ?></td><?php
							}
						?>
					</tr>
				<?php
					$roles_pointer++;
				}//end foreach
			}//end foreach
		}//end foreach

		if ($num_roles > $num_per_page) {
		?>
			<tr>
				<td colspan="3" align="center" class="sq-backend-table-header">
				<?php
				hidden_field($prefix.$pag_id.'_start_at', $start_printing_at);
				if ($start_printing_at > 0) {
					?><a style="text-decoration: none;" href="#" onClick="Javascript: set_hidden_field('<?php echo $prefix.$pag_id; ?>_start_at', '<?php echo $start_printing_at - $num_per_page; ?>'); set_hidden_field('process_form', '0'); submit_form(); return false;"><?php
				} else {
					?><span style="color: #84848F"><?php
				}
				?>
				&lt;&lt; <?php echo translate('Previous Page');


				if ($start_printing_at > 0) {
					echo '</a>';
				} else {
					echo '</span>';
				}
				?>
				&nbsp;<b><?php echo sprintf(translate('Page %1$s of %2$s'), (floor($start_printing_at / $num_per_page) + 1), ceil($num_roles / $num_per_page)); ?></b>&nbsp;


				<?php
				if (($start_printing_at + $num_per_page) < $num_roles) {
					?><a style="text-decoration: none;" href="#" onClick="Javascript: set_hidden_field('<?php echo $prefix.$pag_id; ?>_start_at', '<?php echo $start_printing_at + $num_per_page; ?>'); set_hidden_field('process_form', '0'); submit_form(); return false;"><?php
				} else {
					?><span style="color: #84848F"><?php
				}

				echo translate('Next Page'); ?> &gt;&gt;

				<?php
				if (($start_printing_at + $num_per_page) < $num_roles) {
					echo '</a>';
				} else {
					echo '</span>';
				}
				?>
				</td>
			</tr>
		<?php
		}//end if
		?>
		</table>
		<?php

	}//end _printRoleTable()


	/**
	* Paints the interface re-assigning roles to a user group
	*
	* @param object	&$asset	the user asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintRoles(&$asset, &$o, $prefix)
	{
		$write_access = $asset->adminAccess('roles');
		$string_code_suffix = ($asset instanceof User_Group) ? 'group' : 'user';

		$o->openSection(translate('role_directly_assigned_section_'.$string_code_suffix));
			$o->openRaw();

			if (SQ_CONF_ENABLE_ROLES_PERM_SYSTEM == '0' && SQ_CONF_ENABLE_ROLES_WF_SYSTEM == '0') {
				echo translate('The Roles System is disabled');

			} else {
				$roles = $GLOBALS['SQ_SYSTEM']->am->getRole(NULL, NULL, $asset->id, TRUE, FALSE, FALSE, FALSE);
				if (empty($roles)) {
					echo translate('No roles have been assigned');

				} else {
					$this->_printRoleTable($prefix, 'direct', $roles, $write_access);
					if ($write_access) {
						echo translate('Reassign selected role assignments to').': ';

						echo asset_finder($prefix.'_reassignee', 0, Array('user' => 'D', 'user_group' => 'D'));
					}
				}
			}

			$o->closeRaw();
		$o->closeSection();

		$group_descendants = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants('user_group', TRUE);
		$group_descendants = array_diff($group_descendants, Array('role'));

		$parents = $GLOBALS['SQ_SYSTEM']->am->getParents($asset->id);

		$roles = Array();
		$parent_groups = Array();
		foreach ($parents as $assetid => $type_code) {
			if (in_array($type_code, $group_descendants)) {
				$roles += $GLOBALS['SQ_SYSTEM']->am->getRole(NULL, NULL, $assetid, TRUE, FALSE, FALSE, FALSE);
				$normal_role = $GLOBALS['SQ_SYSTEM']->am->getRole(NULL, NULL, $assetid, TRUE, FALSE, FALSE, FALSE);
				foreach ($normal_role as $roleid => $userids) {
					foreach ($userids as $userid => $assetids) {
						foreach ($assetids as $assetid) {
							$roles[$roleid][$userid][] = $assetid;
							$roles[$roleid][$userid] = array_unique($roles[$roleid][$userid]);
						}
					}
				}
			} else {
				$global_role = $GLOBALS['SQ_SYSTEM']->am->getRole(NULL, $assetid, 0, TRUE, FALSE, FALSE, FALSE);
				foreach ($global_role as $roleid => $userids) {
					foreach ($userids[0] as $assetid) {
						$roles[$roleid][$roleid][] = $assetid;
						$roles[$roleid][$roleid] = array_unique($roles[$roleid][$roleid]);
					}
				}
			}
		}

		if (!empty($roles)) {
			$o->openSection(translate('role_indirectly_assigned_section_'.$string_code_suffix));
				$o->openRaw();
					$this->_printRoleTable($prefix, 'indirect', $roles, FALSE, TRUE);
				$o->closeRaw();
			$o->closeSection();
		}

		return $write_access;

	}//end paintRoles()


	/**
	* Processes the interface re-assigning roles to a user group
	*
	* @param object	&$asset	the user asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processRoles(&$asset, &$o, $prefix)
	{
		$reassignee = 0;
		if (isset($_REQUEST[$prefix.'_reassignee'])) {
			$reassignee = array_get_index($_REQUEST[$prefix.'_reassignee'], 'assetid', 0);
		}

		if (!$reassignee) return;

		$am = $GLOBALS['SQ_SYSTEM']->am;

		$roles = $GLOBALS['SQ_SYSTEM']->am->getRole(NULL, NULL, $asset->id, TRUE);
		$reassign_assetids = array_get_index($_REQUEST, $prefix.'_reassign', Array());

		$role_vars = Array();
		foreach ($roles as $roleid => $userids) {
			foreach ($userids as $userid => $assetids) {
					$changes = array_intersect($reassign_assetids, $assetids);
					if (!empty($changes)) {
							$role_vars['add'][$roleid][] = $reassignee;
							$role_vars['delete'][$roleid][] = $userid;
					}
			}
		}

		if (!empty($reassign_assetids)) {
			$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();

			$vars = Array(
						'assets'			=> $am->getAssetInfo($reassign_assetids, Array(), FALSE, 'type_code'),
						'role_changes'		=> $role_vars,
						'dependants_only'	=> TRUE,
					);

			$hh->queueHipo('hipo_job_edit_roles', $vars);
		}

		return TRUE;

	}//end processRoles()


}//end class

?>
