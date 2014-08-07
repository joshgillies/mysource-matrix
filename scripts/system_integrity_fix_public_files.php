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
* $Id: system_integrity_fix_public_files.php,v 1.4 2012/08/30 01:04:53 ewang Exp $
*
*/

/**
* Fix (update, recover or remove) public files in the public data directory of file and its descendant assets. 
*
* @author  Mohamed Haidar <mhaidar@squiz.com.au>
* @version $Revision: 1.4 $
* @package MySource_Matrix
*/

if (ini_get('memory_limit') != '-1') ini_set('memory_limit', '-1');

error_reporting(E_ALL);
if (count($_SERVER['argv']) < 3 || php_sapi_name() != 'cli') {
        echo "This script needs to be run in the following format:\n\n";
        echo "\tphp system_integrity_fix_public_files.php SYSTEM_ROOT ROOT_ID\n\n";
        exit(1);
}

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT)) {
	echo "ERROR: You need to supply the path to the System Root as the first argument\n";
	exit();
}

if (!is_dir($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT.'/core/include/init.inc')) {
	echo "ERROR: Path provided doesn't point to a Matrix installation's System Root. Please provide correct path and try again.\n";
	exit();
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';

$ROOT_ID = (isset($_SERVER['argv'][2])) ? $_SERVER['argv'][2] : '';
assert_valid_assetid($ROOT_ID, "The ROOT_ID '$ROOT_ID' specified is invalid");
	
$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	echo "ERROR: Failed login in as root user\n";
	exit();
}

//get children of the tree root asset which are file and its descendant types
$assetids = $GLOBALS['SQ_SYSTEM']->am->getChildren($ROOT_ID, 'file', FALSE);
//if the tree root asset is file type, include it
if ($ROOT_ID != '1') {
	$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($ROOT_ID);
	if ($GLOBALS['SQ_SYSTEM']->am->isTypeDecendant($asset->type(), 'file')) {
		$assetids[$asset->id] = Array(0 => Array('type_code' => $asset->type()));
	}
}

foreach ($assetids as $assetid => $asset_info) {
	$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
	//Use this public function to access _checkFileState function which
	//looks after the placing and removing of files in the public directory.
	$asset->permissionsUpdated();
	$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
}

$GLOBALS['SQ_SYSTEM']->restoreCurrentUser();

?>
