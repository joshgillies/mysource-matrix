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
* $Id: system_integrity_deleted_user_perms.php,v 1.10 2006/12/06 05:39:51 bcaldwell Exp $
*
*/

/**
* Delete permissions that exist for deleted or non-existant users (eg. LDAP users that no longer
* exist)
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.10 $
* @package MySource_Matrix
*/
error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) trigger_error("You can only run this script from the command line\n", E_USER_ERROR);

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	echo "ERROR: You need to supply the path to the System Root as the first argument\n";
	exit();
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';

$ROOT_ASSETID = (isset($_SERVER['argv'][2])) ? $_SERVER['argv'][2] : '1';
if ($ROOT_ASSETID == 1) {
	echo "\nWARNING: You are running this integrity checker on the whole system.\nThis is fine but:\n\tit may take a long time; and\n\tit will acquire locks on many of your assets (meaning you wont be able to edit content for a while)\n\n";
}

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
	trigger_error("Failed login in as root user\n", E_USER_ERROR);
}

$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
$db =& $GLOBALS['SQ_SYSTEM']->db;

//--        CLEANING PERMISSIONS        --//
echo 'Cleaning up permissions...'."\n";

$sql = 'SELECT
			DISTINCT userid
		FROM sq_ast_perm
		ORDER BY userid';
$user_ids = $db->getCol($sql);

foreach ($user_ids as $user_id) {

	$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($user_id, '', TRUE);
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

	$sql = 'DELETE FROM sq_ast_perm WHERE userid = '.$db->quote($user_id);

	$result = $db->query($sql);
	if (assert_valid_db_result($result)) {
		// all good
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		printUpdateStatus('FIXED');
	} else {
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
$user_ids = $db->getCol($sql);

foreach ($user_ids as $user_id) {

	$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($user_id, '', TRUE);
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

	$sql = 'DELETE FROM sq_ast_role WHERE userid = '.$db->quote($user_id);

	$result = $db->query($sql);
	if (assert_valid_db_result($result)) {
		// all good
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		printUpdateStatus('FIXED');
	} else {
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
