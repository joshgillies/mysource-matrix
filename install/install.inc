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
* $Id: install.inc,v 1.6 2004/06/28 17:38:13 brobertson Exp $
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
* @return void
* @access public
*/
function uninstall_asset_types()
{
	$db = &$GLOBALS['SQ_SYSTEM']->db;

	$asset_types = $GLOBALS['SQ_SYSTEM']->am->getTypeList();

	// asset types are deleted if the .inc file for them does not exist (eg. for Page asset it would be
	// page.inc) and no assets OR asset types currently depend on them in the system.
	if (!empty($asset_types)) {
		$types_to_delete = Array();
		$deleted_types = Array();
	
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
	
		$missing_files = false;
		foreach($asset_types as $asset_type) {
			$type_info = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($asset_type);
	
			// all three .inc files for the asset should exist
			$exists = 0;
	
			$dir = $type_info['dir'].'/'.$asset_type.'.inc';
			$exists += (file_exists(SQ_SYSTEM_ROOT.'/'.$dir) ? 1 : 0);
	
			$dir = $type_info['dir'].'/'.$asset_type.'_management.inc';
			$exists += (file_exists(SQ_SYSTEM_ROOT.'/'.$dir) ? 1 : 0);
			
			$dir = $type_info['dir'].'/'.$asset_type.'_edit_fns.inc';
			$exists += (file_exists(SQ_SYSTEM_ROOT.'/'.$dir) ? 1 : 0);
	
			if ($exists == 0) {	// does not exist
				// check how many assets are in the system that depend on this type
				$assetid_count = count($GLOBALS['SQ_SYSTEM']->am->getTypeAssetids($asset_type, false));
				if ($assetid_count > 0) {
					// file gone but there are still assets depending on it!
					trigger_error('The files for the asset type \''.$asset_type.'\' has been removed from its location ([SYSTEM_ROOT]/'.$type_info['dir'].'), but '.$assetid_count.' assets exist which depend on this type. The system may be broken until you restore the necessary files to this location', E_USER_WARNING);
					$missing_files = true;
	
				} else if (substr(SQ_SYSTEM_ROOT.'/'.$type_info['dir'], 0, strlen(SQ_CORE_PACKAGE_PATH)) == SQ_CORE_PACKAGE_PATH) {
					// we're not friggin' touching anything in the core -- if it's not there then we have
					// a problem!!
					trigger_error('Cannot uninstall asset type \''.$asset_type.'\' as it is in the core. The system may be broken until you restore this asset type\'s files to its original location.', E_USER_WARNING);
					$missing_files = true;
	
				} else {
					// safe to delete - save the type code so we can delete it later
					$types_to_delete[] = $asset_type;
				}
	
			} elseif ($exists < 3) {		// not all files are there!
				trigger_error('Not all required files for the asset type \''.$asset_type.'\' can be found in its location ([SYSTEM_ROOT]/'.$type_info['dir'].'). The system may be broken until you restore the necessary files to this location', E_USER_WARNING);
				$missing_files = true;
			}
	
		}
	
		// find out what we have left and check to see whether their parents are still there
		$asset_types = array_diff($asset_types, $types_to_delete);
	
		foreach($asset_types as $asset_type) {
			$asc = $GLOBALS['SQ_SYSTEM']->am->getTypeAncestors($asset_type);
			$deleted_parents = array_intersect($types_to_delete, $asc);
			if (count($deleted_parents) > 0) {
				// OMG OMG OMG, one of our parents is gone!
				trigger_error('One or more of the parents for the asset type \''.$asset_type.'\' no longer exists in the system. The system may be broken until you restore the necessary files of the parent asset type to their proper location.'."\n".'\''.$asset_type.'\' depends on: '.implode(', ', $deleted_parents), E_USER_WARNING);
				$missing_files = true;
			}
		}
	
		if ($missing_files) {	// we're screwed, proceed no further!
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			trigger_error('System integrity questionable, uninstall not committed', E_USER_WARNING);
			exit(1);
		}
	
		// now actually delete the types
		foreach($types_to_delete as $asset_type) {
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
			pre_echo('UNINSTALLED the following asset types as their files no longer exist:');
			pre_echo($types_to_delete);
		}
	
	}//end if asset types not empty
	
}//end uninstall_asset_types()


