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
* $Id: move_assets.php,v 1.6 2007/01/30 00:44:51 rong Exp $
*
*/

/**
* Move all assets of a single type from one specified folder to another
* args: system-root, from-folder id, to-folder id, asset-type
*
* @author  Matt Keehan <mkeehan@squiz.co.uk>
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

//FROM FOLDER
$FROM_FOLDER = (isset($_SERVER['argv'][2])) ? $_SERVER['argv'][2] : '';
if (empty($FROM_FOLDER)) {
	echo "ERROR: You need to provide the from-folder-id as the second argument\n";
	exit();
}

//TO FOLDER
$TO_FOLDER = (isset($_SERVER['argv'][3])) ? $_SERVER['argv'][3] : '';
if (empty($TO_FOLDER)) {
	echo "ERROR: You need to provide the to-folder-id as the third argument\n";
	exit();
}

//ASSET_TYPE
$ASSET_TYPE = (isset($_SERVER['argv'][4])) ? $_SERVER['argv'][4] : '';
if (empty($ASSET_TYPE)) {
	echo "ERROR: You need to enter the asset_type as the fourth argument\n";
	exit();
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';

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

//working as root... do whatever the hell we want :)
$assets = $GLOBALS['SQ_SYSTEM']->am->getChildren($FROM_FOLDER, $ASSET_TYPE, false);
$i = 0;
foreach ($assets as $assetid => $type_code) {
	$GLOBALS['SQ_SYSTEM']->am->acquireLock($assetid, 'all');
	_getCurrentLinkId($assetid);
	$GLOBALS['SQ_SYSTEM']->am->moveLink(_getCurrentLinkId($assetid), $TO_FOLDER, SQ_LINK_TYPE_1, 0);
	//now change the assets own record of it's link
	$my_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
	$my_asset->saveAttributes();
	$my_asset->updateLookups();
	$GLOBALS['SQ_SYSTEM']->am->forgetAsset($my_asset);
	$i++;
}
echo 'moved ' . $i . ' assets of type ' . $ASSET_TYPE . "\n";
echo "\nScript has finished\n";

function _getCurrentLinkId($asset){
	$return_info = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset, 1,'',true,'minor');
	return($return_info[0]['linkid']);
}

?>
