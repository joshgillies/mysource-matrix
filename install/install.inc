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
* $Id: install.inc,v 1.21 2004/11/04 00:31:58 mnyeholt Exp $
* $Name: not supported by cvs2svn $
*/

/**
* Install functions
*
* Purpose
* Functions used to install and upgrade the system
*
* @version $version$ - 1.0
* @package MySource_Matrix
* @subpackage install
*/


/**
* Regenerate all the configs used by Matrix
* 
* @return boolean	indicating success or not of all configs saved
* @access public
*/
function regenerate_configs()
{
	// Re-generate the System Config to make sure that we get any new defines that may have been issued
	require_once SQ_INCLUDE_PATH.'/system_config.inc';
	$cfg = new System_Config();
	if (!$cfg->save(Array(), false)) return false;
	
	// Re-generate the External Tools Config to make sure that we get any new defines that may have been issued
	require_once SQ_INCLUDE_PATH.'/external_tools_config.inc';
	$cfg = new External_Tools_Config();
	if (!$cfg->save(Array(), false)) return false;
	
	// Re-generate the Server Config to make sure that we get any new defines that may have been issued
	require_once SQ_SYSTEM_ROOT.'/core/server/squiz_server_config.inc';
	$squiz_server_cfg = new Squiz_Server_Config();
	if (!$squiz_server_cfg->save(Array(), false)) return false;
	
	// Re-generate the HIPO Config to make sure that we get any new defines that may have been issued
	require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_config.inc';
	$hipo_cfg = new HIPO_Config();
	if (!$hipo_cfg->save(Array(), false)) return false;
	
	// Re-generate the Messaging Service Config to make sure that we get any new defines that may have been issued
	require_once SQ_SYSTEM_ROOT.'/core/include/messaging_service_config.inc';
	$ms_cfg = new Messaging_Service_Config();
	if (!$ms_cfg->save(Array(), false)) return false;

	return true;

}//end regenerate_configs()


