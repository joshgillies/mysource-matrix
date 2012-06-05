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
* $Id: system_integrity_run_tidy.php,v 1.10 2012/06/05 06:26:09 akarelia Exp $
*
*/

/**
* Go through all WYSIWYG content types and re-run HTML Tidy
*
* Syntax: system_integrity_run_tidy.php [Matrix_Root] [Root_Assetid]
*
* @author  Avi Miller <avi.miller@squiz.net>
* @version $Revision: 1.10 $
* @package MySource_Matrix
*/
error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) trigger_error("You can only run this script from the command line\n", E_USER_ERROR);

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT)) {
	echo "ERROR: You need to supply the path to the System Root as the first argument\n";
	exit();
}

if (!is_dir($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT.'/core/include/init.inc')) {
	echo "ERROR: Path provided doesn't point to a Matrix installation's System Root. Please provide correct path and try again.\n";
	exit();
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';
require_once SQ_DATA_PATH.'/private/conf/tools.inc';
$ROOT_PATH = SQ_FUDGE_PATH.'/wysiwyg/';
require_once SQ_FUDGE_PATH.'/wysiwyg/plugins/html_tidy/html_tidy.inc';

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
	echo "ERROR: Failed loggin in as root user\n";
	exit();
}

// find the cache dirs that are currently in the cache repository
$tmp->_running_vars['cache_dirs'] = Array();
$dh = opendir(SQ_CACHE_PATH);
while (false !== ($file = readdir($dh))) {
	if ($file == '.' || $file == '..') continue;
	if (!is_dir(SQ_CACHE_PATH.'/'.$file)) continue;

	// cache directories should only be 4 digits long
	if (!preg_match('|\d{4}|', $file)) continue;

	// just add the relative path to the cache dir so
	// we can compare the name with the asset hash
	$tmp->_running_vars['cache_dirs'][] = $file;
}
closedir($dh);

// go through each wysiwyg in the system, lock it, validate it, unlock it
$wysiwygids = $GLOBALS['SQ_SYSTEM']->am->getChildren($ROOT_ASSETID, 'content_type_wysiwyg', false);
foreach ($wysiwygids as $wysiwygid => $type_code_data) {
	$type_code = $type_code_data[0]['type_code'];
	$wysiwyg = &$GLOBALS['SQ_SYSTEM']->am->getAsset($wysiwygid, $type_code);
	printAssetName('WYSIWYG #'.$wysiwyg->id);

	// try to lock the WYSIWYG
	if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($wysiwyg->id, 'attributes')) {
		printUpdateStatus('LOCKED');
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($wysiwyg);
		continue;
	}

	$old_html = $wysiwyg->attr('html');
	$html = $old_html;

	// Start Tidy up
	$tidy = new HTML_Tidy();
	$tidy->process($html);

	if ($tidy->htmltidy_status != 'pass' || !$wysiwyg->setAttrValue('html', $html) || !$wysiwyg->setAttrValue('htmltidy_status', $tidy->htmltidy_status) || !$wysiwyg->setAttrValue('htmltidy_errors', $tidy->htmltidy_errors) || !$wysiwyg->saveAttributes()) {
		printUpdateStatus('TIDY FAIL');
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($wysiwyg);
		unset($tidy);
		continue;
	}

	unset($tidy);

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

			foreach ($tmp->_running_vars['cache_dirs'] as $dir) {
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

}//end printAssetName()


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
