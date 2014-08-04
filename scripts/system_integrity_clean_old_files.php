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
* $Id: system_integrity_clean_old_files.php,v 1.7 2012/10/05 07:20:38 akarelia Exp $
*
*/

/**
* Deletes the old checked-out files from the data directory for file type of assets
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.7 $
* @package MySource_Matrix
*/

error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) trigger_error("You can only run this script from the command line\n", E_USER_ERROR);

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	echo "ERROR: You need to supply the path to the System Root as the first argument\n";
	exit();
}

if (!is_dir($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT.'/core/include/init.inc')) {
	echo "ERROR: Path provided doesn't point to a Matrix installation's System Root. Please provide correct path and try again.\n";
	exit();
}

// Include init first so it can set the right error_reporting levels.
require_once $SYSTEM_ROOT.'/core/include/init.inc';

require_once 'Console/Getopt.php';
$shortopt = '';
$longopt = Array('delete-orphans');

$con = new Console_Getopt();
$args = $con->readPHPArgv();
array_shift($args);
$options = $con->getopt($args, $shortopt, $longopt);

$DELETE = FALSE;
foreach ($options[0] as $option) {
	switch ($option[0]) {
		case '--delete-orphans':
			$DELETE = TRUE;
		break;
	}
}

// check that the correct root password was entered
$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');

// log in as root
if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	echo "ERROR: Failed logging in as root user\n";
	exit();
}

$fv = $GLOBALS['SQ_SYSTEM']->getFileVersioning();
$children = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('file', FALSE);
$count = 0;
$orphans = Array();

// scan the file type of assets and find any orphan files in the data directories
echo "[Total ".count($children)." assets will be checked.]\n\n";
foreach ($children as $assetid) {

	$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
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
				$orphans[$assetid][] = $data_path.'/'.$file;
				$count++;
			}
		}
	}

	printAssetid($assetid);
	if (isset($orphans[$assetid])) {
		printStatus('FOUND');
	} else {
		printStatus('OK');
	}

	$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset, TRUE);

}//end foreach $children


// if there is any orphan files, print them
if (!empty($orphans)) {
	echo "\n[Total $count orphan files found]\n\n";
	foreach ($orphans as $assetid => $files) {
		echo "[ #$assetid ]\n";
		echo implode("\n", $files)."\n\n";
	}
} else {
	echo "No orphan files found\nBye\n";
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

	foreach ($orphans as $assetid => $files) {
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
	echo "USAGE: system_integrity_data_files.php <system_root> [--delete-orphans]\n\n";
	echo "--delete-orphans : If this option is given, the script deletes the found orphans file\n";
	echo "\nNOTE: Please make sure that you run the script without --delete-orphans option first.\n";
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