/**
* Uninstall any packages that don't have a package manager defined for them
* 
* @return void
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
	
				pre_echo(strtoupper($package).' PACKAGE UNINSTALLED');

			}//end if exists

		}//end foreach installed package

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

	}//end if installed packages not empty
	
}//end uninstall_packages()


/**
* Install core assets
* 
* @return void
* @access public
*/
function install_core()
{
	require_once SQ_CORE_PACKAGE_PATH.'/package_manager_core.inc';
	$pm = new Package_Manager_Core();
	$result = $pm->updatePackageDetails();
	pre_echo("CORE PACKAGE ".(($result) ? "DONE SUCCESSFULLY" : "FAILED"));
	if (!$result) exit(1);
	
	// Firstly let's create some Assets that we require to run
	require_once SQ_INCLUDE_PATH.'/system_asset_config.inc';
	$sys_asset_cfg = new System_Asset_Config();
	
	if (file_exists($sys_asset_cfg->config_file)) {
		require $sys_asset_cfg->config_file;
		print(file_get_contents($sys_asset_cfg->config_file));
	
		$GLOBALS['SQ_SYSTEM_ASSETS'] = $system_assets;
	
	} else {
		$GLOBALS['SQ_SYSTEM_ASSETS'] = Array();
	
	}
	
	$result = $pm->installSystemAssets();
	
	// 0 (zero) indicates success, but no system assets were created - suppress in this case
	if ($result != 0) {
		pre_echo("CORE SYSTEM ASSET CREATION ".(($result == -1) ? "FAILED" : (": ".$result." NEW ASSETS CREATED")));
	}
	if ($result == -1) exit(1);
	
	// set the current user object to the root user so we can finish
	// the install process without permission denied errors
	$GLOBALS['SQ_SYSTEM']->setCurrentUser($GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user'));
	
}//end install_core()


/**
* Install all the packages into the system
* 
* @return void
* @access public
*/
function install_packages()
{
	// right now that we have sorted all that out we can install the packages
	$d = dir(SQ_PACKAGES_PATH);
	while (false !== ($entry = $d->read())) {
		if ($entry == '.' || $entry == '..') continue;
		# if this is a directory, process it
		if ($entry != 'CVS' && is_dir(SQ_PACKAGES_PATH.'/'.$entry)) {
			require_once SQ_PACKAGES_PATH.'/'.$entry.'/package_manager_'.$entry.'.inc';
			$class = 'package_manager_'.$entry;
			$pm = new $class();
			$result = $pm->updatePackageDetails();
			pre_echo(strtoupper($entry)." PACKAGE ".(($result) ? "DONE SUCCESSFULLY" : "FAILED"));
			if (!$result) exit(1);
			$result = $pm->installSystemAssets();
			if ($result != 0) {	// 0 indicates success, but no system assets were created - suppress in this case
				pre_echo(strtoupper($entry)." SYSTEM ASSET CREATION ".(($result == -1) ? "FAILED" : (": ".$result." NEW ASSETS CREATED")));
			}
			if ($result == -1) exit(1);
			unset($pm);
		}
	}
	$d->close();
	
}//end install_packages()


/**
* Install authentication types
* 
* @return void
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
	$GLOBALS['SQ_INSTALL'] = true;
	foreach ($auth_types as $type_code) {
		if (in_array($type_code, $installed_auth_types)) continue;
		$GLOBALS['SQ_SYSTEM']->am->includeAsset($type_code);
		$auth = new $type_code();
	
		if (!$auth->create($folder_link)) {
			trigger_error('AUTHENTICATION TYPE "'.strtoupper($type_code).'" NOT CREATED', E_USER_WARNING);
		} else {
			pre_echo('AUTHENTICATION TYPE "'.strtoupper($type_code).'" CREATED: '.$auth->id);
		}
	}
	$GLOBALS['SQ_INSTALL'] = false;

}//end install_authentication_types()


/**
* Generate the global preferences list
* 
* @return void
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
	
	pre_echo('GLOBAL PREFERENCES DONE');
	
}//end generate_global_preferences()


/**
* Install event listeners
* 
* @return void
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
	
	pre_echo('EVENT LISTENERS DONE');
	
}//end install_event_listeners()


?>