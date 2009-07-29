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
* $Id: system_update_lookups.php,v 1.7.2.1 2009/07/29 00:26:31 ewang Exp $
*
*/

/**
* Run updateLookups() on each site-based asset in the system
*
* @author  Blair Robertson <brobertson@squiz.co.uk>
* @version $Revision: 1.7.2.1 $
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

//set to oci commit on success mode to avoid oracle + folk = lost connection issue
if (MatrixDAL::getDbType() === 'oci') {
	  MatrixDAL::setOciCommitOnSuccess(TRUE);
}

$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
if (!$root_user->comparePassword($root_password)) {
	trigger_error("The root password entered was incorrect\n", E_USER_ERROR);
	exit;
}
// log in as root
if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	trigger_error("Failed logging in as root user\n", E_USER_ERROR);
}

$am = $GLOBALS['SQ_SYSTEM']->am;
$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
$site_ids = $am->getTypeAssetids('site', FALSE, TRUE);
$sites = Array();
foreach ($site_ids as $assetid => $info) {
	$sites[] = $am->getAsset($assetid, $info['type_code']);
}


foreach ($sites as $key => $site) {

	$pid = fork();

	// only run this if we are in the forked process
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

		_disconnectDB();

		// exit the child fork process and return to the parent where it's still pcntl_wait()ing.
		exit;
	}
}
if (MatrixDAL::getDbType() === 'oci') {
	  MatrixDAL::setOciCommitOnSuccess(FALSE);
}

exit;


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
	$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection(TRUE);
	$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2', TRUE);

}//end _reconnectDB()


/**
* Disconnects from the Matrix DB
*
* @return void
* @access private
*/
function _disconnectDB()
{
	$conn_id = MatrixDAL::getCurrentDbId();
	if (isset($conn_id) && !empty($conn_id)) {
		MatrixDAL::restoreDb();
		MatrixDAL::dbClose($conn_id);
	}

}//end _disconnectDB()


?>
