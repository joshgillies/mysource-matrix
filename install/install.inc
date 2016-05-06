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
* $Id: install.inc,v 1.103 2013/07/30 06:04:22 lwright Exp $
*
*/

/**
* Install functions
*
* Purpose
* Functions used to install and upgrade the system
*
* @version $Revision: 1.103 $
* @package MySource_Matrix
* @subpackage install
*/


/**
* Regenerate all the configs used by Matrix
*
* Returns TRUE if all configs were successfully saved
*
* @return boolean
* @access public
*/
function regenerate_configs()
{
	// We need to include this so we can find out whether we should save the context
	// config (which is stored in a database rather than a config file)
	require_once SQ_LIB_PATH.'/db_install/db_install.inc';

	// Re-generate the System Config to make sure that we get any new defines that may have been issued
	require_once SQ_INCLUDE_PATH.'/system_config.inc';
	$cfg = new System_Config();
	if (!$cfg->save(Array(), FALSE)) return FALSE;

	// Re-generate the External Tools Config to make sure that we get any new defines that may have been issued
	require_once SQ_INCLUDE_PATH.'/external_tools_config.inc';
	$cfg = new External_Tools_Config();
	if (!$cfg->save(Array(), FALSE)) return FALSE;

	// Re-generate the Server Config to make sure that we get any new defines that may have been issued
	require_once SQ_SYSTEM_ROOT.'/core/server/squiz_server_config.inc';
	$squiz_server_cfg = new Squiz_Server_Config();
	if (!$squiz_server_cfg->save(Array(), FALSE)) {
		return FALSE;
	}

	// Re-generate the Apache Config to make sure that we get any new vars that may have been issued
	require_once SQ_INCLUDE_PATH.'/apache_config.inc';
	$apache_cfg = new Apache_Config();
	if (!$apache_cfg->save(Array(), FALSE)) return FALSE;

	// Re-generate the HIPO Config to make sure that we get any new defines that may have been issued
	require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_config.inc';
	$hipo_cfg = new HIPO_Config();
	if (!$hipo_cfg->save(Array(), FALSE)) return FALSE;

	// Re-generate the Messaging Service Config to make sure that we get any new defines that may have been issued
	require_once SQ_INCLUDE_PATH.'/messaging_service_config.inc';
	$ms_cfg = new Messaging_Service_Config();
	if (!$ms_cfg->save(Array(), FALSE)) return FALSE;

	// Re-generate the Password Rules Config to make sure that we get any new defines that may have been issued
	require_once SQ_INCLUDE_PATH.'/password_rules_config.inc';
	$pwd_cfg = new Password_Rules_Config();
	if (!$pwd_cfg->save(Array(), FALSE)) return FALSE;

	// Re-generate the Proxy Authentication Config to make sure that we get any new defines that may have been issued
	require_once SQ_INCLUDE_PATH.'/proxy_authentication_config.inc';
	$pa_cfg = new Proxy_Authentication_Config();
	if (!$pa_cfg->save(Array(), FALSE)) return FALSE;

	// Re-generate the System Maintenance Config
	require_once SQ_INCLUDE_PATH.'/system_maintenance_config.inc';
	$pa_cfg = new System_Maintenance_Config();
	if (!$pa_cfg->save(Array(), FALSE)) return FALSE;

	// Re-generate the Context Config to ma.... yeah, yeah, you know the deal by now =)
	// But this is a special case - saving to a DB table rather than a config file
	// So need to make sure the table exists before saving to it
	$db_tables = get_database_tables();
	$query_found = file_exists(MatrixDAL::getOvenPath().'/core/coreQueries.inc') ? TRUE : FALSE;
	if ((in_array('sq_ctx_def', $db_tables) === TRUE) && ($query_found === TRUE)) {
		require_once SQ_INCLUDE_PATH.'/context_config.inc';
		$context_cfg = new Context_Config();
		$current = $context_cfg->load();
		if (!$context_cfg->save(Array('contexts' => $current), FALSE)) {
			return FALSE;
		}
	}

	return TRUE;

}//end regenerate_configs()

/**
 * Check if deleting an asset type will affect other asset types
 * that aren't also being deleted.
 *
 * This would break a lot of things, so if this finds any errors, the
 * asset type(s) shouldn't be deleted.
 *
 * @param array $check_types The asset types to check.
 *
 * @return array Returns an array of errors found (if any).
 *               An empty array means no errors.
 */
function check_child_asset_types($check_types)
{
	$errors = array();

	// For the missing types, make sure that deleting them will not break any other types of assets.
	foreach ($check_types as $asset_type) {
		$child_types = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants($asset_type, FALSE);
		if (empty($child_types)) {
			continue;
		}

		// If we're also deleting the child types, that's ok.
		$deleting = array_intersect($child_types, $check_types);

		// Something is left over? Uhoh!
		$not_deleting = array_diff($child_types, $deleting);

		if (!empty($not_deleting)) {
			$msg = "Deleting ${asset_type} will break assets of type(s):\n    ".implode(', ', $not_deleting);
			$errors[] = $msg;
		}
	}

	return $errors;
}

/**
 * Clean up asset types (and inherited types) from the database.
 * This assumes you've already run the types being deleted through check_child_asset_types
 * to make sure you're not going to break lots of things.
 *
 * @param array $asset_types The asset types to delete from the database.
 *                           This only deletes the types, *not* their attributes
 *                           or values.
 *                           This is in case a package goes missing (by accident, or on purpose)
 *                           putting it back will make the old assets still work.
 *
 * @return void
 */
