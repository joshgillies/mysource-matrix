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
* $Id: user_group_edit_fns.inc,v 1.13 2004/04/19 01:40:13 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_CORE_PACKAGE_PATH.'/folder/folder_edit_fns.inc';

/**
* User_Group_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
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
		$this->static_screens['details']['lock_type'] = 'attributes';

	}//end constructor


	/**
	* Get an array of the current security restrictions for the user group
	*
	* @param object User_Group	&$asset	the user group asset
	*
	* @return Array()
	* @access public
	*/
	function getCurrentRestrictions(&$asset)
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = 'SELECT type_code, screen, section
				FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_editing_access ';
		$where = 'userid = '.$db->quote($asset->id);
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);

		$result = $db->getAssoc($sql.$where, false, Array(), DB_FETCHMODE_ASSOC, true);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
		}

		return $result;

	}//end getCurrentRestrictions()


	/**
	* Prints the interface for granting unrestricted access
	*
	* @param object User_Group			&$asset	the user group asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintUnrestricted(&$asset, &$o, $prefix)
	{
		$current = $this->getCurrentRestrictions($asset);

		if (!$asset->adminAccess('security')) {
			if (!$this->isRestricted($asset)) echo 'Unrestricted access has been granted. No security restrictions apply on this user group.';
			else echo 'Unrestricted access has not been granted. Security restrictions do apply on this user group.';
			return false;
		} else {
			check_box($prefix.'_unrestricted', '1', !$this->isRestricted($asset));
			echo 'Grant Unrestricted Access';
			echo '<br /><br />Check this box to give this user group unrestricted access to all assets';
		}

		return true;

	}//end paintUnrestricted()


	/**
	* Processes the interface for granting unrestricted access
	*
	* @param object User_Group			&$asset	the user group asset
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processUnrestricted(&$asset, &$o, $prefix)
	{
		$unrestricted = !$this->isRestricted($asset);
		if (isset($_POST[$prefix.'_unrestricted']) && $_POST[$prefix.'_unrestricted'] && !$unrestricted) {
			$values = Array('userid'    => $asset->id,
							'type_code' => '0',
							'screen'    => '0',
							'section'   => '0',
							);
			if (!$GLOBALS['SQ_SYSTEM']->rollbackInsert('asset_editing_access', $values)) {
				return false;
			}
		} else if (!isset($_POST[$prefix.'_unrestricted']) && $unrestricted) {
			$db = &$GLOBALS['SQ_SYSTEM']->db;
			$where_cond = ' userid          = '.$db->quote($asset->id).'
							  AND type_code = '.$db->quote('0').'
							  AND screen    = '.$db->quote('0').'
							  AND section   = '.$db->quote('0');
			if (!$GLOBALS['SQ_SYSTEM']->rollbackDelete('asset_editing_access', $where_cond)) {
				return false;
			}
		}

		return true;

	}//end processUnrestricted()


	/**
	* Determine if any security restrictions apply to this user group
	*
	* This function will only return FALSE if access has been granted for all screens
	* on all asset types (the global unrestricted flag)
	*
	* @param object User_Group	&$asset	the user group asset
	*
	* @return boolean
	* @access public
	*/
	function isRestricted(&$asset)
	{
		$current = $this->getCurrentRestrictions($asset);
		return !isset($current['0']['0']);

	}//end isRestricted()


	/**
	* Prints the interface for editing type restrictions
	*
	* @param object User_Group			&$asset	the user group asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintTypeRestrictions(&$asset, &$o, $prefix)
	{
		$current = $this->getCurrentRestrictions($asset);
		$admin_access = $asset->adminAccess('security');

		foreach ($current as $type_code => $type_data) {
			if ($type_code == '0') continue;

			$o->openSection($type_code);
				$o->openField('&nbsp;');
					?>
					<table class="sq-backend-table">
						<tr>
							<td class="sq-backend-table-header" style="width: 75%;">Screen Name</td>
							<?php if ($admin_access) { ?>
							<td class="sq-backend-table-header" style="width: 25%;">Remove?</td>
							<?php } ?>
						</tr>
					<?php
					$granted_screens = Array();
					foreach ($type_data as $screen_data) {
						$granted_screens[] = $screen_data['screen'];
						if ($screen_data['screen'] == '0') $screen_name = '<b>All Screens</b>';
						else $screen_name = ucwords(str_replace('_', '', $screen_data['screen']));
						?>
						<tr>
							<td class="sq-backend-table-cell"><?php echo $screen_name; ?></td>
							<?php if ($admin_access) { ?>
							<td class="sq-backend-table-cell">
								<?php check_box($prefix.'_remove_type_screen['.$type_code.']['.$screen_data['screen'].']'); ?>
							</td>
							<?php } ?>
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
							$asset =& new $type_code();
							$ef = &$asset->getEditFns();

							$screens = Array(' ' => ''); // first option is blank
							$screens['0'] = '-- All Screens --'; // second option is 'All Screens'
							foreach (array_keys($ef->static_screens) as $screen) {
								if (in_array($screen, $granted_screens)) continue;
								$screens[$screen] = ucfirst($screen);
							}
							foreach (array_keys($ei->getScreens()) as $screen) {
								if (in_array($screen, $granted_screens)) continue;
								$screens[$screen] = ucfirst($screen);
							}

							echo '<br />Grant access for screen ';
							combo_box($prefix.'_add_type_screen['.$type_code.']', $screens, false, Array());
						} else {
							echo '<br />You have granted access to all screens so you cannot grant access to any more. Remove the <b>\'All Screens\'</b> option to futher customise this asset type.';
							if (count($current[$type_code]) == 1) echo '<br />If you delete the <b>\'All Screens\'</b> option, the <b>\'Details\'</b> screen will be added as a default. You can choose to remove this later.';
						}
					}

				$o->closeField();
			$o->closeSection();
		}

		// allow them to restrict access to asset types
		if ($admin_access) {
			$uc_types = Array();
			$types = $GLOBALS['SQ_SYSTEM']->am->getTypeList();
			asort($types);
			foreach ($types as $type) {
				if (isset($current[$type])) continue;
				$uc_types[$type] = ucwords(str_replace('_',' ', $type));
			}

			$o->openSection('Grant Access to New Type');
				$o->openField('&nbsp;');
					combo_box($prefix.'_restrict_types', $uc_types, true, '', 6);
				$o->closeField();
			$o->closeSection();
		}

		return $admin_access;

	}//end paintTypeRestrictions()


	/**
	* Processes the interface for editing type restrictions
	*
	* @param object User_Group			&$asset	the user group asset
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processTypeRestrictions(&$asset, &$o, $prefix)
	{
		$current = $this->getCurrentRestrictions($asset);

		  //////////////////////////////////////
		 //  ADD ACCESS FOR NEW ASSET TYPES  //
		//////////////////////////////////////
		if (isset($_POST[$prefix.'_restrict_types'])) {
			$new_types = $_POST[$prefix.'_restrict_types'];

			foreach ($new_types as $type) {
				if (isset($current[$type])) continue;
				$values = Array('userid'    => $asset->id,
								'type_code' => $type,
								'screen'    => 'details',
								'section'   => '0',
								);
				if (!$GLOBALS['SQ_SYSTEM']->rollbackInsert('asset_editing_access', $values)) {
					return false;
				}
			}
		}//end if


		  ////////////////////////////////
		 //  REMOVE ACCESS TO SCREENS  //
		////////////////////////////////
		if (isset($_POST[$prefix.'_remove_type_screen'])) {
			$db = &$GLOBALS['SQ_SYSTEM']->db;
			$doomed_screens = $_POST[$prefix.'_remove_type_screen'];
			foreach ($doomed_screens as $type_code => $screen_data) {
				foreach ($screen_data as $doomed_screen => $on) {
					if ($doomed_screen == '0' && count($current[$type_code]) == 1) {
						// deleting the All Screens option without having a fall back
						// option to stop this whole type being deleted
						// so we'll add one
						$values = Array('userid'    => $asset->id,
										'type_code' => $type_code,
										'screen'    => 'details',
										'section'   => '0',
										);
						if (!$GLOBALS['SQ_SYSTEM']->rollbackInsert('asset_editing_access', $values)) {
							return false;
						}
					}
					// note that we need to put quote around the doomed screen to ensure
					// it is treated like a string and thus does not blow away all entries
					// for the type code from the DB
					$where_cond = ' userid          = '.$db->quote($asset->id).'
									  AND type_code = '.$db->quote($type_code).'
									  AND screen    = '.$db->quote("$doomed_screen").'
									  AND section   = '.$db->quote('0');
					if (!$GLOBALS['SQ_SYSTEM']->rollbackDelete('asset_editing_access', $where_cond)) {
						return false;
					}
				}
			}
		}//end if


		  ///////////////////////////////
		 //  GRANT ACCESS TO SCREENS  //
		///////////////////////////////
		if (isset($_POST[$prefix.'_add_type_screen'])) {
			$new_screens = $_POST[$prefix.'_add_type_screen'];
			foreach ($new_screens as $type_code => $new_screen) {
				if (trim($new_screen) == '') continue;
				$values = Array('userid'    => $asset->id,
								'type_code' => $type_code,
								'screen'    => $new_screen,
								'section'   => '0',
								);
				if (!$GLOBALS['SQ_SYSTEM']->rollbackInsert('asset_editing_access', $values)) {
					return false;
				}
			}
		}//end if

	}//end processTypeRestrictions()


	/**
	* Prints the interface for customising prefenences
	*
	* @param object User_Group			&$asset	the user group asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintPreferences(&$asset, &$o, $prefix)
	{
		$write_access = $asset->adminAccess('preferences');

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
				$path = SQ_SYSTEM_ROOT.'/'.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code, 'dir').'/'.$type_code.'_prefs.inc';
				require_once $path;
				$pref_name = $type_code.'_Prefs';
				$prefs = new $pref_name($pref_file_path);
				foreach ($pref_vars as $var_name => $var_data) {
					$o->openField($var_data['name']);
						if (isset($custom_preferences[$type_code][$var_name])) {
							echo '<p>This preference is currently customised. The global preference setting is currently being overridden for users in this user group.';
							if ($write_access) {
								echo '<br />';
								check_box($prefix.'_revert['.$type_code.']['.$var_name.']');
								echo ' <b>Revert to global preference</b>';
							}
							echo '</p>';
							$prefs->paintBackend($o, $write_access, $var_name);
						} else {
							echo '<p>This preference is currently not customised. The global preference setting is currently being used for this user group.';
							if ($write_access) {
								echo '<br />';
								check_box($prefix.'_customise['.$type_code.']['.$var_name.']');
								echo ' <b>Customise this preference</b>';
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
	* @param object User_Group			&$asset	the user group asset
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processPreferences(&$asset, &$o, $prefix)
	{
		if (!$asset->adminAccess('preferences')) return false;

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
						if ($prefs->processBackend($o, true, $var_name)) {
							$new_preferences[$type_code][$var_name] = $prefs->pref_vars[$var_name];
						}
					}
				}
			}
		}

		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		$str = '<'.'?php $preferences = '.var_export($new_preferences, true).'; ?'.'>';
		if (!is_dir($asset->data_path) && !create_directory($asset->data_path)) return false;
		if (!string_to_file($str, $asset->data_path.'/.preferences.inc')) return false;

		return true;

	}//end processPreferences()


}//end class

?>