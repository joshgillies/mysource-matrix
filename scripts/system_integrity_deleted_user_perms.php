<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: system_integrity_deleted_user_perms.php,v 1.2 2004/06/17 00:14:21 lwright Exp $
* $Name: not supported by cvs2svn $
*/

/**
* Delete permissions that exist for deleted or non-existant users (eg. LDAP users that no longer
* exist)
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Version$ - 1.0
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

// go trough each wysiwyg in the system, lock it, validate it, unlock it
$assets = $GLOBALS['SQ_SYSTEM']->am->getChildren($ROOT_ASSETID, 'asset', false);
foreach ($assets as $assetid => $type_code) {
	
	$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $type_code);
	printAssetName($asset);
	
	// try to lock the asset
	if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($asset->id, 'permissions')) {
		printUpdateStatus('LOCK');
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
		continue;
	}
	
	// what is in the permissions table?
	$db =& $GLOBALS['SQ_SYSTEM']->db;
	$sql = 'SELECT userid, permission FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_permission WHERE assetid='.$db->quote($asset->id);
	$perms_info = $db->getAll($sql);
	assert_valid_db_result($perms_info);

	$updates = false;

	foreach($perms_info as $perm_info) {
		// Check that each user is in the asset table, if not then delete the permission
		$user_avail = array_keys($GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($perm_info['userid']), 'user', false));
	
		if(empty($user_avail) && ($perm_info['userid'] != 0)) {
			if (!$GLOBALS['SQ_SYSTEM']->am->deletePermission($asset->id, $perm_info['userid'], $perm_info['permission'])) {
				printUpdateStatus('FAILED');
				continue;
			}
			$updates = true;
		}
	}

	// try to unlock the asset
	if (!$GLOBALS['SQ_SYSTEM']->am->releaseLock($asset->id, 'permissions')) {
		printUpdateStatus('!!');
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
		continue;
	}

	if ($updates) {
		printUpdateStatus('OK');
	} else {
		printUpdateStatus('--');
	}
	$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
	
}//end foreach


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
	printf ('%s%'.(40 - strlen($str)).'s', $str,'');
	
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
