<?php
/**
 * Script for the publishing of static content of the provided comma seperated asset ids.
 * Script will publish every URL associated with each asset. There are no restrictions on asset types.
 * It is upto the user to identify the static/dynamic nature of an asset's content.
 * 
 * This content will be published to the required directory in the following structure:
 * <scheme>/<domain>/<path>/index.html
 * There is an option to publish the _nocache version. If this option is not selected, existing (if any)
 *  _nocache content will be deleted.
 * 
 * @author  Mohamed Haidar <mhaidar@squiz.com.au>
 * @version $Revision: 1.4 $
 * @package MySource_Matrix
 */

error_reporting(E_ALL);

if ((php_sapi_name() != 'cli')) {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}//end if

if (count($_SERVER['argv']) < 4 || count($_SERVER['argv']) > 5) {
	echo "This script needs to be run in the following format:\n\n";
	echo "\tphp publish_static.php SYSTEM_ROOT asset_ids STORAGE_PATH [--_nocache]\n\n";
	echo "\tEg. php scripts/publish_static.php . 21,54,113 /home/static_content\n";
	exit(1);
}

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT)) {
	echo "ERROR: You need to supply the path to the System Root as the first argument\n";
	exit();
}

if (!is_dir($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT.'/core/include/init.inc')) {
	echo "ERROR: Path provided doesn't point to a Matrix installation's System Root. Please provide correct path and try again.\n";
	exit();
}

$asset_ids = Array();
if (isset($_SERVER['argv'][2])) {
	$asset_ids = explode(',', $_SERVER['argv'][2]);
}

$STORAGE_PATH = (isset($_SERVER['argv'][3])) ? $_SERVER['argv'][3] : '';
if (empty($STORAGE_PATH) || !is_dir($STORAGE_PATH)) {
	echo 'ERROR: The directory you specified as the storage root does not exist, or is not a directory';
	exit();
}

$_nocache = (isset($_SERVER['argv'][4])) ? $_SERVER['argv'][4] : FALSE;
if ($_nocache != '--_nocache') $_nocache = FALSE;

require_once $SYSTEM_ROOT.'/core/include/init.inc';

$publish_urls = Array();
foreach ($asset_ids as $id) {
	$urls = $GLOBALS['SQ_SYSTEM']->am->getURLs($id);
	foreach ($urls as $url) {
		$scheme = ($url['https'] == 1) ? 'https' : FALSE;
		if ($scheme != FALSE) {
			$publish_urls[] = $scheme.'://'.$url['url'];
			if ($_nocache) $publish_urls[] = $scheme.'://'.$url['url'].'/_nocache';	
		}
		$scheme = ($url['http'] == 1) ? 'http' : FALSE;
		if ($scheme != FALSE) {
			$publish_urls[] = $scheme.'://'.$url['url'];
			if ($_nocache) $publish_urls[] = $scheme.'://'.$url['url'].'/_nocache';	
		}
	}
}

_disconnectFromMatrixDatabase();
$fork_num = 0;
while (!empty($publish_urls)) {
    $publish_url = array_pop($publish_urls);
	$pid_prepare = pcntl_fork();
	$fork_num++;
	switch ($pid_prepare) {
		case -1:
			trigger_error('Process failed to fork while publishing static content', E_USER_ERROR);
            exit(1);
			break;
		case 0:
			// Connect to DB within the child process
			_connectToMatrixDatabase();
			
			// login as public_user
			$user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('public_user');
			if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($user)) {
				trigger_error("Failed logging in as public user\n", E_USER_ERROR);
				exit(1);
			}
			
			render_static_url($publish_url);

			$GLOBALS['SQ_SYSTEM']->restoreCurrentUser();
			// Disconnect from DB
			_disconnectFromMatrixDatabase();
			
			exit(0);
			break;
		default:
			if (empty($publish_urls)) {
            	// We wait for all the fork child to finish
                while ($fork_num > 0) {
	                $status = null;
	                pcntl_waitpid(-1, $status);
	                $fork_num--;
            	}//end
            }//end if
			break;
	}//end switch

}


function render_static_url($url) {

	global $STORAGE_PATH;
	global $SYSTEM_ROOT;
	global $_nocache;

	// fake some variables required to correctly describe the current url
	$parts = parse_url($url);
	$method = 'GET';
	$protocol = 'HTTP/1.1';
	$scheme = $parts['scheme'];
	$host = $parts['host'];
	$path = $parts['path'];
	$query = '';

	$_SERVER['SERVER_PROTOCOL'] = $protocol;
	$_SERVER['REQUEST_METHOD'] = $method;
	$_SERVER['QUERY_STRING'] = $query;
	$_SERVER['REQUEST_URI'] = $path.(empty($query)?'':'?').$query;
	$_SERVER['SCRIPT_NAME'] = $path;
	$_SERVER['PHP_SELF'] = $path;
	$_SERVER['HTTP_USER_AGENT'] = 'Static Site Generator';
	$_SERVER['HTTP_HOST'] = $host;
	$_SERVER['SERVER_NAME'] = $host;

	// ask Matrix to draw as if we were in apache
	ob_start();
	require_once $SYSTEM_ROOT.'/core/include/init.inc';
	$GLOBALS['SQ_SYSTEM']->start();
	$content = ob_get_clean();

	$storage_dir = "$STORAGE_PATH/$scheme/$host$path/index.html";
	if (create_directory(dirname($storage_dir))){
		file_put_contents($storage_dir, $content);
		if ($_nocache == FALSE && is_dir(dirname($storage_dir).'/_nocache')) {
			delete_directory(dirname($storage_dir).'/_nocache');
		}
	}
}


/**
* Disconnects from the Matrix DB
*
* @return void
* @access private
*/
function _disconnectFromMatrixDatabase()
{
    $conn_id = MatrixDAL::getCurrentDbId();
    if (isset($conn_id) && !empty($conn_id)) {
        MatrixDAL::restoreDb();
        MatrixDAL::dbClose($conn_id);
    }//end if
        
}//end _disconnectFromMatrixDatabase()


/**
* Connects to the Matrix DB
*
* @return void
* @access private
*/
function _connectToMatrixDatabase()
{
    $GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');

}//end _connectToMatrixDatabase()

?>