function cleanup_asset_types($asset_types)
{
	$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
	$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
	$db = MatrixDAL::getDb();

	// now actually delete the types
	foreach ($asset_types as $asset_type) {
		// delete the types
		try {
			$sql = 'DELETE FROM sq_ast_typ WHERE type_code = :type_code';
			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'type_code', $asset_type);
			MatrixDAL::execPdoQuery($query);
		} catch (Exception $e) {
			throw new Exception('Unable to delete type code '.$asset_type.' due to the following database error:'.$e->getMessage());
		}//end try catch


		// remove the inherited types
		try {
			$sql = 'DELETE FROM sq_ast_typ_inhd
				WHERE type_code = :type_code
				OR inhd_type_code = :inhd_type_code';

			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'type_code', $asset_type);
			MatrixDAL::bindValueToPdo($query, 'inhd_type_code', $asset_type);
			MatrixDAL::execPdoQuery($query);
		} catch (Exception $e) {
			throw new Exception('Unable to delete inherited type code '.$asset_type.' due to the following database error:'.$e->getMessage());
		}//end try catch
	}

	$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
	$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

	$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
	$msg_reps = Array(
		'type_code_list'	=> implode("\n", $asset_types),
	);
	$message = $ms->newMessage(Array(), 'install.asset_type.uninstall', $msg_reps);
	$message->subject = translate('Install');
	$message->body    = translate('UNINSTALLED the following asset types as they are no longer recognised by any package:')."\n".'%type_code_list%';
	$message->send();
}


/**
* Uninstall any assets that no longer have a .inc file
*
* @return boolean
* @access public
*/
function uninstall_asset_types()
{
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
		foreach ($packages_installed as $package_array) {
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
		$errors = check_child_asset_types($asset_types);

		// failed dependency tests => bail
		if (!empty($errors)) {
			$msg = implode("\n", $errors)."\n";
			$msg .= "\nAsset hierarchy would be corrupted, uninstall not committed.";
			trigger_error($msg, E_USER_WARNING);
			exit(1);
		}

		cleanup_asset_types($asset_types);

	}//end if asset types not empty

	// clear the type cache
	@unlink(SQ_DATA_PATH.'/private/db/asset_types.inc');

	return TRUE;

}//end uninstall_asset_types()


/**
* Uninstall any packages that don't have a package manager defined for them
*
* @return boolean
* @access public
*/
function uninstall_packages()
{
	// If the package manager doesn't exist for a package, it does not exist.
	// Assumption: packages live in the packages directory (Core never gets uninstalled).
	$packages_installed = $GLOBALS['SQ_SYSTEM']->getInstalledPackages();

	if (!empty($packages_installed)) {
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		foreach ($packages_installed as $package_array) {
			$package = $package_array['code_name'];

			// never delete the core package!
			if ($package == '__core__') continue;

			// package manager should exist for the package to exist
			$dir = 'packages/'.$package;
			$exists = file_exists(SQ_SYSTEM_ROOT.'/'.$dir.'/package_manager_'.$package.'.inc');

			if (!$exists) {
				uninstall_package($package);
			}

		}//end foreach installed package

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

	}//end if installed packages not empty

	return TRUE;

}//end uninstall_packages()


/**
 * Uninstall a particular package from the database and clean up it's asset map files.
 * You must connect to the db (and start a transaction) outside this function.
 *
 * @param string $package The package to delete.
 * @param boolean $cleanDb Whether to clean the database or not.
 *                         Setting to false will clean up the asset map info, and send
 *                         a message saying it was disabled, but leave it in the database
 *                         so running step3 will still report it as missing.
 * @return void
 */
