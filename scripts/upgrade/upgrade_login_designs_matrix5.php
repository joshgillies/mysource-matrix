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
*
*/

/**
* Upgrade login designs for Matrix 5
*
* Usage:  php install/upgrade_system_designs.php [PATH_TO_ROOT]
*
* @author  Edison Wang <ewang@squiz.com.au>
* @package MySource_Matrix
* @subpackage install
*/

ini_set('memory_limit', -1);
error_reporting(E_ALL);
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
	$err_msg .= "Usage: php install/step_01.php <PATH_TO_MATRIX>\n";
	echo $err_msg;
	exit();
}

if (!is_dir($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT.'/core/include/init.inc')) {
	$err_msg = "ERROR: Path provided doesn't point to a Matrix installation's System Root. Please provide correct path and try again.\n";
	$err_msg .= "Usage: php install/step_01.php <PATH_TO_MATRIX>\n";
	echo $err_msg;
	exit();
}

$SYSTEM_ROOT = realpath($SYSTEM_ROOT);

if (!defined('SQ_SYSTEM_ROOT')) {
	define('SQ_SYSTEM_ROOT',  $SYSTEM_ROOT);
}
require_once $SYSTEM_ROOT.'/core/include/init.inc';
require_once $SYSTEM_ROOT.'/install/install.inc';
require_once $SYSTEM_ROOT.'/core/include/general_occasional.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';

echo 'Updating login designs for Matrix 5'."\n";
echo "\n";	

$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);

$design_folder = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('designs_folder');
$children = $GLOBALS['SQ_SYSTEM']->am->getChildren($design_folder->id, 'design', FALSE);
$fv = $GLOBALS['SQ_SYSTEM']->getFileVersioning();

foreach ($children as $id => $content) {
	if(!isset($content[0]['type_code'])) {
		trigger_error('can not find type code of designs');
		continue;
	}

	echo "Updating design #".$id."\n";
	//parse file
	switch ($content[0]['type_code']) {
		case 'login_design':
			$new_parse_file = $SYSTEM_ROOT.'/core/assets/system/login_design/design_files/index.html';
			$reg_match = Array('This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd', 'mysource_files/login_image.gif', 'mysource_files/swish.gif', 'class="sq-credentials-section"');
			break;
		case 'password_change_design':
			$new_parse_file = $SYSTEM_ROOT.'/core/assets/system/password_change_design/design_files/index.html';
			$reg_match = Array('This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd', 'mysource_files/login_image.gif', 'mysource_files/swish.gif', 'MySource_PASSWORD_CHANGE_SECTION');
			break;
		case 'ees_login_design':
			$new_parse_file = $SYSTEM_ROOT.'/core/assets/system/ees_login_design/design_files/index.html';
			$reg_match = Array('<title>Login - Squiz Matrix</title>', 'mysource_files/matrix-logo.png', 'Start CMS Login', 'PAGE GENERATED STAMP GOES HERE.');
			break;
		default:
			continue;
	}//end switch

	$design = $GLOBALS['SQ_SYSTEM']->am->getAsset($id);
	$type_code = ucwords(str_replace('_', ' ', $design->type()));

	
	// only upgrade login design if it matches those keywords, which means it hasn't been customised.
	$parse_file = $design->data_path.'/parse.txt';
	$parse_file_contents = file_get_contents($parse_file);
	$can_upgrade = FALSE;
	foreach ($reg_match as $match) {
	  if (strpos($parse_file_contents, $match) !== FALSE) {
	    $can_upgrade = TRUE;
	    break;
	  }
	}
	
	if (!$can_upgrade) {
		echo 'Parse file in '.$type_code.' was modified. Skipping.'."\n";
	}
	else if (!is_file($parse_file) || !is_file($new_parse_file)) {
		trigger_error ('parse file is not available');
	}
	else {		
		if($design->type() === 'password_change_design') {
		    $design->restoreChangePasswordDesign();
		}
		else {
		    $design->restoreLoginDesign();
		}
			
		echo 'Parse file in '.$type_code.' Successfully updated...'."\n";
	}	
}

echo "\n".'All desings updated successfully'."\n";

$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
$GLOBALS['SQ_SYSTEM']->am->forgetAsset($design_folder->id, TRUE);


?>
