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
* $Id: step_03.php,v 1.30.2.3 2004/02/21 13:28:40 brobertson Exp $
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

// Re-generate the Messaging Service Config to make sure that we get any new defines that may have been issued
require_once SQ_SYSTEM_ROOT.'/core/replication/replication_config.inc';
$rep_cfg = new Replication_Config();
$rep_cfg->save(Array(), false);


$db = &$GLOBALS['SQ_SYSTEM']->db;



//--        INSTALL CORE        --//

require_once SQ_CORE_PACKAGE_PATH.'/package_manager_core.inc';
$pm = new Package_Manager_Core();
$result = $pm->updatePackageDetails();
pre_echo("CORE PACKAGE ".(($result) ? "DONE SUCCESSFULLY" : "FAILED"));
if (!$result) exit(1);

// Firstly let's create some Assets that we require to run

$GLOBALS['SQ_SYSTEM_ASSETS'] = Array();

$root_folder = &$GLOBALS['SQ_SYSTEM']->am->getAsset(1, 'root_folder', true);
// if there is no root folder assume that this section hasn't been run
if (is_null($root_folder)) {
	
	$root_folder = &create_root_folder();
	$GLOBALS['SQ_SYSTEM']->am->acquireLock($root_folder->id, 'all');

	$trash_folder = &create_trash_folder();
	$GLOBALS['SQ_SYSTEM']->am->acquireLock($trash_folder->id, 'all');

	$system_management_folder = &create_system_management_folder();
	$GLOBALS['SQ_SYSTEM']->am->acquireLock($system_management_folder->id, 'all');

	$system_user_group = &create_system_user_group();
	$GLOBALS['SQ_SYSTEM']->am->acquireLock($system_user_group->id, 'all');


	$root_user = &create_root_user();

	// What we have to do here is release all locks on by user nobody, 
	// then re-acquire them when we become the root user below
	$GLOBALS['SQ_SYSTEM']->am->releaseLock($root_user->id,					'all');
	$GLOBALS['SQ_SYSTEM']->am->releaseLock($system_user_group->id,			'all');
	$GLOBALS['SQ_SYSTEM']->am->releaseLock($system_management_folder->id,	'all');
	$GLOBALS['SQ_SYSTEM']->am->releaseLock($trash_folder->id,				'all');
	$GLOBALS['SQ_SYSTEM']->am->releaseLock($root_folder->id,				'all');

	// set the current user object to the root user so we can finish
	// the install process without permission denied errors
	$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user);

	$GLOBALS['SQ_SYSTEM']->am->acquireLock($system_management_folder->id,	'all');
	$GLOBALS['SQ_SYSTEM']->am->acquireLock($system_user_group->id,			'all');

	$authentication_folder = &create_authentication_folder();
	$GLOBALS['SQ_SYSTEM']->am->acquireLock($authentication_folder->id, 'all');

	$cron_manager = &create_cron_manager();
	$search_manager = &create_search_manager();
	$layout_manager = &create_layout_manager();
	$remap_manager = &create_remap_manager();

	$designs_folder = &create_designs_folder();
	$GLOBALS['SQ_SYSTEM']->am->acquireLock($designs_folder->id, 'all');

	$login_design = &create_login_design();
	$GLOBALS['SQ_SYSTEM']->am->acquireLock($login_design->id, 'all');


	//// Now make sure that we release everything ////

	$GLOBALS['SQ_SYSTEM']->am->releaseLock($login_design->id,		'all');
	$GLOBALS['SQ_SYSTEM']->am->releaseLock($designs_folder->id,		'all');
	$GLOBALS['SQ_SYSTEM']->am->releaseLock($system_user_group->id,	'all');


	$sql = 'SELECT MAX(assetid) FROM sq_asset';
	$num_assets = $db->getOne($sql);
	if (DB::isError($num_assets)) {
		trigger_error('Could not reverve assetids for system assets, failed getting current number of assets in the system', E_USER_ERROR);
	} else {
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		for ($i = $num_assets; $i < SQ_NUM_RESERVED_ASSETIDS; $i++) {
			$assetid = $db->nextId('sq_sequence_asset');
			if (DB::isError($assetid)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				trigger_error('Could not reverve assetids for system assets, failed getting id "'.$i.'" in sequence', E_USER_ERROR);
			}
		}
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
	}

	// Re-generate the System Config to make sure that we get any new defines that may have been issued
	require_once SQ_INCLUDE_PATH.'/system_asset_config.inc';
	$sys_asset_cfg = new System_Asset_Config();
	$sys_asset_cfg->save(Array(), false);

	// From here on in, the user needs to be logged in to create assets and links
	$GLOBALS['SQ_INSTALL'] = false;

} else {

	// check for any system assets that have not yet been installed and install them if required
	require_once SQ_INCLUDE_PATH.'/system_asset_config.inc';
	$sys_asset_cfg = new System_Asset_Config();
	require_once $sys_asset_cfg->config_file;
	$updated = false;

	$GLOBALS['SQ_SYSTEM_ASSETS'] = $system_assets;
	foreach ($sys_asset_cfg->system_asset_types as $type) {
		if (!isset($system_assets[$type])) {
			$function = 'create_'.$type;
			$new_system_asset = &$function();
			if ($new_system_asset->id) $updated = true;
		}
	}

	// regen the system assets file if needed
	if ($updated) $sys_asset_cfg->save(Array(), false);

}// end if




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




