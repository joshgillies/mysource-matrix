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
* $Id: system_integrity_deleted_user_perms.php,v 1.14.2.1 2012/10/09 01:04:43 akarelia Exp $
*
*/

/**
* Delete permissions that exist for deleted or non-existant users (eg. LDAP users that no longer
* exist)
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.14.2.1 $
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

$ROOT_ASSETID = (isset($_SERVER['argv'][2])) ? $_SERVER['argv'][2] : '1';
if ($ROOT_ASSETID == 1) {
	echo "\nWARNING: You are running this integrity checker on the whole system.\nThis is fine but:\n\tit may take a long time; and\n\tit will acquire locks on many of your assets (meaning you wont be able to edit content for a while)\n\n";
}

$root_user = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');

// log in as root
if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	echo "ERROR: Failed login in as root user\n";
	exit();
}

$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
$db =& $GLOBALS['SQ_SYSTEM']->db;

//--        CLEANING PERMISSIONS        --//
echo 'Cleaning up permissions...'."\n";

$sql = 'SELECT
			DISTINCT userid
		FROM sq_ast_perm
		ORDER BY userid';
$user_ids = MatrixDAL::executeSqlAssoc($sql, 0);

foreach ($user_ids as $user_id) {

	$id_parts = explode(':', $user_id);
	if (isset($id_parts[1])) {
		$real_assetid = $id_parts[0];
		$bridge = $GLOBALS['SQ_SYSTEM']->am->getAsset($real_assetid, '', TRUE);
		if (is_null($bridge)) {
			// bridge is unknown, we cannot return anything from it
			$asset = NULL;
		} else {
			$asset = $bridge->getAsset($user_id, '', TRUE, TRUE);
		}
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($bridge);

	} else {
		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($user_id, '', TRUE);
	}
	if (!is_null($asset)) {
		// print info the asset, as it exists
		printAssetName($asset);
		printUpdateStatus('OK');

		// conserve memory and move on to the next perm
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
		continue;
	}

	// the asset doesn't exist
	$dummy_asset->id = $user_id;
	$dummy_asset->name = 'Unknown Asset';

	printAssetName($dummy_asset);

	// open the transaction
	$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

	try {
		$sql = 'DELETE FROM sq_ast_perm WHERE userid = :userid';
		$query = MatrixDAL::preparePdoQuery($sql);
		MatrixDAL::bindValueToPdo($query, 'userid', $user_id);
		MatrixDAL::execPdoQuery($query);

		// all good
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		printUpdateStatus('FIXED');
	} catch (DALException $e) {
		// no good
		$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		printUpdateStatus('FAILED');
	}

}//end for

//--        CLEANING ROLES        --//
// Note that userid of 0 is legitimate as this indicates a global role

echo 'Cleaning up roles...'."\n";

$sql = 'SELECT
			DISTINCT userid
		FROM sq_ast_role
		WHERE userid <> 0
		ORDER BY userid';
$user_ids = MatrixDAL::executeSqlAssoc($sql, 0);

foreach ($user_ids as $user_id) {

	$id_parts = explode(':', $user_id);
	if (isset($id_parts[1])) {
		$real_assetid = $id_parts[0];
		$bridge = $GLOBALS['SQ_SYSTEM']->am->getAsset($real_assetid, '', TRUE);
		if (is_null($bridge)) {
			// bridge is unknown, we cannot return anything from it
			$asset = NULL;
		} else {
			$asset = $bridge->getAsset($user_id, '', TRUE, TRUE);
		}
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($bridge);

	} else {
		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($user_id, '', TRUE);
	}
	if (!is_null($asset)) {
		// print info the asset, as it exists
		printAssetName($asset);
		printUpdateStatus('OK');

		// conserve memory and move on to the next perm
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
		continue;
	}

	// the asset doesn't exist
	$dummy_asset->id = $user_id;
	$dummy_asset->name = 'Unknown Asset';

	printAssetName($dummy_asset);

	// open the transaction
	$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

	try {
		$sql = 'DELETE FROM sq_ast_role WHERE userid = :userid';
		$query = MatrixDAL::preparePdoQuery($sql);
		MatrixDAL::bindValueToPdo($query, 'userid', $user_id);
		MatrixDAL::execPdoQuery($query);

		// all good
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		printUpdateStatus('FIXED');
	} catch (DALException $e) {
		// no good
		$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		printUpdateStatus('FAILED');
	}

}//end for


/**
* Prints the name of the Asset as a padded string
*
* Pads name to 40 columns
*
* @param string	$name	the name of the container
*
* @return void
* @access public
*/
function printAssetName(&$asset)
{
	$str = $asset->name . ' [ # '. $asset->id. ' ]';
	printf ('%s%'.(80 - strlen($str)).'s', $str,'');

}//end printAssetName()


/**
* Prints the status of the container integrity check
*
* @param string	status	the status of the check
*
* @return void
* @access public
*/
function printUpdateStatus($status)
{
	echo "[ $status ]\n";

}//end printUpdateStatus()


?>
