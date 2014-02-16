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
* $Id: import_files.php,v 1.16 2012/09/19 04:40:24 ewang Exp $
*
*/

/**
* Creates file assets based on uploaded files sitting on the server
* You need to create a directory for each parent asset you will be
* uploading files to, e.g. import/20, import/24 (where 20 and 24 are the
* asset IDs of the target parents. Each file found in those directory's will
* be linked appropriately.
*
* if a third argument is provided the whole folder and its children (files and sub folders)
* will be imported into the matrix system.
*
*************************************************************************************************************************
* = IMPORTANT: by DEFAULT unrestricted Access is set to FALSE                                                           *
*if you need to upload the files with unrestricted access set to true you will need to add a fourth argument equal to 1 *
*************************************************************************************************************************
*
* The name of the folders will be used to create matrix asset
* (i.e.: The file and folder structure you want to import can be:
* folders_to_import
* 		|- folder_1
* 		|		|- image.jpg
* 		|		|- folder_1_2
* 		|				|- another_picture.jpg
* 		|- file1.pdf
* 		|- another_document.doc
*
* The exact same structure will be created into matrix)
*
*
* USAGE:
* php scripts/import/import_files.php  . folders_to_import 66 [--sort]
* or
* php scripts/import/import_files.php  . folders_to_import 66 1 [--sort]
* or
* php scripts/import/import_files.php  . folders_to_import [--sort]
*
* first argument matrix root folder
* second argument folder to import
* third argument matrix asset id where you want to import the folders and files
* fourth argument is equals to 1 allow unrestricted access will be set to be true
* fifth argument, if set to --sort the files be alphanumerically sorted before importing
*
* @author Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.16 $
* @package MySource_Matrix
*/
error_reporting(E_ALL);
if (ini_get('memory_limit') != '-1') ini_set('memory_limit', '-1');

if ((php_sapi_name() != 'cli')) {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT)) {
	echo "ERROR: You need to supply the path to the System Root as the first argument\n";
	exit();
}

if (!is_dir($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT.'/core/include/init.inc')) {
	echo "ERROR: Path provided doesn't point to a Matrix installation's System Root. Please provide correct path and try again.\n";
	exit();
}

$import_home_dir = (isset($_SERVER['argv'][2])) ? $_SERVER['argv'][2] : '';
if (empty($import_home_dir) || !is_dir($import_home_dir)) {
	echo "ERROR: You need to supply the path to the import directory as the second argument\n";
	exit();
}

$SORTING = FALSE;
$matrix_root_assetid = 0;
$allow_unrestricted_access = FALSE;

if (isset($_SERVER['argv'][3]) && $_SERVER['argv'][3] != '--sort') {
	$matrix_root_assetid = $_SERVER['argv'][3];
} else if (isset($_SERVER['argv'][3]) && $_SERVER['argv'][3] == '--sort') {
	$SORTING = TRUE;
}

if (isset($_SERVER['argv'][4]) && ($_SERVER['argv'][4] != '--sort') && ($_SERVER['argv'][4] == 1)) {
		$allow_unrestricted_access = TRUE;
} else if (isset($_SERVER['argv'][4]) && $_SERVER['argv'][4] == '--sort') {
	$SORTING = TRUE;
}

if (isset($_SERVER['argv'][5]) && $_SERVER['argv'][5] == '--sort') {
	$SORTING = TRUE;
} else if (isset($_SERVER['argv'][5]) && $_SERVER['argv'][5] != '--sort') {
	echo "ERROR: Fifth argument passed can only be '--sort'";
	exit();
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';

$GLOBALS['SQ_SYSTEM']->am->includeAsset('file');
$GLOBALS['SQ_SYSTEM']->am->includeAsset('image');
$GLOBALS['SQ_SYSTEM']->am->includeAsset('pdf_file');
$GLOBALS['SQ_SYSTEM']->am->includeAsset('word_doc');
$GLOBALS['SQ_SYSTEM']->am->includeAsset('excel_doc');
$GLOBALS['SQ_SYSTEM']->am->includeAsset('powerpoint_doc');
$GLOBALS['SQ_SYSTEM']->am->includeAsset('rtf_file');
$GLOBALS['SQ_SYSTEM']->am->includeAsset('text_file');

$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);


