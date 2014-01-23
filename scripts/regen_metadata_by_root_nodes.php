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
* This script will take an argument of a list of root nodes separated by commas, 
* then it will go find all the children of those root nodes and regenerate metadata for these child assets.
*
* @author  Huan Nguyen <hnguyen@squiz.net>
* @version $Revision: 1.8 $
* @package MySource_Matrix
*/

error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) {
    trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}//end if

if (count($argv) < 3) {
    echo "Usage: php scripts/regen_metadata_by_root_nodes.php <SYSTEM_ROOT> <ASSETID[, ASSETID]> <MAX_THREAD_NUM> <BATCH_SIZE> <--skip-asset-update> <--direct-children-only> \n";
    exit();
}//end if

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT)) {
	echo "ERROR: You need to supply the path to the System Root as the first argument\n";
	exit();
}

if (!is_dir($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT.'/core/include/init.inc')) {
	echo "ERROR: Path provided doesn't point to a Matrix installation's System Root. Please provide correct path and try again.\n";
	exit();
}

$assetids = (isset($_SERVER['argv'][2])) ? $_SERVER['argv'][2] : '';
if (empty($assetids)) {
    echo "ERROR: You need to specify the root nodes to regenerate metadata from as the second argument\n";
	exit();
}//end if

$max_thread_num = (isset($_SERVER['argv'][3])) ? $_SERVER['argv'][3] : '';
if (empty($max_thread_num) || ($max_thread_num > 5)) $max_thread_num = 3;

$batch_size = (isset($_SERVER['argv'][4])) ? $_SERVER['argv'][4] : '';
if (empty($batch_size)) $batch_size = 50;

$update_assets = TRUE;
$max_asset_depth = NULL;
if (isset($_SERVER['argv'][5])) {
	$options = array_slice($_SERVER['argv'], 5);
	foreach ($options as $option) {
		if ($option == '--skip-asset-update') {
			$update_assets = FALSE;
		} else if ($option == '--direct-children-only') {
			$max_asset_depth = 1;
		}//end if
	}
}


define('LOG_FILE', $SYSTEM_ROOT.'/data/private/logs/regen_metadata_by_root_nodes.log');			// This is the log file
define('SYNCH_FILE', $SYSTEM_ROOT.'/data/private/logs/regen_metadata_by_root_nodes.assetid');		// We need this file to store the assetids of those to be regenerated
define('BATCH_SIZE', $batch_size);																	// The number of assets being processed in one thread.
define('MAX_CONCURRENCY', $max_thread_num);															// The number of simultaneous threads can be spawned.


// Replace space with empty string
$assetids = preg_replace('/[\s]*/', '', $assetids);

$pid_prepare    = pcntl_fork();
    switch ($pid_prepare) {
        case -1:
            break;
        case 0:
            
            require_once $SYSTEM_ROOT.'/core/include/init.inc';          

            $root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');

            // This ridiculousness allows us to workaround Oracle, forking and CLOBs
            // if a query is executed that returns more than 1 LOB before a fork occurs,
            // the Oracle DB connection will be lost inside the fork.
            // In this case, because a user asset has more than 1 attribute and custom_val in sq_ast_attr_val
            // is of type CLOB, we attempt to check the root password inside our forked process.
            // log in as root
            if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
                echo "ERROR: Failed logging in as root user\n";
                exit(1);
            }//end if

			// Explode them so we have the list in array
			$rootnodes    = getRootNodes($assetids);
			
            $children = Array();
            foreach ($rootnodes as $rootnode_id) {
                $children    += array_merge($children, array_keys(($GLOBALS['SQ_SYSTEM']->am->getChildren($rootnode_id, '', TRUE, NULL, NULL, NULL, TRUE, 1, $max_asset_depth))));
            }//end foreach

			// Save the list into a file so we can access the list from the parent process
            file_put_contents(SYNCH_FILE, implode(',', $children));


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
$chunk_children    = array_chunk($children, BATCH_SIZE);
$current_child_list    = Array();

log_to_file('======================= Start Regenerating Metadata '.date('d-m-Y h:i:s').' =======================', LOG_FILE);
log_to_file("Regenerating for: " . var_export(count($children),TRUE) . " assets \n", LOG_FILE);

        $fork_num    = 0;		// Determine how many child process we have forked
        while (!empty($chunk_children)) {
            $current_child_list    = array_pop($chunk_children);
      	    $pid	= pcntl_fork();
            $fork_num++;
            switch ($pid) {
                case -1: 
                    trigger_error('Process failed to fork while regenerating metadata', E_USER_ERROR);
                    exit(1);
                    break;
                case 0:
                    
                    require_once $SYSTEM_ROOT.'/core/include/init.inc';
                    $GLOBALS['SQ_SYSTEM']->setCurrentUser($GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user'));
                    
                    $mm    = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
                    
                    foreach ($current_child_list as $child_assetid) {
                        $child_asset    = $GLOBALS['SQ_SYSTEM']->am->getAsset($child_assetid);
                        if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($child_assetid, 'metadata')) {
                            log_to_file('Unable to acquire metadata lock for assetid ' .$child_assetid.'. Skipping this asset.', LOG_FILE);
                            continue;
                        }//end if
        
                        if (!$child_asset->writeAccess('metadata')) {
                            log_to_file('Do not have write access for assetid ' .$child_assetid .'. Skipping this asset.', LOG_FILE);
                            continue;
                        }//end if
                        
                        if (!$mm->regenerateMetadata($child_assetid, 'all', $update_assets)) {
                            log_to_file('Failed regenerating metadata for assetid ' .$child_assetid .'.', LOG_FILE);
                            continue;
                        }//end if
                        
                        log_to_file('Regenerated Metadata for child assetid '.$child_assetid, LOG_FILE);
                        
                        $GLOBALS['SQ_SYSTEM']->am->releaseLock($child_assetid, 'metadata');
                        $GLOBALS['SQ_SYSTEM']->am->forgetAsset($child_asset);
                        $child_asset = NULL;
                        unset($child_asset);
                        
                    }//end foreach

                    $GLOBALS['SQ_SYSTEM']->restoreCurrentUser();

                    exit(0);
                    // waiting for child exit signal
                    $status = null;
                    pcntl_waitpid(-1, $status);

                    break;
                default:
					// We only want to fork a maximum number of child process, so if we've already reached the max num, sit and wait
                    if ($fork_num >= MAX_CONCURRENCY) {
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
    

    log_to_file('======================= Finished Regenerating Metadata '.date('d-m-Y h:i:s').' =======================', LOG_FILE);    
 	if (file_exists(SYNCH_FILE)) {
		unlink(SYNCH_FILE);
	}//end if
    exit(0);



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
* This function basically save the content input to a file
*
*/
function log_to_file($content, $file_name="regen_metadata_by_root_nodes.log")
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
        exit(1);
    }//end if   
 
    return $rootnodes;
}//end getRootNodes()

?>