function uninstall_package($package, $cleanDb=TRUE)
{
	if ($cleanDb) {
		// folder or the package manager does not exist, safe to delete
		$sql = 'DELETE FROM sq_pkg WHERE code_name = :package_name';
		$query = MatrixDAL::preparePdoQuery($sql);
		MatrixDAL::bindValueToPdo($query, 'package_name', $package);
		MatrixDAL::execPdoQuery($query);
	}

	$pkg_xml = SQ_DATA_PATH.'/private/asset_map/'.$package.'.xml';
	// remove its asset map file from the data directory
	if (file_exists($pkg_xml)) {
		if (!unlink($pkg_xml)) {
			trigger_error('Could not delete the asset map file for "'.$package.'"', E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			exit(1);
		}
	}

	$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
	$message_body = 'Uninstalled '.$package;
	$msg_reps = Array(
					'package'	=> $package,
				);
	$message = $ms->newMessage(Array(), 'install.package.uninstall', $msg_reps);
	$message->subject = translate('Install');
	$message->body    = translate('Uninstalled %package_name% Package');
	$message->send();
}//end uninstall_package()


/**
 * Find missing packages.
 * These are ones that exist in the database, but not on the filesystem.
 *
 * @return array Returns an array of missing packages, with the name (description) and the
 *               code_name.
 */
function find_missing_packages()
{
	$missing_packages = array();
	$packages = $GLOBALS['SQ_SYSTEM']->getInstalledPackages();
	foreach ($packages as $k => $package) {
		// The core always exists.
		if ($package['code_name'] == '__core__') {
			continue;
		}
		$package_dir = SQ_PACKAGES_PATH.'/'.$package['code_name'];
		$package_name = $package['name'];
		$package_code = $package['code_name'];
		if (!is_dir($package_dir)) {
			$missing_packages[$package_name] = $package_code;
			continue;
		}

		$package_path = $package_dir.'/package_manager_'.$package['code_name'].'.inc';
		if (!file_exists($package_path)) {
			$missing_packages[$package_name] = $package_code;
			continue;
		}
	}

	return $missing_packages;
}//end find_missing_packages()


/**
 * Find missing asset types.
 * Finds types that exist in the database, but are no longer available on the filesystem.
 * Used when reporting missing packages to also show which types will be unavailable.
 *
 * @return array Returns an array of ungrouped type codes (just listing all missing type codes),
 *               and an array of grouped codes (grouped by package name).
 */
function find_missing_types()
{
	// Because some packages are missing, check what asset types this will affect.
	$asset_types = $GLOBALS['SQ_SYSTEM']->am->getAssetTypes();
	$missing_types = array();
	$missing_type_codes = array();
	foreach ($asset_types as $type_info) {
		$type_name = $type_info['name'];
		$type_code = $type_info['type_code'];

		$dir = $type_info['dir'];
		if (is_dir(SQ_SYSTEM_ROOT.'/'.$dir)) {
			continue;
		}

		// There's nothing to say for certainty that a particular type is part of a particular package,
		// so just look at the directory and make an assumption.
		// The first part is 'packages/', the 2nd is the package name.
		$pkg_info  = explode('/', $dir);
		$pkg_group = $pkg_info[1];

		if (!isset($missing_types[$pkg_group])) {
			$missing_types[$pkg_group] = array();
		}

		$missing_types[$pkg_group][$type_name] = $type_code;
		$missing_type_codes[] = $type_code;
	}

	$results = array(
		'grouped'   => $missing_types,
		'ungrouped' => $missing_type_codes,
	);

	return $results;

}//end find_missing_types()


/**
 * Disable missing packages.
 * Looks for packages that are found in the db but not on the filesystem.
 * If any are found, it will report them as missing (along with the affected type codes),
 * and clean up the affected type codes from the database.
 * This leaves attributes alone so if the package becomes available again,
 * any existing attributes and assets will work again.
 *
 * @return void
 */
function disable_missing_packages()
{
	$missing_packages = find_missing_packages();
	if (empty($missing_packages)) {
		// Even though there's nothing missing, clear the cache in case we're
		// adding a new package and asset types to the system.
		@unlink(SQ_DATA_PATH.'/private/db/asset_types.inc');
		return;
	}

	$missing_type_info  = find_missing_types();
	$missing_types      = $missing_type_info['grouped'];
	$missing_type_codes = $missing_type_info['ungrouped'];

	$msg = "The following packages are missing and will affect the following asset types:\n";
	asort($missing_packages);
	foreach ($missing_packages as $package_name => $package_code) {
		$msg .= $package_name." (".$package_code.")\n";
		if (!isset($missing_types[$package_code])) {
			$msg .= "    - (unknown - already cleaned up)\n";
			continue;
		}

		$missing_package_types = $missing_types[$package_code];
		asort($missing_package_types);
		foreach ($missing_package_types as $type_name => $type_code) {
			$msg .= "    - ".$type_name." (".$type_code.")\n";
		}
	}

	$errors = check_child_asset_types($missing_type_codes);
	if (!empty($errors)) {
		$msg = implode("\n", $errors)."\n";
		$msg .= "\nAsset hierarchy would be corrupted, uninstall not committed.";
		trigger_error($msg, E_USER_WARNING);
		exit(1);
	}

	trigger_error($msg, E_USER_WARNING);

	// Don't uninstall the missing packages, leave them as-is
	// so step_3 can keep reporting them as missing.
	foreach ($missing_packages as $package_name => $package_code) {
		uninstall_package($package_code, TRUE);
	}


	// Clean up the missing type codes though.
	// If the package becomes available again, they will be re-installed.
	cleanup_asset_types($missing_type_codes);

	// clear the type cache
	@unlink(SQ_DATA_PATH.'/private/db/asset_types.inc');

}//end disable_missing_packages()


/**
* Install core assets
*
* @param array	$package_list	Lists what packages are going to be installed
*								and what assets in those packages.
*
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
			return TRUE;
		} else {
			$assets = $package_list['core'];
			// If it wasn't an array at the core index, just make it empty
			if (!is_array($assets)) $assets = Array();
		}
    }

	echo 'Installing core .. ';

	require_once SQ_CORE_PACKAGE_PATH.'/package_manager_core.inc';
	$pm = new Package_Manager_Core();
	$result = $pm->updatePackageDetails($assets);
	build_string_file('__core__');

	$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
	$msg_reps = Array(
					'package'	=> 'Core',
				);
	$message = $ms->newMessage(Array(), 'install.package.'.($result ? 'success' : 'fail'), $msg_reps);
	$message->subject = translate('Install');
    if ($result) {
        $message->body = translate('%package_name% Package Installed Succesfully');
    } else {
        $message->body = translate('%package_name% Package Install FAILED');
    }
	$message->send();

	if (!$result) exit(1);

	$result = $pm->installSystemAssets();

	// 0 (zero) indicates success, but no system assets were created - suppress in this case
	if ($result != 0) {
		$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
		$msg_reps = Array(
						'package'		=> 'Core',
						'num_assets'	=> $result,
					);
		$message = $ms->newMessage(Array(), 'install.system_assets.'.($result != -1 ? 'success' : 'fail'), $msg_reps);
		if ($result !== -1) {
		    $message->body = translate('%package_name% Package System Asset Creation Succeeded : %num_assets% Assets Created');
		} else {
		    $message->body = translate('%package_name% Package System Asset Creation FAILED');
		}
		$message->subject = translate('Install');
		$message->send();

	}
	if ($result == -1) exit(1);

	// set the current user object to the root user so we can finish
	// the install process without permission denied errors
	$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
	$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user);

	echo "Done.\n";

	return TRUE;

}//end install_core()


/**
* Returns the entire list of packages that exist in the package directory
*
* @return array
* @access public
*/
function get_package_list()
{
	$packages = Array();
	$d = dir(SQ_PACKAGES_PATH);

	while (FALSE !== ($entry = $d->read())) {
		if ($entry == '.' || $entry == '..') {
			continue;
		}
		// if this is a directory, process it
		if ($entry != 'CVS' && is_dir(SQ_PACKAGES_PATH.'/'.$entry)) {
			$packages[] = $entry;
		}
	}
	$d->close();

	return $packages;

}//end get_package_list()


/**
* Install all the packages into the system. Uses the $packages(Assets) arrays
* if defined to determine which assets to install. If a $package is defined,
* then it checks to see if the list of assets is defined for that package. If
* not, will install ALL assets.
*
* @param array	$package_assets	An array of assets inside packages to install.
*								Requires that the package name be defined in
*								the $packages array.
*								Array('package_name' => Array('assetname'),)
*
* @return true|array Returns true if all packages were installed successfully.
*                    If some have been deferred, this returns an array of
*                    the deferred packages so they can be retried.
* @access public
*/
function install_packages($package_assets=Array())
{
	// right now that we have sorted all that out we can install the packages
	$d = dir(SQ_PACKAGES_PATH);

	$deferred_packages = Array();

	// Do we only install named packages?
	$limit_packages = count($package_assets);
	$packages = get_package_list();

	foreach ($packages as $package) {
		$assets = Array();
		if ($limit_packages) {
			// Found an existing package that doesn't need re-installing?
			// That's good, it means it's installed completely.
			if (!isset($package_assets[$package])) {
				continue;
			}

			$assets = $package_assets[$package];
			// If it wasn't an array at the core index, just quit.
			if (!is_array($assets)) $assets = Array();
		}

		$result = install_package($package, $assets);
		if (is_array($result)) {
			$deferred_packages[$package] = $result;
		}
	}

	return count($deferred_packages) ? $deferred_packages : TRUE;

}//end install_packages()


/**
* Installs a single package into the system.
*
* Returns boolean TRUE on success, otherwise an array of type_codes
* of assets that couldn't be installed yet.
*
* @param string	$package		The package to install
* @param array	$package_assets	An array of asset types to install for this package.
*
* @return mixed array|boolean
* @access public
*/
function install_package($package, $package_assets=Array())
{
	echo 'Installing '.$package.' .. ';

	require_once SQ_PACKAGES_PATH.'/'.$package.'/package_manager_'.$package.'.inc';
	require_once SQ_LIB_PATH.'/db_install/db_install.inc';
	$class = 'package_manager_'.$package;
	$pm = new $class();
	$result = $pm->updatePackageDetails($package_assets);
	build_string_file($package);

	$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();

	if (is_array($result)) {
		$msg_reps = Array(
						'package'	=> $package,
					);
		$message = $ms->newMessage(Array(), 'install.package.deferred', $msg_reps);
		$message->subject = translate('Install');
		$message->body    = translate('%package_name% Package Deferred for Dependency Resolution');
		$message->send();
		return $result;
	}

	$message_body = strtoupper($package).' PACKAGE '.(($result) ? 'DONE SUCCESSFULLY' : 'FAILED');
	$msg_reps = Array(
					'package'	=> $package,
				);
	$message = $ms->newMessage(Array(), 'install.package.'.($result ? 'success' : 'fail'), $msg_reps);
	$message->subject = translate('Install');
    if ($result) {
        $message->body = translate('%package_name% Package Installed Succesfully');
    } else {
        $message->body = translate('%package_name% Package Install FAILED');
    }
	$message->send();

	if (!$result) exit(1);
	$result = $pm->installSystemAssets();
	if ($result != 0) {	// 0 indicates success, but no system assets were created - suppress in this case
		$msg_reps = Array(
						'package'		=> $package,
						'num_assets'	=> $result,
					);
		$message_body = strtoupper($package).' SYSTEM ASSET CREATION '.(($result == -1) ? 'FAILED' : (': '.$result.' NEW ASSETS CREATED'));
		$message = $ms->newMessage(Array(), 'install.system_assets.'.($result ? 'success' : 'fail'), $msg_reps);
		$message->subject = translate('Install');
		if ($result !== -1) {
		    $message->body = translate('%package_name% Package System Asset Creation Succeeded : %num_assets% Assets Created');
		} else {
		    $message->body = translate('%package_name% Package System Asset Creation FAILED');
		}
		$message->send();
	}

	if ($result == -1) exit(1);
	unset($pm);

	echo "Done.\n";

	return TRUE;

}//end install_package()


/**
* Try to install a list of deferred packages recursively until the list is the same as the start
*
* Returns an array of any failed installed packages, or TRUE if it all worked okay
*
* @param array	$packages	the array of packages to attempt to install
*
* @return mixed array|boolean
* @access public
*/
function install_deferred($packages)
{
	$deferred = install_packages($packages);

	if ($deferred === TRUE) return TRUE;

	// if the arrays are identical, return the array
	if ($deferred == $packages) return $deferred;

	// the list should be smaller now...
	return install_deferred($deferred);

}//end install_deferred()


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
	$auth_folder = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('authentication_folder');
	$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($auth_folder->id, SQ_LINK_TYPE_1, 'authentication', FALSE);
	$installed_auth_types = Array();
	foreach ($links as $link_data) {
		$installed_auth_types[] = $link_data['minor_type_code'];
	}

	// install all systems that are not currently installed
	$folder_link = Array(
					'asset'			=> &$auth_folder,
					'link_type'		=> SQ_LINK_TYPE_1,
					'is_exclusive'	=> 1,
				   );
	foreach ($auth_types as $type_code) {
		if (in_array($type_code, $installed_auth_types)) {
			continue;
		}
		$GLOBALS['SQ_SYSTEM']->am->includeAsset($type_code);
		$auth = new $type_code();

		if (!$auth->create($folder_link)) {
			trigger_error('AUTHENTICATION TYPE "'.strtoupper($type_code).'" NOT CREATED', E_USER_WARNING);
		} else {
			$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
			$msg_reps = Array(
							'auth_type_name'	=> strtoupper($type_code),
							'assetid'			=> $auth->id,
						);
			$message = $ms->newMessage(Array(), 'install.authentication', $msg_reps);
			$message->subject = translate('Install');
			$message->body    = translate('%auth_type_name% Authentication Type Created : %assetid%');
			$message->send();

		}
	}

	return TRUE;

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

	// Get a list of missing packages, so their preferences can be skipped.
	$missing_packages = find_missing_packages();

	$preferences = Array();
	if (is_file(SQ_DATA_PATH.'/private/conf/preferences.inc')) {
		include SQ_DATA_PATH.'/private/conf/preferences.inc';
	}

	foreach ($packages as $package) {
		// slight change for the core package
		if ($package['code_name'] == '__core__') {
			require_once SQ_CORE_PACKAGE_PATH.'/package_manager_core.inc';
			$class = 'package_manager_core';
		} else {
			if (in_array($package['code_name'], $missing_packages)) {
				continue;
			}

			require_once SQ_PACKAGES_PATH.'/'.$package['code_name'].'/package_manager_'.$package['code_name'].'.inc';
			$class = 'package_manager_'.$package['code_name'];
		}

		$pm = new $class();
		$pm->installUserPreferences($preferences);
		unset($pm);
	}
	$str = '<'.'?php $preferences = '.var_export($preferences, TRUE).'; ?'.'>';
	if (!string_to_file($str, SQ_DATA_PATH.'/private/conf/preferences.inc')) {
		return FALSE;
	}

	$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
	$message = $ms->newMessage(Array(), 'install.prefs', Array());
	$message->subject = translate('Install');
	$message->body    = translate('Global Preferences Done');
	$message->send();

	return TRUE;

}//end generate_global_preferences()