//--        FUNCTIONS TO CREATE SYSTEM ASSETS        --//

/**
* Create the root folder system asset
*
* @return object Root_Folder
* @access public
*/
function &create_root_folder()
{
	$GLOBALS['SQ_SYSTEM']->am->includeAsset('root_folder');
	
	$root_folder = new Root_Folder();
	$link = Array();
	if (!$root_folder->create($link)) trigger_error('ROOT FOLDER NOT CREATED', E_USER_ERROR);
	pre_echo('Root Folder Asset Id : '.$root_folder->id);
	if ($root_folder->id != 1) {
		trigger_error('Major Problem: The new Root Folder Asset was not given assetid #1. This needs to be fixed by You, before the installation/upgrade can be completed', E_USER_ERROR);
	}
	
	$GLOBALS['SQ_SYSTEM_ASSETS']['root_folder'] = $root_folder->id;
	return $root_folder;

}//end create_root_folder()


/**
* Create the trash folder system asset
*
* @return object Trash_Folder
* @access public
*/
function &create_trash_folder()
{
	$root_folder = &$GLOBALS['SQ_SYSTEM']->am->getAsset($GLOBALS['SQ_SYSTEM_ASSETS']['root_folder']);
	
	$GLOBALS['SQ_SYSTEM']->am->includeAsset('trash_folder');
	$trash_link = Array('asset' => &$root_folder, 'link_type' => SQ_LINK_TYPE_1, 'exclusive' => 1);
	$GLOBALS['SQ_SYSTEM']->am->includeAsset('trash_folder');
	$trash_folder = new Trash_Folder();
	if (!$trash_folder->create($trash_link)) trigger_error('TRASH FOLDER NOT CREATED', E_USER_ERROR);
	pre_echo('Trash Asset Id : '.$trash_folder->id);
	
	$GLOBALS['SQ_SYSTEM_ASSETS']['trash_folder'] = $trash_folder->id;
	return $trash_folder;

}//end create_trash_folder()


/**
* Create the system management folder system asset
*
* @return object System_Management_Folder
* @access public
*/
function &create_system_management_folder()
{
	$root_folder = &$GLOBALS['SQ_SYSTEM']->am->getAsset($GLOBALS['SQ_SYSTEM_ASSETS']['root_folder']);
	
	$GLOBALS['SQ_SYSTEM']->am->includeAsset('system_management_folder');
	$system_management_folder = new System_Management_Folder();
	$system_management_folder_link = Array('asset' => &$root_folder, 'link_type' => SQ_LINK_TYPE_1, 'exclusive' => 1);
	if (!$system_management_folder->create($system_management_folder_link)) trigger_error('SYSTEM MANAGEMENT FOLDER NOT CREATED', E_USER_ERROR);
	pre_echo('System Management Asset Id : '.$system_management_folder->id);
	
	$GLOBALS['SQ_SYSTEM_ASSETS']['system_management_folder'] = $system_management_folder->id;
	return $system_management_folder;

}//end create_system_management_folder()


