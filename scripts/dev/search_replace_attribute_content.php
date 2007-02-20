<?php
echo 'Uncomment the code that stops this from working. Protection against accidental execution.';
echo "\n";
exit;

# Assets you want to modify are here
$to_process = Array();

# Supply Your Strings Here
$search_for = 'NOTHING';
$replace_with = 'SOMETHING';

$attribute_name = 'html';

## --
## No configuration options below this comment
## --

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

// start matrix

// load asset ids

$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

$db =& $GLOBALS['SQ_SYSTEM']->db;
$am =& $GLOBALS['SQ_SYSTEM']->am;


foreach ($to_process as $assetid) {
	$asset =& $am->getAsset($assetid);
	$attr_content = $asset->attr($attribute_name);

	$result_content = preg_replace($search_for, $replace_with, $attr_content);

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
echo "DONE";
echo "\n";

?>
