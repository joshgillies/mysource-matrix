<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: upgrade_safe_editing_for_thumbnails.php,v 1.1 2012/02/22 23:32:47 akarelia Exp $
*
*/

/**
* Purpose : 
*			This script will pick up all the assets that are in safe edit 
* 			and check if they have thumbnails on it. If they do it will
*			write the link information to the .sq_notice_links file
*
*
* @author Ash Karelia <akarelia@squiz.com.au>
* @version $Revision: 1.1 $
* @package MySource_Matrix
*/
error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	trigger_error("You need to supply the path to the System Root as the first argument\n", E_USER_ERROR);
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';

// log in as root
$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	trigger_error("Failed logging in as root user\n", E_USER_ERROR);
}

// forced run level
$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);


$am = $GLOBALS['SQ_SYSTEM']->am;
$count = 0;
$done_asset = 1;
$bad_assetids = Array();

// get all the assets in the system who are in safe edit status and have a thumbnail attached to it
$sql = "SELECT assetid from sq_ast where status = '64' and assetid IN (select majorid from sq_ast_lnk where value = 'thumbnail');";

$results = MatrixDAL::executeSqlAssoc($sql);
echo "Found ".count($results)." assets that are in 'Safe Editing' status and has thumbnail applied.\n";

foreach ($results as $result) {
	$content = Array();
	echo "Updating $done_asset of ".count($results)." Assets\r";
	$assetid = $result['assetid'];
	$asset   = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
	$notice_link = $GLOBALS['SQ_SYSTEM']->am->getLink($assetid, SQ_LINK_NOTICE, '', TRUE, 'thumbnail');

	// if file exists, write to it else create new one
	if (file_exists($asset->data_path.'/.sq_system/.sq_notice_links')) {
		$content = unserialize(file_to_string($asset->data_path.'/.sq_system/.sq_notice_links'));
		$content[] = $notice_link;
		if (!string_to_file(serialize($content), $asset->data_path.'/.sq_system/.sq_notice_links')) {
			$count++;
			$bad_assetids[] = $assetid;
		}
	} else {
		$content[] = $notice_link;
		if (!string_to_file(serialize($content), $asset->data_path.'/.sq_system/.sq_notice_links')) {
			$count++;
			$bad_assetids[] = $assetid;
		}
	}
	$done_asset++;

	unset($assetid);
}


// Warn the user if an error happened
if (!empty($count)) {
	echo "There were $count errors in creating .sq_notice_links files.\nThese assets may need to be reviewed manually.\n";
	echo "Assetids that need to be looked into (These are in error.log too):\n";
	print_r($bad_assetids);

	// put them in error.log too
	log_dump("There were ".$count." errors in creating .sq_notice_links files.\nThese assets may need to be reviewed manually.");
	log_dump("Assetids that need to be looked into :");
	log_dump($bad_assetids);
} else {
	echo "Successfully upgraded ".count($results)." assets.\n";
}

$GLOBALS['SQ_SYSTEM']->restoreRunLevel();


?>