/**
* Generate language characters map file
*
* @return boolean
* @access public
*/
function generate_lang_char_map()
{
	$lang_char_map = Array(
						'pl'	=>  Array(
										'ą' => 'a',
										'ć' => 'c',
										'ę' => 'e',
										'ł' => 'l',
										'ń' => 'n',
										'ó' => 'o',
										'ś' => 's',
										'ż' => 'z',
										'ź' => 'z',
										'Ą' => 'a',
										'Ć' => 'c',
										'Ę' => 'e',
										'Ł' => 'l',
										'Ń' => 'n',
										'Ó' => 'o',
										'Ś' => 's',
										'Ż' => 'z',
										'Ź' => 'z',
									),
						'en'	=>  Array (
										'ā' => 'a',
										'ē' => 'e',
										'ī' => 'i',
										'ō' => 'o',
										'ū' => 'u',
										'Ā' => 'A',
										'Ē' => 'E',
										'Ī' => 'I',
										'Ō' => 'O',
										'Ū' => 'U',
										'á' => 'a',
										'é' => 'e',
										'í' => 'i',
										'ó' => 'o',
										'ú' => 'u',
										'ü' => 'u',
										'ñ' => 'n',
										'Á' => 'A',
										'É' => 'E',
										'Í' => 'I',
										'Ó' => 'O',
										'Ú' => 'U',
										'Ü' => 'U',
										'Ñ' => 'N',
									  ),
			 );

	$str = '<'.'?php $lang_char_map = '.var_export($lang_char_map, TRUE).'; ?'.'>';
	if (!string_to_file($str, SQ_DATA_PATH.'/private/conf/lang_char_map.inc')) {
		return FALSE;
	}

	return TRUE;

}//end generate_lang_char_map()


