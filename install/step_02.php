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
$GLOBALS['SQ_SYSTEM']->db->createSequence('sq_sequence_asset_permission');
$GLOBALS['SQ_SYSTEM']->db->createSequence('sq_sequence_internal_message');

// Let everyone know we are installing
$GLOBALS['SQ_INSTALL'] = true;

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
	$root_user->setAttrValue('email', 'root@'.$_SERVER['HTTP_HOST']);
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

	$login_design->releaseLock();
	$designs_folder->releaseLock();
	$root_user->releaseLock();
	$system_group->releaseLock();
	$trash_folder->releaseLock();
	$root_folder->releaseLock();

	// now we need to reserve some assetids for system assets in the future
	// so work out how many assets we currently have in the system
	$db = &$GLOBALS['SQ_SYSTEM']->db;
	
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
	}
	
	// From here on in, the user needs to be logged in to create assets and links
	$GLOBALS['SQ_INSTALL'] = false;

	// set the current user object to the root user so we can finish
	// the install process without permission denied errors
	$GLOBALS['SQ_SYSTEM']->user = $root_user;

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
