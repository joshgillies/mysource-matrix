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
* $Id: system_update_lookups.php,v 1.10.8.2 2011/08/08 23:08:22 akarelia Exp $
*
*/

/**
* Run updateLookups() on each site-based asset in the system
* 
* Example usage:	
* php scripts/system_update_lookups .
* or 
* php scripts/system_update_lookups . 46 70
* 
* First argument specifies system root path
* Following arguments specifies root asset ids (sites) 
* also you can specify verbose eg. php scripts/system_update_lookups.php . 46 70 --verbose
*
* @author  Blair Robertson <brobertson@squiz.co.uk>
* @version $Revision: 1.10.8.2 $
* @package MySource_Matrix
*/
error_reporting(E_ALL);
if (ini_get('memory_limit') != '-1') ini_set('memory_limit', '-1');
if ((php_sapi_name() != 'cli')) {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	trigger_error("You need to supply the path to the System Root as the first argument\n", E_USER_ERROR);
}
$ROOT_ASSETID = 1;
$ROOT_ASSETID_ARG = Array();
$VERBOSE = FALSE;

//Read in the asset id for those assets to be updated
for($i=2; $i<count($_SERVER['argv']); $i++) {
	$arg = $_SERVER['argv'][$i];
	$arg = ltrim($arg, '-');
	$arg = strtolower($arg);
	if ($arg == 'verbose') {
		$VERBOSE = TRUE;
	} else {
		$ROOT_ASSETID_ARG[] = $_SERVER['argv'][$i];
	}//end if
}//end for

if (count($ROOT_ASSETID_ARG) == 0) {
	echo "\nWARNING: You are running this update lookup on the whole system.\nThis is fine but it may take a long time\n\nYOU HAVE 5 SECONDS TO CANCEL THIS SCRIPT... ";
	for ($i = 1; $i <= 5; $i++) {
		sleep(1);
		echo $i.' ';
	}
	echo "\n\n";
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
if (count($ROOT_ASSETID_ARG) == 0) {
	foreach ($site_ids as $assetid => $info) {
		$sites[] = $am->getAsset($assetid, $info['type_code']);
	}
//Check to see if given assetid is a site asset  
} else {
	for($i=0; $i<count($ROOT_ASSETID_ARG); $i++) {
		if (isset($site_ids[$ROOT_ASSETID_ARG[$i]])) {
			$sites[] = $am->getAsset($ROOT_ASSETID_ARG[$i], $site_ids[$ROOT_ASSETID_ARG[$i]]['type_code']);
		} else {
			echo "\n".'ERROR: The asset id "',$ROOT_ASSETID_ARG[$i],'" is not a valid site asset.'."\n";  
			exit;
		}
	}
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
				printVerboseErrors($status_errors, $VERBOSE);
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
* Print the list of errors if verbose was selected 
*
* @param array		$errors		the list of errors to print
* @param boolean	$verbose	was verbose selected?
*
* @return void
* @access public
*/
function printVerboseErrors($errors, $verbose=FALSE)
{
	foreach ($errors as $error) {
		$line = array_get_index($error, 'message', '');
		if (!empty($line) && $verbose) {
			echo "\t".$line."\n";
		}//end if
	}//end foreach

}//end printVerboseErrors()


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
