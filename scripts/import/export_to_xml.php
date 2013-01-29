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
* $Id: export_to_xml.php,v 1.27.2.1 2013/01/29 00:07:21 ewang Exp $
*
*/

/**
* Creates XML based on an asset ID provided.
*
* @author  Edison Wang <ewang@squiz.net>
* @author  Avi Miller <amiller@squiz.net>
* @version $Revision: 1.27.2.1 $
* @package MySource_Matrix
*/



/**
* Creates XML based on an asset ID provided.
*
* @author  David Schoen <dschoen@squiz.net>
* @author  Edison Wang <ewang@squiz.net>
* @author  Avi Miller <amiller@squiz.net>
* @version $Revision: 1.27.2.1 $
* @package MySource_Matrix
*/

/*
 *
 *
 * Example usage:
 * php scripts/import/export_to_xml.php . 3:35,4:36 1 >export.xml
 * 
 * First argument specifies system root path
 *
 * Second argument specifies which asset should be moved underneath which parent asset, 
 * 3:35 means asset with id 3 will be moved underneath parent asset with id 35
 *
 * Third argument specifies create link type
 *
 */

error_reporting(E_ALL);
ini_set('memory_limit', '1024M');
if ((php_sapi_name() != 'cli')) trigger_error("You can only run this script from the command line\n", E_USER_ERROR);

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT)) {
	echo "ERROR: You need to supply the path to the System Root as the first argument\n";
	exit(1);
}

if (!is_dir($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT.'/core/include/init.inc')) {
	echo "ERROR: Path provided doesn't point to a Matrix installation's System Root. Please provide correct path and try again.\n";
	exit(1);
}

$asset_infos = (isset($_SERVER['argv'][2])) ? explode(',',$_SERVER['argv'][2]) : Array();
if (empty($asset_infos)) {
	echo "ERROR: You need to supply the asset id for the asset you want to export and parent asset it will link to as the second argument with format 3:75,4:46 (assetid 3 links to assetid 75, assetid 4 links to asset id 46)\n";
	exit(1);
}

$initial_link_type = (isset($_SERVER['argv'][3])) ? $_SERVER['argv'][3] : '';
if (empty($initial_link_type)) {
	echo "ERROR: You need to supply the initial link type as the third argument\n";
	exit(1);
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';
require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';

// log in as root
$root_user = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	echo "Failed logging in as root user\n";
	exit(1);
}

$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();

$vars = array(
	'export_path' => getcwd(),
	'root_node_mapping' => $asset_infos,
	'stdout' => true,
);

$errors = $hh->freestyleHipo('hipo_job_export_assets_to_xml', $vars, SQ_PACKAGES_PATH.'/import_tools/hipo_jobs');
if (count($errors)) {
    print_r($errors);
    exit(1);
}
