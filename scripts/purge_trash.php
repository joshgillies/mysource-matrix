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
* $Id: purge_trash.php,v 1.1 2007/04/23 04:03:27 rong Exp $
*
*/

/**
* Use this script to clear the trash
*
* Usage: php scripts/purge_trash.php [SYSTEM ROOT]
* Runs a Freestyle HIPO that purges all assets from the trash
* Best suited to be run at a scheduled time by cron or similar.
*
* @version $Revision: 1.1 $
* @package MySource_Matrix
*/

error_reporting(E_ALL);
if (php_sapi_name() != 'cli') {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	trigger_error("You need to supply the path to the System Root as the first argument\n", E_USER_ERROR);
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';
require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

error_reporting(E_ALL);
ini_set('memory_limit', '-1');

// ask for the root password for the system
echo 'Enter the root password for "'.SQ_CONF_SYSTEM_NAME.'": ';
$root_password = rtrim(fgets(STDIN, 4094));

// check that the correct root password was entered
$root_user = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
if (!$root_user->comparePassword($root_password)) {
	echo "ERROR: The root password entered was incorrect\n";
	exit();
}

if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	trigger_error("Failed logging in as root user\n", E_USER_ERROR);
}

$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();
$vars = Array();
$errors = $hh->freestyleHipo('hipo_job_purge_trash', $vars);
if (count($errors)) {
	echo print_r($errors, true);
} else {
	echo "\npurge_trash.php: Completed.\n";
}

?>

