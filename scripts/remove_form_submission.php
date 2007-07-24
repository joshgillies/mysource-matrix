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
* $Id: remove_form_submission.php,v 1.2.2.4 2007/07/24 03:39:48 rong Exp $
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
* @version $Revision: 1.2.2.4 $
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

require_once $SYSTEM_ROOT.'/core/include/init.inc';

// check date range frmat
// if the user supply an invalid date range, select query will not return anything
if (count($argv) != 5) {
	echo 'Usage: remove_form_submission.php <system_root> <custom_form_id> <from_date> <to_date>'."\n";
	echo 'Date format: YYYY-MM-DD'."\n";
	die;
} else if (preg_match('/^[0-9]{4}-[0-9]{2}-[0-9]{2}/',$argv[3]) != TRUE) {
	// simple date format YYYY-MM-DD check, nothing fancy
	trigger_error('\'From date\' must be in the format \'YYYY-MM-DD\'', E_USER_WARNING);
	die;
} else if (preg_match('/^[0-9]{4}-[0-9]{2}-[0-9]{2}/',$argv[4]) != TRUE) {
	trigger_error('\'To date\' must be in the format \'YYYY-MM-DD\'', E_USER_WARNING);
	die;
}
require_once SQ_FUDGE_PATH.'/general/datetime.inc';
$from_value = iso8601_date_component($argv[3]).' 00:00:00';
$to_value = iso8601_date_component($argv[4]).' 23:59:59';

// check assetid and asset type
$assetid = $argv[2];
$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
if (is_null($asset)) {
	trigger_error("#$assetid is not a valid asset ID", E_USER_WARNING);
	die;
}
if (!is_a($asset, 'page_custom_form')) {
	trigger_error("Asset #$assetid is not a custom form asset", E_USER_WARNING);
	die;
} else {
	// changed so that this part works for 3.12
	// assume that all submission is type3-linked to the form_email asset
	$form_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_TYPE_2, 'form_email', TRUE, '', 'major');
	$form =& $GLOBALS['SQ_SYSTEM']->am->getAsset($form_link['minorid'], $form_link['minor_type_code']);
}


require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';
$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
$db =& $GLOBALS['SQ_SYSTEM']->db;


$sql = 'SELECT
			a.assetid
		FROM
			('.SQ_TABLE_RUNNING_PREFIX.'ast a
			JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_typ_inhd i ON a.type_code = i.type_code)
			JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l
			ON l.minorid = a.assetid';
// $where = 'l.majorid IN ('.$db->quote($asset->id).', '.$db->quote($sub_folder->id).')
$where = 'l.majorid = '.$db->quote($form->id).'
		AND i.inhd_type_code = '.$db->quote('form_submission').'
		AND a.created BETWEEN '.db_extras_todate($db, $from_value).'
			AND '.db_extras_todate($db, $to_value);
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
$sql = $sql.$where.' ORDER BY a.created DESC';
$assetids = $db->getCol($sql);
assert_valid_db_result($assetids);

if (empty($assetids)) {
	echo "No form submission found for '$asset->name' (#$assetid) within the specified date range\n";
	die;
}
echo 'Found '.count($assetids)." form submission(s) for '$asset->name' (#$assetid)\n(Date range: $from_value to $to_value)\n";

// quote the assetids to be used in the IN clause
foreach ($assetids as $key => $assetid) {
	$assetids[$key] = $db->quoteSmart((String)$assetid);
}

// break up the assets into chunks of 1000 so that oracle does not complain
$assetid_in = Array();
foreach (array_chunk($assetids, 999) as $chunk) {
	$assetid_in[] = ' assetid IN ('.implode(', ', $chunk).')';
}
$in1 = '('.implode(' OR ', $assetid_in).')';

$minorid_in = Array();
foreach (array_chunk($assetids, 999) as $chunk) {
	$minorid_in[] = ' minorid IN ('.implode(', ', $chunk).')';
}
$in2 = '('.implode(' OR ', $minorid_in).')';


// start removing entries from the database
echo "Removing assets ...\n";
$res = $db->query('DELETE FROM sq_ast WHERE '.$in1);
assert_valid_db_result($res);

echo "\tUpdating link table...\n";
$res = $db->query('DELETE FROM sq_ast_lnk WHERE '.$in2);
assert_valid_db_result($res);

echo "\tUpdating link tree table ...\n";
$res = $db->query('DELETE FROM sq_ast_lnk_tree WHERE linkid NOT IN (SELECT linkid FROM sq_ast_lnk)');
assert_valid_db_result($res);

echo "\tUpdating attribute value table ...\n";
$res = $db->query('DELETE FROM sq_ast_attr_val WHERE '.$in1);
assert_valid_db_result($res);

$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

echo "Done\n";
