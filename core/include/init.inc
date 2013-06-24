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
* $Id: init.inc,v 1.126.2.1 2013/06/24 23:53:29 akarelia Exp $
*
*/

/**
* Init File
*
* The file that needs to be included to do anything
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.126.2.1 $
* @package MySource_Matrix
*/



/**
 * This code block came from http://www.php.net/phpversion
 */
if (!defined('PHP_VERSION_ID')) {
	$version = explode('.', PHP_VERSION);
	define('PHP_VERSION_ID', ($version[0] * 10000 + $version[1] * 100 + $version[2]));
}


// set the level of PHP reported errors and some other
// PHP thingies we want done OUR way
if (PHP_VERSION_ID < 50300) {
	// pear http/client module contains deprecated syntax which will cause trouble
	// E_DEPRECATED is introduced in PHP 5.3 and included in E_ALL, so has to remove E_DEPRECATED for php 5.3 and above
	error_reporting(E_ALL);
} else if (PHP_VERSION_ID >= 50400) {
	error_reporting(E_ALL ^ E_DEPRECATED ^ E_STRICT);
} else {
	error_reporting(E_ALL ^ E_DEPRECATED);
}

# Before 5.3.0 we have to set magic quotes runtime off
if (PHP_VERSION_ID < 50300) {
	set_magic_quotes_runtime(0);
}

// we need this so that our error handler won't kill itself attempting to try and remove
// all existing buffers - especially important when Zend Performance Suite is installed because
// it adds another OB level
define('SQ_INIT_OB_LEVEL', ob_get_level());

// are we running from the command line ?
define('SQ_PHP_CLI', (php_sapi_name() == 'cli'));


// Let's get our bearings as to where everything is from here.
// These paths may be relative or absolute
if (!defined('SQ_SYSTEM_ROOT')) {
	define('SQ_SYSTEM_ROOT', dirname(dirname(dirname(realpath(__FILE__)))));
}
define('SQ_INCLUDE_PATH',      SQ_SYSTEM_ROOT.'/core/include');
define('SQ_CORE_PACKAGE_PATH', SQ_SYSTEM_ROOT.'/core/assets');
define('SQ_ATTRIBUTES_PATH',   SQ_SYSTEM_ROOT.'/core/attributes');
define('SQ_LIB_PATH',          SQ_SYSTEM_ROOT.'/core/lib');
define('SQ_DATA_PATH',         SQ_SYSTEM_ROOT.'/data');
define('SQ_CACHE_PATH',        SQ_SYSTEM_ROOT.'/cache');
define('SQ_PACKAGES_PATH',     SQ_SYSTEM_ROOT.'/packages');
define('SQ_WEB_PATH',          SQ_SYSTEM_ROOT.'/core/web');
define('SQ_FUDGE_PATH',        SQ_SYSTEM_ROOT.'/fudge');
define('SQ_TEMP_PATH',         SQ_SYSTEM_ROOT.'/data/temp');
define('SQ_LOG_PATH',          SQ_SYSTEM_ROOT.'/data/private/logs');


// include the system version setttings
require_once SQ_INCLUDE_PATH.'/version.inc';

// include the main system config file
require_once SQ_DATA_PATH.'/private/conf/main.inc';
if (!SQ_PHP_CLI) {
	ini_set('memory_limit', SQ_CONF_WEB_MEMORY_LIMIT.'M');
}

// This sets the default timezone so date functions don't throw E_STRICT.
if (defined('SQ_CONF_TIMEZONE')) {
	date_default_timezone_set(SQ_CONF_TIMEZONE);
}

// include installation key if it exists
if (file_exists(SQ_DATA_PATH.'/private/conf/licence.inc')) {
	require_once SQ_DATA_PATH.'/private/conf/licence.inc';
}

// To be able to use symlinks without MySource Matrix tripping over the index.php, strip
// it out of the PHP_SELF variable (pretend we're someone we're not). When
// the symlinked directory gets redirected by the .htaccess file, it has
// the index.php appended to the url, which causes all sorts of problems for
// the system as it expects php_self to be just a name, as it is normally
// aliased to the index.php directly. This will
// quite probably need to be changed to a function like get_real_self()

// if there's a index.php/ that's not at the end (ie it has a trailing slash) in the
// php_self variable, remove it as it breaks everything (symlinks only).
if (strrpos($_SERVER['PHP_SELF'], 'index.php/')) {
	$_SERVER['PHP_SELF'] = str_replace('index.php/','', $_SERVER['PHP_SELF']);
}

// sanitize PHP_SELF against XSS attack <script> should never gets to PHP_SELF
$_SERVER['PHP_SELF'] = htmlspecialchars($_SERVER['PHP_SELF']);

// Load general everyday handy functions for developers
require_once SQ_FUDGE_PATH.'/dev/dev.inc';
require_once SQ_FUDGE_PATH.'/general/general.inc';

