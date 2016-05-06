<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: user_edit_fns.inc,v 1.39 2012/08/30 01:09:17 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* User_Edit_Fns
*
* Purpose
*	  Editing interfaces for the (standard) user asset - one that can log in
*	  but has no backend editing privileges. This also defines the baseline
*	  interface for all user assets in the system (whether they be standard,
*	  backend, system admin, etc).
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.39 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class User_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function User_Edit_Fns()
	{
		$this->Asset_Edit_Fns();

	}//end constructor


	/**
	* Paints interface for entering password
	*
	* @param object	&$asset	the user asset
	* @param object	&$o		the backend outputter object
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintPasswordNote(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess('attributes')) {
			$o->note(translate('Enter password once, then again to confirm.').'<br />'.$asset->getPasswordRulesDescription());

			return TRUE;
		}
		return FALSE;

	}//end paintPasswordNote()


	/**
	* Returns a list of groups that the user is linked to (in getAssetInfo() format)
	*
	* @param object	&$asset	the user asset
	*
	* @return array
	* @access private
	*/
	function _getLinkedGroups(&$asset)
	{
		$group_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, 1,'user_group', FALSE, 'minor');

		$group_assetids = Array();
		foreach ($group_links as $group_link) {
			$group_assetids[] = $group_link['majorid'];
		}

		$groups_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($group_assetids);

		return $groups_info;

	}//end _getLinkedGroups()


	/**
	* Paints interface for managing user restrictions
	*
	* @param object	&$asset	the user asset
	* @param object	&$o		the backend outputter object
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function paintUserConditions(&$asset, &$o, $prefix)
	{
		// get a list of directly linked groups
		$groups = $this->_getLinkedGroups($asset);

		if (empty($groups)) {
			$o->openField('');
				echo translate('This user is not a member of any user groups; it cannot be restricted');

			$o->closeField();
			return FALSE;
		}

		$admin_access = $asset->adminAccess('attributes');

		if ($admin_access) {
			$conditions = Array('' => '-- Select Type --') + $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($GLOBALS['SQ_SYSTEM']->am->getTypeDescendants('condition'), 'name');
			asort($conditions);
		}

		// get a list of user restrictions
		$restrictions = $asset->attr('restrictions');

		foreach ($groups as $group_assetid => $group) {
			$o->openSection($group['name'].' (#'.$group_assetid.')');
			$o->openField('');

			// if user has restrictions related to this group
			if (!array_get_index($restrictions, $group_assetid, FALSE)) {
				echo sprintf(translate('There are no membership conditions set for "%s". This user is always a member of this group.'), $group['name']);

			} else {
				?>
				This user is part of this group when...
				<table class="sq-backend-table" border="1">
					<tr>
						<th>&nbsp;&nbsp;&nbsp;</th>
						<th>Condition</th>
					<?php
					if ($admin_access) {
						?>
						<th>Delete</th>
						<?php
					}
					?>
					</tr>
				<?php
				foreach ($restrictions[$group_assetid] as $key => $restriction) {
					$condition_prefix = $prefix.'_condition_'.$group_assetid.'_'.$key;
					?>
					<tr>
						<td><?php echo $key; ?></td>
						<td>
							<?php
							$edit_fns_classname = $restriction['name'].'_edit_fns';
							$GLOBALS['SQ_SYSTEM']->am->includeAsset($restriction['name'], TRUE);

							$match_options = call_user_func(Array($restriction['name'], 'getMatchKeywords'), $restriction['name']);
							if ($admin_access) {
								combo_box($condition_prefix.'_match', $match_options, FALSE, $restriction['match']);
							} else {
								echo $match_options[$restriction['match']];
							}
							echo '<br />';
							call_user_func(Array($edit_fns_classname, 'paintEditInterface'), $restriction['condition_data'], $o, $condition_prefix, $admin_access, Array());
							?>
						</td>
					<?php
					if ($admin_access) {
						?>
						<td><?php check_box($condition_prefix.'[delete]', '1'); ?></td>
						<?php
					}
					?>
					</tr>
					<?php
				}//end foreach restriction
				?>
				</table>
				<?php
			}//end else - membership conditions

			if ($admin_access) {
				?>
				<p>Add new condition: <?php combo_box($prefix.'_'.$group_assetid.'_new_condition', $conditions, FALSE, ''); ?></p>
				<?php
			}

			$o->closeField();
			$o->closeSection();

		}//end foreach $groups

		// field closed by asset_edit_functions

		return TRUE;

	}//end paintUserConditions()


	/**
	* Processses user restrictions
	*
	* @param object	&$asset	the user asset
	* @param object	&$o		the backend outputter object
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function processUserConditions(&$asset, &$o, $prefix)
	{
		// get a list of directly linked groups
		$groups = $this->_getLinkedGroups($asset);
		$admin_access = $asset->adminAccess('attributes');
		$restrictions = $asset->attr('restrictions');

		// -- PROCESS CONDITIONS --//
		foreach ($groups as $group_assetid => $group) {

			// if user has restrictions related to this group
			if (array_get_index($restrictions, $group_assetid, NULL)) {

				// get array of restrictions for this group
				foreach ($restrictions[$group_assetid] as $key => $restriction) {
					$condition_prefix = $prefix.'_condition_'.$group_assetid.'_'.$key;

					// if this condition has been marked to delete, unset it now
					if (isset($_POST[$condition_prefix]) && array_get_index($_POST[$condition_prefix], 'delete', 0)) {
						unset($restrictions[$group_assetid][$key]);
					} else {
						// prepare condition data
						$dir = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($restriction['name'], 'dir');
						$edit_fns_classname = $restriction['name'].'_edit_fns';
						require_once SQ_SYSTEM_ROOT.'/'.$dir.'/'.$edit_fns_classname.'.inc';
						$condition_data = call_user_func(Array($edit_fns_classname, 'processEditInterface'), $o, $condition_prefix);
						$restrictions[$group_assetid][$key]['condition_data'] = $condition_data;
						$restrictions[$group_assetid][$key]['match'] = array_get_index($_POST, $condition_prefix.'_match', '1');
					}
				}
			}
			if ($new_name = array_get_index($_POST, $prefix.'_'.$group_assetid.'_new_condition')) {
				$restrictions[$group_assetid][] = Array('name' => $new_name, 'match' => 1, 'condition_data' => Array());
			}
		}

		return $asset->setAttrValue('restrictions', $restrictions);

	}//end processUserConditions()


	/**
	* Interface painting function for locale settings
	*
	* @param object	&$asset	the user asset
	* @param object	&$o		the backend outputter object
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintLocale(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		require SQ_FUDGE_PATH.'/standards_lists/locales.inc';
		$current_locale = $asset->attr('locale');

		if ($write_access) {
			$standards_lists_locales = array_reverse($standards_lists_locales);
			$standards_lists_locales[''] = '** Use System Default **';
			$standards_lists_locales = array_reverse($standards_lists_locales);
			combo_box($prefix.'_locale', $standards_lists_locales, FALSE, $current_locale);
		} else {
			if (isset($standards_lists_locales[$current_locale])) {
				echo $standards_lists_locales[$current_locale];
			} else if ($current_locale == '') {
				// add system default to this message when it is determined
				echo sprintf(translate('<i>No locale is set for this user - system default "%s" used</i>'), $standards_lists_locales[SQ_CONF_DEFAULT_BACKEND_LOCALE]);

			} else {
				echo sprintf(translate('<i>Unknown locale "%s" set</i>'), $current_locale);

			}
		}

		return $write_access;

	}//end paintLocale()


	/**
	* Processes the interface for locale settings
	*
	* @param object	&$asset	the user asset
	* @param object	&$o		the backend outputter object
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processLocale(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;
		$asset->setAttrValue('locale', $_POST[$prefix.'_locale']);
		return $asset->saveAttributes();

	}//end processLocale()


	/**
	* Paints the interface re-assigning roles to a user
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
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('user_group');
		$user_group = new User_Group();
		$user_group_edit_fns = $user_group->getEditFns();

		return $user_group_edit_fns->paintRoles($asset, $o, $prefix);

	}//end paintRoles()


	/**
	* Processes the interface re-assigning roles to a user
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
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('user_group');
		$user_group = new User_Group();
		$user_group_edit_fns = $user_group->getEditFns();

		return $user_group_edit_fns->processRoles($asset, $o, $prefix);

	}//end processRoles()


	/**
	* Paints the current Locks held by the user
	*
	* @param object	&$asset	the user asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintLockDetails(&$asset, &$o, $prefix)
	{
		$current_locks = $asset->getUserLocks();

		if ($current_locks === FALSE) {
			$o->openField(translate('Current Locks'));

				echo 'No ability to show active locks in current lock method';
			$o->closeField();
			return FALSE;
		}

		$num_locks = count($current_locks);
		$write_access = $asset->writeAccess('attributes');
		$o->openField(translate('Current Locks'));

			echo sprintf(translate('%s currently holds %d Locks.'), htmlspecialchars($asset->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET), $num_locks);

			?>
			<script type="text/javascript">
			function sq_user_held_locks_info()
			{
				var userLockInfo = document.getElementById('sq_user_held_locks_details');
				var userLockToggle = document.getElementById('sq_user_held_locks_toggle');
				if (userLockInfo.style.display == 'none') {
					userLockInfo.style.display = 'block';
					userLockToggle.innerHTML = '<?php echo translate('Hide User Held Lock Details'); ?>';

				} else {
					userLockInfo.style.display = 'none';
					userLockToggle.innerHTML = '<?php echo translate('Show User Held Lock Details'); ?>';

				}
			}
			</script>
			<?php
			// Restrict to the top 100 other the page will take forever to load up
			$restricted_locks = array_slice($current_locks, 0, 100);
			echo '<br /><b class="clickable" onclick="sq_user_held_locks_info();" id="sq_user_held_locks_toggle">'.translate('Show User Held Lock Details').'</b><br />';

			echo '<div id="sq_user_held_locks_details" style="display:none;">';
			echo sprintf(translate('Showing %1$s out of %2$s locks'), count($restricted_locks), $num_locks);

			echo '<table class="sq-backend-table">';
			echo '<tr>';
			echo '<td class="sq-backend-table-header">Asset</td>';
			echo '<td class="sq-backend-table-header">Screen</td>';
			echo '</tr>';
			foreach ($restricted_locks as $current_lock) {
				if (isset($current_lock['lockid'])) {
					$shown = FALSE;
					if (strpos($current_lock['lockid'], 'asset') === 0) {
						$asset_details = explode('.', $current_lock['lockid']);
						if (isset($asset_details[1]) && isset($asset_details[2])) {
							$locked_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($asset_details[1], '', TRUE);
							if (!is_null($locked_asset)) {
								echo '<tr><td>';
								echo get_asset_tag_line($locked_asset->id);
								echo '</td><td>';
								echo ucwords($asset_details[2]);
								echo '</td></tr>';
								$shown = TRUE;
								$GLOBALS['SQ_SYSTEM']->am->forgetAsset($locked_asset);
							}//end if
						}//end if
					}//end if

					if (!$shown) {
						$unknown_lock_id = ucwords(str_replace('_', ' ', $current_lock['lockid']));
						echo '<tr>';
						echo '<td colspan="2">'.$unknown_lock_id.'</td>';
						echo '</tr>';
					}//end if
				}//end if
			}//end foreach
			echo '</table></div>';

		$o->closeField();
		$o->openField(translate('Release Held Locks'));

			if (!$write_access) {
				?>
				<script type="text/javascript">

					// Confirms that this is what the user wants to do
					function onReleaseAllClick() {
						if (confirm('Are you sure you want to release all user locks?')) {
							set_hidden_field("process_form", "1");
							return true;
						} else {
							return false;
						}
					}
				</script>

				<?php
				// No lock, so we need to create our own commit button
				submit_button($prefix.'_release_all_locks', 'Release All Locks', 'return onReleaseAllClick();');
			} else {
				// We have the lock, so we can use the normal commit button. Just print a dropdown box
				$options = Array(
							'yes'	=> translate('Yes'),
							'no'	=> translate('No'),
						   );
				combo_box($prefix.'_release_all_locks_dropdown', $options, FALSE, 'no');
			}
			$o->note(translate('Releasing all locks will delete any editing changes that this user has not yet committed.'));

		$o->closeField();

		return FALSE;

	}//end paintLockDetails()


	/**
	* Process the current Locks held by the user
	*
	* @param object	&$asset	the user asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processLockDetails(&$asset, &$o, $prefix)
	{
		$admin_access = $asset->adminAccess();
		if (!$admin_access) return FALSE;

		if ((isset($_POST[$prefix.'_release_all_locks'])) || (isset($_POST[$prefix.'_release_all_locks_dropdown']) && ($_POST[$prefix.'_release_all_locks_dropdown'] == 'yes'))) {
			$asset->releaseUserLocks();
		}
		return TRUE;

	}//end processLockDetails()



	/**
	* Paints the Sugar integration details
	*
	* @param object	&$asset	the user asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintSugarDetails(&$asset, &$o, $prefix)
	{

		// root user or sysadmin user
		$can_edit = $asset->adminAccess('attributes') && ($GLOBALS['SQ_SYSTEM']->userRoot() || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());

		// do we have existing link?
		$bind_vars = Array(
						'matrix_userid'      => $asset->id
					    );
		$results = MatrixDAL::executeAll('sugar_package', 'getAllLinks', $bind_vars);
		if(!empty($results)) {
			foreach ($results as $index => $result) {
				$module = array_get_index($result, 'module', '');
				$sugar_id = array_get_index($result, 'sugar_record_id', '');

				$o->openField(sprintf(translate('Sugar %s ID'), htmlspecialchars($module, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET)));
					if($can_edit) {
						text_box($prefix.'_sugar_id_'.$module, $sugar_id, 40);
					}
					else {
						echo htmlspecialchars($sugar_id, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET);
					}

				$o->closeField();
			}
		}
		
		return FALSE;

	}//end paintSugarDetails()



	/**
	* Process the Sugar integration details
	*
	* @param object	&$asset	the user asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processSugarDetails(&$asset, &$o, $prefix)
	{

		// root user or sysadmin user
		$can_edit = $asset->adminAccess('attributes') && ($GLOBALS['SQ_SYSTEM']->userRoot() || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());


		// do we have existing link?
		$bind_vars = Array(
						'matrix_userid'      => $asset->id
					    );
		$results = MatrixDAL::executeAll('sugar_package', 'getAllLinks', $bind_vars);
		if(!empty($results)) {
			foreach ($results as $index => $result) {
				$module = array_get_index($result, 'module', '');
				$sugar_id = array_get_index($result, 'sugar_record_id', '');

				if(isset($_POST[$prefix.'_sugar_id_'.$module])) {
					$new_record_id = $_POST[$prefix.'_sugar_id_'.$module];
					// Update record id
					$sql = 'UPDATE
								sq_sugar_lnk
							SET
								sugar_record_id = :new_record_id
							WHERE
									sugar_record_id	=  :old_record_id
									AND	module	= :module';

					$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

					try {
						$query = MatrixDAL::preparePdoQuery($sql);
						MatrixDAL::bindValueToPdo($query, 'new_record_id', $new_record_id);
						MatrixDAL::bindValueToPdo($query, 'old_record_id', $sugar_id);
						MatrixDAL::bindValueToPdo($query, 'module', $module);
						MatrixDAL::execPdoQuery($query);
					} catch (Exception $e) {
						throw new Exception('Unable to update sugar record ID "'.$sugar_id.' to new ID '.$new_record_id.', due to database error: '.$e->getMessage());
					}

					$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
				}
			}
		}
		
		return TRUE;

	}//end processSugarDetails()


	/**
	* Is this user linked to a Sugar record
	*
	* @param object	&$asset	the user asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function isSugarRecord(&$asset, &$o)
	{

		// do we have existing link?
		$bind_vars = Array(
						'matrix_userid'      => $asset->id
					    );
		$results = MatrixDAL::executeAll('sugar_package', 'getAllLinks', $bind_vars);
		if(!empty($results)) {
			return TRUE;
		}		
		return FALSE;
	}//end isSugarRecord()



}//end class

?>