/**
* Uninstall any assets that no longer have a .inc file 
* 
* @return boolean
* @access public
*/
function uninstall_asset_types()
{
	$db = &$GLOBALS['SQ_SYSTEM']->db;

	// always contains the full list of asset type_codes currently registered in the database 
	$all_asset_types = $GLOBALS['SQ_SYSTEM']->am->getTypeList();

	// contains full list (yes we are getting this twice), but will be diff'd with
	// $package_asset_types leaving what type_codes can be safely deleted
	$asset_types = $GLOBALS['SQ_SYSTEM']->am->getTypeList();

	// type_codes of assets which are being claimed by a package
	$package_asset_types = Array();

	// find which packages are installed
	$packages_installed = $GLOBALS['SQ_SYSTEM']->getInstalledPackages();
	
	if (!empty($packages_installed)) {
		foreach($packages_installed as $package_array) {
			$package = $package_array['code_name'];
	
			// core package is something else
			if ($package == '__core__') {
				require_once SQ_CORE_PACKAGE_PATH.'/package_manager_core.inc';
				$pm = new Package_Manager_Core();
				$package_asset_types = array_merge($package_asset_types, array_keys($pm->assets));
			} else {
				// only process a package if the PM file exists for it, because we need it
				$package_path = SQ_PACKAGES_PATH.'/'.$package.'/package_manager_'.$package.'.inc';
				if (file_exists($package_path)) {
					require_once $package_path;
					$class = 'package_manager_'.$package;
					$pm = new $class();
					$package_asset_types = array_merge($package_asset_types, array_keys($pm->assets));
				}
			}
		}
	}

	// What's left after the diff are assets which no package thinks it owns, and can
	// safely be deleted
	$asset_types = array_diff($asset_types, $package_asset_types);

	// Asset types are deleted when no package manager thinks they have the asset type.
	// They think they have the asset type when the asset.xml says so
	if (!empty($asset_types)) {
		$errors = false;

		// check those asset types that are still going to be there after uninstall
		// to see whether it depended on something that is being deleted... if it is then
		// we have massive problems and should bail
		foreach(array_diff($all_asset_types,$asset_types) as $asset_type) {
			$asc = $GLOBALS['SQ_SYSTEM']->am->getTypeAncestors($asset_type);
			$deleted_parents = array_intersect($asset_types, $asc);
			
			// are our parents going to be binned?
			if (count($deleted_parents) > 0) {
				trigger_error('One or more of the parents for the asset type \''.$asset_type.'\' no longer exists in the system. The system may be broken until you restore the necessary files of the parent asset type to their proper location.'."\n".'\''.$asset_type.'\' depends on: '.implode(', ', $deleted_parents), E_USER_WARNING);
				$errors = true;
			}
		}
	
		// failed dependency tests => we're stuffed; bail
		if ($errors) {
			trigger_error('Asset hierarchy would be corrupted, uninstall not committed', E_USER_WARNING);
			exit(1);
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// now actually delete the types
		foreach($asset_types as $asset_type) {
			// delete the types
			$sql = 'DELETE FROM sq_asset_type WHERE type_code = '.$db->quote($asset_type);
	
			$result = $db->query($sql);
			if (DB::isError($result)) {
				trigger_error('SQL error while deleting asset type \''.$asset_type.'\': ' .$result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				exit(1);
			}
	
			// remove the inherited types
			$sql = 'DELETE FROM sq_asset_type_inherited
					WHERE type_code = '.$db->quote($asset_type).'
					  OR inherited_type_code = '.$db->quote($asset_type);
	
			$result = $db->query($sql);
			if (DB::isError($result)) {
				trigger_error('SQL error while deleting asset type \''.$asset_type.'\': ' .$result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				exit(1);
			}
	
			// remove the attributes
			$sql = 'DELETE FROM sq_asset_attribute WHERE type_code = '.$db->quote($asset_type);
	
			$result = $db->query($sql);
			if (DB::isError($result)) {
				trigger_error('SQL error while deleting asset type \''.$asset_type.'\': ' .$result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				exit(1);
			}
		}
	
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
	
		// report if we did uninstall some asset types
		if (!empty($types_to_delete)) {
			$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
			$message_body = 'UNINSTALLED the following asset types as they are no longer recognised by any package:'."\n".implode("\n", $types_to_delete);
			$message = $ms->newMessage(Array(), 'Install', $message_body, 'install.system');
			$message->send();
		}
	
	}//end if asset types not empty
	
	return true;

}//end uninstall_asset_types()


/**
* Uninstall any packages that don't have a package manager defined for them
* 
* @return boolean
* @access public
*/
function uninstall_packages()
{
	$db = &$GLOBALS['SQ_SYSTEM']->db;
	
	// If the package manager doesn't exist for a package, it does not exist.
	// Assumption: packages live in the packages directory (Core never gets uninstalled).
	$packages_installed = $GLOBALS['SQ_SYSTEM']->getInstalledPackages();
	
	if (!empty($packages_installed)) {
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
	
		foreach($packages_installed as $package_array) {
			$package = $package_array['code_name'];
	
			// never delete the core package!
			if ($package == '__core__') continue;
	
			// package manager should exist for the package to exist
			$dir = 'packages/'.$package;
			$exists = file_exists(SQ_SYSTEM_ROOT.'/'.$dir.'/package_manager_'.$package.'.inc');

			if (!$exists) {
				// folder or the package manager does not exist, safe to delete
				$sql = 'DELETE FROM sq_package WHERE code_name = '.$db->quote($package);
				$result = $db->query($sql);
				if (DB::isError($result)) {
					trigger_error('SQL error while deleting package "'.$package.'": ' .$result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					exit(1);
				}
	
				// remove its asset map file from the data directory
				if (!unlink(SQ_DATA_PATH.'/private/asset_map/'.$package.'.xml')) {
					trigger_error('Could not delete the asset map file for "'.$package.'"', E_USER_WARNING);
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					exit(1);
				}
	
				$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
				$message_body = 'Uninstalled '.$package;
				$message = $ms->newMessage(Array(), 'Install', $message_body, 'install.system');
				$message->send();

			}//end if exists

		}//end foreach installed package

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

	}//end if installed packages not empty
	
	return true;
	
}//end update_package_locations()


/**
* Install core assets
* 
* @param Array $packageList 		Lists what packages are going to be installed
*									and what assets in those packages.
* @return boolean
* @access public
*/
function install_core($package_list=Array())
{
	// If there's some packages defined in the array, make sure the Core 
	// is one of them, otherwise break out.
	
	$assets = Array();
	
	if (count($package_list) > 0) {
		if (!isset($package_list['core'])) {
			// Make sure to include the system asset config, otherwise other
			// packages that add system assets can get all messed up because 
			// the system assets haven't been defined.
			require_once SQ_INCLUDE_PATH.'/system_asset_config.inc';
			$sys_asset_cfg = new System_Asset_Config();
			
			if (file_exists($sys_asset_cfg->config_file)) {
				require $sys_asset_cfg->config_file;
			
				$GLOBALS['SQ_SYSTEM_ASSETS'] = $system_assets;
			
			} else {
				$GLOBALS['SQ_SYSTEM_ASSETS'] = Array();
			}
			return true;
		}
		else {
			$assets = $package_list['core'];
			// If it wasn't an array at the core index, just make it empty
			if (!is_array($assets)) {
				$assets = Array();
			}
		}
	}
	require_once SQ_CORE_PACKAGE_PATH.'/package_manager_core.inc';
	$pm = new Package_Manager_Core();
	$result = $pm->updatePackageDetails($assets);
	
	$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
	$message_body = 'Core Package '.(($result) ? 'done successfully' : 'FAILED');
	$message = $ms->newMessage(Array(), 'Install', $message_body, 'install.system');
	$message->send();
	
	if (!$result) exit(1);
	
	// Firstly let's create some Assets that we require to run
	require_once SQ_INCLUDE_PATH.'/system_asset_config.inc';
	$sys_asset_cfg = new System_Asset_Config();
	
	if (file_exists($sys_asset_cfg->config_file)) {
		require $sys_asset_cfg->config_file;
	
		$GLOBALS['SQ_SYSTEM_ASSETS'] = $system_assets;
	
	} else {
		$GLOBALS['SQ_SYSTEM_ASSETS'] = Array();
	}
	
	$result = $pm->installSystemAssets();
	
	// 0 (zero) indicates success, but no system assets were created - suppress in this case
	if ($result != 0) {
		$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
		$message_body = 'Core System Asset '.(($result == -1) ? 'FAILED' : (': '.$result.' new assets created'));
		$message = $ms->newMessage(Array(), 'Install', $message_body, 'install.system');
		$message->send();
		
	}
	if ($result == -1) exit(1);
	
	// set the current user object to the root user so we can finish
	// the install process without permission denied errors
	$GLOBALS['SQ_SYSTEM']->setCurrentUser($GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user'));
	
	return true;
	
}//end install_core()


/**
* Install all the packages into the system. Uses the $packages(Assets) arrays
* if defined to determine which assets to install. If a $package is defined, 
* then it checks to see if the list of assets is defined for that package. If
* not, will install ALL assets.
* 
* @param Array $packageList	An array of assets inside packages to install.
*								Requires that the package name be defined in
*								the $packages array.
								Array('package_name' => Array('assetname'),)
* @return boolean
* @access public
*/
function install_packages($package_list=Array())
{
	// right now that we have sorted all that out we can install the packages
	$d = dir(SQ_PACKAGES_PATH);
	
	$deferred_packages = Array();
	
	// Do we only install named packages?
	$limit_packages = count($package_list);
	while (false !== ($entry = $d->read())) {
		if ($entry == '.' || $entry == '..') continue;
		// if this is a directory, process it
		if ($entry != 'CVS' && is_dir(SQ_PACKAGES_PATH.'/'.$entry)) {
			// If the package isn't in the packages array, then just skip it.
			$assets = Array();
			if ($limit_packages) {
				if (!isset($package_list[$entry])) {
					echo "\nSkipping package $entry \n";
					continue;
				}
				$assets = $package_list[$entry];
				// If it wasn't an array at the core index, just quit.
				if (!is_array($assets)) {
					$assets = Array();
				}
			}
			
			$result = install_package($entry, $assets);
			if (is_array($result)) {
				$deferred_packages[$entry] = $result;
			}
		}
	}
	$d->close();
	
	return count($deferred_packages) ? $deferred_packages : true;
	
}//end install_packages()


/**
* Installs a single package into the system.
* 
* @param Array $packageAssets	An array of asset types to install for this 
*								package.
* @return mixed					void on success, otherwise an array of type_codes
* 								of assets that couldn't be installed yet.
* @access public
*/
function install_package($package, $package_assets=Array())
{
	require_once SQ_PACKAGES_PATH.'/'.$package.'/package_manager_'.$package.'.inc';
	$class = 'package_manager_'.$package;
	$pm = new $class();
	$result = $pm->updatePackageDetails($package_assets);
	
	$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
	
	if (is_array($result)) {
		$message_body = strtoupper($package)." PACKAGE DEFERRED FOR DEPENDENCY RESOLUTION";
		$message = $ms->newMessage(Array(), 'Install', $message_body, 'install.system');
		$message->send();
		return $result;
	}
	$message_body = strtoupper($package)." PACKAGE ".(($result) ? "DONE SUCCESSFULLY" : "FAILED");
	$message = $ms->newMessage(Array(), 'Install', $message_body, 'install.system');
	$message->send();
	
	if (!$result) exit(1);
	$result = $pm->installSystemAssets();
	if ($result != 0) {	// 0 indicates success, but no system assets were created - suppress in this case
		$message_body = strtoupper($package)." SYSTEM ASSET CREATION ".(($result == -1) ? "FAILED" : (": ".$result." NEW ASSETS CREATED"));
		$message = $ms->newMessage(Array(), 'Install', $message_body, 'install.system');
		$message->send();
	}
	if ($result == -1) exit(1);
	unset($pm);
	return true;
}//end install_package()


/**
* Install authentication types
* 
* @return boolean
* @access public
*/
function install_authentication_types()
{
	// get all the authentication types that are currently installed
	$auth_types = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants('authentication');
	
	// get installed authentication systems
	$auth_folder = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('authentication_folder');
	$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($auth_folder->id, SQ_LINK_TYPE_1, 'authentication', false);
	$installed_auth_types = Array();
	foreach ($links as $link_data) $installed_auth_types[] = $link_data['minor_type_code'];
	
	// install all systems that are not currently installed
	$folder_link = Array('asset' => &$auth_folder, 'link_type' => SQ_LINK_TYPE_1, 'exclusive' => 1);
	foreach ($auth_types as $type_code) {
		if (in_array($type_code, $installed_auth_types)) continue;
		$GLOBALS['SQ_SYSTEM']->am->includeAsset($type_code);
		$auth = new $type_code();
	
		if (!$auth->create($folder_link)) {
			trigger_error('AUTHENTICATION TYPE "'.strtoupper($type_code).'" NOT CREATED', E_USER_WARNING);
		} else {
			$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
			$message_body = 'AUTHENTICATION TYPE "'.strtoupper($type_code).'" CREATED: '.$auth->id;
			$message = $ms->newMessage(Array(), 'Install', $message_body, 'install.system');
			$message->send();

		}
	}

	return true;

}//end install_authentication_types()


/**
* Generate the global preferences list
* 
* @return boolean
* @access public
*/
function generate_global_preferences()
{
	// we need to install any event listeners here, now that we have installed all the asset types.
	$packages = $GLOBALS['SQ_SYSTEM']->getInstalledPackages();

	$preferences = Array();
	if (is_file(SQ_DATA_PATH.'/private/conf/preferences.inc')) include SQ_DATA_PATH.'/private/conf/preferences.inc';
	
	foreach ($packages as $package) {
		// slight change for the core package
		if ($package['code_name'] == '__core__') {
			require_once SQ_CORE_PACKAGE_PATH.'/package_manager_core.inc';
			$class = 'package_manager_core';
		} else {
			require_once SQ_PACKAGES_PATH.'/'.$package['code_name'].'/package_manager_'.$package['code_name'].'.inc';
			$class = 'package_manager_'.$package['code_name'];
		}
	
		$pm = new $class();
		$pm->installUserPreferences($preferences);
		unset($pm);
	}
	$str = '<'.'?php $preferences = '.var_export($preferences, true).'; ?'.'>';
	if (!string_to_file($str, SQ_DATA_PATH.'/private/conf/preferences.inc')) return false;
	
	$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
	$message_body = 'Global Preferences Done';
	$message = $ms->newMessage(Array(), 'Install', $message_body, 'install.system');
	$message->send();
	
	return true;
	
}//end generate_global_preferences()


/**
* Install event listeners
* 
* @return boolean
* @access public
*/
function install_event_listeners()
{
	// we need to install any event listeners here, now that we have installed all the asset types.
	$packages = $GLOBALS['SQ_SYSTEM']->getInstalledPackages();
	
	foreach ($packages as $package) {
		// slight change for the core package
		if ($package['code_name'] == '__core__') {
			require_once SQ_CORE_PACKAGE_PATH.'/package_manager_core.inc';
			$class = 'package_manager_core';
		} else {
			require_once SQ_PACKAGES_PATH.'/'.$package['code_name'].'/package_manager_'.$package['code_name'].'.inc';
			$class = 'package_manager_'.$package['code_name'];
		}
	
		$pm = new $class();
		$pm->installEventListeners();
		unset($pm);
	}
	$em = &$GLOBALS['SQ_SYSTEM']->getEventManager();
	$em->writeStaticEventsCacheFile();
	
	$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
	$message_body = 'Event Listeners Done';
	$message = $ms->newMessage(Array(), 'Install', $message_body, 'install.system');
	$message->send();
	
	return true;
	
}//end install_event_listeners()


?>