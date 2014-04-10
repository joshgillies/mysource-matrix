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
* $Id: system_integrity_clean_cache_dir.php,v 1.6 2012/10/08 00:18:25 akarelia Exp $
*
*/

/**
* Delete cache files that exist for deleted/expired cache entries in sq_cache
*
* @author Rayn Ong <rong@squiz.net>
* @version $Revision: 1.6 $
* @package MySource_Matrix
*/
error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) {
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

echo "\nWarning: Please make sure you have the correct permission to remove cache files.\n";
echo 'SQ_CACHE_PATH is \''.SQ_CACHE_PATH."'\n\n";

// log in as root
$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	echo "ERROR: Failed login in as root user\n";
	exit();
}

$cache_path_len = strlen(SQ_CACHE_PATH) + 1;

// get all unique file paths in the sq_cache table
$cache_manager =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');
$valid_files = $cache_manager->getAllFilePaths('col');

// get all the cache directory names
exec('find '.SQ_CACHE_PATH."  -type d -name '[0-9]*'", $current_dirs);
$count = 0;
$total = 0;
// loop through each directory, to make it less memory intensive
foreach ($current_dirs as $dir) {

	$current_files = Array();
	// remove the file if there isnt a corresponding entry in the sq_cache table
	exec("find $dir -type f -name '[a-z0-9]*'", $current_files);
	foreach ($current_files as $file) {
		$file_name = substr($file, $cache_path_len);
		if (!in_array($file_name, $valid_files)) {
			$total++;
			printFileName($file_name);
			$status = @unlink(SQ_CACHE_PATH.'/'.$file_name);
			$ok = ($status) ? 'OK' : 'FAILED';
			printStatus($ok);
			if ($status) $count++;
		}
	}

}

echo "\nSummary: $count/$total cache file(s) removed.\n";
if ($count != $total) {
	$problematic = $total - $count;
	trigger_error("$problematic file(s) cannot be removed, please check file permission.", E_USER_WARNING);
}


/**
* Prints the file path to be removed
*
* @param string	$file_name	the name of the cache file
*
* @return void
* @access public
*/
function printFileName($file_name)
{
	$str = "\tRemoving ".$file_name;
	printf ('%s%'.(50 - strlen($str)).'s', $str,'');

}//end printFileName()


/**
* Prints the status of the container integrity check
*
* @param string	$status	the status of the check
*
* @return void
* @access public
*/
function printStatus($status)
{
	echo "[ $status ]\n";

}//end printStatus()


?>
