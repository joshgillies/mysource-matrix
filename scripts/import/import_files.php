<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: import_files.php,v 1.6 2005/02/24 21:54:57 gsherwood Exp $
*
*/

/**
* Creates file assets based on uploaded files sitting on the server
* You need to create a directory for each parent asset you will be
* uploading files to, e.g. import/20, import/24 (where 20 and 24 are the
* asset IDs of the target parents. Each file found in those directory's will
* be linked appropriately.
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.6 $
* @package MySource_Matrix
*/
error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) trigger_error("You can only run this script from the command line\n", E_USER_ERROR);

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	trigger_error("You need to supply the path to the System Root as the first argument\n", E_USER_ERROR);
}

$import_home_dir = (isset($_SERVER['argv'][2])) ? $_SERVER['argv'][2] : '';
if (empty($import_home_dir) || !is_dir($import_home_dir)) {
	trigger_error("You need to supply the path to the import directory as the second argument\n", E_USER_ERROR);
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';

$GLOBALS['SQ_SYSTEM']->am->includeAsset('file');
$GLOBALS['SQ_SYSTEM']->am->includeAsset('image');
$GLOBALS['SQ_SYSTEM']->am->includeAsset('pdf_file');
$GLOBALS['SQ_SYSTEM']->am->includeAsset('word_doc');

$import_dirs = list_dirs($import_home_dir, false);
foreach ($import_dirs as $import_dir) {

	$import_path = $import_home_dir.'/'.$import_dir;

	$parent_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset(trim($import_dir));
	if (is_null($parent_asset)) trigger_error("New parent asset #$parent_assetid does not exist\n", E_USER_ERROR);

	$import_link = Array('asset' => &$parent_asset, 'link_type' => SQ_LINK_TYPE_1);

	// get a list of all files in the import directory
	$files = list_files($import_path);
	$GLOBALS['SQ_INSTALL'] = true;
	foreach ($files as $filename) {
		switch (get_file_type($filename)) {
			case 'doc' :
			case 'dot' :
				$new_asset_type = 'word_doc';
				break;
			case 'pdf' :
				$new_asset_type = 'pdf_file';
				break;
			case 'gif' :
			case 'jpg' :
			case 'jpeg' :
			case 'png' :
				$new_asset_type = 'image';
				break;
			default :
				$new_asset_type = 'file';
				break;
		}

		// create an asset under the new parent of the correct type
		$temp_info = Array('name' => $filename, 'tmp_name' => $import_path.'/'.$filename, 'non_uploaded_file' => true);

		$new_file = new $new_asset_type();
		$new_file->_tmp['uploading_file'] = true;
		$new_file->setAttrValue('name', $filename);

		if (!$new_file->create($import_link, $temp_info)) {
			trigger_error('Failed to import '.$new_asset_type.' '.$filename, E_USER_WARNING);
		} else {
			bam('New '.$new_file->type().' asset created for file '.$filename.' - asset ID #'.$new_file->id);
		}
	}
}

unset($GLOBALS['SQ_INSTALL']);

?>