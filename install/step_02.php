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
* $Id: step_02.php,v 1.54 2005/03/21 06:25:03 gsherwood Exp $
*
*/

/**
* Install Step 2
*
* Purpose
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.54 $
* @package MySource_Matrix
* @subpackage install
*/
ini_set('memory_limit', -1);
error_reporting(E_ALL);

$SYSTEM_ROOT = '';
// from cmd line
if ((php_sapi_name() == 'cli')) {
	if (isset($_SERVER['argv'][1])) $SYSTEM_ROOT = $_SERVER['argv'][1];
	$err_msg = "You need to supply the path to the System Root as the first argument\n";

} else {
	if (isset($_GET['SYSTEM_ROOT'])) $SYSTEM_ROOT = $_GET['SYSTEM_ROOT'];
	$err_msg = '
	<div style="background-color: red; color: white; font-weight: bold;">
		You need to supply the path to the System Root as a query string variable called SYSTEM_ROOT
	</div>
	';
}

if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	trigger_error($err_msg, E_USER_ERROR);
}


require_once $SYSTEM_ROOT.'/core/include/init.inc';
$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
require_once 'XML/Tree.php';
$db = &$GLOBALS['SQ_SYSTEM']->db;

// re-generate the config to make sure that we get any new defines that may have been issued
require_once SQ_INCLUDE_PATH.'/system_config.inc';
$cfg = new System_Config();
$cfg->save(Array(), false);


$cached_table_columns = Array();

require_once SQ_LIB_PATH.'/db_install/db_install.inc';

$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

if (!db_install(SQ_CORE_PACKAGE_PATH.'/tables.xml')) {
	$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
	trigger_error('TABLE INSTALL FAILURE', E_USER_ERROR);
}

require_once SQ_LIB_PATH.'/file_versioning/file_versioning.inc';
if (!File_Versioning::initRepository($db)) {
	trigger_error('Unable to initialise File Versioning Repository', E_USER_ERROR);
}

// its all good
$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

?>