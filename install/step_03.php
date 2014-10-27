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
* $Id: step_03.php,v 1.88 2013/06/05 04:20:18 akarelia Exp $
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
* @version $Revision: 1.88 $
* @package MySource_Matrix
* @subpackage install
*/
ini_set('memory_limit', -1);
$SYSTEM_ROOT = '';

// from cmd line
if ((php_sapi_name() == 'cli')) {
	if (isset($_SERVER['argv'][1])) {
		$SYSTEM_ROOT = $_SERVER['argv'][1];
	}
} else {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

if (empty($SYSTEM_ROOT)) {
	$err_msg = "ERROR: You need to supply the path to the System Root as the first argument.\n";
	$err_msg .= "Usage: php install/step_03.php <PATH_TO_MATRIX>\n";
	echo $err_msg;
	exit(1);
}

if (!is_dir($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT.'/core/include/init.inc')) {
	$err_msg = "ERROR: Path provided doesn't point to a Matrix installation's System Root. Please provide correct path and try again.\n";
	$err_msg .= "Usage: php install/step_03.php <PATH_TO_MATRIX>\n";
	echo $err_msg;
	exit(1);
}

if (!defined('SQ_SYSTEM_ROOT')) {
	define('SQ_SYSTEM_ROOT',  $SYSTEM_ROOT);
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';

// firstly let's check that we are OK for the version
if (version_compare(PHP_VERSION, SQ_REQUIRED_PHP_VERSION, '<')) {
	trigger_error('<i>'.SQ_SYSTEM_LONG_NAME.'</i> requires PHP Version '.SQ_REQUIRED_PHP_VERSION.'.<br/> You may need to upgrade.<br/> Your current version is '.PHP_VERSION, E_USER_ERROR);
}

// only use console stuff if we're running from the command line
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

// check to see if the default/ tech email in main.inc are provided and are correct
// for more info see bug report 5804 Default and Tech Emails shouldnt break install
require_once SQ_FUDGE_PATH.'/general/www.inc';

$SQ_CONF_DEFAULT_EMAIL = SQ_CONF_DEFAULT_EMAIL;
if (!empty($SQ_CONF_DEFAULT_EMAIL) && !valid_email($SQ_CONF_DEFAULT_EMAIL)) {
	echo "Value '$SQ_CONF_DEFAULT_EMAIL' configued for 'SQ_CONF_DEFAULT_EMAIL' in main.inc is not valid.\nPlease fix it and try running the script again.\n";
	exit(1);
}
$SQ_CONF_TECH_EMAIL = SQ_CONF_TECH_EMAIL;
if (!empty($SQ_CONF_TECH_EMAIL) && !valid_email($SQ_CONF_TECH_EMAIL)) {
	echo "Value '$SQ_CONF_TECH_EMAIL' configured for 'SQ_CONF_TECH_EMAIL' in main.inc is not valid.\nPlease fix it and try running the script again.\n";
	exit(1);
}

// Clean up any remembered data.
require_once $SYSTEM_ROOT.'/core/include/deja_vu.inc';
$deja_vu = new Deja_Vu();
if ($deja_vu->enabled()) {
    $deja_vu->forgetAll();
}

// get the list of functions used during install
require_once $SYSTEM_ROOT.'/install/install.inc';

$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

// let everyone know we are installing
$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);

// Install all DAL core and package queries upfront
$result = install_dal_core_queries();
if (!$result) {
	trigger_error('Unable to install core dal queries.', E_USER_ERROR);
	exit(1);
}

$packages = get_package_list();
foreach ($packages as $package) {
	$result = install_dal_package_queries($package);
	if (!$result) {
		trigger_error('Unable to install queries for package '.$package, E_USER_ERROR);
		exit(1);
	}
}

// call all the steps
if (!regenerate_configs()) {
	trigger_error('Config Generation Failed', E_USER_ERROR);
}
require_once $SYSTEM_ROOT.'/install/generate_install_key.php';

// check if the $packageList variable has been defined at all
if (!isset($package_list)) $package_list = Array();

// generate the char map first, creating asset will need this
generate_lang_char_map();

generate_import_tools_manager_config();

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

echo "\n";

install_authentication_types();
generate_global_preferences();
install_event_listeners();
cache_asset_types();
generate_performance_config();
generate_file_bridge_config();
minify_css_files();

// Regenerate again now everything is installed.
if (!regenerate_configs()) {
	trigger_error('Config Generation Failed', E_USER_ERROR);
}


$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

echo "\n";
echo "Step 3 completed successfully.\n";
echo "\n";

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


?>
