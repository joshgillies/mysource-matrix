<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: regen_system_assets_config.php,v 1.1 2003/10/17 05:53:17 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

/**
* Install Step 3
*
* Purpose
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