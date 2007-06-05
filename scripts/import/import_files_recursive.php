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
*
*/

/**
* This script will help to import a whole folder with its sub folders and files
* 1. it will create in matrix the matching folder structure
* 2. it will prepare in the working folder a file and folder structure ready to be imported using the import_file.php script (working folder)
* 3. Finally, the user need to run a php command to import all the files into matrix
*
* Usage: php scripts/import/import_files_recursive.php . scripts/import/to_import tmp_import 229
* scripts/import/to_import: is the folder to import
* tmp_import: is the working directory
* 229: is the id of the destination folder in matrix
*
* @author  Christophe Olivar <colivar@squiz.net>
* @version $Revision: 1.1.2.1 $
* @package MySource_Matrix
*/
error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	trigger_error("You need to supply the path to the System Root as the first argument\n", E_USER_ERROR);
}

$import_home_dir = (isset($_SERVER['argv'][2])) ? $_SERVER['argv'][2] : '';
if (empty($import_home_dir) || !is_dir($import_home_dir)) {
	trigger_error("You need to supply the path to the import directory as the second argument\n", E_USER_ERROR);
}

$working_directory = (isset($_SERVER['argv'][3])) ? $_SERVER['argv'][3] : '';
if (empty($working_directory) || !is_dir($working_directory)) {
	trigger_error("You need to supply the path to a working directory as the third argument (e.g.: tmp_import_try_01)\n", E_USER_ERROR);
}

$matrix_dir_id = (isset($_SERVER['argv'][4])) ? $_SERVER['argv'][4] : '';
if (empty($matrix_dir_id)) {
	trigger_error("You need to supply the asset id of the folder where you want to import the files\n", E_USER_ERROR);
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';

$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
$GLOBALS['SQ_SYSTEM']->am->includeAsset('folder');


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

	$folder_destination =& $GLOBALS['SQ_SYSTEM']->am->getAsset($folder_destination_id);
	if (empty($folder_destination)) {
		trigger_error("could not get the asset $folder_destination_id\n", E_USER_ERROR);
	}

	$folder =& new Folder();
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


$directory_list = Array($import_home_dir);
$directory_id = Array($matrix_dir_id);
$file_list = Array();

// this will construct the folder in matrix and will copy the files to the working directory in the corresponding folder
while ((!empty($directory_list)) || (!empty($file_list))) {
	// if the file are empty we need to open the directory list and add all the files to the file list
	if (empty($file_list)) {
		$current_directory = array_pop($directory_list);
		$current_id = array_pop($directory_id);

		// create a folder with the current id as a name
		pre_echo('create folder '.$current_directory.' (#'.$current_id.')');
		mkdir($working_directory.'/'.$current_id);
		chmod($working_directory.'/'.$current_id, 0775);

		$sub_directory = list_dirs($current_directory);
		$sub_file = list_files($current_directory);

		foreach ($sub_directory as $value) {
			$directory_list[] = $current_directory.'/'.$value;
			// create the folder under current_id asset
			$directory_id[] = createFolder($current_id, $value);
		}

		foreach ($sub_file as $value) {
			$file_list[] = $current_directory.'/'.$value;

			// copy every file to the folder with current id as a name
			pre_echo('copy the file '.$current_directory.'/'.$value.' to '.$working_directory.'/'.$current_id.'/'.basename($value));
			if (!copy($current_directory.'/'.$value, $working_directory.'/'.$current_id.'/'.basename($value))) {
				trigger_error('could not copy the file '.$current_directory.'/'.$value.' to '.$working_directory.'/'.$current_id.'/'.basename($value)."\n", E_USER_ERROR);
			}
		}
	} else {
		$current_file = array_pop($file_list);
	}
}//end while

// this will use the import script to import every files in the working directory
$current_command =  'php '.$SYSTEM_ROOT.'/scripts/import/import_files.php '.$SYSTEM_ROOT.' '.$working_directory;
pre_echo('To start importing the file please run the following command: ');
pre_echo($current_command);

$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

?>
