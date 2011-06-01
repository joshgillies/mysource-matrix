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
* $Id: add_user_own_access.php,v 1.6 2008/09/16 06:58:23 ewang Exp $
*
*/

/**
* Ensures that each user in the system has
* read access themselves, provided as a
* failsafe in case the permissions have
* become corrupted.
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.6 $
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

// go through each user in the system, lock it, set permissions, unlock it
$assets = $GLOBALS['SQ_SYSTEM']->am->getChildren($ROOT_ASSETID, 'user', false);
foreach ($assets as $assetid => $type_code_data) {
	$type_code = $type_code_data[0]['type_code'];
	$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $type_code);
	printAssetName($asset);

	// try to lock the asset
	if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($asset->id, 'permissions')) {
		printUpdateStatus('LOCK');
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
		continue;
	}

	// give the user read access to him/herself
	if (!$GLOBALS['SQ_SYSTEM']->am->setPermission($asset->id, $asset->id, SQ_PERMISSION_READ, 1)) {
		printUpdateStatus('FAILED');
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
		continue;
	}

	$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'asset', false, 'major', '', 1);
	foreach ($links as $link) {
		if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($link['minorid'], 'permissions')) {
			printUpdateStatus('SUB LOCK');
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			continue;
		}

		if (!$GLOBALS['SQ_SYSTEM']->am->setPermission($link['minorid'], $asset->id, SQ_PERMISSION_READ, 1)) {
			printUpdateStatus('SUB FAILED');
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			continue;
		}

		// try to unlock the asset
		if (!$GLOBALS['SQ_SYSTEM']->am->releaseLock($link['minorid'], 'permissions')) {
			printUpdateStatus('SUB !!');
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			continue;
		}
	}

	// try to unlock the asset
	if (!$GLOBALS['SQ_SYSTEM']->am->releaseLock($asset->id, 'permissions')) {
		printUpdateStatus('!!');
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
		continue;
	}

	printUpdateStatus('OK');
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
