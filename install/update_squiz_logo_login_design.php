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
* $Id: update_squiz_logo_login_design.php,v 1.3 2010/08/12 00:43:40 ewang Exp $
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
	echo "Note: If the Logo image have been modified in above designs, this script will skip it. Proceed? (Yes/No) : ";
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
			$file_name = 'login_image.gif';
			$path = $SYSTEM_ROOT.'/core/assets/system/login_design/design_files/files/'.$file_name;
			$md5 = '99029df0982274ce2f1c3404b6902f0f';
			break;
		case 'password_change_design':
			$file_name = 'login_image.gif';
			$path = $SYSTEM_ROOT.'/core/assets/system/password_change_design/design_files/files/'.$file_name;
			$md5 = '99029df0982274ce2f1c3404b6902f0f';
			break;
		case 'ees_login_design':
			$file_name = 'matrix-logo.png';
			$path = $SYSTEM_ROOT.'/core/assets/system/ees_login_design/design_files/files/'.$file_name;
			$md5 = '9ade1af14bab1611aef70b91f1401d3e';
			break;
	}

	$design = $GLOBALS['SQ_SYSTEM']->am->getAsset($id);
	$info = Array();
	$info['name'] = $file_name;
	$info['tmp_name'] = $path;
	$info['non_uploaded_file'] = TRUE;

	$existing_ids = Array();
	$existing = $GLOBALS['SQ_SYSTEM']->am->getLinks($design->id, SQ_LINK_TYPE_2, 'file', FALSE);

	foreach ($existing as $link) {
		$existing_ids[$link['minorid']] = $link['linkid'];
	}

	$existing_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_keys($existing_ids));

	// must web-path-ify the new file name to compare, as this is eventually
	// it's what the file will be named
	$file_name = make_valid_web_paths(Array($info['name']));
	$file_name = array_shift($file_name);

	$existing_fileid = 0;
	foreach ($existing_info as $asset_id => $asset_info) {
		// if the name is the same, delete the link
		if ($asset_info['name'] == $file_name) {
			$existing_fileid = $asset_id;
			break;
		}
	}
	
	if(empty($existing_fileid)) {
		trigger_error('Can not find original logo image');
		exit();
	}
	
	// we already have a design file with the same name, so just upload over the top of it
	$file = $GLOBALS['SQ_SYSTEM']->am->getAsset($existing_fileid);

	// check MD5 hash, make sure it is the file we want to update
	$file_info = $file->getExistingFile();
	if(md5_file($file_info['path']) !== $md5) {
		trigger_error('MD5 hash does not match, '.$file_name.' has been previously modified. Skip...');
		continue;
	}
	
	$lock_status = $GLOBALS['SQ_SYSTEM']->am->acquireLock($file->id, 'attributes');
	$edit_fns = $file->getEditFns();
	$o = NULL;
	$success = $edit_fns->processFileUpload($file, $o, $file->getPrefix(), $info);

	if ($lock_status === 1) {
		$GLOBALS['SQ_SYSTEM']->am->releaseLock($file->id, 'attributes');
	}

	if(!$success) {
		trigger_error('Failed to update squiz logo image');
		exit();
	}
	echo $file_name.' in '.$content[0]['type_code'].' has been successfully updated'."\n";
}

echo 'Done'."\n";

$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

?>
