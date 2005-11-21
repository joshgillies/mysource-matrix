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
* $Id: system_update_lookups.php,v 1.2.2.2 2005/11/21 23:24:05 ndvries Exp $
*
*/

/**
* Upgrade the *_ast_lookup_design table to *_ast_lookup_value
*
* @author  Blair Robertson <brobertson@squiz.co.uk>
* @version $Revision: 1.2.2.2 $
* @package MySource_Matrix
*/
error_reporting(E_ALL);
ini_set('memory_limit', '256M');
if ((php_sapi_name() != 'cli')) {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	trigger_error("You need to supply the path to the System Root as the first argument\n", E_USER_ERROR);
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


$am = &$GLOBALS['SQ_SYSTEM']->am;
$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();

$sites = $am->getTypeAssetids('site', false, true);

foreach ($sites as $assetid => $type_code) {
	$site = &$am->getAsset($assetid, $type_code);

	$pid = fork();

	// Only run this if we are the forked process
	if (!$pid) {
		_reconnectDB();

		printName('Updating Lookups for "'.$site->name.'" #'.$site->id);

		$vars = Array('assetids' => Array($site->id));
		$status_errors = $hh->freestyleHipo('hipo_job_update_lookups', $vars);
		if (empty($status_errors)) {
			printUpdateStatus('OK');
		} else {
			printUpdateStatus('!!!');
		}

		// Exit the child fork process and return to the parent where it's
		// still pcntl_wait()ing.
		exit;
	}

	_reconnectDB();

	$am->forgetAsset($site);
}

exit();


//--        HELPER FUNCTIONS        --//


/**
* Print the current action of the script
*
* @param string	$name	Current action
*
* @return void
* @access public
*/
function printName($name)
{
	printf ('%s%'.(60 - strlen($name)).'s', $name, '');

}//end printName()


/**
* Print whether or not the last action was successful
*
* @param string	$status	'OK' for success, or '!!!' for fail
*
* @return void
* @access public
*/
function printUpdateStatus($status)
{
	echo "[ $status ]\n";

}//end printUpdateStatus()


/**
* Fork a child process. The parent process will sleep until the child
* exits
*
* @return string
* @access public
*/
function fork()
{
	$child_pid = pcntl_fork();
	switch ($child_pid) {
		case -1:
			trigger_error('Uh-oh, Spaghettios!');
			return null;
		break;
		case 0:
			return $child_pid;
		break;
		default :
			$status = null;
			pcntl_waitpid(-1, $status);
			return $child_pid;
		break;
	}

}//end fork()


/**
* Clear out the database connections and establish a new one
*
* @return void
* @access private
*/
function _reconnectDB()
{
	$GLOBALS['SQ_SYSTEM']->db->disconnect();
	unset($GLOBALS['SQ_SYSTEM']->_db_conns);
	$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db');
	$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

}//end _reconnectDB()


?>
