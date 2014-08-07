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
* $Id: update_squiz_logo_login_design.php,v 1.8 2013/06/05 04:20:18 akarelia Exp $
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

echo 'Updating new Squiz Logo for login design, password change design, EES login design ...'."\n";

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
			//parse file
			$new_parse_file = $SYSTEM_ROOT.'/core/assets/system/ees_login_design/design_files/index.html';
			$parse_file_md5_3_28 = 'cd9c6761bae67fdc78474a540a096e58';
			$parse_file_md5_3_29 = '15c8ab31d536fac3c6035da4d601e690';
			break;
		default:
			continue;
	}

	$design = $GLOBALS['SQ_SYSTEM']->am->getAsset($id);
	$design_edit_fns = $design->getEditFns();
	$type_code = $design->type();

	
	// Update design parse file for ees_login_design, there is some css changes
	if($type_code === 'ees_login_design') {
		$parse_file = $design->data_path.'/parse.txt';
		$parse_file_md5 = md5(file_get_contents($parse_file));
		if($parse_file_md5 !== $parse_file_md5_3_28 && $parse_file_md5 !== $parse_file_md5_3_29) {
			echo 'Parse file in '.$type_code.' was modified. Skip...'."\n";
		}
		else if (!is_file($parse_file) || !is_file($new_parse_file)) {
			trigger_error ('parse file is not available');
		}
		else {		
			
			// update the parse file
			if(!_updateFile($new_parse_file, 'parse.txt', $design->data_path, $design->data_path_suffix)) {
				trigger_error('failed to update ees parse file '.$file_name);
				exit();
			}

			$design_edit_fns->parseAndProcessFile($design);
			$design->generateDesignFile();
			
			echo 'Parse file in ees_login_design. Successfully updated...'."\n";
		}	
	}
	
	
	// Update logo
	$existing_ids = Array();
	$existing = $GLOBALS['SQ_SYSTEM']->am->getLinks($design->id, SQ_LINK_TYPE_2, 'file', FALSE);

	foreach ($existing as $link) {
		$existing_ids[$link['minorid']] = $link['linkid'];
	}

	$existing_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_keys($existing_ids));

	// must web-path-ify the new file name to compare, as this is eventually
	// it's what the file will be named
	$file_name = make_valid_web_paths(Array($file_name));
	$file_name = array_shift($file_name);

	$existing_fileid = 0;
	// only pick up the file we want to update
	foreach ($existing_info as $asset_id => $asset_info) {
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
		echo $file_name.' in '.$type_code.' was modified. Skip...'."\n";
		continue;
	}
	
	// update the actual logo file
	if(!_updateFile($path, $file_name, $file->data_path, $file->data_path_suffix)) {
		trigger_error('failed to update logo '.$file_name);
		exit();
	}


	echo $file_name.' in '.$type_code.'. Successfully updated...'."\n";
	
}

echo 'Done'."\n";

$GLOBALS['SQ_SYSTEM']->restoreRunLevel();




/**
 * _updateFile
 * Update the target file by removing it first and then replace with a new file, update file versioning of it
 *
 * @return Boolean
 */
function _updateFile ($new_file, $file_name, $data_path, $data_path_suffix) {
	require_once SQ_FUDGE_PATH.'/general/file_system.inc';
	$fv = $GLOBALS['SQ_SYSTEM']->getFileVersioning();
	
	$file_path = $data_path.'/'.$file_name;
	
	if (!unlink($file_path)) {
		trigger_error('failed to remove old file '.$file_name);
		return FALSE;
	}
	
	// copy over the new logo
	if (string_to_file(file_get_contents($new_file), $file_path)) {
		// add a new version to the repository
		$file_status = $fv->upToDate($file_path);
		if (FUDGE_FV_MODIFIED & $file_status) {
			if (!$fv->commit($file_path, '')) {
				trigger_localised_error('CORE0160', E_USER_WARNING);
			}
		}
	} else {
		trigger_error('Can not overwrite old file '.$file_name);
	}

	// make sure we have the latest version of our file
	if (!$fv->checkOut($data_path_suffix.'/'.$file_name, $data_path)) {
		trigger_localised_error('CORE0032', E_USER_WARNING);
		return FALSE;
	}//end if
	

	return TRUE;
}//end _updateFile

?>
