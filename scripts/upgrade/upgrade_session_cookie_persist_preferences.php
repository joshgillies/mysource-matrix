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
*/

/**
* Upgrade the session expiry preference as per SquizMap #6898
*
* This script will basically add default values for new session expiry components
* which are persist, 'expiry' and 'refresh_threshold'. The the script will also create an
* backup copy the prefernce file just in case we want to rollback the preference changes.
*
* Usage: php upgrade_session_cookie_persist_preferences.php [MATRIX_ROOT]
*/


error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';

if (empty($SYSTEM_ROOT)) {
	echo "ERROR: You need to supply the path to the System Root as the first argument\n";
	exit(1);
}

if (!is_dir($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT.'/core/include/init.inc')) {
	echo "ERROR: Path provided doesn't point to a Matrix installation's System Root. Please provide correct path and try again.\n";
	exit(1);
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';

$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');

// log in as root
if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	echo "ERROR: Failed logging in as root user\n";
	exit(1);
}

$am = $GLOBALS['SQ_SYSTEM']->am;
$count = 0;

// Upgrade the global preferences
$global_prefs_file = SQ_DATA_PATH.'/private/conf/preferences.inc';
if (is_file($global_prefs_file)) {
	echo 'Updating global session expiry references ... ';
	if (!_upgrade($global_prefs_file)) $count++;
}

// Upgrade the group preferences
$groupids = $am->getTypeAssetids('user_group');
foreach ($groupids as $groupid) {
	$asset = $am->getAsset($groupid);
	echo 'Updating session expiry references for '.$asset->name.' (#'.$asset->id.')... ';
	$current_prefs_file = $asset->data_path.'/.preferences.inc';
	if (is_file($current_prefs_file)) {
		if (!_upgrade($current_prefs_file)) {
			$count++;
		}
	} else {
		echo "Not required updating\n";
	}
}//end foreach

// Warn the user if an error happened
if (!empty($count)) {
	echo "There were $count errors in upgrading the preferences.\nThese files may need to be reviewed manually.\n";
}//end if

echo "\n";
echo "Finished updating the session expiry global/user preferences.\n";
exit();


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
	include_once($prefs_file);

	$require_updating = FALSE;
	if (isset($preferences['user']['SQ_USER_SESSION_PREFS']['default']) && is_array($preferences['user']['SQ_USER_SESSION_PREFS']['default'])) {
		foreach($preferences['user']['SQ_USER_SESSION_PREFS']['default'] as $index => $pref) {
			if (is_array($pref) && !isset($pref['expiry'])) {
				$pref['persist'] = 0;
				$pref['expiry'] = '2592000';
				$pref['refresh_threshold'] = '86400';
				$preferences['user']['SQ_USER_SESSION_PREFS']['default'][$index] = $pref;

				$require_updating = TRUE;
			} else if (isset($pref['timeout']) && !isset($pref['expiry'])) {
				// Single pref array
				$pref['persist'] = 0;
				$pref['expiry'] = '2592000';
				$pref['refresh_threshold'] = '86400';
				$preferences['user']['SQ_USER_SESSION_PREFS']['default'] = $pref;
			}
		}//end foreach

		// Save the new preferences
		if ($require_updating) {
			if (!_savePreferences($preferences, $prefs_file)) {
				echo "Error saving file\n";
				return FALSE;
			}
		}
	}//end if

	// Cleanup
	unset($preferences);

	// Finish
	echo $require_updating  ? "Done" : "Already updated";
	echo "\n";

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
