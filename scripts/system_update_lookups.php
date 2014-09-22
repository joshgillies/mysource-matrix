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
* $Id: system_update_lookups.php,v 1.18 2012/10/22 04:50:52 cupreti Exp $
*
*/

/**
* Script to update lookups for one or more root nodes and all children.
*
* This script forks PHP processes to stop memory leaks from a long running process.
*
* @author  Huan Nguyen <hnguyen@squiz.net>
* @author  Geoffroy Noel <gnoel@squiz.co.uk>
* @author  James Hurst <jhurst@squiz.co.uk>
* @author  Daniel Simmons <dsimmons@squiz.co.uk>
* @version $Revision: 1.18 $
* @package MySource_Matrix
*/

error_reporting(E_ALL);
if (ini_get('memory_limit') != '-1') ini_set('memory_limit', '-1');
if ((php_sapi_name() != 'cli')) {
    trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}//end if

$config = array();

define('LOG_FILE_NAME', 'update_lookups.log');

// Get config from command line args.
process_args($config);

require_once $config['system_root'].'/core/include/init.inc';
require_once SQ_INCLUDE_PATH.'/assertions.inc';

define('LOG_FILE', SQ_SYSTEM_ROOT.'/data/private/logs/'.LOG_FILE_NAME);			// This is the log file
define('SYNCH_FILE', SQ_TEMP_PATH.'/update_lookups.assetid');	// We need this file to store the assetids

if (empty($config['assetids'])) {
	// We are running the script over the whole system
	$rootnodes = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('site', FALSE);
} else {
	// Replace space with empty string
	$assetids = preg_replace('/[\s]*/', '', $config['assetids']);

	// Explode them so we have the list in array
	$rootnodes    = getRootNodes($assetids);
}

_disconnectFromMatrixDatabase();

// This ridiculousness allows us to workaround Oracle, forking and CLOBs
// if a query is executed that returns more than 1 LOB before a fork occurs,
// the Oracle DB connection will be lost inside the fork.
$pid_prepare    = pcntl_fork();
    switch ($pid_prepare) {
        case -1:
            break;
        case 0:
            // Connect to DB within the child process
            _connectToMatrixDatabase();

            // log in as root
            $root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
            if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
                trigger_error("Failed logging in as root user\n", E_USER_ERROR);
                exit(1);
            }//end if

			echo "Determining assets to process...\n";
			$children = getTreeSortedChildren($rootnodes);

			// Save the list into a file so we can access the list from the parent process
            file_put_contents(SYNCH_FILE, implode(',', $children));

            // Disconnect from DB
            _disconnectFromMatrixDatabase();

            exit(0);
            // waiting for child exit signal
            $status = null;
            pcntl_waitpid(-1, $status);

            break;
        default:
            $status = null;
            pcntl_waitpid(-1, $status);
            break;
    }//end switch


$children    = Array();
if (file_exists(SYNCH_FILE)) {
    $children_str    = file_get_contents(SYNCH_FILE);
} else {
    echo "Unable to find Synch File, probably because the root user was not able to log in, or the user executing this script does not have permission to write to this folder.\n";
    exit(0);
}//end else

$children    = explode(',', $children_str);
$children    = array_unique($children);

// Chunk them up so we can process each batch when forking
$chunk_children    = array_chunk($children, $config['batch_size']);
$current_child_list    = Array();

echo "Updating lookups for " . count($children) . " assets...\n";

log_to_file('======================= Start updating lookups '.date('d-m-Y h:i:s').' =======================', LOG_FILE);
log_to_file("Updating lookups for " . var_export(count($children),TRUE) . " assets \n", LOG_FILE);

