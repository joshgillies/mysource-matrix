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
* $Id: remove_asset_type.php,v 1.5 2012/06/05 06:26:09 akarelia Exp $
*
*/

/**
* Remove all assets of the specified type and then remove any record of the asset type itself
*
* Note this script is not at all asset-inheritance-aware: it will only remove
* assets of exactly the type you specify
*
* @author  Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix
*/
error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) trigger_error("You can only run this script from the command line\n", E_USER_ERROR);

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT)) {
	echo "ERROR: You need to supply the path to the System Root as the first argument\n";
	exit();
}

if (!is_dir($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT.'/core/include/init.inc')) {
	echo "ERROR: Path provided doesn't point to a Matrix installation's System Root. Please provide correct path and try again.\n";
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
$root_user = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
if (!$root_user->comparePassword($root_password)) {
	echo "ERROR: The root password entered was incorrect\n";
	exit();
}

// log in as root
if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	echo "ERROR: Failed login in as root user\n";
	exit();
}

// make some noiz
$db = $GLOBALS['SQ_SYSTEM']->db;

$sql = 'SELECT assetid FROM sq_ast WHERE type_code = :type_code';
$query = MatrixDAL::preparePdoQuery($sql);
MatrixDAL::bindValueToPdo($query, 'type_code', $DELETING_ASSET_TYPE);
$assets_of_type = MatrixDAL::executePdoAssoc($query, 0);

if (!empty($assets_of_type)) {
	$asset_ids_set = '('.implode(', ', $assets_of_type).')';
	$sql = 'DELETE FROM sq_ast_attr_val WHERE assetid in '.$asset_ids_set;
	$query = MatrixDAL::preparePdoQuery($sql);
	MatrixDAL::execPdoQuery($query);

	$sql = 'DELETE FROM sq_ast_lnk WHERE minorid in '.$asset_ids_set;
	$query = MatrixDAL::preparePdoQuery($sql);
	MatrixDAL::execPdoQuery($query);

	$sql = 'DELETE FROM sq_ast WHERE type_code = :type_code';
	$query = MatrixDAL::preparePdoQuery($sql);
	MatrixDAL::bindValueToPdo($query, 'type_code', $DELETING_ASSET_TYPE);
	MatrixDAL::execPdoQuery($query);
}

$sql = 'DELETE FROM sq_ast_attr WHERE type_code = :type_code OR owning_type_code = :owning_type_code';
$query = MatrixDAL::preparePdoQuery($sql);
MatrixDAL::bindValueToPdo($query, 'type_code', $DELETING_ASSET_TYPE);
MatrixDAL::bindValueToPdo($query, 'owning_type_code', $DELETING_ASSET_TYPE);
MatrixDAL::execPdoQuery($query);

$sql = 'DELETE FROM sq_ast_typ WHERE type_code = :type_code';
$query = MatrixDAL::preparePdoQuery($sql);
MatrixDAL::bindValueToPdo($query, 'type_code', $DELETING_ASSET_TYPE);
MatrixDAL::execPdoQuery($query);

$sql = 'DELETE FROM sq_ast_typ_inhd WHERE type_code = :type_code';
$query = MatrixDAL::preparePdoQuery($sql);
MatrixDAL::bindValueToPdo($query, ':type_code', $DELETING_ASSET_TYPE);
MatrixDAL::execPdoQuery($query);

assert_true(unlink(dirname(dirname(__FILE__)).'/data/private/db/asset_types.inc'), 'failed removing asset_types.inc');
echo "\nDone\n";
?>
