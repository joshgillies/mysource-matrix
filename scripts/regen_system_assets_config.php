<?php
/**
* +--------------------------------------------------------------------+
* | MySource 3 - MySource Matrix                                       |
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
* $Id: regen_system_assets_config.php,v 1.3 2003/11/18 15:37:38 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

/**
* This small script generates the systerm assets config for installed systems
* that get updated and need this file generated
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/
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

// Dont set SQ_INSTALL flag before this include because we want
// a complete load now that the database has been created
require_once $SYSTEM_ROOT.'/core/include/init.inc';


// Re-generate the System Config to make sure that we get any new defines that may have been issued
require_once SQ_INCLUDE_PATH.'/system_asset_config.inc';
$sys_asset_cfg = new System_Asset_Config();
$sys_asset_cfg->save(Array(), true);

?>