while (!empty($chunk_children)) {
	$current_child_list = array_pop($chunk_children);
	$current_remaining = count($current_child_list);
	$pid = pcntl_fork();

	switch ($pid) {
		case -1:
			trigger_error('Process failed to fork', E_USER_ERROR);
			exit(1);
			break;
		case 0:

			// Connect to DB within the child process
			_connectToMatrixDatabase();
			$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);

			$conn_id = MatrixDAL::getCurrentDbId();

			foreach ($current_child_list as $child_assetid) {
				$child_asset    = $GLOBALS['SQ_SYSTEM']->am->getAsset($child_assetid);

				// Update lookups
				if (!is_object($child_asset)) {
				    log_to_file('Asset #'.$child_assetid.' does not exist, skipping', LOG_FILE);
				    if ($config['verbose']) echo '-- Asset #'.$child_assetid.' does not exist, skipping' . "\n";
				    continue;
				}
				$child_asset->updateLookups(FALSE);
				log_to_file('Updated lookups for #'.$child_assetid, LOG_FILE);

				$current_remaining--;
				if ($config['verbose']) echo '-- Remaining: ' . (string) ($current_remaining + (count($chunk_children) * $config['batch_size'])) . "\n";

				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($child_asset);
				$child_asset = NULL;
				unset($child_asset);

			}//end foreach

			$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
			// Disconnect from DB
			_disconnectFromMatrixDatabase();

			exit(0);
			break;

		default:
			$status = null;
			pcntl_waitpid(-1, $status);
			break;

	}//end switch
//}//end foreach
}//end while

echo "Done\n";

log_to_file('======================= Finished updating lookups '.date('d-m-Y h:i:s').' =======================', LOG_FILE);
if (file_exists(SYNCH_FILE)) {
	unlink(SYNCH_FILE);
}//end if
exit(0);




/**
 * Prints the usage statement.
 *
 * @return void
 */
function usage() {
	echo "\n";
	echo "Usage: php {$_SERVER['argv'][0]} <system_root> [assetid[,assetid]] [--batch-size <num>] [--verbose]\n";
	echo "\n";
	echo "  [assetid]            Assets to update lookups for (and their children). If this argument is omitted the script will run on the whole system\n";
	echo "  --batch-size <num>   Number of assets to process per fork. Default is 1000.\n";
	echo "  --verbose            Print number of assets remaining to stdout.\n";
	echo "\n";
	echo "Results are logged to data/private/logs/".LOG_FILE_NAME."\n";

}//end usage()


/**
 * Process command line args into $config array.
 *
 * @param $config Global config.
 * @return void
 */
function process_args(&$config) {

	$config['system_root'] = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
	if (empty($config['system_root'])) {
		echo "ERROR: You need to supply the path to the System Root as the first argument\n";
		usage();
		exit();
	}

	if (!is_dir($config['system_root']) || !is_readable($config['system_root'].'/core/include/init.inc')) {
		echo "ERROR: Path provided doesn't point to a Matrix installation's System Root. Please provide correct path and try again.\n";
		usage();
		exit();
	}

	$config['assetids'] = (isset($_SERVER['argv'][2])) ? trim($_SERVER['argv'][2]) : '';
	if (empty($config['assetids']) || strpos($config['assetids'], '--') === 0) {
		echo "\nWARNING: You are running this update lookup on the whole system.\nThis is fine but it may take a long time\n\nYOU HAVE 5 SECONDS TO CANCEL THIS SCRIPT... ";
		for ($i = 1; $i <= 5; $i++) {
			sleep(1);
			echo $i.' ';
		}
		$config['assetids'] = '';
	}

	$config['batch_size'] = (int) get_parameterised_arg('--batch-size', 1000);
	$config['verbose'] = (int) get_boolean_arg('--verbose');

	echo "\n";
	echo "Updating lookups from asset(s): {$config['assetids']}.\n";
	echo "Batch size: {$config['batch_size']}\n";
	echo "\n";

}//end function processArgs()


/**
 * Gets a boolean switch from the command line.
 *
 * @param $arg
 * @return boolean
 */
function get_boolean_arg($arg) {
	$key = array_search($arg, $_SERVER['argv']);
	if ($key) {
		return TRUE;
	} else {
		return FALSE;
	}//end if

}//end get_boolean_arg()


/**
 * Gets a parameterised switch from the command line.
 *
 * @param $arg The parameter to get.
 * @param $default The default value if no parameter of value for parameter exists.
 * @return boolean
 */
function get_parameterised_arg($arg, $default) {
	$key = array_search($arg, $_SERVER['argv']);
	// Look for the parameter to this arg
	if (($key) && (isset($_SERVER['argv'][$key + 1]))) {
		return $_SERVER['argv'][$key + 1];
	} else {
		return $default;
	}

}//end get_parameterised_arg()


