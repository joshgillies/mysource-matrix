<?php
/**
* Install Step 2
*
* Purpose
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve
*/
error_reporting(E_ALL);
$SYSTEM_ROOT = '';
// from cmd line
if ((php_sapi_name() == 'cli')) {
	if (isset($_SERVER['argv'][1])) $SYSTEM_ROOT = $_SERVER['argv'][1];
	$err_msg = "You need to supply the path to the Resolve System as the first argument\n";

} else { 
	if (isset($_GET['SYSTEM_ROOT'])) $SYSTEM_ROOT = $_GET['SYSTEM_ROOT'];
	$err_msg = '
	<div style="background-color: red; color: white; font-weight: bold;">
		You need to supply the path to the Resolve System as a query string variable called SYSTEM_ROOT
	</div>
	';
}

if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	echo $err_msg;
	exit(1);
}

// Dont set SQ_INSTALL flag before this include because we want
// a complete load now that the database has been created
require_once $SYSTEM_ROOT.'/core/include/init.inc';
$db = &$GLOBALS['SQ_SYSTEM']->db;

/* INSTALL CORE */

// Let everyone know we are installing
$GLOBALS['SQ_INSTALL'] = true;

require_once(SQ_INCLUDE_PATH.'/package_manager.inc');
$pm = new Package_Manager('__core__');
if (!$pm->updatePackageDetails()) exit(1);
pre_echo("CORE PACKAGE DONE");

// Firstly let's create some Assets that we require to run