// nice little boolean to use for testing whether we happen to be in the backend or not
$in_backend = (trim(SQ_CONF_BACKEND_SUFFIX) != '') && (basename($_SERVER['PHP_SELF']) == SQ_CONF_BACKEND_SUFFIX);
define('SQ_IN_BACKEND', $in_backend);

// nice little boolean to use for testing whether we happen to be in limbo (frontend editing) or not
$in_limbo = (trim(SQ_CONF_LIMBO_SUFFIX) != '') && (basename($_SERVER['PHP_SELF']) == SQ_CONF_LIMBO_SUFFIX);
define('SQ_IN_LIMBO', $in_limbo);

// nice little boolean to use for testing whether we happen to be in login interface or not
$in_login = (trim(SQ_CONF_LOGIN_SUFFIX) != '') && (basename($_SERVER['PHP_SELF']) == SQ_CONF_LOGIN_SUFFIX);
define('SQ_IN_LOGIN', $in_login);

// we are in performance results view
$in_performance = defined('SQ_CONF_PERFORMANCE_SUFFIX') && (trim(SQ_CONF_PERFORMANCE_SUFFIX) != '') && (basename($_SERVER['PHP_SELF']) == SQ_CONF_PERFORMANCE_SUFFIX);
define('SQ_IN_PERFORMANCE', $in_performance);

// we are timing performance data
$in_performance_timing = defined('SQ_CONF_PERFORMANCE_TIMING_SUFFIX') && (trim(SQ_CONF_PERFORMANCE_TIMING_SUFFIX) != '') && (basename($_SERVER['PHP_SELF']) == SQ_CONF_PERFORMANCE_TIMING_SUFFIX);
if($in_performance_timing)
    define('SQ_IN_PERFORMANCE_TIMING', $in_performance_timing);

// we are showing performance result
$in_performance_result = defined('SQ_CONF_PERFORMANCE_RESULT_SUFFIX') && (trim(SQ_CONF_PERFORMANCE_RESULT_SUFFIX) != '') && (basename($_SERVER['PHP_SELF']) == SQ_CONF_PERFORMANCE_RESULT_SUFFIX);
define('SQ_IN_PERFORMANCE_RESULT', $in_performance_result);

// boolean to use for testing whether we're running cron scripts
if (!defined('SQ_IN_CRON')) {
	define('SQ_IN_CRON', FALSE);
}

// make sure there is a trailing slash if we are in the backend
if (SQ_IN_BACKEND && substr($_SERVER['PHP_SELF'], -1) != '/') {
	header('Location: '.$_SERVER['PHP_SELF'].'/'.(($_SERVER['QUERY_STRING']) ? '?'.$_SERVER['QUERY_STRING'] : ''));
	exit();
}


// the current output type for the system, can be changed during run-time (eg to XML)
// main use is so that the error_reporter doesn't error in HTML to something expecting XML
if (!isset($GLOBALS['SQ_OUTPUT_TYPE'])) {
	$GLOBALS['SQ_OUTPUT_TYPE'] = (SQ_PHP_CLI) ? 'text' : 'html';
}


// we are not purging the trash
$GLOBALS['SQ_PURGING_TRASH'] = FALSE;

// we are not reverting to a system version
$GLOBALS['SQ_REVERT_TO_SYSTEM_VERSION'] = FALSE;


/*
* LINK TYPES - way in which assets can be associated with each other
*/
define('SQ_LINK_TYPE_1', 1); // link is displayed on frontend and backend navigation
define('SQ_LINK_TYPE_2', 2); // same power/rank as a TYPE_1 link but is only displayed on backend navigation
define('SQ_LINK_TYPE_3', 4); // same power/rank as a TYPE_1 link, but is not displayed on any navigation
define('SQ_LINK_NOTICE', 8); // for information purposes only, (eg register HREF's from one asset to another)

define('SQ_SC_LINK_ALL',          15); // short hand what all the main link types add up to
define('SQ_SC_LINK_FRONTEND_NAV',  1); // short hand for TYPE_1
define('SQ_SC_LINK_BACKEND_NAV',   3); // short hand for TYPE_1 | TYPE_2
define('SQ_SC_LINK_SIGNIFICANT',   7); // short hand for TYPE_1 | TYPE_2 | TYPE_3
define('SQ_SC_LINK_WEB_PATHS',     7); // short hand for links affected by web path changes


/*
* PERMISSION TYPES - different access levels to control access to an asset
*/
define('SQ_PERMISSION_WORKFLOW', 0); // workflow
define('SQ_PERMISSION_READ',     1); // read access
define('SQ_PERMISSION_WRITE',    2); // write access
define('SQ_PERMISSION_ADMIN',    3); // admin access


