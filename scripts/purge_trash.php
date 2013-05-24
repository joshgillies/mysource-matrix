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
* $Id: purge_trash.php,v 1.13 2013/05/24 02:21:52 cupreti Exp $
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
* @version $Revision: 1.13 $
* @package MySource_Matrix
*/

error_reporting(E_ALL);
if (ini_get('memory_limit') != '-1') ini_set('memory_limit', '-1');

if (php_sapi_name() != 'cli') {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT)) {
	printStdErr("You need to supply the path to the System Root as the first argument\n");
	exit(1);
}

if (!is_dir($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT.'/core/include/init.inc')) {
	printStdErr("Path provided doesn't point to a Matrix installation's System Root. Please provide correct path and try again.\n");
	exit(1);
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';
require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	printStdErr("Failed logging in as root user\n");
	exit(1);
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
		printStdErr("Purge root node assetid id #".$purge_rootnode." not found\n");
		exit(1);
	}
}

$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
$errors = $hh->freestyleHipo('hipo_job_purge_trash', $vars);
if (!empty($errors)) {
	$error_msg = '';
	foreach($errors as $error) {
		$error_msg .= ' * '.$error['message'];
	}
	printStdErr("Following errors occured while deleting asset(s):\n$error_msg\n");
	exit(1);
}

/**
* Prints the supplied string to "standard error" (STDERR) instead of the "standard output" (STDOUT) stream
*
* @param string $string The string to write to STDERR
*
* @return void
* @access public
*/
function printStdErr($string)
{
	fwrite(STDERR, "$string");

}//end printStdErr()


?>
