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
* $Id: system_integrity_content_links.php,v 1.1 2004/03/25 05:22:59 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/

/**
* Check the integrity of image/file asset NOTICE links in bodycopy contents
*
* @author  Greg Sherwood <greg@squiz.net>
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

$ROOT_ASSETID = (isset($_SERVER['argv'][2])) ? $_SERVER['argv'][2] : '1';
if ($ROOT_ASSETID == 1) {
	echo "\nWARNING: You are running this integrity checker on the whole system.\nThis is fine but:\n\tit may take a long time; and\n\tit will acquire locks on many of your assets (meaning you wont be able to edit content for a while)\n\n";
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

// go trough each bodycopy_container in the system, lock it, validate it, unlock it
$containerids = $GLOBALS['SQ_SYSTEM']->am->getChildren($ROOT_ASSETID, 'bodycopy_container', false);
foreach ($containerids as $containerid => $type_code) {
	
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
