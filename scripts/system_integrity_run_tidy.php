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
* $Id: system_integrity_run_tidy.php,v 1.1 2004/10/18 21:30:07 amiller Exp $
* $Name: not supported by cvs2svn $
*/

/**
* Go through all WYSIWYG content types and re-run HTML Tidy
*
* @author  Avi Miller <avi.miller@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/
error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) trigger_error("You can only run this script from the command line\n", E_USER_ERROR);

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	echo "ERROR: You need to supply the path to the System Root as the first argument\n";
	exit();
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';
require_once SQ_DATA_PATH.'/private/conf/tools.inc';

$ROOT_ASSETID = (isset($_SERVER['argv'][2])) ? $_SERVER['argv'][2] : '1';
if ($ROOT_ASSETID == 1) {
	echo "\nWARNING: You are running this checker on the whole system.\nThis is fine but:\n\tit may take a long time; and\n\tit will acquire locks on many of your assets (meaning you wont be able to edit content for a while)\n\n";
}

// ask for the root password for the system
echo 'Enter the root password for "'.SQ_CONF_SYSTEM_NAME.'": ';
$root_password = rtrim(fgets(STDIN, 4094));
	
// check that the correct root password was entered
$root_user = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
if (!$root_user->comparePassword($root_password)) {
	echo "ERROR: The root password entered was incorrect\n";
	exit();
}

// log in as root
if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	trigger_error("Failed loggin in as root user\n", E_USER_ERROR);
}

// find the cache dirs that are currently in the cache repository
$this->_running_vars['cache_dirs'] = Array();
$dh = opendir(SQ_CACHE_PATH);
while (false !== ($file = readdir($dh))) {
	if ($file == '.' || $file == '..') continue;
	if (!is_dir(SQ_CACHE_PATH.'/'.$file)) continue;

	// cache directories should only be 4 digits long
	if (!preg_match('|\d{4}|', $file)) continue;

	// just add the relative path to the cache dir so
	// we can compare the name with the asset hash
	$this->_running_vars['cache_dirs'][] = $file;
}
closedir($dh);

// go through each wysiwyg in the system, lock it, validate it, unlock it
$wysiwygids = $GLOBALS['SQ_SYSTEM']->am->getChildren($ROOT_ASSETID, 'content_type_wysiwyg', false);
foreach ($wysiwygids as $wysiwygid => $type_code) {
	
	$wysiwyg = &$GLOBALS['SQ_SYSTEM']->am->getAsset($wysiwygid, $type_code);
	printAssetName('WYSIWYG #'.$wysiwyg->id);
	
	// try to lock the WYSIWYG
	if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($wysiwyg->id, 'attributes')) {
		printUpdateStatus('LOCKED');
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($wysiwyg);
		continue;
	}
	
	$old_html = $wysiwyg->attr('html');
	
			// If HTML Tidy is enabled, let's rock'n'roll
		if (SQ_TOOL_HTML_TIDY_ENABLED) {

			// tidy the HTML produced using the HTMLTidy program
			$path_to_tidy = SQ_TOOL_HTML_TIDY_PATH;
			
			// is_executable doesn't exist on windows pre 5.0.0
			if (function_exists('is_executable')) {
				if (is_executable($path_to_tidy)) {
					$command = "/bin/echo ".escapeshellarg($old_html)." | $path_to_tidy -iq --show-body-only y --show-errors 0 --show-warnings 0 --wrap 0 -asxml --word-2000 1 --force-output 1";
				
					$tidy = Array();
					exec($command, $tidy);
					$new_html = implode("\n", $tidy);
					unset($tidy);
				}
			}
		}
	
	if (!$wysiwyg->setAttrValue('html', $new_html) || !$wysiwyg->saveAttributes()) {
		printUpdateStatus('TIDY FAIL');
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($wysiwyg);
		continue;
	}
	
	// try to unlock the WYSIWYG
	if (!$GLOBALS['SQ_SYSTEM']->am->releaseLock($wysiwyg->id, 'attributes')) {
		printUpdateStatus('UNLOCK FAIL');
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($wysiwyg);
		continue;
	}

	printUpdateStatus('TIDY PASS');
	
	// try to recreate the parent content_files (bodycopy container and bodycopy)
	$parents = $GLOBALS['SQ_SYSTEM']->am->getDependantParents($wysiwyg->id);
	foreach ($parents as $parent) {
		$parent_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($parent);
		if (!$parent_asset->saveSystemVersion()) {
			printAssetName($parent_asset->name.' (#'.$parent_asset->id.')');
			printUpdateStatus('CONTENT FAIL');
			continue;
		} else {
			printAssetName($parent_asset->name.' (#'.$parent_asset->id.')');
			printUpdateStatus('CONTENT PASS');
			
			// Clear the Cache for each asset
			$cm = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');
			
			// get the group that is used as the first 2 digits of the
			// cache directory names, and get the asset id code whics is used as
			// the first string on each of the cache files

			$assetid_code = md5($parent_asset->id);
			$group = $cm->getAssetHash($assetid_code);
			
			foreach ($this->_running_vars['cache_dirs'] as $dir) {
				if (substr($dir, 0, strlen($group)) == $group) {

					// this directory contains cache entries for this particular asset
					// now files the files in this cache dir that belong to this asset
					$dh = opendir(SQ_CACHE_PATH.'/'.$dir);

					while (false !== ($file = readdir($dh))) {
						$abs_path = SQ_CACHE_PATH.'/'.$dir.'/'.$file;
						if (is_dir($abs_path)) continue;
						// if the file starts with the assetid_code for the asset
						// then back up in the unlinking with the resurection
						if (substr($file, 0, strlen($assetid_code)) == $assetid_code) unlink($abs_path);
						printAssetName($parent_asset->name.' (#'.$parent_asset->id.')');
						printUpdateStatus('CACHE CLEAR');
					}
					closedir($dh);
				}
			}//end foreach
			
		}
	}

	$GLOBALS['SQ_SYSTEM']->am->forgetAsset($wysiwyg);
	
}//end foreach


/**
* Prints the name of the WYSIWYG as a padded string
*
* Padds to 30 columns
*
* @param string	$name	the name of the container
*
* @return void
* @access public
*/
function printAssetName($name)
{
	printf ('%s%'.(30 - strlen($name)).'s', $name, ''); 
	
}//end printWYSIWYGName()


/**
* Prints the status of the container integrity check
*
* @param string	status	the status of the check
*
* @return void
* @access public
*/
function printUpdateStatus($status)
{
	echo " [ $status ] \n";
	
}//end printUpdateStatus()

?>
