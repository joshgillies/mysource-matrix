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
* $Id: step_02.php,v 1.83 2013/06/05 04:20:18 akarelia Exp $
*
*/

/**
* Install Step 2
*
* Purpose
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.83 $
* @package MySource_Matrix
* @subpackage install
*/

ini_set('memory_limit', -1);
error_reporting(E_ALL);

$SYSTEM_ROOT = '';

if ((php_sapi_name() == 'cli')) {
	if (isset($_SERVER['argv'][1])) {
		$SYSTEM_ROOT = $_SERVER['argv'][1];
	}
} else {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

if (empty($SYSTEM_ROOT)) {
	$err_msg = "ERROR: You need to supply the path to the System Root as the first argument.\n";
	$err_msg .= "Usage: php install/step_02.php <PATH_TO_MATRIX>\n";
	echo $err_msg;
	exit(1);
}

if (!is_dir($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT.'/core/include/init.inc')) {
	$err_msg = "ERROR: Path provided doesn't point to a Matrix installation's System Root. Please provide correct path and try again.\n";
	$err_msg .= "Usage: php install/step_02.php <PATH_TO_MATRIX>\n";
	echo $err_msg;
	exit(1);
}

define('SQ_SYSTEM_ROOT',  $SYSTEM_ROOT);
require_once $SYSTEM_ROOT.'/core/include/init.inc';
require_once $SYSTEM_ROOT.'/install/install.inc';
require_once SQ_LIB_PATH.'/db_install/db_install.inc';
require_once SQ_INCLUDE_PATH.'/system_config.inc';
require_once SQ_LIB_PATH.'/file_versioning/file_versioning.inc';

$exitRC = 0;

$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);

// Re-generate the Config to make sure that we get any new defines that may have been issued

$cfg = new System_Config();
$cfg->save(Array(), FALSE);

// check that we have valid DEFAULT and TECH email addresses - warn if missing
if (!(SQ_CONF_TECH_EMAIL) && !(SQ_CONF_DEFAULT_EMAIL)) {
	trigger_error('Neither the System Default nor Tech email addresses have been set', E_USER_WARNING);
} else {
	if (!SQ_CONF_TECH_EMAIL) {
		trigger_error('The System Tech email address has not been set', E_USER_WARNING);
	}
	if (!SQ_CONF_DEFAULT_EMAIL) {
		trigger_error('The System Default email address has not been set', E_USER_WARNING);
	}
}

if (DAL::getDbType() === 'oci') {
	$query      = "SELECT value FROM nls_session_parameters WHERE parameter='NLS_DATE_FORMAT'";
	$value      = DAL::executeSqlAssoc($query, 0);
	$nls_format = $value[0];
	$expected   = 'YYYY-MM-DD HH24:MI:SS';
	if ($nls_format !== $expected) {
		trigger_error('NLS_DATE_FORMAT has not been set correctly. It needs to be '.$expected, E_USER_ERROR);
	}
}

$cached_table_columns = Array();

// we need to do this before starting the transaction because the
// set_timestamp for postgres is required to start a transaction
$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
$db =& $GLOBALS['SQ_SYSTEM']->db;

try {
	install_stored_relations('functions', NULL, FALSE);
} catch (Exception $e) {
	$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
	$msg  = "Unable to install sql functions for core system:\n";
	$msg .= $e->getMessage();
	trigger_error($msg, E_USER_ERROR);
	exit(1);
}

if (file_exists(SQ_DATA_PATH.'/private/db/table_columns.inc')) {
	unlink(SQ_DATA_PATH.'/private/db/table_columns.inc');
}

if (!db_install(SQ_CORE_PACKAGE_PATH.'/tables.xml', FALSE)) {
	$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
	trigger_error('Unable to install tables for the core system.', E_USER_ERROR);
	exit(1);
}

// install any tables needed by the packages
$packages = get_package_list();

