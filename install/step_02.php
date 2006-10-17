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
* $Id: step_02.php,v 1.66 2006/10/17 00:08:25 lwright Exp $
*
*/

/**
* Install Step 2
*
* Purpose
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.66 $
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

	$err_msg = "You need to supply the path to the System Root as the first argument\n";

} else {
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

define ('SQ_SYSTEM_ROOT', $SYSTEM_ROOT);
require_once $SYSTEM_ROOT.'/core/include/init.inc';
require_once $SYSTEM_ROOT.'/install/install.inc';
require_once SQ_LIB_PATH.'/db_install/db_install.inc';
require_once SQ_INCLUDE_PATH.'/system_config.inc';
require_once SQ_LIB_PATH.'/file_versioning/file_versioning.inc';
require_once 'XML/Tree.php';


$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);

// Re-generate the Config to make sure that we get any new defines that may have been issued

$cfg =& new System_Config();
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


$cached_table_columns = Array();

// we need to do this before starting the transaction because the
// set_timestamp for postgres is required to start a transaction
$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
$db =& $GLOBALS['SQ_SYSTEM']->db;

install_stored_relations('functions');

if (file_exists(SQ_DATA_PATH.'/private/db/table_columns.inc')) {
	unlink(SQ_DATA_PATH.'/private/db/table_columns.inc');
}

if (!db_install(SQ_CORE_PACKAGE_PATH.'/tables.xml')) {
	$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
	trigger_error('TABLE INSTALL FAILURE', E_USER_ERROR);
}

// install any tables needed by the packages
$packages = get_package_list();

foreach ($packages as $package) {
	$xml_file = SQ_PACKAGES_PATH.'/'.$package.'/tables.xml';
	if (file_exists($xml_file)) {
		if (!db_install($xml_file)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			trigger_error('TABLE INSTALL FAILURE', E_USER_ERROR);
		}
		install_stored_relations('functions', $package);
	}
}

install_stored_relations('views');

// grant permissions to the tables for the secondary user
grant_secondary_user_perms();

if (!File_Versioning::initRepository()) {
	trigger_error('Unable to initialise File Versioning Repository', E_USER_ERROR);
}

$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

?>
