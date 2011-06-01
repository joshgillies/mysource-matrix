<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: upgrade_session_expiry_preferences.php,v 1.2 2009/01/13 22:45:07 bpearson Exp $
*
*/

/**
*
* @author Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix
*/
error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	trigger_error("You need to supply the path to the System Root as the first argument\n", E_USER_ERROR);
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';

// ask for the root password for the system
echo 'Enter the root password for "'.SQ_CONF_SYSTEM_NAME.'": ';
$root_password = rtrim(fgets(STDIN, 4094));

// check that the correct root password was entered
$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
if (!$root_user->comparePassword($root_password)) {
	trigger_error("The root password entered was incorrect\n", E_USER_ERROR);
}

// log in as root
if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	trigger_error("Failed logging in as root user\n", E_USER_ERROR);
}

$am = $GLOBALS['SQ_SYSTEM']->am;
$count = 0;

// Upgrade the global preferences
echo 'Updating global preferences ... ';
$global_prefs_file = SQ_DATA_PATH.'/private/conf/preferences.inc';
if (!_upgrade($global_prefs_file)) $count++;

// Upgrade the group preferences
$groupids = $am->getTypeAssetids('user_group');
foreach ($groupids as $groupid) {
	$asset = $am->getAsset($groupid);
	echo 'Updating preferences for '.$asset->name.' ('.$asset->id.')... ';
	$current_prefs_file = $asset->data_path.'/.preferences.inc';
	if (!_upgrade($current_prefs_file)) $count++;
}//end foreach

// Warn the user if an error happened
if (!empty($count)) {
	echo "There were $count errors in upgrading the preferences.\nThese files may need to be reviewed manually.\n";
}//end if


/**
* Upgrade the passed preferences file
*
* @param string	$prefs_file	The preferences file to upgrade
*
* @return boolean
* @access public
*/
function _upgrade($prefs_file)
{
	// Start the upgrade
	if (!file_exists($prefs_file)) {
		echo "No Preferences found\n";
		return FALSE;
	}//end if
	include_once($prefs_file);

	if (isset($preferences['user']['SQ_USER_SESSION_PREFS']['default'])) {
		$current = $preferences['user']['SQ_USER_SESSION_PREFS']['default'];
		$test = array_pop($preferences['user']['SQ_USER_SESSION_PREFS']['default']);
		if (!is_array($test)) {
			unset($preferences['user']['SQ_USER_SESSION_PREFS']['default']);
			$preferences['user']['SQ_USER_SESSION_PREFS']['default'] = Array($current);
			// Save the new preferences
			if (!_savePreferences($preferences, $prefs_file)) {
				echo "Error saving file\n";
				return FALSE;
			}//end if
		}//end if
	}//end if

	// Cleanup
	unset($preferences);

	// Finish
	echo "Done\n";
	return TRUE;

}//end _upgrade


/**
* Save the preferences file
*
* @param array	$preferences	The preferences to save
* @param string	$prefs_file		The preferences file to save
*
* @return boolean
* @access public
*/
function _savePreferences($preferences, $prefs_file)
{
	if (empty($preferences)) return FALSE;

	$str = '<'.'?php $preferences = '.var_export($preferences, TRUE).'; ?'.'>';

	// Backup the existing preference file
	if (file_exists($prefs_file)) {

		$i = 0;
		do {
			$i++;
			$old_version = $prefs_file.'.'.$i;
		} while (file_exists($old_version));

		if (!copy($prefs_file, $old_version)) {
			return FALSE;
		}

	}// endif

	require_once SQ_FUDGE_PATH.'/general/file_system.inc';
	// make sure the directory exists
	if (!create_directory(dirname($prefs_file))) {
		return FALSE;
	}

	// and save the file
	if (!string_to_file($str, $prefs_file)) {
		return FALSE;
	}

	// Otherwise a OK!
	return TRUE;

}//end _savePreferences()


?>