/**
* Creates a cached file of the asset types and their information
*
* @return void
* @access public
*/
function cache_asset_types()
{
	$asset_types = $GLOBALS['SQ_SYSTEM']->am->getAssetTypes();

	$missing_type_info  = find_missing_types();
	$missing_type_codes = $missing_type_info['ungrouped'];

	foreach ($asset_types as $type_code => $details) {
		if (in_array($type_code, $missing_type_codes)) {
			unset($asset_types[$type_code]);
			continue;
		}

		$asset_types[$type_code]['ancestor_types'] = $GLOBALS['SQ_SYSTEM']->am->getTypeAncestors($type_code, FALSE, TRUE);
	}
	$output = '<'.'?php'."\n".' $asset_types = ';
	$output .= var_export($asset_types, TRUE);
	$output .= "\n?".'>';

	if (!string_to_file($output, SQ_DATA_PATH.'/private/db/asset_types.inc')) {
		trigger_error('Could not cache the asset types', E_USER_ERROR);
	}

	echo "Asset types have been cached.\n";

}//end cache_asset_types()


/**
* Install event listeners
*
* @return boolean
* @access public
*/
function install_event_listeners()
{
	// we need to install any event listeners here, now that we have installed all the asset types
	$packages = $GLOBALS['SQ_SYSTEM']->getInstalledPackages();

	$em = $GLOBALS['SQ_SYSTEM']->getEventManager();
	$em->writeStaticEventsCacheFile();

	$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
	$message = $ms->newMessage(Array(), 'install.event_listeners', Array());
	$message->subject = translate('Install');
	$message->body    = translate('Event Listeners Done');
	$message->send();

	return TRUE;

}//end install_event_listeners()


