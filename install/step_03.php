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
* $Id: step_03.php,v 1.36 2004/06/02 00:27:19 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/

/**
* Install Step 3
*
* Purpose
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
* @subpackage install
*/
ini_set('memory_limit', -1);
error_reporting(E_ALL);
$SYSTEM_ROOT = '';
// from cmd line
if ((php_sapi_name() == 'cli')) {
	if (isset($_SERVER['argv'][1])) $SYSTEM_ROOT = $_SERVER['argv'][1];
	$err_msg = "You need to supply the path to the System Root as the first argument\n";

} else { 
	if (isset($_GET['SYSTEM_ROOT'])) $SYSTEM_ROOT = $_GET['SYSTEM_ROOT'];
	$err_msg = '
	<div style="background-color: red; color: white; font-weight: bold;">
		You need to supply the path to the System Root as a query string variable called SYSTEM_ROOT
	</div>
	';
}

if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	trigger_error($err_msg, E_USER_ERROR);
}

// Dont set SQ_INSTALL flag before this include because we want
// a complete load now that the database has been created
require_once $SYSTEM_ROOT.'/core/include/init.inc';

// Firstly let's check that we are OK for the version
if(version_compare(PHP_VERSION, SQ_REQUIRED_PHP_VERSION, '<')) {
	trigger_error('<i>'.SQ_SYSTEM_LONG_NAME.'</i> requires PHP Version '.SQ_REQUIRED_PHP_VERSION.'.<br/> You may need to upgrade.<br/> Your current version is '.PHP_VERSION, E_USER_ERROR);
}

// Let everyone know we are installing
$GLOBALS['SQ_INSTALL'] = true;

// Re-generate the System Config to make sure that we get any new defines that may have been issued
require_once SQ_INCLUDE_PATH.'/system_config.inc';
$cfg = new System_Config();
$cfg->save(Array(), false);

// Re-generate the Server Config to make sure that we get any new defines that may have been issued
require_once SQ_SYSTEM_ROOT.'/core/server/squiz_server_config.inc';
$hipo_cfg = new Squiz_Server_Config();
$hipo_cfg->save(Array(), false);

// Re-generate the HIPO Config to make sure that we get any new defines that may have been issued
require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_config.inc';
$hipo_cfg = new HIPO_Config();
$hipo_cfg->save(Array(), false);

// Re-generate the Messaging Service Config to make sure that we get any new defines that may have been issued
require_once SQ_SYSTEM_ROOT.'/core/include/messaging_service_config.inc';
$ms_cfg = new Messaging_Service_Config();
$ms_cfg->save(Array(), false);


$db = &$GLOBALS['SQ_SYSTEM']->db;



//--        INSTALL CORE        --//

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


//--        INSTALL PACKAGES        --//

// Right now that we have sorted all that out we can install the packages
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




//--        INSTALL AUTHENTICATION TYPES        --//

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




//--        GENERATE GLOBAL PREFERENCES        --//

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




//--        INSTALL EVENT LISTENERS        --//

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

?>