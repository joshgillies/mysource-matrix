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
* $Id: system_integrity_content_links.php,v 1.7 2012/10/05 07:20:38 akarelia Exp $
*
*/

/**
* Check the integrity of image/file asset NOTICE links in bodycopy contents
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.7 $
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

$ROOT_ASSETID = (isset($_SERVER['argv'][2])) ? $_SERVER['argv'][2] : '1';
if ($ROOT_ASSETID == 1) {
	echo "\nWARNING: You are running this integrity checker on the whole system.\nThis is fine but:\n\tit may take a long time; and\n\tit will acquire locks on many of your assets (meaning you wont be able to edit content for a while)\n\n";
}

$root_user = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');

// log in as root
if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	echo "ERROR: Failed loggin in as root user\n";
	exit();
}

// go trough each bodycopy_container in the system, lock it, validate it, unlock it
$containerids = $GLOBALS['SQ_SYSTEM']->am->getChildren($ROOT_ASSETID, 'bodycopy_container', false);
foreach ($containerids as $containerid => $type_code_data) {
	$type_code = $type_code_data[0]['type_code'];
	$container = &$GLOBALS['SQ_SYSTEM']->am->getAsset($containerid, $type_code);
	printContainerName('Container #'.$container->id);

	// try to lock the container
	if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($container->id, 'links')) {
		printUpdateStatus('LOCK');
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($container);
		continue;
	}

	$edit_fns = $container->getEditFns();
	if (!$edit_fns->generateContentFile($container)) {
		printUpdateStatus('FAILED');
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($container);
		continue;
	}

	// try to unlock the container
	if (!$GLOBALS['SQ_SYSTEM']->am->releaseLock($container->id, 'links')) {
		printUpdateStatus('!!');
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($container);
		continue;
	}
	printUpdateStatus('OK');
	$GLOBALS['SQ_SYSTEM']->am->forgetAsset($container);

}//end foreach


/**
* Prints the name of the continer as a padded string
*
* Padds to 30 columns
*
* @param string	$name	the name of the container
*
* @return void
* @access public
*/
function printContainerName($name)
{
	printf ('%s%'.(30 - strlen($name)).'s', $name, '');

}//end printContainerName()


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
	echo "[ $status ]\n";

}//end printUpdateStatus()


?>
