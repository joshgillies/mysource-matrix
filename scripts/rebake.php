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
* $Id: rebake.php,v 1.8 2013/05/27 08:58:50 cupreti Exp $
*
*/


/**
* DAL Install Queries ("Rebake") Script
*
* Installs DAL queries packages into the MySource system. Installing (or
* "baking") involves taking the queries specified by the "queries.xml" file
* in the core, packages and assets and generating an SQL representation
* which is (in theory) tailored towards the database system in question.
*
* The script takes one parameter - the root of the MySource installation.
*
*    php install_queries.php /system/root
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.8 $
* @package MySource_Matrix
* @subpackage install
*/
if (ini_get('memory_limit') != '-1') ini_set('memory_limit', '-1');
error_reporting(E_ALL);
$SYSTEM_ROOT = '';

// from cmd line
if ((php_sapi_name() == 'cli')) {
	if (isset($_SERVER['argv'][1])) {
		$SYSTEM_ROOT = $_SERVER['argv'][1];
	}
	$err_msg = "You need to supply the path to the System Root as the first argument\n";
} else {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

if (empty($SYSTEM_ROOT)) {
	echo $err_msg;
	exit();
}

if (!is_dir($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT.'/core/include/init.inc')) {
	echo "ERROR: Path provided doesn't point to a Matrix installation's System Root. Please provide correct path and try again.\n";
	exit();
}

if (!defined('SQ_SYSTEM_ROOT')) {
	define('SQ_SYSTEM_ROOT',  $SYSTEM_ROOT);
}

// Include init first so it can set the right error_reporting levels.
require_once $SYSTEM_ROOT.'/core/include/init.inc';

require_once 'Console/Getopt.php';

$shortopt = '';
$longopt = Array('package=');

$con = new Console_Getopt;
$args = $con->readPHPArgv();
array_shift($args);
$options = $con->getopt($args, $shortopt, $longopt);

if (is_array($options[0])) {
    $package_list = get_console_list($options[0]);
}

// get the list of functions used during install
require_once $SYSTEM_ROOT.'/install/install.inc';

$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

// firstly let's check that we are OK for the version
if (version_compare(PHP_VERSION, SQ_REQUIRED_PHP_VERSION, '<')) {
	trigger_error('<i>'.SQ_SYSTEM_LONG_NAME.'</i> requires PHP Version '.SQ_REQUIRED_PHP_VERSION.'.<br/> You may need to upgrade.<br/> Your current version is '.PHP_VERSION, E_USER_ERROR);
}

$old_path = ini_get('include_path');
ini_set('include_path', SQ_LIB_PATH);
require_once SQ_LIB_PATH.'/MatrixDAL/MatrixDALBaker.inc';

$packages = $GLOBALS['SQ_SYSTEM']->getInstalledPackages();

$asset_sql = 'SELECT type_code FROM sq_ast_typ';
$asset_types = MatrixDAL::executeSqlAssoc($asset_sql, 0);

print_status_name('Installing queries for MySource Matrix core...');
try {
	MatrixDALBaker::addCoreQueries();
	MatrixDALBaker::bakeQueriesFile('core');
	print_status_result('OK');
} catch (Exception $e) {
	print_status_result('FAILED');
	echo '(Exception: '.$e->getMessage().')'."\n";
	exit(1);
}

if (count($packages) === 0) {
	print_status_name('No packages currently installed...');
	print_status_result('SKIPPED');
} else {
	print_status_name('Installing queries for installed packages ('.count($packages).' packages)...');
	try {
		foreach	($packages as $package) {
			$package_name = $package['code_name'];
			if ($package_name == '__core__') {
				$package_name = 'core';
			}
			MatrixDALBaker::addPackageQueries($package_name);
			MatrixDALBaker::bakeQueriesFile($package_name.'_package');
		}
		print_status_result('OK');
	} catch (Exception $e) {
		print_status_result('FAILED');
		echo '(Exception at '.$package_name.' package: '.$e->getMessage().')'."\n";
		exit(1);
	}
}

if (count($asset_types) === 0) {
	print_status_name('No assets currently installed...');
	print_status_result('SKIPPED');
} else {
	print_status_name('Installing queries for installed assets ('.count($asset_types).' assets)...');
	try {
		foreach	($asset_types as $type_code) {
			MatrixDALBaker::addAssetTypeQueries($type_code);
			MatrixDALBaker::bakeQueriesFile($type_code);
		}
		print_status_result('OK');
	} catch (Exception $e) {
		print_status_result('FAILED');
		echo '(Exception at '.$type_code.' asset: '.$e->getMessage().')'."\n";
		exit(1);
	}
}

ini_set('include_path', $old_path);
pre_echo('Query Installation Complete');
exit(0);




/**
* Gets a list of supplied package options from the command line arguments given
*
* Returns an array in the format needed for package_list
*
* @param array	$options	the options as retrieved from Console::getopts
*
* @return array
* @access public
*/
function get_console_list($options)
{
	$list = Array();

	foreach ($options as $option) {
		// if nothing set, skip this entry
		if (!isset($option[0]) || !isset($option[1])) {
			continue;
		}

		if ($option[0] != '--package') continue;

		// Now process the list
		$parts = explode('-', $option[1]);

		$types = Array();
		if (count($parts) == 2 && strlen($parts[1])) {
			$types = explode(',', $parts[1]);
		}

		$list[$parts[0]] = $types;
	}

	return $list;

}//end get_console_list()


/**
* Prints a name of a status checkpoint, padded to 60 characters.
*
* @param string	$name	the text to print
*
* @return void
* @access public
*/
function print_status_name($name)
{
	printf ('%s%'.(60 - strlen($name)).'s', $name, '');

}//end printWYSIWYGName()


/**
* Prints the status' result. To be used with print_status_name().
*
* @param string	$status	the status result to print
*
* @return void
* @access public
*/
function print_status_result($status)
{
	echo "[ $status ]\n";

}//end printUpdateStatus()


?>
