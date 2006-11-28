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
* $Id: system_integrity_clean_old_files.php,v 1.1.2.2 2006/11/28 04:58:55 skim Exp $
*
*/

/**
* Deletes the old checked-out files from the data directory for file type of assets
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.1.2.2 $
* @package MySource_Matrix
*/

error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) trigger_error("You can only run this script from the command line\n", E_USER_ERROR);

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	echo "ERROR: You need to supply the path to the System Root as the first argument\n";
	exit();
}

require_once 'Console/Getopt.php';
$shortopt = '';
$longopt = Array('delete-orphants');

$args = Console_Getopt::readPHPArgv();
array_shift($args);
$options = Console_Getopt::getopt($args, $shortopt, $longopt);

$DELETE = FALSE;
foreach ($options[0] as $option) {
	switch ($option[0]) {
		case '--delete-orphants':
			$DELETE = TRUE;
		break;
	}

}

require_once $SYSTEM_ROOT.'/core/include/init.inc';

// ask for the root password for the system
echo 'Enter the root password for "'.SQ_CONF_SYSTEM_NAME.'": ';
$root_password = rtrim(fgets(STDIN, 4094));

// check that the correct root password was entered
$root_user = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
if (!$root_user->comparePassword($root_password)) {
	trigger_error("The root password entered was incorrect\n", E_USER_ERROR);
}

// log in as root
if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	trigger_error("Failed logging in as root user\n", E_USER_ERROR);
}

$fv =& $GLOBALS['SQ_SYSTEM']->getFileVersioning();
$children = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('file', FALSE);
$count = 0;
$orphants = Array();

// scan the file type of assets and find any orphant files in the data directories
echo "[Total ".count($children)." assets will be checked.]\n\n";
foreach ($children as $assetid) {

	$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
	$file_name = $asset->attr('name');

	$data_paths = Array($asset->data_path);
	if (file_exists($asset->data_path_public)) {
		$data_paths[] = $asset->data_path_public;
	}

	// find the old version of files for this file asset
	foreach ($data_paths as $data_path) {
		$files = list_files($data_path);
		$ffiles = list_files($data_path.'/.FFV');
		$diff = array_intersect($files, $ffiles);
		if (count($diff) == 1) {
			$file = array_pop($diff);
			if ($file == $file_name) continue;
		} else {
			$ophs = array_diff($diff, Array($file_name));
			foreach ($ophs as $file) {
				$orphants[$assetid][] = $data_path.'/'.$file;
				$count++;
			}
		}
	}

	printAssetid($assetid);
	if (isset($orphants[$assetid])) {
		printStatus('FOUND');
	} else {
		printStatus('OK');
	}

	$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset, TRUE);

}//end foreach $children


// if there is any orphant files, print them
if (!empty($orphants)) {
	echo "\n[Total $count orphant files found]\n\n";
	foreach ($orphants as $assetid => $files) {
		echo "[ #$assetid ]\n";
		echo implode("\n", $files)."\n\n";
	}
} else {
	echo "No orphant files found\nBye\n";
	exit();
}

// are we really going to delete them?
if ($DELETE) {
	do {
		echo "DO YOU REALLY WANT TO DELETE THESE FILES? (Yes/No) : ";
		$answer = rtrim(fgets(STDIN, 4094));
		if ($answer == 'Yes') {
			break;
		} else if ($answer == 'No') {
			echo "\nBye\n";
			exit();
		}
	} while (TRUE);

	foreach ($orphants as $assetid => $files) {
		echo "[Processing $assetid]\n";
		foreach ($files as $file) {
			if (unlink($file)) {
				echo str_repeat(' ', 4)."Successful : $file deleted\n";
			} else {
				trigger_error("Failed to delete $file\n", E_USER_ERROR);
			}
		}
	}
}


/**
* Prints the usage for this script and exits
*
* @return void
* @access public
*/
function usage()
{
	echo "USAGE: system_integrity_data_files.php <system_root> [--delete-orphants]\n\n";
	echo "--delete-orphants : If this option is given, the script deletes the found orphants file\n";
	echo "\nNOTE: Please make sure that you run the script without --delete-orphants option first.\n";
	exit();

}//end usage()


/**
* Prints the name of the Asset as a padded string
*
* Pads name to 40 columns
*
* @param string	$assetid	the id of the asset of which we want to print the name
*
* @return void
* @access public
*/
function printAssetid($assetid)
{
	$str = '[ #'.$assetid.' ]';
	if (strlen($str) > 36) {
		$str = substr($str, 0, 36).'...';
	}
	printf ('%s%'.(40 - strlen($str)).'s', $str,'');

}//end printAssetName()


/**
* Prints the status of the container integrity check
*
* @param string	$status	the status of the check
*
* @return void
* @access public
*/
function printStatus($status)
{
	echo "[ $status ]\n";

}//end printUpdateStatus()


?>