/**
* Prints the specified prompt message and returns the line from stdin
*
* @param string $prompt the message to display to the user
*
* @return string
* @access public
*/
function get_line($prompt='')
{
    echo $prompt;
    // now get their entry and remove the trailing new line
    return rtrim(fgets(STDIN, 4096));

}//end get_line()


/**
* Disconnects from the Oracle Matrix DB
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
* Connects to the Oracle Matrix DB
*
* @return void
* @access private
*/
function _connectToMatrixDatabase()
{
    $GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');

}//end _connectToMatrixDatabase()


/**
* This function basically save the content input to a file
*
*/
function log_to_file($content, $file_name="set_metadata_schema.log")
{
    file_put_contents($file_name, '['.date('d-m-Y h:i:s').'] '.$content."\n", FILE_APPEND);

}//end log_to_file();


function getRootNodes($action)
{
    $rootnodes    = explode(',', $action);

    // Check if each of these rootnodes exists in the system
    $rootnodes_exists    = $GLOBALS['SQ_SYSTEM']->am->assetExists($rootnodes);

    $not_exists         = array_diff($rootnodes, $rootnodes_exists);
    if (!empty($not_exists)) {
        $list_not_exists    = implode(', ', $not_exists);
        echo "These rootnode ids do not exists in the system: $list_not_exists \n";
        exit();
    }//end if

    return $rootnodes;
}//end getRootNodes()

/**
* Gets the children of the root nodes in the correct order from highest in the tree first to the
* lowest. Taken from HIPO_Job_Update_Lookups->prepare().
*
* @return array
* @access public
*/
function getTreeSortedChildren($assetids)
{
	$db = MatrixDAL::getDb();

	$todo_normal = Array();
	$todo_shadows = Array();

	foreach ($assetids as $assetid) {
		// check if we are updating lookups for a shadow asset, or a bridge
		$id_parts = explode(':', $assetid);
		if (isset($id_parts[1])) {
			$todo_shadows = array_merge($todo_shadows, array_keys($GLOBALS['SQ_SYSTEM']->am->getChildren($assetid)));
		} else {
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			if ($asset instanceof Bridge) {
				if (!method_exists($asset, 'getChildren')) {
					trigger_localised_error('SYS0204', translate('Shadow asset handler "%s" can not get children'), E_USER_WARNING, $asset->name);
				} else {
					$todo_shadows = array_merge($todo_shadows, array_keys($asset->getChildren($assetid)));
				}
			}

			$where = 'l.minorid = :assetid';
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 't');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
			$sql = 'SELECT t.treeid
					FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk_tree t INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l ON t.linkid = l.linkid
					'.$where;
			$sql = db_extras_modify_limit_clause($sql, MatrixDAL::getDbType(), 1);

			try {
				$query = MatrixDAL::preparePdoQuery($sql);
				MatrixDAL::bindValueToPdo($query, 'assetid', $assetid);
				$treeid = MatrixDAL::executePdoOne($query);
			} catch (Exception $e) {
				throw new Exception('Unable to get treeid for minorid: '.$assetid.' due to database error: '.$e->getMessage());
			}

			$sql = 'SELECT l.minorid, MAX(LENGTH(t.treeid)) as length
					FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk_tree t
							 INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l ON t.linkid = l.linkid
					';
			$where = 't.treeid LIKE :treeid
					  GROUP BY l.minorid ORDER BY length';

			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 't');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');

			try {
				$query = MatrixDAL::preparePdoQuery($sql.$where);
				MatrixDAL::bindValueToPdo($query, 'treeid', $treeid.'%');
				$new_assets = MatrixDAL::executePdoAssoc($query);
			} catch (Exception $e) {
				throw new Exception('Unable to get minorids for treeid: '.$treeid[0]['treeid'].' due to database error: '.$e->getMessage());
			}

			$todo_normal = array_merge($todo_normal, $new_assets);
		}//end else

	}//end foreach

	// Make sure lower assets are done after higher ones
	usort($todo_normal, create_function('$a, $b', 'return $a[\'length\'] > $b[\'length\'];'));
	$todo_assetids = Array();
	foreach($todo_normal as $asset_info) {
		$todo_assetids[] = $asset_info['minorid'];
	}

	$todo_assetids = array_unique(array_merge($todo_assetids, $todo_shadows));

	return $todo_assetids;

}//end getTreeSortedChildren()

?>