/**
* Generate performance config file
*
* @return boolean
* @access public
*/
function generate_performance_config()
{
	$str = "<?php\n";
	$str .= "#Constants for performance timeline chart\n";
	$str .= "define('PERFORMANCE_TIME_PRINT_PRECISION', 2);\n";
	$str .= "define('PERFORMANCE_PERCENTAGE_PRINT_PRECISION', 8);\n";

	$performance_config = Array(
							'page_asset_listing' => Array(
									'report'	=>	'http://www.squizlabs.com/squiz-matrix/matrix-performance-report-asset-listing-page',
							)
						 );

	$str .= '$performance_config = '.var_export($performance_config, TRUE).";\n";
	$str .= "?>";
	if (!string_to_file($str, SQ_DATA_PATH.'/private/conf/performance_config.inc')) {
		return FALSE;
	}

	return TRUE;

}//end generate_performance_config()


/**
* Generate file bridge config file
*
* @return boolean
* @access public
*/
function generate_file_bridge_config()
{
								if(is_file(SQ_DATA_PATH.'/private/conf/file_bridge.inc')) return TRUE;

	$str = "<?php\n";
	$str .= "#path of file bridge\n";
	$str .= "define('FILE_BRIDGE_PATH', SQ_DATA_PATH.'/public');\n";
	$str .= "?>";
	if (!string_to_file($str, SQ_DATA_PATH.'/private/conf/file_bridge.inc')) {
		return FALSE;
	}

	return TRUE;

}//end generate_file_bridge_config()



/**
* Generate import tools manager config file
*
* @return boolean
* @access public
*/
function generate_import_tools_manager_config()
{
	if(is_file(SQ_DATA_PATH.'/private/conf/import_tools_manager.inc')) return TRUE;
	$structured_import_dir = '';
	$bulk_file_import_dir = '';
	if( $GLOBALS['SQ_SYSTEM']->am->installed('import_tools_manager')) {
		$itm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('import_tools_manager');
		$structured_import_dir = $itm->attr('import_dir');
		$bulk_file_import_dir = $itm->attr('bulk_file_import_dir');
	}


	$str = "<?php\n";

	$str .= "#path of structured file import directory \n";
	$str .= "define('STRUCTURED_FILE_IMPORT_DIR', '$structured_import_dir');\n";

	$str .= "#path of bulk file import directory \n";
	$str .= "define('BULK_FILE_IMPORT_DIR', '$bulk_file_import_dir');\n";

	$str .= "?>";
	if (!string_to_file($str, SQ_DATA_PATH.'/private/conf/import_tools_manager.inc')) {
		return FALSE;
	}

	return TRUE;

}//end generate_import_tools_manager_config()



/**
* Minify js files for EES improvement
*
* @return boolean
* @access public
*/
function minify_css_files()
{
	require_once SQ_SYSTEM_ROOT.'/scripts/ees_minify/jsmin.php';

	$file = SQ_DATA_PATH.'/public/matrix.min.js';

	$source_files = Array (
	SQ_SYSTEM_ROOT.'/fudge/var_serialise/var_serialise.js',
	SQ_SYSTEM_ROOT.'/core/lib/js/general.js',
	SQ_SYSTEM_ROOT.'/core/lib/js/debug.js',
	SQ_SYSTEM_ROOT.'/core/lib/js/edit.js',
	SQ_SYSTEM_ROOT.'/core/lib/js/tooltip.js',
	SQ_SYSTEM_ROOT.'/core/lib/js/translation.js',
	SQ_SYSTEM_ROOT.'/core/lib/js/layer_handler.js',
	SQ_SYSTEM_ROOT.'/core/lib/html_form/html_form.js',
	SQ_SYSTEM_ROOT.'/core/lib/js/detect.js',
	SQ_SYSTEM_ROOT.'/core/assets/bodycopy/bodycopy/js/bodycopy_edit_divs.js',
	SQ_SYSTEM_ROOT.'/core/assets/metadata/metadata_fields/metadata_field_select/js/metadata_field_select.js',
	SQ_SYSTEM_ROOT.'/core/assets/metadata/metadata_fields/metadata_field_multiple_text/js/metadata_field_multiple_text.js',
	SQ_SYSTEM_ROOT.'/core/assets/metadata/metadata_fields/metadata_field_hierarchy/js/metadata_field_hierarchy.js',
	);

	// Output a minified version
	$string = '';
	foreach ($source_files as $source) {
		$string .= JSMin::minify(file_get_contents($source));
	}
	$result = file_put_contents($file, $string);

	if($result === FALSE) {
	trigger_error ("FAILED TO GENREATE MINIFIED JS FILES FOR EES");
	return FALSE;
	}

	echo "JS Files have been minified successfully.\n";

	return TRUE;
}


