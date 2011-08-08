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
* $Id: import_files_from_bridge.php,v 1.3.2.1 2011/08/08 05:39:25 akarelia Exp $
*
*/

/**
* Import files from a file bridge
* Usage: php import_files_from_bridge.php matrix_root bridge_id parent_id recursive [y/n]
*
* @author  Benjamin Pearson <bpearson@squiz.com.au>
* @version $Revision: 1.3.2.1 $
* @package file
*/
error_reporting(E_ALL);
ini_set('display_errors', 1);
if (ini_get('memory_limit') != '-1') ini_set('memory_limit', '-1');

if ((php_sapi_name() != 'cli')) {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
	exit;
}//end if

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	trigger_error("You need to supply the path to the System Root as the first argument\n", E_USER_ERROR);
	exit;
}//end if

define('SQ_SYSTEM_ROOT', $SYSTEM_ROOT);
require_once $SYSTEM_ROOT.'/core/include/init.inc';

$root_node = (isset($_SERVER['argv'][2])) ? $_SERVER['argv'][2] : '';
if (empty($root_node) || !$GLOBALS['SQ_SYSTEM']->am->assetExists($root_node)) {
	trigger_error("You need to supply a root node to the import files from as the second argument\n", E_USER_ERROR);
	exit;
}//end if

$parent_id = (isset($_SERVER['argv'][3])) ? $_SERVER['argv'][3] : '';
if (empty($parent_id) || !$GLOBALS['SQ_SYSTEM']->am->assetExists($parent_id)) {
	trigger_error("You need to supply the parent id to the import files to as the third argument\n", E_USER_ERROR);
	exit;
}//end if

$recursive = (isset($_SERVER['argv'][4]) && strtolower($_SERVER['argv'][4]) == 'y') ? TRUE : FALSE;

// Whether to import "index.html" file as a Standard Page asset
$index_file = (isset($_SERVER['argv'][5]) && strtolower($_SERVER['argv'][5]) == 'y') ? TRUE : FALSE;

echo 'START IMPORTING'."\n";
$GLOBALS['SQ_SYSTEM']->setCurrentUser($GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user'));
$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
$vars = Array (
				'root_assetid'      => Array($root_node),
				'parent_assetid'	=> $parent_id,
				'recursive'			=> $recursive,
				'index_file'		=> $index_file,
			);
$errors = $hh->freestyleHipo('hipo_job_import_file', $vars, SQ_PACKAGES_PATH.'/filesystem/hipo_jobs');
$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
$GLOBALS['SQ_SYSTEM']->restoreCurrentUser();
echo 'FINISHED';
if (!empty($errors)) {
	echo '... with errors'."\n";
	foreach ($errors as $error) {
		echo is_array($error) ? implode($error,"\n")."\n" : $error."\n";
	}//end foreach
}//end if
echo "\n";
?>
