<?php

/*
* Set the level of PHP reported errors
* And some other PHP thingies we want
* done OUR way.
*/
error_reporting(E_ALL);
set_magic_quotes_runtime(0);

// We need this so that our error handler won't kill itself attempting to try and remove 
// all existing buffers, especially important when Zend Performance Suite is installed because
// it adds another OB level
define('SQ_INIT_OB_LEVEL', ob_get_level());


/*
* Set the current version of the system core
* We mustn't forget to update this (until someone
* comes up with an automated method)
*/
define('SQ_SYSTEM_VERSION',		'0.1.1');
define('SQ_SYSTEM_SHORT_NAME',	'Resolve');
define('SQ_SYSTEM_LONG_NAME',	'Squiz Resolve v'.SQ_SYSTEM_VERSION);
define('SQ_SYSTEM_URL',			'http://www.squiz.net/');

define('SQ_REQUIRED_PHP_VERSION',   '4.3.0');

/*
* Let's get our bearings as to where everything is from here.
* These paths may be relative or absolute
*/
define('SQ_SYSTEM_ROOT',		dirname(dirname(dirname(realpath(__FILE__)))));
define('SQ_INCLUDE_PATH',		SQ_SYSTEM_ROOT.'/core/include');
define('SQ_CORE_PACKAGE_PATH',	SQ_SYSTEM_ROOT.'/core/assets');
define('SQ_ATTRIBUTES_PATH',	SQ_SYSTEM_ROOT.'/core/attributes');
define('SQ_LIB_PATH',			SQ_SYSTEM_ROOT.'/core/lib');
define('SQ_DATA_PATH',			SQ_SYSTEM_ROOT.'/data');
define('SQ_PACKAGES_PATH',		SQ_SYSTEM_ROOT.'/packages');
define('SQ_WEB_PATH',			SQ_SYSTEM_ROOT.'/core/web');
define('SQ_FUDGE_PATH',			SQ_SYSTEM_ROOT.'/fudge');

// include the main system config file
require_once SQ_DATA_PATH.'/conf/main.inc';


// Load general everyday handy functions for developers
require_once SQ_FUDGE_PATH.'/dev/dev.inc';

// nice little boolean to use for testing whether we happen to be in the backend or not
define('SQ_IN_BACKEND', (basename($_SERVER['PHP_SELF']) == SQ_CONF_BACKEND_SUFFIX));
// make sure there is a trailing slash if we are in the backend
if (SQ_IN_BACKEND && substr($_SERVER['PHP_SELF'], -1) != '/') {
	header('Location: '.$_SERVER['PHP_SELF'].'/'.(($_SERVER['QUERY_STRING']) ? '?'.$_SERVER['QUERY_STRING'] : ''));
	exit();
}

// the current output type for the system, can be changed during run-time (eg to XML)
// main use is so that the error_reporter doesn't barf in HTML to something expecting XML
$GLOBALS['SQ_OUTPUT_TYPE'] = 'html';


// we are not installing anything
$GLOBALS['SQ_INSTALL'] = false;


// Load general functions, including the error handler
require_once SQ_INCLUDE_PATH.'/general.inc';
ini_set('display_errors', 1); // we'll manage whether to display the errors or not in the error_handler
set_error_handler('sq_error_handler');

// Now that we've got error reporting lets generate a few errors!
if(version_compare(PHP_VERSION, SQ_REQUIRED_PHP_VERSION, '<')) {
	trigger_error('<i>'.SQ_SYSTEM_LONG_NAME.'</i> requires PHP Version '.SQ_REQUIRED_PHP_VERSION.'.<br/> You may need to upgrade.<br/> Your current version is '.phpversion().'.', E_USER_ERROR);
}


/*
* LINK TYPES - used with bitwise operators
*/
define('SQ_LINK_EXCLUSIVE',    1); // minor can only be SIGNIFICANT linked to major, and is a child of major
define('SQ_LINK_UNITE',        2); // minor is a child of major, but can be linked to many major's
define('SQ_LINK_NOTICE',       4); // for information purposes only, used to register href's from one asset to another so that deletions can warn of impending issues
define('SQ_LINK_USES',         8); // minor asset is used by the major asset, but it is not something that is in the navigation, minor can be linked to many major's

define('SQ_LINK_ALL',         15); // what all the main link types add up to
define('SQ_LINK_NAV',          3); // short hand for EXCLUSIVE | UNITE
define('SQ_LINK_SIGNIFICANT', 11); // short hand for EXCLUSIVE | UNITE | USES


/*
* PERMISSION TYPES - different access levels to control access to an asset
*/
define('SQ_PERMISSION_READ',     1); // read access
define('SQ_PERMISSION_WRITE',    2); // write access
define('SQ_PERMISSION_ADMIN',    3); // admin access
define('SQ_PERMISSION_WORKFLOW', 4); // workflow


/*
* STATUS TYPES - statii an asset can currently have
*/
define('SQ_STATUS_UNDER_CONSTRUCTION', 1); // asset is under construction
define('SQ_STATUS_PENDING_APPROVAL',   2); // asset is currently in workflow
define('SQ_STATUS_LIVE',               3); // asset is live


// Create the system object
require_once SQ_INCLUDE_PATH.'/resolve.inc';
$GLOBALS['SQ_SYSTEM'] = new Resolve();

?>