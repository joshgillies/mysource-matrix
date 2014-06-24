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
*/

/**
* Script to set permissions to one or more root nodes and all children.
*
* This script forks multiple PHP processes to stop memory leaks from a long running process.
*
* @author  Huan Nguyen <hnguyen@squiz.net>
* @author  Geoffroy Noel <gnoel@squiz.co.uk>
* @author  James Hurst <jhurst@squiz.co.uk>
* @version $Revision: 1.5 $
* @package MySource_Matrix
*/

error_reporting(E_ALL);
if (ini_get('memory_limit') != '-1') ini_set('memory_limit', '-1');
if ((php_sapi_name() != 'cli')) {
    trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}//end if

$config = array();

define('LOG_FILE_NAME', 'set_permissions.log');

if (count($argv) < 4) {
	usage();
    exit();
}//end if

// Get config from command line args.
process_args($config);

require_once $config['system_root'].'/core/include/init.inc';
require_once SQ_INCLUDE_PATH.'/assertions.inc';

define('LOG_FILE', SQ_SYSTEM_ROOT.'/data/private/logs/'.LOG_FILE_NAME);			// This is the log file
define('SYNCH_FILE', SQ_TEMP_PATH.'/set_permissions.assetid');		// We need this file to store the assetids

// Check perms.
check_permissions_args($config['permissions']);

// Replace space with empty string
$assetids = preg_replace('/[\s]*/', '', $config['assetids']);

// Explode them so we have the list in array
$rootnodes    = getRootNodes($assetids);

_disconnectFromMatrixDatabase();
$pid_prepare    = pcntl_fork();
    switch ($pid_prepare) {
        case -1:
            break;
        case 0:
            // Connect to DB within the child process
            _connectToMatrixDatabase();          

            $root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');

            // This ridiculousness allows us to workaround Oracle, forking and CLOBs
            // if a query is executed that returns more than 1 LOB before a fork occurs,
            // the Oracle DB connection will be lost inside the fork.
            // In this case, because a user asset has more than 1 attribute and custom_val in sq_ast_attr_val
            // is of type CLOB, we attempt to check the root password inside our forked process.
            // log in as root
            if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
                echo "Failed logging in as root user\n";
                exit(1);
            }//end if

            $children = $rootnodes; // Start off with the root nodes.
            foreach ($rootnodes as $rootnode_id) {
				$children    += array_merge($children, array_keys(($GLOBALS['SQ_SYSTEM']->am->getChildren($rootnode_id))));
				
            }//end foreach

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

$children     = explode(',', $children_str);
$children    = array_unique($children);		// We are only generate metadata for each asset once, despite they might be linked in different plaecs

// Chunk them up so we can process each batch when forking
$chunk_children    = array_chunk($children, $config['batch_size']);
$current_child_list    = Array();