foreach ($packages as $package) {
	$xml_file = SQ_PACKAGES_PATH.'/'.$package.'/tables.xml';
	if (file_exists($xml_file)) {
		if (!db_install($xml_file, FALSE)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			trigger_error('Unable to install tables for package '.$package.'.', E_USER_ERROR);
			exit(1);
		}
		try {
			install_stored_relations('functions', $package, FALSE);
		} catch (Exception $e) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$msg  = "Unable to install sql functions for package $package:\n";
			$msg .= $e->getMessage();
			trigger_error($msg, E_USER_ERROR);
			exit(1);
		}
	}
}

// Install all views except for Roles-related views which are handled further below
try {
	install_stored_relations('views', NULL, FALSE);
} catch (Exception $e) {
	$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
	$msg  = "Unable to install sql views:\n";
	$msg .= $e->getMessage();
	trigger_error($msg, E_USER_ERROR);
	exit(1);
}

$fv = $GLOBALS['SQ_SYSTEM']->getFileVersioning();

if (!$fv->initRepository()) {
	trigger_error('Unable to initialise File Versioning Repository', E_USER_ERROR);
}

$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

/*
* Verify that Roles views are all cool as determined by the system-wide config settings.
* This is especially important if we have upgraded from a previous version where Roles were
* disabled as at this point Matrix will have the default definitions.
*/

// With respect to #4440 Feature Request : Spliting System Roles into System Permission Roles and System Workflow Roles
// lets split up the constants and regenerate the config file. Also in this block decide if we have to re-create views
if (!defined('SQ_CONF_ENABLE_ROLES_PERM_SYSTEM') && !defined('SQ_CONF_ENABLE_ROLES_WF_SYSTEM')) {
	if(defined('SQ_CONF_ENABLE_ROLES_SYSTEM') && SQ_CONF_ENABLE_ROLES_SYSTEM == '1' ) {
		$vars['SQ_CONF_ENABLE_ROLES_PERM_SYSTEM'] = '1';
		$vars['SQ_CONF_ENABLE_ROLES_WF_SYSTEM'] = '1';
		$enabled = TRUE;
	} else {
		$vars['SQ_CONF_ENABLE_ROLES_PERM_SYSTEM'] = '0';
		$vars['SQ_CONF_ENABLE_ROLES_WF_SYSTEM'] = '0';
		$enabled = FALSE;
	}
	$cfg->save($vars, FALSE);
} else {
	$enabled = ((SQ_CONF_ENABLE_ROLES_PERM_SYSTEM == '1' ) || (SQ_CONF_ENABLE_ROLES_WF_SYSTEM == '1'));
}

if (!defined('SQ_CONF_COOKIE_OPTION_HTTP_ONLY') || !defined('SQ_CONF_COOKIE_OPTION_SECURE')) {
	if (!defined('SQ_CONF_COOKIE_OPTION_HTTP_ONLY')) {
		$vars['SQ_CONF_COOKIE_OPTION_HTTP_ONLY'] = FALSE;
	} 
	if (!defined('SQ_CONF_COOKIE_OPTION_SECURE')) {
		$vars['SQ_CONF_COOKIE_OPTION_SECURE'] = FALSE;
	}
	$cfg->save($vars);
}

// Install the applicable views from the common_views_roles.xml file
try {
	$roles_configured = $cfg->configureRoleTables($enabled, SQ_CONF_ENABLE_GLOBAL_ROLES);
	if (!$roles_configured) {
		$exitRC = 1;
		echo "Unable to configure roles. Existing definition retained\n\n";
	}
} catch (Exception $e) {
	echo "Unable to configure roles: ";
	echo $e->getMessage()."\n";
	exit(1);
}

// grant permissions to the tables for the secondary user
try {
	grant_secondary_user_perms(FALSE);
} catch (Exception $e) {
	echo "Unable to grant secondary permissions:\n";
	echo $e->getMessage()."\n";
	exit(1);
}

$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

if ($exitRC == 0) {
	echo "\n";
	echo "Step 2 completed successfully.\n";
	echo "\n";
} else {
	exit(1);
}

?>
