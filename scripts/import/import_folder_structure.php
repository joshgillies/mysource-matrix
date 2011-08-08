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
* $Id: import_folder_structure.php,v 1.4.4.1 2011/08/08 23:08:22 akarelia Exp $
*
*/

/**
* Directory structure to Folder Asset Import Script
* Filesystem to Matrix System
*
* @author	Mark Brydon <mbrydon@squiz.net>
* 30 Nov 2007
*
*
* Purpose:
*	To create Folder assets under a parent asset with names based on directories in the filesystem.
*
*	This script will create the folders directly on the Matrix System and optionally rename the directories on the
*	filesystem to the asset IDs of the Folder assets. The filesystem can then be used with the file import script
*   (scripts/import/import_files.php) to populate the relevant Folder assets with files.
*
*   The output of this script is a CSV file containing the results of the import in the format:
*		folder_name, asset_id
*
* Documentation:
*	This script takes three parameters:
*		- Matrix system root directory
*		- Directory containing the folders to be created
*		- Parent asset ID under which to create the Folder assets
*
*	A final confirmation will be displayed informing the user that the directory names will be modified
*	on the filesystem. The renaming of directories on the filesystem is optional, however it will not
*	be possible to use the files directly with the file import script if this is not selected.
*
*	This script assumes that all directories are in the specified folders directory and that there are no
*	other directories within these directories.
*
*/


/**
* Prints out some basic help info detailing how to use this script
*
* @return void
* @access public
*/
function printUsage()
{
	printStdErr("Directory structure to Folder Asset Matrix importer\n\n");
	printStdErr("Usage: import_folder_structure [system root] [folder dir] [parent id]\n");
	printStdErr("system root            : The Matrix System root directory\n");
	printStdErr("folder dir             : A filesystem directory containing the folders to be imported\n");
	printStdErr("parent id              : The asset ID of a Folder etc. under which the assets are to reside\n");

}//end printUsage()


/**
* Prints the supplied string to "standard error" (STDERR) instead of the "standard output" (STDOUT) stream
*
* @param string	$string	The string to write to STDERR
*
* @return void
* @access public
*/
function printStdErr($string)
{
	fwrite(STDERR, "$string");

}//end printStdErr()


/**
* Formats a value for output to a CSV file by double quoting any double quote characters.
* Numeric values remain untouched apart from a slight trim()
*
* @param string	$string	The string to be formatted
*
* @return string
* @access public
*/
function formatCSVValue($string)
{
	$string = trim($string);

	if (!is_numeric($string)) {
		// Double quote double quotes
		$string = str_replace('"', '""', $string);
		$string = '"'.$string.'"';
	}

	return $string;

}//end formatCSVValue()


/**
* Creates a simple "TYPE 1" link between assets
*
* @param object	&$parent_asset	The parent asset
* @param object	&$child_asset	The child asset
*
* @return int
* @access public
*/
function createLink(&$parent_asset, &$child_asset)
{
	// Link the asset to the parent asset
	$link = Array(
				'asset'			=> &$parent_asset,
				'link_type'		=> 1,
				'value'			=> '',
				'sort_order'	=> NULL,
				'is_dependant'	=> FALSE,
				'is_exclusive'	=> FALSE,
			);

	$link_id = $child_asset->create($link);

	return $link_id;

}//end createLink()


/**
* Creates a Folder asset with the specified name and parent
*
* @param string	$name			The name of the Folder to create
* @param object	&$parent_folder	The parent asset
*
* @return int
* @access public
*/
function createFolder($name, &$parent_folder)
{
	printStdErr('- Creating Folder '.$name);

	$folder = new Folder();
	printStdErr('.');

	// Set Folder name
	$folder->setAttrValue('name', $name);
	printStdErr('.');

	// Link the new asset under the parent folder
	$link_id = createLink($parent_folder, $folder);
	printStdErr('.');

	// Free memory
	$GLOBALS['SQ_SYSTEM']->am->forgetAsset($folder);

	printStdErr(' => asset ID '.$folder->id."\n");

	echo formatCSVValue($name).','.$folder->id;

	return $folder->id;

}//end createFolder()


/************************** MAIN PROGRAM ****************************/

if (ini_get('memory_limit') != '-1') ini_set('memory_limit', '-1');

if ((php_sapi_name() != 'cli')) {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

// Matrix system root directory
$argv = $_SERVER['argv'];
$GLOBALS['SYSTEM_ROOT'] = (isset($argv[1])) ? $argv[1] : '';
if (empty($GLOBALS['SYSTEM_ROOT'])) {
	printUsage();
	printStdErr("* The Matrix system root directory must be specified as the first parameter\n\n");
	exit(-99);
}

require_once $GLOBALS['SYSTEM_ROOT'].'/core/include/init.inc';

$GLOBALS['SQ_SYSTEM']->am->includeAsset('folder');

// Has a directory name been supplied?
$filesystem_dir = $argv[2];
if (empty($filesystem_dir)) {
	printUsage();
	printStdErr("* A source filesystem directory must be specified as the second parameter\n\n");
	exit(-2);
} else {
	if (!is_dir($filesystem_dir)) {
		printUsage();
		printStdErr("* The supplied filesystem directory is either not a directory or was not found\n\n");
		exit(-3);
	}
}

// Has a parent ID been supplied?
$parent_id  = (int)$argv[3];
if ($parent_id == 0) {
	printUsage();
	printStdErr("* A Parent asset ID must be specified as the third parameter\n\n");
	exit(-4);
}

// Prompt the user regarding directory renaming
printStdErr("** The directories on the filesystem can be renamed to the asset IDs of their respective Folder Assets.\n");
printStdErr("   This will allow any files to be imported by using the import_files.php script.\n");
printStdErr("   Would you like this to occur ** THIS CANNOT BE UNDONE **\n");

$valid_input = FALSE;
$valid_choices = Array('y','n');
while (!$valid_input) {
	printStdErr('   (y / n): ');
	$user_input = rtrim(strtolower(fgets(STDIN, 1024)));
	$valid_input = in_array($user_input, $valid_choices);
}

$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);

// Find the parent asset
$parent_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($parent_id);

// Find all directories and create Folder assets for each
if ($dh = opendir($filesystem_dir)) {
	while (($file = readdir($dh)) !== FALSE) {
		$is_directory = (filetype($filesystem_dir.'/'.$file) == 'dir');

		// Skip reserved "." and ".." directories
		if (($is_directory) && ($file != '.') && ($file != '..')) {
			$asset_id = createFolder(trim($file), $parent_asset);

			// Rename filesystem directories if requested
			if ($user_input == 'y') {
				rename($filesystem_dir.'/'.$file, $filesystem_dir.'/'.$asset_id);
			}
		}
	}

	closedir($dh);
}

$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

?>