/**
* Format an array of packages => Array(type_codes) for display
*
* @param array	$packages	the array of deferred types to format
*
* @return string
* @access public
*/
function format_deferred_packages($packages)
{
	$out = '';
	foreach ($packages as $package => $types) {
		$out .= "\n".$package.":\n";
		foreach ($types as $type) {
			$out .= "    ".$type."\n";
		}
	}

	return $out;

}//end format_deferred_packages()


/**
 * Takes a PO string file from the specified package (or core) and turns it
 * into a Matrix serialised string
 *
 * Package name of "__core__" will build the strings of the Core package.
 * Returns 0 on success, 1 if failed, -1 if nothing was found.
 *
 * @param string $package   The name of the package.
 * @param string $locale    The specific locale to build (NULL = all of them).
 *
 * @return integer
 */
function build_string_file($package, $locale=NULL) {
    if ($package === '__core__') {
        $path = realpath(SQ_SYSTEM_ROOT).'/core/locale';
    } else {
        $path = realpath(SQ_SYSTEM_ROOT).'/packages/'.$package.'/locale';
    }

    $ok = 0;
    $build_locales = Array();

    if ($locale === NULL) {
        $files = glob($path.'/*.po');
        foreach ($files as $file) {
            $build_locales[] = basename($file, '.po');
        }

        if (count($build_locales) === 0) {
            $ok = -1;
        }
    } else {
        if (is_file($path.'/'.$locale.'.po') === FALSE) {
            $ok = -1;
        } else {
            $build_locales = Array($locale);
        }
    }

    foreach ($build_locales as $locale) {
        $source_filename = $path.'/'.$locale.'.po';
        $target_filename = SQ_DATA_PATH.'/private/packages/'.$package.'/strings.'.$locale;
        $lines = file($source_filename);

		if (!file_exists(SQ_DATA_PATH.'/private/packages/'.$package)) {
			mkdir(SQ_DATA_PATH.'/private/packages/'.$package);
		}
		
        $strings = Array();
        $current    = NULL;
        $line_index = NULL;
        $flags      = NULL;
        $msgid      = NULL;
        $msgstr     = NULL;

        // Create closure to build the strings file.
        $add_to_strings = function($msgid, $msgstr) use (&$strings) {
        	if (($msgstr !== '') && ($msgid !== '')) {
            	$strings[$msgid] = $msgstr;
        	}
        };

        foreach ($lines as $i => $line) {
            $line = trim($line);
            if (strpos($line, 'msgid "') === 0) {
                // Message ID.
                $current = 'msgid';
                $add_to_strings($msgid, $msgstr);
                if (empty($msgid) === FALSE) {
                    $flags = '';
                }
                $msgid  = '';
                $msgstr = '';
                $this_str = substr($line, strlen('msgid "'), -1);
            } else if (strpos($line, 'msgstr[') === 0) {
                // Plural option.
                list($line_index, $this_str) = sscanf($line, 'msgstr[%d] %s');
                $current = 'msgstr_plural';
                $msgstr  = Array();
                $this_str = substr(trim($this_str), -1, 1);
            } else if (strpos($line, 'msgstr "') === 0) {
                // Single translation.
                $current  = 'msgstr';
                $msgstr   = '';
                $this_str = substr($line, strlen('msgstr "'), -1);
            } else if (strpos($line, '"') === 0) {
                // Continuation of previous string.
                $this_str = substr($line, -1, 1);
            } else {
                $this_str = '';
            }

            // Normalise the string: handle any embedded newlines (remember
            // actual LFs don't count), unescape double quotes and backslashes.
            $this_str = str_replace('\\n', "\n", $this_str);
            $this_str = str_replace('\\"', '"', $this_str);
            $this_str = str_replace('\\\\', '\\', $this_str);

            switch ($current) {
                case 'msgid':
                    $msgid .= $this_str;
                break;

                case 'msgstr':
                    $msgstr .= $this_str;
                break;

                case 'msgstr_plural':
		    if (array_key_exists($line_index, $msgstr) === FALSE) {
			$msgstr[$line_index] = '';
		    }
                    $msgstr[$line_index] .= $this_str;
                break;
            }//end switch
        }//end foreach

        $add_to_strings($msgid, $msgstr);
        file_put_contents($target_filename, serialize($strings));
    }//end foreach

    return $ok;
}//end build_string_file()


/**
 * Takes a PO string file from all packages and turns it into
 * a JavaScript locale file stored in data/public.
 *
 * This scans the PO files for any strings marked with the flag "js-translate"
 * and adds them to the translated strings JS file.
 * Flags are lines before msgid lines starting with "#,".
 * This is to be done at the end of step 3, after all packages are installed.
 *
 * @return integer
 */
