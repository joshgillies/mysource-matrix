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
* $Id: search_replace_attribute_content.php,v 1.3 2011/12/19 01:20:27 ewang Exp $
*
*/

/**
* Replaces one string with another in a given attribute of a list of assets
*
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix
*/
echo 'Comment the code that stops this script from working. Protection against accidental execution.';
echo "\n";
exit;

// Assets you want to modify are here
$to_process = Array();

// Supply Your Strings Here
$search_for = 'NOTHING';
$replace_with = 'SOMETHING';

$attribute_name = 'html';

// --
// No configuration options below this comment
// --

$search_for = '/'.preg_quote($search_for, '/').'/';
$replace_with = $replace_with;

error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	trigger_error("You need to supply the path to the System Root as the first argument\n", E_USER_ERROR);
}

define('SQ_SYSTEM_ROOT', $SYSTEM_ROOT);
require_once SQ_SYSTEM_ROOT.'/core/include/init.inc';


$root_user =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');

// log in as root
if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	trigger_error("Failed logging in as root user\n", E_USER_ERROR);
}

$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

$db =& $GLOBALS['SQ_SYSTEM']->db;
$am =& $GLOBALS['SQ_SYSTEM']->am;

$start = microtime(TRUE);
$done = 0;
$total = count($to_process);
foreach ($to_process as $assetid) {
	// display progress
	$done++;
	if ($done % 50 == 0) {
		$elapsed = microtime(TRUE) - $start;
		if ($elapsed > 0) {
			$frac = $done / $total;
			$remain = $elapsed / $frac - $elapsed;
			$pct = $frac * 100;
			printf("%.2f%%: %d assets checked in %.2f seconds, %.2f remaining.\n", $pct, $done, $elapsed, $remain);
		}
	}

	$asset =& $am->getAsset($assetid);
	$attr_content = $asset->attr($attribute_name);

	$result_content = preg_replace($search_for, $replace_with, $attr_content);
	if ($result_content === $attr_content) {
		// No change - don't bother saving.
		echo "No change: $assetid\n";
		$am->forgetAsset($asset);
		continue;
	}

	$lock_success = $am->acquireLock($assetid, 'attributes');
	if (!$lock_success) {
		echo "\n".'FAILED Processing asset: '.$assetid."\n";
	}
	$asset->setAttrValue($attribute_name, $result_content);

	$asset->saveAttributes();
	$am->releaseLock($assetid, 'attributes');

	$am->forgetAsset($asset);

	echo "DONE: $assetid\n";
}


$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

echo "\n";
echo 'DONE';
echo "\n";

?>
