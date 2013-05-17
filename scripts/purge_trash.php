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
* $Id: purge_trash.php,v 1.9.4.2 2013/05/17 09:56:46 cupreti Exp $
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
* @version $Revision: 1.9.4.2 $
* @package MySource_Matrix
*/

error_reporting(E_ALL);
if (ini_get('memory_limit') != '-1') ini_set('memory_limit', '-1');

if (php_sapi_name() != 'cli') {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT)) {
	echo "ERROR: You need to supply the path to the System Root as the first argument\n";
	exit();
}

if (!is_dir($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT.'/core/include/init.inc')) {
	echo "ERROR: Path provided doesn't point to a Matrix installation's System Root. Please provide correct path and try again.\n";
	exit();
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';
require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	echo "ERROR: Failed logging in as root user\n";
	exit();
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
	} else {
		echo "ERROR: Purge root node assetid id #".$purge_rootnode." not be found\n";
		exit(1);
	}
}

$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
$errors = $hh->freestyleHipo('hipo_job_purge_trash', $vars);
if (count($errors)) {
	trigger_error(print_r($errors, TRUE), E_USER_WARNING);
	exit(1);
}

?>
