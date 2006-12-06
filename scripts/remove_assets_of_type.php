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
* $Id: remove_assets_of_type.php,v 1.6 2006/12/06 05:39:51 bcaldwell Exp $
*
*/

/**
* Remove all assets of the specified type in the matrix system
*
* Note this script is not at all asset-inheritance-aware: it will only remove
* assets of exactly the type you specify
*
* @author  Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.6 $
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

$db =& $GLOBALS['SQ_SYSTEM']->db;

$assets_of_type = $db->getCol('SELECT assetid FROM sq_ast WHERE type_code='.$db->quote($DELETING_ASSET_TYPE));
assert_valid_db_result($assets_of_type);
if (!empty($assets_of_type)) {
	$asset_ids_set = '('.implode(', ', $assets_of_type).')';

	$res =& $db->query('DELETE FROM sq_ast_attr_val WHERE assetid in '.$asset_ids_set);
	assert_valid_db_result($res);
	$res =& $db->query('DELETE FROM sq_ast WHERE type_code = '.$db->quote($DELETING_ASSET_TYPE));
	assert_valid_db_result($res);

	$res =& $db->getAll('SELECT linkid FROM sq_ast_lnk WHERE minorid in '.$asset_ids_set.' OR majorid in '.$asset_ids_set);
	assert_valid_db_result($res);
	if (!empty($res)) {
		$links_array = Array();
		foreach ($res as $value) {
			array_push($links_array, $value['linkid']);
		}
		$links_set = '('.implode(', ', $links_array).')';

		$res =& $db->query('DELETE FROM sq_ast_lnk WHERE linkid in '.$links_set);
		assert_valid_db_result($res);
		$res =& $db->query('UPDATE sq_ast_lnk_tree SET linkid=0 where linkid in '.$links_set);
		assert_valid_db_result($res);
	}
	$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
	$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
}


?>