/**
* Create the system user group system asset
*
* @return object System_User_Group
* @access public
*/
function &create_system_user_group()
{
	$system_management_folder = &$GLOBALS['SQ_SYSTEM']->am->getAsset($GLOBALS['SQ_SYSTEM_ASSETS']['system_management_folder']);
	
	$GLOBALS['SQ_SYSTEM']->am->includeAsset('system_user_group');
	$system_user_group = new System_User_Group();
	$system_link = Array('asset' => &$system_management_folder, 'link_type' => SQ_LINK_TYPE_1, 'exclusive' => 1);
	if (!$system_user_group->create($system_link)) trigger_error('SYSTEM ADMIN GROUP NOT CREATED', E_USER_ERROR);
	pre_echo('System Administrators User Group Asset Id : '.$system_user_group->id);
	
	$GLOBALS['SQ_SYSTEM_ASSETS']['system_user_group'] = $system_user_group->id;
	return $system_user_group;

}//end create_system_user_group()


/**
* Create the root user system asset
*
* @return object Root_User
* @access public
*/
function &create_root_user()
{
	$system_user_group = &$GLOBALS['SQ_SYSTEM']->am->getAsset($GLOBALS['SQ_SYSTEM_ASSETS']['system_user_group']);
	
	$GLOBALS['SQ_SYSTEM']->am->includeAsset('root_user');
	$root_user = new Root_User();
	$user_link = Array('asset' => &$system_user_group, 'link_type' => SQ_LINK_TYPE_1);

	$root_user->setAttrValue('password',   'root');
	$root_user->setAttrValue('first_name', 'Root');
	$root_user->setAttrValue('last_name',  'User');
	$root_email = (SQ_CONF_DEFAULT_EMAIL) ? SQ_CONF_DEFAULT_EMAIL : ('root@'.((SQ_PHP_CLI) ? $_SERVER['HOSTNAME'] : $_SERVER['HTTP_HOST']));
	$root_user->setAttrValue('email', $root_email);

	if (!$root_user->create($user_link)) trigger_error('ROOT USER NOT CREATED', E_USER_ERROR);
	pre_echo('Root User Asset Id : '.$root_user->id);
	
	$GLOBALS['SQ_SYSTEM_ASSETS']['root_user'] = $root_user->id;
	return $root_user;

}//end create_root_user()


/**
* Create the cron manager system asset
*
* @return object Cron_Manager
* @access public
*/
function &create_cron_manager()
{
	$system_management_folder = &$GLOBALS['SQ_SYSTEM']->am->getAsset($GLOBALS['SQ_SYSTEM_ASSETS']['system_management_folder']);
	
	$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_manager');
	$cron_manager = new Cron_Manager();
	$cron_manager_link = Array('asset' => &$system_management_folder, 'link_type' => SQ_LINK_TYPE_1, 'exclusive' => 1);
	if (!$cron_manager->create($cron_manager_link)) trigger_error('Cron Manager NOT CREATED', E_USER_ERROR);
	pre_echo('Cron Manager Asset Id : '.$cron_manager->id);
	
	$GLOBALS['SQ_SYSTEM_ASSETS']['cron_manager'] = $cron_manager->id;
	return $cron_manager;

}//end create_cron_manager()


/**
* Create the search manager system asset
*
* @return object Search_Manager
* @access public
*/
function &create_search_manager()
{
	$system_management_folder = &$GLOBALS['SQ_SYSTEM']->am->getAsset($GLOBALS['SQ_SYSTEM_ASSETS']['system_management_folder']);
	
	$GLOBALS['SQ_SYSTEM']->am->includeAsset('search_manager');
	$search_manager = new Search_Manager();
	$search_manager_link = Array('asset' => &$system_management_folder, 'link_type' => SQ_LINK_TYPE_1, 'exclusive' => 1);
	if (!$search_manager->create($search_manager_link)) trigger_error('Search Manager NOT CREATED', E_USER_ERROR);
	pre_echo('Search Manager Asset Id : '.$search_manager->id);
	
	$GLOBALS['SQ_SYSTEM_ASSETS']['search_manager'] = $search_manager->id;
	return $search_manager;

}//end create_search_manager()


/**
* Create the layout manager system asset
*
* @return object Layout_Manager
* @access public
*/
function &create_layout_manager()
{
	$system_management_folder = &$GLOBALS['SQ_SYSTEM']->am->getAsset($GLOBALS['SQ_SYSTEM_ASSETS']['system_management_folder']);
	
	$GLOBALS['SQ_SYSTEM']->am->includeAsset('layout_manager');
	$layout_manager = new Layout_Manager();
	$layout_manager_link = Array('asset' => &$system_management_folder, 'link_type' => SQ_LINK_TYPE_1, 'exclusive' => 1);
	if (!$layout_manager->create($layout_manager_link)) trigger_error('Layout Manager NOT CREATED', E_USER_ERROR);
	pre_echo('Layout Manager Asset Id : '.$layout_manager->id);
	
	$GLOBALS['SQ_SYSTEM_ASSETS']['layout_manager'] = $layout_manager->id;
	return $layout_manager;

}//end create_layout_manager()