function build_js_string_files() {
    $packages = $GLOBALS['SQ_SYSTEM']->getInstalledPackages();
    $locales  = Array();

    $build_files = Array();
    foreach ($packages as $package) {
        if ($package['code_name'] === '__core__') {
            $path = realpath(SQ_SYSTEM_ROOT).'/core/locale';
        } else {
            $path = realpath(SQ_SYSTEM_ROOT).'/packages/'.$package['code_name'].'/locale';
        }

        $files = glob($path.'/*.po');
        foreach ($files as $file) {
            $build_files[] = Array(
                'filename' => $file,
                'locale'   => basename($file, '.po'),
            );
        }
    }//end foreach

    $strings = Array();
    $ok      = 0;

    foreach ($build_files as $source) {
        $source_filename = $source['filename'];
        $locale          = $source['locale'];
        $lines = file($source_filename);

        if (array_key_exists($locale, $strings) === FALSE) {
            $strings[$locale] = Array();
        }

        $current    = NULL;
        $flags      = NULL;
        $line_index = NULL;
        $msgid      = NULL;
        $msgstr     = NULL;

        // Create closure to build the strings array.
        $add_to_strings = function($msgid, $msgstr, $flags) use (&$strings, $locale) {
            if ((empty($msgid) === FALSE) && (empty($flags) === FALSE) && (strpos($flags, 'js-translate') !== FALSE)) {
                $strings[$locale][$msgid] = $msgstr;
            }
        };

        foreach ($lines as $line) {
            $line = trim($line);
            if (strpos($line, '#, ') === 0) {
                $add_to_strings($msgid, $msgstr, $flags);
                $msgid  = '';
                $msgstr = '';
                $flags = substr($line, 3);
            } else if (strpos($line, 'msgid "') === 0) {
                // Message ID.
                $current = 'msgid';
                if (empty($msgid) == FALSE) {
                    $add_to_strings($msgid, $msgstr, $flags);
                    $flags = '';
                }
                $msgid  = '';
                $msgstr = '';
                $this_str = substr($line, strlen('msgid "'), -1);
            } else if (strpos($line, 'msgstr[') === 0) {
                // Plural option.
                list($line_index, $this_str) = sscanf($line, 'msgstr[%d] %s');
                $current = 'msgstr_plural';
                $msgstr  = Array();
                $this_str = substr(trim($this_str), -1, 1);
            } else if (strpos($line, 'msgstr "') === 0) {
                // Single translation.
                $current  = 'msgstr';
                $msgstr   = '';
                $this_str = substr($line, strlen('msgstr "'), -1);
            } else if (strpos($line, '"') === 0) {
                // Continuation of previous string.
                $this_str = substr($line, -1, 1);
            } else {
                $this_str = '';
            }

            // Normalise the string: handle any embedded newlines (remember
            // actual LFs don't count), unescape double quotes and backslashes.
            $this_str = str_replace('\\n', "\n", $this_str);
            $this_str = str_replace('\\"', '"', $this_str);
            $this_str = str_replace('\\\\', '\\', $this_str);

            switch ($current) {
                case 'msgid':
                    $msgid .= $this_str;
                break;

                case 'msgstr':
                    $msgstr .= $this_str;
                break;

                case 'msgstr_plural':
		    if (array_key_exists($line_index, $msgstr) === FALSE) {
			$msgstr[$line_index] = '';
		    }
                    $msgstr[$line_index] .= $this_str;
                break;
            }//end switch
        }//end foreach

        $add_to_strings($msgid, $msgstr, $flags);
    }//end foreach

    foreach ($strings as $locale => $locale_strings) {
        if (count($locale_strings) > 0) {
            $js_file  = 'SQ_CURRENT_LOCALE = \''.$locale.'\';'."\n";
            $js_file .= 'translated_strings[\''.$locale.'\'] = {'."\n";
            foreach ($locale_strings as $msgid => $msgstr) {
                $js_file .= '\''.addslashes($msgid).'\': \''.addslashes($msgstr).'\','."\n";
            }

            // Pull out the last comma from the file.
            $js_file .= '};'."\n";
            $js_file = str_replace(','."\n".'}', "\n".'}', $js_file);

            $target_filename = SQ_DATA_PATH.'/public/system/core/js_strings.'.$locale.'.js';
            create_directory(dirname($target_filename));
            file_put_contents($target_filename, $js_file);
        }
    }

    return $ok;
}//end build_string_file()


/**
* Generate an install key for this system
*
* The install key is simply an md5_file() on the main.inc file. This function
* should be called the FIRST time step_03.php is called, then again when the
* generate_install_key.php script is run.
*
* This function will return NULL if the main.inc file does not exist.
*
* @param boolean	$create_key_file	If TRUE, also add to licence config
*										(if key is not NULL)
*
* @return mixed string|null
* @access public
*/
function generate_install_key($create_key_file=FALSE)
{
	$host = php_uname('n');
	$ip = gethostbyname($host);
	$install_key = md5($host.'-'.$ip.'-'.SQ_SYSTEM_ROOT);

	if ($create_key_file) {
		require_once SQ_INCLUDE_PATH.'/system_config.inc';
		$cfg = new System_Config();
		ob_start();
		if (!$cfg->save(Array('SQ_LICENCE_INSTALL_KEY' => $install_key), FALSE, FALSE)) {
			ob_end_clean();
			return NULL;
		}
		ob_end_clean();
	}

	return $install_key;

}//end generate_install_key()


/**
* Generates the baked Database Abstraction Layer (DAL) queries for the package
*
* Package name of '__core__' does the Core queries.
*
* @param string	$name	The name of the package in question
*
* @return boolean
* @access public
*/
function install_dal_queries($name)
{
	$old_path = ini_get('include_path');
	ini_set('include_path', SQ_LIB_PATH);
	require_once SQ_LIB_PATH.'/MatrixDAL/MatrixDALBaker.inc';

	// Set this to true in case a package doesn't have any queries.
	$result = TRUE;

    if ($name === '__core__') {
        $queries_found = MatrixDALBaker::addCoreQueries();
        if ($queries_found) {
            $result = MatrixDALBaker::bakeQueriesFile('core');
        }
    } else {
        $queries_found = MatrixDALBaker::addPackageQueries($name);
        if ($queries_found) {
            $result = MatrixDALBaker::bakeQueriesFile($name.'_package');
        }
    }

	ini_set('include_path', $old_path);

	return $result;

}//end install_dal_queries()

?>