log_to_file('======================= Start applying permissions '.date('d-m-Y h:i:s').' =======================', LOG_FILE);
log_to_file("Applying permissions for: " . var_export(count($children),TRUE) . " assets \n", LOG_FILE);

        $fork_num    = 0;		// Determine how many child process we have forked
        while (!empty($chunk_children)) {
            $current_child_list    = array_pop($chunk_children);
			$current_remaining = count($current_child_list);
      	    $pid	= pcntl_fork();
            $fork_num++;
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
						if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($child_assetid, 'permissions')) {
							log_to_file('Unable to acquire permissions lock for assetid ' .$child_assetid.'. Skipping this asset.', LOG_FILE);
							continue;
						}//end if

						if (!$child_asset->writeAccess('permissions')) {
							log_to_file('Do not have write access for assetid ' .$child_assetid .'. Skipping this asset.', LOG_FILE);
							$GLOBALS['SQ_SYSTEM']->am->releaseLock($child_assetid, 'permissions');
							$GLOBALS['SQ_SYSTEM']->am->forgetAsset($child_asset);
							continue;
						}//end if
						
						// For each permission setting in the permissions string
						foreach ($config['permissions'] as $p) {
							if ($p['granted'] == '/') {
								if (!$GLOBALS['SQ_SYSTEM']->am->deletePermission($child_assetid, $p['userid'], $p['type'])) {
									log_to_file('Failed deleting permission for assetid ' . $child_assetid . '.', LOG_FILE);
									continue;
								}
							} else {
								$permission_granted = (($p['granted'] == '+') ? TRUE : FALSE);
								if (!$GLOBALS['SQ_SYSTEM']->am->setPermission($child_assetid, $p['userid'], $p['type'], $permission_granted, $config['cascades'], $config['force'])) {
									log_to_file('Failed setting permission for assetid ' . $child_assetid . '.', LOG_FILE);
									continue;
								}
							}
						}//end foreach
						
						log_to_file('Applied permissions for child assetid '.$child_assetid, LOG_FILE);

						$current_remaining--;
						echo '-- Remaining: ' . (string) ($current_remaining + (count($chunk_children) * $config['batch_size'])) . "\n";

						$GLOBALS['SQ_SYSTEM']->am->releaseLock($child_assetid, 'permissions');
						$GLOBALS['SQ_SYSTEM']->am->forgetAsset($child_asset);
						$child_asset = NULL;
						unset($child_asset);

					}//end foreach

                    $GLOBALS['SQ_SYSTEM']->restoreRunLevel();
                    // Disconnect from DB
                    _disconnectFromMatrixDatabase();

                    exit(0);
                    // waiting for child exit signal
                    $status = null;
                    pcntl_waitpid(-1, $status);

                    break;
                default:
					// We only want to fork a maximum number of child process, so if we've already reached the max num, sit and wait
                    if ($fork_num >= $config['max_threads']) {
                        $status = null;
                        pcntl_waitpid(-1, $status);
                        $fork_num--;
                    }//end if

                    if (empty($chunk_children)) {
                        // We wait for all the fork child to finish
                        while ($fork_num > 0) {
                            $status = null;
                            pcntl_waitpid(-1, $status);
                            $fork_num--;
                        }//end
                    }//end if

                    break;

        	}//end switch & thread
        //}//end foreach
       }//end while
    

    log_to_file('======================= Finished Applying Permissions '.date('d-m-Y h:i:s').' =======================', LOG_FILE);    
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
	echo "Usage: php {$_SERVER['argv'][0]} <system_root> <assetid[,assetid]> \"<permission_strings>\"\n";
	echo "       php {$_SERVER['argv'][0]} <system_root> <assetid[,assetid]> \"<permission_strings>\" [-nc] [-f]\n";
	echo "       php {$_SERVER['argv'][0]} <system_root> <assetid[,assetid]> \"<permission_strings>\" [--max-threads <max_threads>] [--batch-size <batch_size>]\n";
	echo "\n";
	echo "  <permission_strings> A string representing the permissions to change.\n";
	echo "                       example: 'r+101;w-102;a/103' => add read permission to User #101,\n";
	echo "                       deny write permission to user #102 and remove admin permission for user #103.\n";
	echo "   -nc                 No cascade. Don't set the cascade flag, which allows new child assets to inherit the permissions.\n";
	echo "   -f                  Force permissions, even if the permissions are already applied or granted/denied are in conflict.\n";
	echo "  --max-threads        Maximum concurrency. Default is 3, max allowed is 5.\n";
	echo "  --batch-size         Default is 50.\n";
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
		exit();
	}

	if (!is_dir($config['system_root']) || !is_readable($config['system_root'].'/core/include/init.inc')) {
		echo "ERROR: Path provided doesn't point to a Matrix installation's System Root. Please provide correct path and try again.\n";
		exit();
	}

	$config['assetids'] = (isset($_SERVER['argv'][2])) ? $_SERVER['argv'][2] : '';
	if (empty($config['assetids'])) {
	    echo "ERROR: You need to specify the root nodes to apply the schema from as the second argument\n";
		exit();
	}//end if
	
	$config['permissions_string'] = (isset($_SERVER['argv'][3])) ? $_SERVER['argv'][3] : '';
	if (empty($config['permissions_string'])) {
		echo "ERROR: You need to specify a permissions string as the third argument\n";
		exit();
	}//end if
	
	// Permissions string			
	$config['permissions'] = array();
	$config['permission_types'] = Array('r' => 1, 'w' => 2, 'a' => 3);
			
	$perms = explode(';', $config['permissions_string'].';');
	foreach ($perms as $p) {
		if (!empty($p)) {
			array_push($config['permissions'], Array(
													'type' => (int) $config['permission_types'][$p{0}],
													'granted' => $p{1},
													'userid' => substr($p, 2),
												));
		}
	}
	
	$config['cascades'] = !get_boolean_arg('-nc'); // Invert.. -nc means no cascade.
	$config['force'] = get_boolean_arg('-f');
	
	$config['max_threads'] = (int) get_parameterised_arg('--max-threads', 3);
	if ($config['max_threads'] > 5) {
		$config['max_threads'] = 5;
	} elseif ($config['max_threads'] < 1) {
		$config['max_threads'] = 1;
	}//end if
	
	$config['batch_size'] = (int) get_parameterised_arg('--batch-size', 50);
	if ($config['batch_size'] < 5) {
		$config['batch_size'] = 50;
	}
	
	echo "\n";
	echo "Will attempt to set permissions to asset(s) {$config['assetids']}.\n\n";
	echo "  Max threads: {$config['max_threads']}\n";
	echo "  Batch size: {$config['batch_size']}\n";	
	echo "\n";

}//end function processArgs()


function check_permissions_args($permissions) {
	foreach ($permissions as $p) {
		if (($p['type'] > 3) || ($p['type'] < 1)) {
			 trigger_error("Invalid permission type in permissions string.\n", E_USER_ERROR);
		}
		
		if (($p['granted'] != '+') && ($p['granted'] != '-') && ($p['granted'] != '/')) {
			trigger_error("Invalid granted flag in permissions string.\n", E_USER_ERROR);
		}
		
		if (!assert_valid_assetid($p['userid'])) {
			trigger_error("Invalid userid in permissions string.\n", E_USER_ERROR);
		}
	}
}

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

?>
