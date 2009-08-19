<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: remove_assets_of_type.php,v 1.7.6.1 2009/08/19 03:22:01 akarelia Exp $
*
*/

/**
* Remove all assets of the specified type in the matrix system
*
* Note this script is not at all asset-inheritance-aware: it will only remove
* assets of exactly the type you specify
*
* @author  Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.7.6.1 $
* @package MySource_Matrix
*/
error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	echo "ERROR: You need to supply the path to the System Root as the first argument\n";
	exit();
}

$DELETING_ASSET_TYPE = (isset($_SERVER['argv'][2])) ? $_SERVER['argv'][2] : '';
if (empty($DELETING_ASSET_TYPE)) {
	echo "ERROR: You need to supply an asset type code as the second argument\n";
	exit();
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';
require_once SQ_DATA_PATH.'/private/conf/tools.inc';


// ask for the root password for the system
echo 'Enter the root password for "'.SQ_CONF_SYSTEM_NAME.'": ';
$root_password = rtrim(fgets(STDIN, 4094));

// check that the correct root password was entered
$root_user =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
if (!$root_user->comparePassword($root_password)) {
	echo "ERROR: The root password entered was incorrect\n";
	exit();
}

// log in as root
if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	trigger_error("Failed loggin in as root user\n", E_USER_ERROR);
	exit();
}

// make some noiz
$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

$sql = 'SELECT assetid FROM sq_ast WHERE type_code = :type_code';
$query = MatrixDAL::preparePdoQuery($sql);
MatrixDAL::bindValueToPdo($query, 'type_code', $DELETING_ASSET_TYPE);
$assets_of_type = MatrixDAL::executePdoAssoc($query, 0);

if (!empty($assets_of_type)) {

	// bugfix #3820	, use MatrixDAL to quote the asset ids
	foreach($assets_of_type as $index => $asset_id) {
		$assets_of_type[$index] = MatrixDAL::quote($asset_id);
	}
	$asset_ids_set = '('.implode(', ', $assets_of_type).')';

	$sql = 'SELECT linkid FROM sq_ast_lnk WHERE minorid in '.$asset_ids_set.' OR majorid in '.$asset_ids_set;
	$query = MatrixDAL::preparePdoQuery($sql);
	// bugfix #3820 
	$res = MatrixDAL::executePdoAssoc($query);

	// bugfix #3849 Script remove_assets_of_type.php leaves unwanted table entries
	// while deleting the link make sure we call asset_manager function to do the same
	// we should not be just deleting the links and treeids from the table as it can
	// lead to unexpected behaviour. for more information see the Bug report
	if (!empty($res)) {
		$links_array = Array();
		foreach ($res as $value) {
			$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($value['linkid']);
			array_push($links_array, MatrixDAL::quote($value['linkid']));
		}
		$links_set = '('.implode(', ', $links_array).')';

		$sql = 'DELETE FROM sq_ast_lnk WHERE linkid in '.$links_set;
		$query = MatrixDAL::preparePdoQuery($sql);
		MatrixDAL::execPdoQuery($query);
	}
	// we need to delete asset from sq_ast table after deleteAssetLink() call
	// or else then function call will throw errors.
	$sql = 'DELETE FROM sq_ast_attr_val WHERE assetid in '.$asset_ids_set;
	$query = MatrixDAL::preparePdoQuery($sql);
	MatrixDAL::execPdoQuery($query);

	$sql = 'DELETE FROM sq_ast WHERE type_code = :type_code';
	$query = MatrixDAL::preparePdoQuery($sql);
	MatrixDAL::bindValueToPdo($query, 'type_code', $DELETING_ASSET_TYPE);
	MatrixDAL::execPdoQuery($query);

	$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
	$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
}


?>