/**
* create a folder in matrix and return the id of the newly created folder
*
* @param int	$folder_destination_id	parent in matrix
* @param string	$new_folder_name		new folder's name
*
* @return int
* @access private
*/
function createFolder($folder_destination_id, $new_folder_name)
{
	$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
	$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

	$GLOBALS['SQ_SYSTEM']->am->includeAsset('folder');

	$folder_destination = $GLOBALS['SQ_SYSTEM']->am->getAsset($folder_destination_id);
	if (empty($folder_destination)) {
		trigger_error("could not get the asset $folder_destination_id\n", E_USER_ERROR);
	}

	$folder = new Folder();
	$folder->setAttrValue('name', $new_folder_name);

	$folder_link = Array('asset' => &$folder_destination, 'link_type' => SQ_LINK_TYPE_2, '', 'is_dependant' => 0, 'is_exclusive' => 0);

	if (!$folder->create($folder_link)) {
		$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		trigger_error("could not create the folder $new_folder_name under the asset $folder_destination_id\n", E_USER_ERROR);
	} else {
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
	}

	return $folder->id;

}//end createFolder()


/**
* This function will get the the corresponding matrix id for a folder
* If the matrix is equal to zero it will make sure create the folder
* it will also make sure that all the parent folders are created in matrix
*
* @param string	$folder_path	folder's path
* @param array	&$matrix_ids	array containing all the matrix ids
*
* @return int
* @access private
*/
function getMatrixFolderId($folder_path, &$matrix_ids)
{
	if (empty($matrix_ids[$folder_path])) {
		$folder_to_create = Array();
		array_push($folder_to_create, $folder_path);

		while (!empty($folder_to_create)) {
			// take current folder and get its parent
			// if the parent folder has a matrix id then create the folder
			// if the parent folder doesnt add the parent folder to the $folder_to_create list
			$current_folder = array_pop($folder_to_create);

			// if the current folder is empty means that we went too far
			if (empty($current_folder)) {
				trigger_error("We could not found the asset id for the folder $folder_path", E_USER_ERROR);
			}
			$parent_folder = dirname($current_folder);
			if (!empty($matrix_ids[$parent_folder])) {
				$matrix_ids[$current_folder] = createFolder($matrix_ids[$parent_folder], basename($current_folder));
			} else {
				array_push($folder_to_create, $parent_folder);
			}
		}
	}
	return $matrix_ids[$folder_path];

}//end getMatrixFolderId()


// hash table that contains the name of the folder and its corresponding matrix asset id
// if the matrix asset id is empty (equals to zero) means that the folder doesnt exist yet
$matrix_ids = Array();

if (empty($matrix_root_assetid)) {
	$import_dirs = list_dirs($import_home_dir, FALSE);
} else {
	$import_dirs = Array();

	// get the list of all the subfolders
	$import_dirs = list_dirs($import_home_dir, TRUE, Array(), TRUE);

	// initialise the matrix_ids
	foreach ($import_dirs as $value) {
		$matrix_ids[$value] = 0;
	}

	// add the import_home_dir to the $import_dirs array and matrix_ids
	$import_dirs = array_merge(Array($import_home_dir), list_dirs($import_home_dir, TRUE, Array(), TRUE));
	$matrix_ids[$import_home_dir] = $matrix_root_assetid;

}

foreach ($import_dirs as $import_dir) {

	$import_path = $import_home_dir.'/'.$import_dir;

	if (empty($matrix_root_assetid)) {
		$parent_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset(trim($import_dir));
	} else {
		$parent_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset(getMatrixFolderId(trim($import_dir), $matrix_ids));
		// overwrite the import path because we are using fullpath
		$import_path = $import_dir;
	}
	if (is_null($parent_asset)) {
		trigger_error("New parent asset #$parent_assetid does not exist\n", E_USER_ERROR);
	}

	$import_link = Array('asset' => $parent_asset, 'link_type' => SQ_LINK_TYPE_1);

	// get a list of all files in the import directory
	$files = list_files($import_path);
	if ($SORTING) sort($files);

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
			case 'xls' :
				$new_asset_type = 'excel_doc';
			break;
			case 'ppt' :
				$new_asset_type = 'powerpoint_doc';
			break;
			case 'rtf' :
				$new_asset_type = 'rtf_file';
			break;
			case 'txt' :
				$new_asset_type = 'text_file';
			break;
			default :
				$new_asset_type = 'file';
			break;
		}

		// create an asset under the new parent of the correct type
		$temp_info = Array('name' => $filename, 'tmp_name' => $import_path.'/'.$filename, 'non_uploaded_file' => TRUE);

		$new_file = new $new_asset_type();
		$new_file->_tmp['uploading_file'] = TRUE;
		$new_file->setAttrValue('name', $filename);
		$new_file->setAttrValue('allow_unrestricted', $allow_unrestricted_access);

		if (!$new_file->create($import_link, $temp_info)) {
			trigger_error('Failed to import '.$new_asset_type.' '.$filename, E_USER_WARNING);
		} else {
			echo 'New '.$new_file->type().' asset created for file '.$filename.' - asset ID #'.$new_file->id."\n";
		}
	}//end foreach
}//end foreach

$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

?>
