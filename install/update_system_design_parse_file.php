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
* $Id: update_system_design_parse_file.php,v 1.5 2013/06/05 04:20:18 akarelia Exp $
*
*/

/**
* Fixes the EES Design parse file. 
* Bug #5038: HTML Doctype header should at first line in the HTML document
*
* Usage:  php install/update_ees_login_parse_file.php [PATH_TO_ROOT]
*
* @author  Edison Wang <ewang@squiz.com.au>
* @version $Revision: 1.5 $
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

echo 'Updating parse files of login designs for #5038'."\n";
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
			$source_parse_file_md5 = '38c31c49d16ae6dee3e2a47e693ba003';
			break;
		case 'password_change_design':
			$new_parse_file = $SYSTEM_ROOT.'/core/assets/system/password_change_design/design_files/index.html';
			$source_parse_file_md5 = '228a10aa65433b652f32f1c3b4037b3e';
			break;
		case 'ees_login_design':
			$new_parse_file = $SYSTEM_ROOT.'/core/assets/system/ees_login_design/design_files/index.html';
			$source_parse_file_md5 = 'f0acced582b5b9d355ce2a4b22595cc1';
			break;
		default:
			continue;
	}//end switch

	$new_file = file_get_contents($new_parse_file);
	$design = $GLOBALS['SQ_SYSTEM']->am->getAsset($id);
	$design_edit_fns = $design->getEditFns();
	$type_code = ucwords(str_replace('_', ' ', $design->type()));

	
	// Update design parse file for ees_login_design, there is some css changes
	$parse_file = $design->data_path.'/parse.txt';
	$ext_file = file_get_contents($parse_file);
	$parse_file_md5 = md5(file_get_contents($parse_file));
	if ($parse_file_md5 != $source_parse_file_md5) {
		echo 'Parse file in '.$type_code.' was modified. Skipping.'."\n";
	}
	else if (!is_file($parse_file) || !is_file($new_parse_file)) {
		trigger_error ('parse file is not available');
	}
	else {		
		
		// update the parse file
		if(!_updateFile($new_parse_file, 'parse.txt', $design->data_path, $design->data_path_suffix)) {
			trigger_error('failed to update parse file '.$new_parse_file);
			exit();
		}

		$design_edit_fns->parseAndProcessFile($design);
		$design->generateDesignFile();
			
		echo 'Parse file in '.$type_code.' Successfully updated...'."\n";
	}	
}

echo "\n".'All desings updated successfully'."\n";

$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
$GLOBALS['SQ_SYSTEM']->am->forgetAsset($design_folder->id, TRUE);



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
