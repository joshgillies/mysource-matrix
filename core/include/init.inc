<?php
/**
* Init File
*
* The file that needs to be included to do anything
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve
*/

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

// Are we running from the command line ?
define('SQ_PHP_CLI', (php_sapi_name() == 'cli'));


/*
* Set the current version of the system core
* We mustn't forget to update this (until someone
* comes up with an automated method)
*/
define('SQ_SYSTEM_VERSION',       '0.1.1');
define('SQ_SYSTEM_SHORT_NAME',    'Resolve FX');
define('SQ_SYSTEM_LONG_NAME',     'Squiz Resolve FX v'.SQ_SYSTEM_VERSION);
define('SQ_SYSTEM_URL',           'http://www.squiz.net/');

define('SQ_REQUIRED_PHP_VERSION', '4.3.0');

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
require_once SQ_DATA_PATH.'/private/conf/main.inc';


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
if (!isset($GLOBALS['SQ_OUTPUT_TYPE'])) $GLOBALS['SQ_OUTPUT_TYPE'] = (SQ_PHP_CLI) ? 'text' : 'html';

// we are not installing anything
$GLOBALS['SQ_INSTALL'] = false;

// we are not purging the trash
$GLOBALS['SQ_PURGING_TRASH'] = false;

// the number of assetids reserved for system assets
define('SQ_NUM_RESERVED_ASSETIDS', 128);


/*
* LINK TYPES - used with bitwise operators
*/
define('SQ_LINK_TYPE_1',        1); // minor asset is a child of the major asset, link is displayed on frontend and backend navigation systems
define('SQ_LINK_TYPE_2',        2); // same power/rank as a TYPE_1 link, but is only displayed on backend navigation systems
define('SQ_LINK_TYPE_3',        4); // same power/rank as a TYPE_1 link, but is not displayed on any navigation systems
define('SQ_LINK_NOTICE',        8); // for information purposes only, (eg used to register href's from one asset to another) so that deletions can warn of impending issues

define('SQ_LINK_ALL',          15); // what all the main link types add up to
define('SQ_LINK_FRONTEND_NAV',  1); // short hand for TYPE_1
define('SQ_LINK_BACKEND_NAV',   3); // short hand for TYPE_1 | TYPE_2
define('SQ_LINK_SIGNIFICANT',   7); // short hand for TYPE_1 | TYPE_2 | TYPE_3


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

define('SQ_STATUS_NOT_LIVE',      15); // short hand for SQ_STATUS_ARCHIVED | SQ_STATUS_UNDER_CONSTRUCTION | SQ_STATUS_PENDING_APPROVAL | SQ_STATUS_APPROVED
define('SQ_STATUS_CAN_APPROVE',   66); // short hand for SQ_STATUS_UNDER_CONSTRUCTION | SQ_STATUS_EDITING
define('SQ_STATUS_PENDING',      164); // short hand for SQ_STATUS_PENDING_APPROVAL | SQ_STATUS_EDITING_APPROVAL | SQ_STATUS_LIVE_APPROVAL
define('SQ_STATUS_ALL_APPROVED', 136); // short hand for SQ_STATUS_APPROVED | SQ_STATUS_EDITING_APPROVED
define('SQ_STATUS_SAFE_EDITING', 448); // short hand for SQ_STATUS_EDITING | SQ_STATUS_EDITING_APPROVAL | SQ_STATUS_EDITING_APPROVED


// Load general functions, including the error handler
require_once SQ_INCLUDE_PATH.'/general.inc';
ini_set('display_errors', 1); // we'll manage whether to display the errors or not in the error_handler
set_error_handler('sq_error_handler');

// Now that we've got error reporting lets generate a few errors!
if(version_compare(PHP_VERSION, SQ_REQUIRED_PHP_VERSION, '<')) {
	trigger_error('<i>'.SQ_SYSTEM_LONG_NAME.'</i> requires PHP Version '.SQ_REQUIRED_PHP_VERSION.'.<br/> You may need to upgrade.<br/> Your current version is '.phpversion().'.', E_USER_ERROR);
}


// Create the system object
require_once SQ_INCLUDE_PATH.'/resolve.inc';
$GLOBALS['SQ_SYSTEM'] = new Resolve();
$GLOBALS['SQ_SYSTEM']->init();


?>
