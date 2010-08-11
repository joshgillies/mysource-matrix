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
* $Id: update_squiz_logo_login_design.php,v 1.2 2010/08/11 05:12:51 ewang Exp $
*
*/

/**
* Update new Squiz Matrix logo images for Matrix 4.0.0 in all login designs including
* System Login Design, EES Login Design, Password Change Design
*
* Usage:  php install/update_squiz_logo_login_design.php [PATH_TO_ROOT]
*
* @author  Edison Wang <ewang@squiz.com.au>
* @version $$
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

	$err_msg = "You need to supply the path to the System Root as the first argument\n";

} else {
	if (isset($_GET['SYSTEM_ROOT'])) {
		$SYSTEM_ROOT = $_GET['SYSTEM_ROOT'];
	}

	$err_msg = '
	<div style="background-color: red; color: white; font-weight: bold;">
		You need to supply the path to the System Root as a query string variable called SYSTEM_ROOT
	</div>
	';
}

if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	trigger_error($err_msg, E_USER_ERROR);
}

$SYSTEM_ROOT = realpath($SYSTEM_ROOT);

if (!defined('SQ_SYSTEM_ROOT')) {
	define('SQ_SYSTEM_ROOT',  $SYSTEM_ROOT);
}
require_once $SYSTEM_ROOT.'/core/include/init.inc';
require_once $SYSTEM_ROOT.'/install/install.inc';
require_once $SYSTEM_ROOT.'/core/include/general_occasional.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';

echo 'Updating new Squiz Logo for login design, password change design, EES login design ...'."\n";

do {
	echo "Note: If the Squiz Logo image have been customized/modified in above login designs, this script will overwrite the change. Proceed? (Yes/No) : ";
	$answer = rtrim(fgets(STDIN, 4094));
	if ($answer == 'Yes') {
		break;
	} else if ($answer == 'No') {
		echo "\nBye\n";
		exit();
	}
} while (TRUE);
	
$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);

$design_folder = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('designs_folder');
$children = $GLOBALS['SQ_SYSTEM']->am->getChildren($design_folder->id, 'design', FALSE);
$fv = $GLOBALS['SQ_SYSTEM']->getFileVersioning();

foreach ($children as $id => $content) {
	if(!isset($content[0]['type_code'])) {
		trigger_error('can not find type code of designs');
		continue;
	}
	switch ($content[0]['type_code']) {
		case 'login_design':
			$file = 'login_image.gif';
			$path = $SYSTEM_ROOT.'/core/assets/system/login_design/design_files/files/'.$file;
			break;
		case 'password_change_design':
			$file = 'login_image.gif';
			$path = $SYSTEM_ROOT.'/core/assets/system/password_change_design/design_files/files/'.$file;
			break;
		case 'ees_login_design':
			$file = 'matrix-logo.png';
			$path = $SYSTEM_ROOT.'/core/assets/system/ees_login_design/design_files/files/'.$file;
			break;
	}

	$design = $GLOBALS['SQ_SYSTEM']->am->getAsset($id);
	$ef = $design->getEditFns();
	$info = Array();
	$info['name'] = $file;
	$info['tmp_name'] = $path;
	$info['non_uploaded_file'] = TRUE;
	if (!$ef->_processUploadedFile($design, $info)) {
		trigger_localised_error('CORE0030', E_USER_WARNING, $file);
	}
}

echo 'Squiz logo has been successfully updated'."\n";

$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

?>