/*
* STATUS TYPES - statii an asset can currently have
* It is very important that the statii stay in the order they are defined
* as they are compared to eachother when changing the status of an asset
*/
define('SQ_STATUS_ARCHIVED',           1); // asset is archived
define('SQ_STATUS_UNDER_CONSTRUCTION', 2); // asset is under construction
define('SQ_STATUS_PENDING_APPROVAL',   4); // asset is currently in workflow
define('SQ_STATUS_APPROVED',           8); // asset is approved waiting to go live from under construction
define('SQ_STATUS_LIVE',              16); // asset is live
define('SQ_STATUS_LIVE_APPROVAL',     32); // asset is up for review
define('SQ_STATUS_EDITING',           64); // asset is currently safe editing
define('SQ_STATUS_EDITING_APPROVAL', 128); // asset is currently in workflow from safe edit
define('SQ_STATUS_EDITING_APPROVED', 256); // asset is approved waiting to go live from safe edit

define('SQ_SC_STATUS_NOT_LIVE',      15); // short cut for ARCHIVED | UNDER_CONSTRUCTION | PENDING_APPROVAL | APPROVED
define('SQ_SC_STATUS_LIVE_EDIT',     48); // short cut for LIVE | LIVE_APPROVAL
define('SQ_SC_STATUS_CAN_APPROVE',   66); // short cut for UNDER_CONSTRUCTION | EDITING
define('SQ_SC_STATUS_PENDING',      164); // short cut for PENDING_APPROVAL | EDITING_APPROVAL | LIVE_APPROVAL
define('SQ_SC_STATUS_ALL_APPROVED', 264); // short cut for APPROVED | EDITING_APPROVED
define('SQ_SC_STATUS_SAFE_EDITING', 448); // short cut for EDITING | EDITING_APPROVAL | SQ_STATUS_EDITING_APPROVED
define('SQ_SC_STATUS_ALL',          511); // short cut for SQ_STATUS_*


/*
* RUN LEVEL SETTINGS - security systems that can be enabled and disabled
*/
define('SQ_SECURITY_PERMISSIONS',          1); // enable permissions checking
define('SQ_SECURITY_LOCKING',              2); // enable locking
define('SQ_SECURITY_DATA_VALIDATION',      4); // enable data value validation
define('SQ_SECURITY_PASSWORD_VALIDATION',  8); // enable password rule checking
define('SQ_SECURITY_LINK_INTEGRITY',      16); // enable link validation
define('SQ_SECURITY_INTEGRITY',           32); // enable additional actions to ensure system/asset integrity
define('SQ_SECURITY_STATUS_INTEGRITY',    64); // enable status change checking
define('SQ_SECURITY_TRIGGERS',           128); // enable triggering/event system

define('SQ_RUN_LEVEL_OPEN',     0); // no security
define('SQ_RUN_LEVEL_FORCED', 236); // DATA_VALIDATION, PASSWORD_VALIDATION, INTEGRITY, STATUS_INTEGRITY and TRIGGERS
define('SQ_RUN_LEVEL_SECURE', 255); // full security


// this is where parameter map and trigger actions will store session variables
define('SQ_SESSION_SANDBOX_INDEX', '__SQ_SESSION_SANDBOX');


// load assertion functions, for integrated testing
require_once SQ_INCLUDE_PATH.'/assertions.inc';


// load general functions, including the error handler
require_once SQ_INCLUDE_PATH.'/general.inc';
ini_set('display_errors', 1); // we'll manage whether to display the errors or not in the error_handler
set_error_handler('sq_error_handler');
set_exception_handler('sq_exception_handler');

// turn off error reporting on the front end (errors are still caught and displayed/hidden by the error handler)
if (!(SQ_IN_BACKEND | SQ_IN_LIMBO | SQ_IN_CRON) & hide_frontend_errors()) {
	ini_set('display_errors', 0);
}

// Logging to syslog? If so, turn it on now (it won't connect to syslog until the first actual error)
if ((boolean)SQ_CONF_ERRORS_LOG_TO_SYSLOG === TRUE) {
	$ident = SQ_CONF_SYSTEM_NAME;
	if (empty($ident) === TRUE) {
		$ident = SQ_SYSTEM_LONG_NAME;
	}
	$consts = get_defined_constants();
	$facility = $consts['LOG_'.strtoupper(SQ_CONF_ERRORS_SYSLOG_FACILITY)];
	openlog($ident, LOG_ODELAY, $facility);
}

// if there is magic quotes, strip them out
if (get_magic_quotes_gpc()) {
	stripslashes_array($_GET, TRUE);
	stripslashes_array($_POST, TRUE);
	stripslashes_array($_COOKIE, TRUE);
	stripslashes_array($_REQUEST, TRUE);
	if (isset($_SERVER['QUERY_STRING'])) {
		$_SERVER['QUERY_STRING'] = stripslashes($_SERVER['QUERY_STRING']);
	}
}

// create and init the system object
require_once SQ_INCLUDE_PATH.'/mysource.inc';
$GLOBALS['SQ_SYSTEM'] = new MySource();
$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_SECURE);
$GLOBALS['SQ_SYSTEM']->init();
?>
