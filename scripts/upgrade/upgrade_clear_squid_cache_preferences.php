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
* $Id: upgrade_clear_squid_cache_preferences.php,v 1.3 2012/08/30 01:04:53 ewang Exp $
*
*/

/**
* This script moves the tool_clear_squid_cache preference to External Tools Config file
* It removes tool_clear_squid_cache field for global and user group custom preferences.
* Squid cache settings should really be treated as an external tool config, not a user preference. 
* 
*
* @author Edison Wang <ewang@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix
*/
error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT)) {
	echo "ERROR: You need to supply the path to the System Root as the first argument\n";
	exit();
}

if (!is_dir($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT.'/core/include/init.inc')) {
	echo "ERROR: Path provided doesn't point to a Matrix installation's System Root. Please provide correct path and try again.\n";
	exit();
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';
// log in as root
$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	echo "ERROR: Failed logging in as root user\n";
	exit();
}

// forced run level
$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);

$am = $GLOBALS['SQ_SYSTEM']->am;
$count = 0;

echo 'Moving Squid Cache preferences to External Tools Config'."\n";

// Moving Squid Cache global preferences to External Tools Config
echo 'Moving global preferences... ';
$global_prefs_file = SQ_DATA_PATH.'/private/conf/preferences.inc';
if (!_movePreference($global_prefs_file)) $count++;


echo 'Updating global preferences... ';
if (!_upgrade($global_prefs_file)) $count++;

// Remove the Squid Cache group preferences
$groupids = $am->getTypeAssetids('user_group');
foreach ($groupids as $groupid) {
	$asset = $am->getAsset($groupid);
	echo 'Updating preferences for '.$asset->name.' ('.$asset->id.')... ';
	$current_prefs_file = $asset->data_path.'/.preferences.inc';
	if (!_upgrade($current_prefs_file)) $count++;
}//end foreach

// Warn the user if an error happened
if (!empty($count)) {
	echo "There were $count errors in upgrading the Squid Cache preferences.\nThese files may need to be reviewed manually.\n";
}//end if

$GLOBALS['SQ_SYSTEM']->restoreRunLevel();



/**
* Move the preference to external tool config
* 
* @param string	$prefs_file	The preferences file to upgrade
*
* @return boolean
* @access public
*/
function _movePreference($prefs_file)
{
	// Start the upgrade
	if (!file_exists($prefs_file)) {
		echo "Preference file not found\n";
		return FALSE;
	}//end if
	include_once($prefs_file);

	if (isset($preferences['tool_clear_squid_cache'])) {
		$vars['SQ_TOOL_SQUID_CACHE_HOSTNAMES'] = $preferences['tool_clear_squid_cache']['SQ_SQUID_CACHE_HOSTNAMES']['default'];
		$vars['SQ_TOOL_SQUID_CACHE_PATH'] = $preferences['tool_clear_squid_cache']['SQ_SQUID_CACHE_PATH']['default'];
		$vars['SQ_TOOL_SQUID_CACHE_PORT'] = $preferences['tool_clear_squid_cache']['SQ_SQUID_CACHE_PORT']['default'];
		$vars['SQ_TOOL_SQUID_CACHE_OPTION'] = $preferences['tool_clear_squid_cache']['SQ_SQUID_CACHE_OPTION']['default'];
		$vars['SQ_TOOL_SQUID_CACHE_SLASH'] = $preferences['tool_clear_squid_cache']['SQ_SQUID_CACHE_SLASH']['default'];
		$vars['SQ_TOOL_SQUID_URL_PORT'] = $preferences['tool_clear_squid_cache']['SQ_SQUID_URL_PORT']['default'];
		
	    
		require_once SQ_INCLUDE_PATH.'/external_tools_config.inc';
		$cfg = new External_Tools_Config();
		if (!$cfg->save($vars, FALSE, TRUE)) {
		    echo "Preference tool_clear_squid_cache can not be saved\n";
		    return FALSE;
		}
		
	}//end if
	else {
	    	echo "Preference not found\n";
		return TRUE;
	}

	// Cleanup
	unset($preferences);

	// Finish
	echo 'Done'."\n";
	return TRUE;

}//end _movePreference




/**
* Upgrade the passed preferences file
* Remove  tool_clear_squid_cache field, it shouldn't belong to preference system
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
		echo "Ignored\n";
		return TRUE;
	}//end if
	include($prefs_file);
	if (isset($preferences['tool_clear_squid_cache'])) {
	    unset($preferences['tool_clear_squid_cache']);	
	    // Save the new preferences
	    if (!_savePreferences($preferences, $prefs_file)) {
		    echo "Error saving file\n";
		    return FALSE;
	    }//end if
		
	}//end if
	else {
	    	echo "Ignored\n";
		return TRUE;
	}

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
