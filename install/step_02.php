<?php

if (empty($_GET['SYSTEM_ROOT']) || !is_dir($_GET['SYSTEM_ROOT'])) {
	?>
		<div style="background-color: red; color: white; font-weight: bold;">
			You need to supply the path to the Resolve System as a query string variable called SYSTEM_ROOT
		</div>
	<?php
	exit(1);
}


require_once $_GET['SYSTEM_ROOT'].'/core/include/init.inc';
$GLOBALS['SQ_SYSTEM']->am = new Asset_Manager();

/* INSTALL CORE */

// Create any necessary sequences
$GLOBALS['SQ_SYSTEM']->db->createSequence('sq_sequence_asset');
$GLOBALS['SQ_SYSTEM']->db->createSequence('sq_sequence_asset_link');
$GLOBALS['SQ_SYSTEM']->db->createSequence('sq_sequence_asset_attribute');
$GLOBALS['SQ_SYSTEM']->db->createSequence('sq_sequence_asset_url');
$GLOBALS['SQ_SYSTEM']->db->createSequence('sq_sequence_internal_message');

require_once(SQ_INCLUDE_PATH.'/package_manager.inc');
$pm = new Package_Manager('__core__');
if (!$pm->updatePackageDetails()) exit(1);
echo "CORE PACKAGE DONE<br>";

// Firstly let's create some Assets that we require to run

$root_folder = &$GLOBALS['SQ_SYSTEM']->am->getAsset(1, 'root_folder', true);
// if there is no root folder assume that this section hasn't been run
if (is_null($root_folder)) {

	$GLOBALS['SQ_SYSTEM']->am->includeAsset('root_folder');
	$root_folder = new Root_Folder();
	if (!$root_folder->create(Array())) die();
	pre_echo('Root Folder Asset Id : '.$root_folder->id);
	if ($root_folder->id != 1) {
		trigger_error('Major Problem: The new Root Folder Asset was not given assetid #1. This needs to be fixed by You, before the installation/upgrade can be completed', E_USER_ERROR);
	}

	$GLOBALS['SQ_SYSTEM']->am->includeAsset('root_user');
	$root_user = new Root_User();
	$root_user->id = 2;

	// set the current user object to the root user so we can finish
	// the install process without permission denied errors
	$GLOBALS['SQ_SYSTEM']->user = $root_user;

	// Create some useful folders
	$users_folder = new Folder();
	$root_link = Array('asset' => &$root_folder, 'link_type' => SQ_LINK_UNITE);
	if (!$users_folder->create($root_link, 'Users')) die();
	$users_folder->createLink($root_user, SQ_LINK_UNITE);


	pre_echo('Root User Asset Id : '.$root_user->id);
	if ($root_user->id != 2) {
		trigger_error('Major Problem: The new Root User Asset was not given assetid #2. This needs to be fixed by You, before the installation/upgrade can be completed', E_USER_ERROR);
	}
	if (!$root_user->create(Array(), 'root@'.$_SERVER['HTTP_HOST'])) die();


	$GLOBALS['SQ_SYSTEM']->am->includeAsset('trash_folder');
	$trash_folder = new Trash_Folder();
	$trash_link = Array('asset' => &$root_folder, 'link_type' => SQ_LINK_EXCLUSIVE);
	if (!$trash_folder->create($trash_link)) die();
	pre_echo('Trash Asset Id : '.$trash_folder->id);
	if ($trash_folder->id != 3) {
		trigger_error('Major Problem: The new Trash Asset was not given assetid #3. This needs to be fixed by You, before the installation/upgrade can be completed', E_USER_ERROR);
	}


	$GLOBALS['SQ_SYSTEM']->am->includeAsset('system_user_group');
	$system_group = new System_User_Group();
	$system_link = Array('asset' => &$root_folder, 'link_type' => SQ_LINK_EXCLUSIVE);
	if (!$system_group->create($system_link)) die();
	pre_echo('System Administrators User Group Asset Id : '.$system_group->id);
	if ($system_group->id != 4) {
		trigger_error('Major Problem: The new System Administrators User Group Asset was not given assetid #4. This needs to be fixed by You, before the installation/upgrade can be completed', E_USER_ERROR);
	}
	$system_group->grantPermission($root_user->id, 'A');

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
			echo strtoupper($entry)." PACKAGE DONE<br>";
		}
	}
}
$d->close();


?>
