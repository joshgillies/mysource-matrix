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
* $Id: system_integrity_clear_cache_dir.php,v 1.1 2006/07/28 04:20:18 rong Exp $
*
*/

/**
* Delete cache files that exist for deleted/expired cache entries in sq_cache
*
* @author Rayn Ong <rong@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix
*/
error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	echo "ERROR: You need to supply the path to the System Root as the first argument\n";
	exit();
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';

echo "\nWarning: Please make sure you have the correct permission to remove cache files.\n";
echo 'SQ_CACHE_PATH is \''.SQ_CACHE_PATH."'\n\n";
// ask for the root password for the system
echo 'Enter the root password for "'.SQ_CONF_SYSTEM_NAME.'": ';
$root_password = rtrim(fgets(STDIN, 4094));

// check that the correct root password was entered
$root_user = & $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
if (!$root_user->comparePassword($root_password)) {
	echo "ERROR: The root password entered was incorrect\n";
	exit();
}

// log in as root
if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	trigger_error("Failed login in as root user\n", E_USER_ERROR);
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
