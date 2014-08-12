<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: upgrade_asset_metadata_db_entries.php,v 1.1 2013/08/22 00:54:28 cupreti Exp $
*
*/

/**
* After #6484 changes, ast_mdata_val table contains new field 'use_default' (default value '1').
* If value set in non-default context and field in non-contextable, its value is set to '0'
* 
* Updates the value of 'use_default' column in the ast_mdata_val table to '0'
* for all the existing entries having non-contextable value or if is default context
*
* @author Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: 1.1 $
* @package MySource_Matrix
*/

// Usage: php upgrade_asset_metadata_db_entries.php <SYSTEM_ROOT>

if (ini_get('memory_limit') != '-1') ini_set('memory_limit', -1);
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

echo "Upgrading asset metadata db entris ";

$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
        echo "ERROR: Failed logging in as root user\n";
        exit();
}

// Get all the non-default context metadata entries in the system
$sql = "SELECT assetid, fieldid, contextid FROM sq_ast_mdata_val WHERE contextid <> '0'";
$records = MatrixDAL::executeSqlAssoc($sql);

// Group by metadata field assetid to limit the am->getAsset() call
$field_metadata = Array();
foreach($records as $count => $record) {
	if (!$record['assetid'] || !$record['fieldid'] || !$record['contextid']) {
		continue;
	}
	$field_metadata[$record['fieldid']][] = $record;
}//end foreach

$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

$count = 0;
foreach($field_metadata as $fieldid => $field_data) {
	$field = $GLOBALS['SQ_SYSTEM']->am->getAsset($fieldid);
	if ($field->attr('is_contextable') && !($field instanceof Metadata_Field_Select)) {
		foreach($field_data as $record) {

			$sql = "UPDATE sq_ast_mdata_val SET use_default = '0' WHERE assetid=:aid AND fieldid=:fid AND contextid=:cid";
			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'aid', $record['assetid']);
			MatrixDAL::bindValueToPdo($query, 'fid', $record['fieldid']);
			MatrixDAL::bindValueToPdo($query, 'cid', $record['contextid']);
			$success = MatrixDAL::execPdoQuery($query);
			echo ++$count%50 ? '' : '.';
		}//end foreach
	}//end if
	$GLOBALS['SQ_SYSTEM']->am->forgetAsset($field, TRUE);
	echo '.';

}//end foreach

$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

echo "\n".$count. " asset metadata db entries were upgraded.";
echo "\n";
?>
