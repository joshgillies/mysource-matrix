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
* $Id: generate_install_key.php,v 1.8 2013/06/05 04:20:18 akarelia Exp $
*
*/

/**
* Generate Install Key
*
* Purpose
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.8 $
* @package MySource_Matrix
* @subpackage install
*/
ini_set('memory_limit', -1);
$SYSTEM_ROOT = '';

if ((php_sapi_name() == 'cli')) {
	if (isset($_SERVER['argv'][1])) {
		$SYSTEM_ROOT = $_SERVER['argv'][1];
	}

	$err_msg = "ERROR: You need to supply the path to the System Root as the first argument\n";

} else {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

if (empty($SYSTEM_ROOT)) {
	$err_msg .= "Usage: php install/step_02.php <PATH_TO_MATRIX>\n";
	echo $err_msg;
	exit();
}

if (!is_dir($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT.'/core/include/init.inc')) {
	$err_msg = "ERROR: Path provided doesn't point to a Matrix installation's System Root. Please provide correct path and try again.\n";
	$err_msg .= "Usage: php install/step_02.php <PATH_TO_MATRIX>\n";
	echo $err_msg;
	exit();
}

if (!defined('SQ_SYSTEM_ROOT')) {
	define('SQ_SYSTEM_ROOT',  $SYSTEM_ROOT);
}
require_once $SYSTEM_ROOT.'/core/include/init.inc';
require_once $SYSTEM_ROOT.'/install/install.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';

echo 'Generating install key...'."\n";

$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
$install_key = generate_install_key(TRUE);
$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

if (is_null($install_key)) {
	echo 'Could not generate an install key because the main.inc file was not found or is not accessible'."\n";
} else {
	echo 'Your system\'s install key is [ '.$install_key.' ]'."\n";
}
?>
