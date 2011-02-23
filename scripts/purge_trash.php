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
* $Id: purge_trash.php,v 1.4.12.1 2011/02/23 23:49:03 mhaidar Exp $
*
*/

/**
* Use this script to clear the trash
*
* Usage: php scripts/purge_trash.php [SYSTEM ROOT] [PURGE_ROOTNODE]
* Runs a Freestyle HIPO that purges all assets from the trash
* Best suited to be run at a scheduled time by cron or similar.
*
* Added: optional argument PURGE_ROOTNODE,
*        all assets underneath this rootnode (inclusive) will be purged from the trash folder.
*        useful when the system runs out of memory when purging all assets
*
* @version $Revision: 1.4.12.1 $
* @package MySource_Matrix
*/

error_reporting(E_ALL);
if (php_sapi_name() != 'cli') {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	trigger_error("You need to supply the path to the System Root as the first argument\n", E_USER_ERROR);
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';
require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

error_reporting(E_ALL);
ini_set('memory_limit', '-1');

// ask for the root password for the system
echo 'Enter the root password for "'.SQ_CONF_SYSTEM_NAME.'": ';
$root_password = rtrim(fgets(STDIN, 4094));

// check that the correct root password was entered
$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
if (!$root_user->comparePassword($root_password)) {
	trigger_error("The root password entered was incorrect\n", E_USER_ERROR);
}

if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	trigger_error("Failed logging in as root user\n", E_USER_ERROR);
}

$vars = Array();

// if a second argument is supplied
$purge_rootnode = (isset($_SERVER['argv'][2])) ? $_SERVER['argv'][2] : 0;
if (!empty($purge_rootnode)) {
	// do some checking to make sure there is a link to the trash folder
	$trash_folder = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('trash_folder');
	$db = $GLOBALS['SQ_SYSTEM']->db;
	$sql = 'select
				linkid
			from
				sq_ast_lnk
			where
				minorid = :root_node
				and
				majorid = :trash_assetid';
				
	$query = MatrixDAL::preparePdoQuery($sql);
	MatrixDAL::bindValueToPdo($query, 'root_node',     $purge_rootnode);
	MatrixDAL::bindValueToPdo($query, 'trash_assetid', $trash_folder->id);
	$linkid = MatrixDAL::executePdoOne($query);
	
	if (!empty($linkid)) {
		// purge trash hipo will know what to do
		$vars['purge_root_linkid'] = $linkid;
	}
}

$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
$errors = $hh->freestyleHipo('hipo_job_purge_trash', $vars);
if (count($errors)) {
	trigger_error(print_r($errors, TRUE), E_USER_WARNING);
	exit(1);
} else {
	echo "\npurge_trash.php: Completed.\n";
}

?>
