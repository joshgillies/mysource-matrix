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
* $Id: upgrade_menu_design_areas.php,v 1.1 2004/04/13 02:03:35 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/

/**
* Upgrade menu design areas
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/
error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) trigger_error("You can only run this script from the command line\n", E_USER_ERROR);

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	echo "ERROR: You need to supply the path to the System Root as the first argument\n";
	exit();
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';

// ask for the root password for the system
echo 'Enter the root password for "'.SQ_CONF_SYSTEM_NAME.'": ';
$root_password = rtrim(fgets(STDIN, 4094));
	
// check that the correct root password was entered
$root_user = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
if (!$root_user->comparePassword($root_password)) {
	echo "ERROR: The root password entered was incorrect\n";
	exit();
}

// log in as root
if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	trigger_error("Failed loggin in as root user\n", E_USER_ERROR);
}


$designs_to_reparse = Array();
$trash = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('trash_folder');


  ////////////////////////////////
 //  DELETE MENU DESIGN AREAS  //
////////////////////////////////
$menuids = $GLOBALS['SQ_SYSTEM']->am->getChildren('1', 'design_area_menu_type', false);
foreach ($menuids as $menuid => $type_code) {
	
	$menu = &$GLOBALS['SQ_SYSTEM']->am->getAsset($menuid);
	$id_name = $menu->attr('id_name').' (#'.$menu->id.')';
	if (strpos($id_name, '__sub_menu') !== false) continue;
	
	// if this asset is in the trash, we dont have to do it
	if ($GLOBALS['SQ_SYSTEM']->am->assetInTrash($menu->id)) continue;
	
	// if this menu design area doesnt have a sub menu design area, we dont have to do it
	$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($menu->id, SQ_LINK_TYPE_3, 'design_area_menu_type', false);
	if (empty($links)) {
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($menu);
		continue;
	}
	
	// okay, we got to here so this menu has subs - we're going for it!!
	printName($id_name);

	// work out what design this menu belongs to
	$designs = &$menu->getDesigns();
	if (empty($designs)) {
		printUpdateStatus('!!');
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($menu);
		continue;
	}

	foreach ($designs as $design) {
		// move this design area to the trash
		$design_links = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($design->id, $menu->id, SQ_LINK_TYPE_3, null, 'major', true);
		foreach ($design_links as $design_link) {
			if (!$GLOBALS['SQ_SYSTEM']->am->moveLink($design_link['linkid'], $trash->id, SQ_LINK_TYPE_2, null)) {
				printUpdateStatus('!!');
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($menu);
				continue;
			}
		}

		if ($design->type() != 'design') continue;
		$designs_to_reparse[] = $design->id;
	}
	
	printUpdateStatus('OK');
	$GLOBALS['SQ_SYSTEM']->am->forgetAsset($menu);
	
}//end foreach


  ////////////////////////////////
 //  REPARSE AFFECTED DESIGNS  //
////////////////////////////////
if (!empty($designs_to_reparse)) {
	echo "\n";
	foreach ($designs_to_reparse as $designid) {
		$design = &$GLOBALS['SQ_SYSTEM']->am->getAsset($designid);
		printName('Reparse design "'.$design->name.'"');
		
		// try to lock the design
		if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($design->id, 'parsing')) {
			printUpdateStatus('LOCK');
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($design);
			continue;
		}

		$edit_fns = $design->getEditFns();
		if (!$edit_fns->parseAndProcessFile($design)) printUpdateStatus('FAILED');

		// try to unlock the design
		if (!$GLOBALS['SQ_SYSTEM']->am->releaseLock($design->id, 'parsing')) {
			printUpdateStatus('!!');
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($design);
			continue;
		}

		printUpdateStatus('OK');
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($design);
	}
}


  ////////////////////////
 //  HELPER FUNCTIONS  //
////////////////////////
function printName($name)
{
	printf ('%s%'.(40 - strlen($name)).'s', $name, ''); 
	
}//end printName()


function printUpdateStatus($status)
{
	echo "[ $status ]\n";
	
}//end printUpdateStatus()


?>