$root_folder = &$GLOBALS['SQ_SYSTEM']->am->getAsset(1, 'root_folder', true);
// if there is no root folder assume that this section hasn't been run
if (is_null($root_folder)) {

	require_once SQ_FUDGE_PATH.'/file_versioning/file_versioning.inc';
	if (!File_Versioning::initRepository(SQ_DATA_PATH.'/file_repository', $GLOBALS['SQ_SYSTEM']->db)) {
		trigger_error('Unable to initialise File Versioning Repository', E_USER_ERROR);
	}

	$GLOBALS['SQ_SYSTEM']->am->includeAsset('root_folder');
	$root_folder = new Root_Folder();
	$link = Array();
	if (!$root_folder->create($link)) die('ROOT FOLDER NOT CREATED');
	pre_echo('Root Folder Asset Id : '.$root_folder->id);
	if ($root_folder->id != 1) {
		trigger_error('Major Problem: The new Root Folder Asset was not given assetid #1. This needs to be fixed by You, before the installation/upgrade can be completed', E_USER_ERROR);
	}


	$GLOBALS['SQ_SYSTEM']->am->includeAsset('trash_folder');
	$trash_folder = new Trash_Folder();
	$trash_link = Array('asset' => &$root_folder, 'link_type' => SQ_LINK_TYPE_1, 'exclusive' => 1);
	if (!$trash_folder->create($trash_link)) die('TRASH FOLDER NOT CREATED');
	pre_echo('Trash Asset Id : '.$trash_folder->id);
	if ($trash_folder->id != 2) {
		trigger_error('Major Problem: The new Trash Asset was not given assetid #2. This needs to be fixed by You, before the installation/upgrade can be completed', E_USER_ERROR);
	}


	$GLOBALS['SQ_SYSTEM']->am->includeAsset('system_user_group');
	$system_group = new System_User_Group();
	$system_link = Array('asset' => &$root_folder, 'link_type' => SQ_LINK_TYPE_1, 'exclusive' => 1);
	if (!$system_group->create($system_link)) die('SYSTEM ADMIN GROUP NOT CREATED');
	pre_echo('System Administrators User Group Asset Id : '.$system_group->id);
	if ($system_group->id != 3) {
		trigger_error('Major Problem: The new System Administrators User Group Asset was not given assetid #3. This needs to be fixed by You, before the installation/upgrade can be completed', E_USER_ERROR);
	}

	// Create the root user
	$GLOBALS['SQ_SYSTEM']->am->includeAsset('root_user');
	$root_user = new Root_User();
	$user_link = Array('asset' => &$system_group, 'link_type' => SQ_LINK_TYPE_1);
	$root_user->setAttrValue('email', 'root@'.((SQ_PHP_CLI) ? $_SERVER['HOSTNAME'] : $_SERVER['HTTP_HOST']));
	if (!$root_user->create($user_link)) die('ROOT USER NOT CREATED');
	pre_echo('Root User Asset Id : '.$root_user->id);
	if ($root_user->id != 4) {
		trigger_error('Major Problem: The new Root User Asset was not given assetid #4. This needs to be fixed by You, before the installation/upgrade can be completed', E_USER_ERROR);
	}

	// Create the designs folder
	$GLOBALS['SQ_SYSTEM']->am->includeAsset('designs_folder');
	$designs_folder = new Designs_Folder();
	$designs_folder_link = Array('asset' => &$root_folder, 'link_type' => SQ_LINK_TYPE_1, 'exclusive' => 1);
	if (!$designs_folder->create($designs_folder_link)) die('Designs Folder NOT CREATED');
	pre_echo('Design Folder Asset Id : '.$designs_folder->id);
	if ($designs_folder->id != 7) {
		trigger_error('Major Problem: The new Designs Folder Asset was not given assetid #7. This needs to be fixed by You, before the installation/upgrade can be completed', E_USER_ERROR);
	}

	// Create the login design
	$GLOBALS['SQ_SYSTEM']->am->includeAsset('login_design');
	$login_design = new Login_Design();
	$login_design_link = Array('asset' => &$designs_folder, 'link_type' => SQ_LINK_TYPE_1, 'exclusive' => 1);
	$login_design->setAttrValue('id_name', 'login_design');
	if (!$login_design->create($login_design_link)) die('Login Design NOT CREATED');
	pre_echo('Login Design Asset Id : '.$login_design->id);
	if ($login_design->id != 8) {
		trigger_error('Major Problem: The new Login Design Asset was not given assetid #8. This needs to be fixed by You, before the installation/upgrade can be completed', E_USER_ERROR);
	}

	// Create the cron manager
	$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_manager');
	$cron_manager = new Cron_Manager();
	$cron_manager_link = Array('asset' => &$root_folder, 'link_type' => SQ_LINK_TYPE_1, 'exclusive' => 1);
	if (!$cron_manager->create($cron_manager_link)) die('Cron Manager NOT CREATED');
	pre_echo('Cron Manager Asset Id : '.$cron_manager->id);
	if ($cron_manager->id != 10) {
		trigger_error('Major Problem: The new Cron Manager Asset was not given assetid #10. This needs to be fixed by You, before the installation/upgrade can be completed', E_USER_ERROR);
	}

	$cron_manager->releaseLock();
	$login_design->releaseLock();
	$designs_folder->releaseLock();
	$root_user->releaseLock();
	$system_group->releaseLock();
	$trash_folder->releaseLock();
	$root_folder->releaseLock();

	$sql = 'SELECT MAX(assetid) FROM sq_asset';
	$num_assets = $db->getOne($sql);
	if (DB::isError($num_assets)) {
		trigger_error('Could not reverve assetids for system assets, failed getting current number of assets in the system', E_USER_ERROR);
	} else {
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		for ($i = $num_assets; $i < SQ_NUM_RESERVED_ASSETIDS; $i++) {
			$assetid = $db->nextId('sq_sequence_asset');
			if (DB::isError($assetid)) {
				trigger_error('Could not reverve assetids for system assets, failed getting id "'.$i.'" in sequence', E_USER_ERROR);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			}
		}
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
	}
	
	// From here on in, the user needs to be logged in to create assets and links
	$GLOBALS['SQ_INSTALL'] = false;

	// set the current user object to the root user so we can finish
	// the install process without permission denied errors
	$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user);

}// end if

/* INSTALL PACKAGES */

// Right now that we have sorted all that out we can install the packages
$d = dir(SQ_PACKAGES_PATH);
while (false !== ($entry = $d->read())) {
	if ($entry == '.' || $entry == '..') continue;
	# if this is a directory, process it
	if (is_dir(SQ_PACKAGES_PATH.'/'.$entry)) {
		$pm = new Package_Manager($entry);
		if ($pm->package) {
			$result = $pm->updatePackageDetails();
			pre_echo(strtoupper($entry)." PACKAGE DONE");
		}
	}
}
$d->close();

?>