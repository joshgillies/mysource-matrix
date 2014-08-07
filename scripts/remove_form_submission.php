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
* $Id: remove_form_submission.php,v 1.16 2013/08/09 03:28:18 akarelia Exp $
*
*/

/**
* This script does the following:
* 1. take 3 arguments from the command line: asset_id, from_date, to_date*
* 2. get assets of type 'form_submission'
*	(a) in the submission folder of form (#asset_id)
*	(b) created between 'from_date 00:00:00' and 'to_date 23:59:59'
* 3. remove assets permanently from database
*
*		Make sure that no one is editing any form submission asset
*		Require Matrix version 3.12 or newer
*
* @author  Rayn Ong <rong@squiz.net>
* @version $Revision: 1.16 $
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

require_once $SYSTEM_ROOT.'/core/include/init.inc';

// check date range frmat
// if the user supply an invalid date range, select query will not return anything
if (count($argv) != 5) {
	echo 'Usage: remove_form_submission.php <system_root> <custom_form_id> <from_date> <to_date>'."\n";
	echo 'Date format: YYYY-MM-DD'."\n";
	exit(1);
} else if (preg_match('/^[0-9]{4}-[0-9]{2}-[0-9]{2}/',$argv[3]) != TRUE) {
	// simple date format YYYY-MM-DD check, nothing fancy
	echo"ERROR: 'From date' must be in the format 'YYYY-MM-DD'\n";
	exit(1);
} else if (preg_match('/^[0-9]{4}-[0-9]{2}-[0-9]{2}/',$argv[4]) != TRUE) {
	echo "ERROR: 'To date' must be in the format 'YYYY-MM-DD'\n";
	exit(1);
}
require_once SQ_FUDGE_PATH.'/general/datetime.inc';
$from_value = iso8601_date_component($argv[3]).' 00:00:00';
$to_value = iso8601_date_component($argv[4]).' 23:59:59';


$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user);

// check assetid and asset type
$assetid = $argv[2];

$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);

if (is_null($asset)) {
	echo "ERROR: #$assetid is not a valid asset ID";
	exit(1);
}
if (!is_a($asset, 'page_custom_form')) {
	echo "ERROR: Asset #$assetid is not a custom form asset";
	exit(1);
} else {
	$form = $asset->getForm();
	$sub_folder = $form->getSubmissionsFolder();
}


require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';
$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
$db = $GLOBALS['SQ_SYSTEM']->db;


$sql = 'SELECT
			a.assetid
		FROM
			('.SQ_TABLE_RUNNING_PREFIX.'ast a
			JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_typ_inhd i ON a.type_code = i.type_code)
			JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l
			ON l.minorid = a.assetid';
$where = 'l.majorid IN (:assetid, :subfolder_assetid)
			AND i.inhd_type_code = :inhd_type_code
			AND a.created BETWEEN '.db_extras_todate(MatrixDAL::getDbType(), ':created_from', FALSE).'
			AND '.db_extras_todate(MatrixDAL::getDbType(), ':created_to', FALSE);
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
$sql = $sql.$where.' ORDER BY a.created DESC';

$query = MatrixDAL::preparePdoQuery($sql);
MatrixDAL::bindValueToPdo($query, 'assetid',           $asset->id);
MatrixDAL::bindValueToPdo($query, 'subfolder_assetid', $sub_folder->id);
MatrixDAL::bindValueToPdo($query, 'inhd_type_code',    'form_submission');
MatrixDAL::bindValueToPdo($query, 'created_from',      $from_value);
MatrixDAL::bindValueToPdo($query, 'created_to',        $to_value);
$assetids = MatrixDAL::executePdoAssoc($query, 0);

if (empty($assetids)) {
	echo "No form submission found for '$asset->name' (#$assetid) within the specified date range\n";
	exit();
}
echo 'Found '.count($assetids)." form submission(s) for '$asset->name' (#$assetid)\n(Date range: $from_value to $to_value)\n";

$unquoted_assetids = $assetids;

// quote the assetids to be used in the IN clause
foreach ($assetids as $key => $assetid) {
	$assetids[$key] = MatrixDAL::quote((String)$assetid);
}

// break up the assets into chunks of 1000 so that oracle does not complain
$assetid_in = Array();
foreach (array_chunk($assetids, 999) as $chunk) {
	$assetid_in[] = ' assetid IN ('.implode(', ', $chunk).')';
}
$in_assetid = '('.implode(' OR ', $assetid_in).')';

$assetid_in = Array();
foreach (array_chunk($assetids, 999) as $chunk) {
	$assetid_in[] = ' minorid IN ('.implode(', ', $chunk).')';
}
$in_minorid = '('.implode(' OR ', $assetid_in).')';

$assetid_in = Array();
foreach (array_chunk($assetids, 999) as $chunk) {
	$assetid_in[] = ' majorid IN ('.implode(', ', $chunk).')';
}
$in_majorid = '('.implode(' OR ', $assetid_in).')';

// start removing entries from the database
echo "Removing assets ...\n";
$sql = 'DELETE FROM sq_ast WHERE '.$in_assetid;
$delete_count = MatrixDAL::executeSql($sql);

echo "\tUpdating link table...\n";
$sql = 'DELETE FROM sq_ast_lnk WHERE '.$in_minorid;
MatrixDAL::executeSql($sql);

echo "\tUpdating link tree table ...\n";
$sql = 'DELETE FROM sq_ast_lnk_tree t WHERE NOT EXISTS (SELECT linkid FROM sq_ast_lnk WHERE linkid = t.linkid)';
MatrixDAL::executeSql($sql);
$sql = "UPDATE sq_ast_lnk_tree SET num_kids=num_kids-$delete_count WHERE linkid = (SELECT linkid FROM sq_ast_lnk WHERE minorid = '".$sub_folder->id."' AND link_type = '".SQ_LINK_TYPE_2."')";
MatrixDAL::executeSql($sql);

echo "\tUpdating attribute value table ...\n";
$sql = 'DELETE FROM sq_ast_attr_val WHERE '.$in_assetid;
MatrixDAL::executeSql($sql);

echo "\tUpdating metadata table ...\n";
$sql = 'DELETE FROM sq_ast_mdata WHERE '.$in_assetid;
MatrixDAL::executeSql($sql);

echo "\tUpdating metadata value table ...\n";
$sql = 'DELETE FROM sq_ast_mdata_val WHERE '.$in_assetid;
MatrixDAL::executeSql($sql);

echo "\tUpdating workflow table...\n";
$sql = 'DELETE FROM sq_ast_wflow WHERE '.$in_assetid;
MatrixDAL::executeSql($sql);

echo "\tUpdating permissions table...\n";
$sql = 'DELETE FROM sq_ast_perm WHERE '.$in_assetid;
MatrixDAL::executeSql($sql);

echo "\tUpdating roles table...\n";
$sql = 'DELETE FROM sq_ast_role WHERE '.$in_assetid;
MatrixDAL::executeSql($sql);

echo "\tUpdating shadow link table...\n";
$sql = 'DELETE FROM sq_shdw_ast_lnk WHERE '.$in_majorid;
MatrixDAL::executeSql($sql);

$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

echo "\tDeleting asset data directories...\n";
require_once SQ_FUDGE_PATH.'/general/file_system.inc';
foreach ($unquoted_assetids as $sub_assetid){
	
	$data_path_suffix = asset_data_path_suffix('form_submission', $sub_assetid);
	$data_path = SQ_DATA_PATH.'/private/'.$data_path_suffix;
	$data_path_public = SQ_DATA_PATH.'/public/'.$data_path_suffix;
	
	if (is_dir($data_path)) {
		if (!delete_directory($data_path)) {
			trigger_error("Could not delete private data directory for Form Submission (Id: #$assetid)", E_USER_WARNING);
		}
	}
	
	if (is_dir($data_path_public)) {
		if (!delete_directory($data_path_public)) {
			trigger_error("Could not delete public data directory for Form Submission (Id: #$assetid)", E_USER_WARNING);
		}
	}
}

echo "Done\n";