/**
* Create the remap manager system asset
*
* @return object Remap_Manager
* @access public
*/
function &create_remap_manager()
{
	$system_management_folder = &$GLOBALS['SQ_SYSTEM']->am->getAsset($GLOBALS['SQ_SYSTEM_ASSETS']['system_management_folder']);
	
	$GLOBALS['SQ_SYSTEM']->am->includeAsset('remap_manager');
	$remap_manager = new Remap_Manager();
	$remap_manager_link = Array('asset' => &$system_management_folder, 'link_type' => SQ_LINK_TYPE_1, 'exclusive' => 1);
	if (!$remap_manager->create($remap_manager_link)) trigger_error('Remap Manager NOT CREATED', E_USER_ERROR);
	pre_echo('Remap Manager Asset Id : '.$remap_manager->id);
	
	$GLOBALS['SQ_SYSTEM_ASSETS']['remap_manager'] = $remap_manager->id;
	return $remap_manager;

}//end create_remap_manager()


/**
* Create the designs folder system asset
*
* @return object Designs_Folder
* @access public
*/
function &create_designs_folder()
{
	$system_management_folder = &$GLOBALS['SQ_SYSTEM']->am->getAsset($GLOBALS['SQ_SYSTEM_ASSETS']['system_management_folder']);
	
	$GLOBALS['SQ_SYSTEM']->am->includeAsset('designs_folder');
	$designs_folder = new Designs_Folder();
	$designs_folder_link = Array('asset' => &$system_management_folder, 'link_type' => SQ_LINK_TYPE_1, 'exclusive' => 1);
	if (!$designs_folder->create($designs_folder_link)) trigger_error('Designs Folder NOT CREATED', E_USER_ERROR);
	pre_echo('Design Folder Asset Id : '.$designs_folder->id);
	
	$GLOBALS['SQ_SYSTEM_ASSETS']['designs_folder'] = $designs_folder->id;
	return $designs_folder;

}//end create_designs_folder()


/**
* Create the authentication folder system asset
*
* @return object Authentication_Folder
* @access public
*/
function &create_authentication_folder()
{
	$system_management_folder = &$GLOBALS['SQ_SYSTEM']->am->getAsset($GLOBALS['SQ_SYSTEM_ASSETS']['system_management_folder']);
	
	$GLOBALS['SQ_SYSTEM']->am->includeAsset('authentication_folder');
	$authentication_folder = new Authentication_Folder();
	$authentication_folder_link = Array('asset' => &$system_management_folder, 'link_type' => SQ_LINK_TYPE_1, 'exclusive' => 1);
	if (!$authentication_folder->create($authentication_folder_link)) trigger_error('Authentication Folder NOT CREATED', E_USER_ERROR);
	pre_echo('Authentication Folder Asset Id : '.$authentication_folder->id);
	
	$GLOBALS['SQ_SYSTEM_ASSETS']['authentication_folder'] = $authentication_folder->id;
	return $authentication_folder;

}//end create_authentication_folder()


/**
* Create the login design system asset
*
* @return object Login_Design
* @access public
*/
function &create_login_design()
{
	$designs_folder = &$GLOBALS['SQ_SYSTEM']->am->getAsset($GLOBALS['SQ_SYSTEM_ASSETS']['designs_folder']);
	
	$GLOBALS['SQ_SYSTEM']->am->includeAsset('login_design');
	$login_design = new Login_Design();
	$login_design_link = Array('asset' => &$designs_folder, 'link_type' => SQ_LINK_TYPE_1, 'exclusive' => 1);
	$login_design->setAttrValue('id_name', 'login_design');
	if (!$login_design->create($login_design_link)) trigger_error('Login Design NOT CREATED', E_USER_ERROR);
	pre_echo('Login Design Asset Id : '.$login_design->id);
	
	$GLOBALS['SQ_SYSTEM_ASSETS']['login_design'] = $login_design->id;
	return $login_design;

}//end create_login_design()

?>