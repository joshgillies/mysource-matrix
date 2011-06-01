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
* $Id: step_03.php,v 1.73 2008/04/02 23:29:09 lwright Exp $
*
*/


/**
* Install Step 3
*
* Installs packages into the MySource system. You can optionally specify what
* packages and assets to run the script for in the following manner:
*
*    php step_03.php /system/root --package=packagename[-assettype,assettype,assettype]
*
* You may specify several --package= entries. If the packagename is followed by
* a hyphen, entries after the hyphen will be taken to be asset types.
*
*    php step_03.php /system/root --package=core-page,page_standard
*
* would only update the page and page_standard assets within the core package
*
*    php step_03.php /system/root --package=core --package=cms
*
* would update all the asset types for core and cms only
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.73 $
* @package MySource_Matrix
* @subpackage install
*/
ini_set('memory_limit', -1);
error_reporting(E_ALL);
$SYSTEM_ROOT = '';

$cli = TRUE;

// from cmd line
if ((php_sapi_name() == 'cli')) {
	if (isset($_SERVER['argv'][1])) {
		$SYSTEM_ROOT = $_SERVER['argv'][1];
	}
	$err_msg = "You need to supply the path to the System Root as the first argument\n";

} else {
	$cli = FALSE;
	if (isset($_GET['SYSTEM_ROOT'])) {
		$SYSTEM_ROOT = $_GET['SYSTEM_ROOT'];
	}
	$err_msg = '
	<div style="background-color: red; color: white; font-weight: bold;">
		You need to supply the path to the System Root as a query string variable called SYSTEM_ROOT
	</div>
	';
}

if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	trigger_error($err_msg, E_USER_ERROR);
}


// only use console stuff if we're running from the command line
if ($cli) {
	require_once 'Console/Getopt.php';

	$shortopt = '';
	$longopt = Array('package=');

	$con =& new Console_Getopt;
	$args = $con->readPHPArgv();
	array_shift($args);
	$options = $con->getopt($args, $shortopt, $longopt);

	if (is_array($options[0])) {
		$package_list = get_console_list($options[0]);
	}
}

require_once $SYSTEM_ROOT.'/install/generate_install_key.php';
if (!defined('SQ_SYSTEM_ROOT')) {
	define('SQ_SYSTEM_ROOT',  $SYSTEM_ROOT);
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';

// get the list of functions used during install
require_once $SYSTEM_ROOT.'/install/install.inc';

$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

// firstly let's check that we are OK for the version
if (version_compare(PHP_VERSION, SQ_REQUIRED_PHP_VERSION, '<')) {
	trigger_error('<i>'.SQ_SYSTEM_LONG_NAME.'</i> requires PHP Version '.SQ_REQUIRED_PHP_VERSION.'.<br/> You may need to upgrade.<br/> Your current version is '.PHP_VERSION, E_USER_ERROR);
}

// let everyone know we are installing
$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);

// Install all DAL core and package queries upfront
install_dal_core_queries();
$packages = get_package_list();
foreach ($packages as $package) {
	install_dal_package_queries($package);
}

// call all the steps
if (!regenerate_configs()) {
	trigger_error('Config Generation Failed', E_USER_ERROR);
}

// check if the $packageList variable has been defined at all
if (!isset($package_list)) $package_list = Array();

uninstall_asset_types();
uninstall_packages();

install_core($package_list);
$deferred = install_packages($package_list);
// if there were deferred packages, try to reinstall them.
if (is_array($deferred)) {
	// try and install the deferred packages again in a loop until the result
	// package is the same as the install package, at which point we know
	// the dependency has failed.
	$deferred = install_deferred($deferred);
	if (is_array($deferred)) {
		trigger_error('The following assets could not be installed due to dependency failures (see previous warnings for details): '."\n".format_deferred_packages($deferred), E_USER_ERROR);
	}
}

install_authentication_types();
generate_global_preferences();
install_event_listeners();
cache_asset_types();

$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();


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
		$parts = split('-', $option[1]);

		$types = Array();
		if (count($parts) == 2 && strlen($parts[1])) {
			$types = split(',', $parts[1]);
		}

		$list[$parts[0]] = $types;
	}

	return $list;

}//end get_console_list()


?>
