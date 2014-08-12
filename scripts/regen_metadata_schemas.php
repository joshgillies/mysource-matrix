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
* $Id: regen_metadata_schemas.php,v 1.7 2012/08/30 01:04:53 ewang Exp $
*
*/

/**
* This script regenerates metadata for specified metadata schemas in the system.
* If no schema is specified, then all schemas in the system are regenerated.
*
* @author  Edison Wang <ewang@squiz.com.au>
* @version $Revision: 1.7 $
* @package MySource_Matrix
*/

if (ini_get('memory_limit') != '-1') ini_set('memory_limit', '-1');
error_reporting(E_ALL);
$metadata_schemas = Array();

if (php_sapi_name() != 'cli') {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

// Check for valid system root
$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT)) {
	echo "ERROR: You need to supply the path to the System Root as the first argument\n";
	echo 'Usage: '.basename($_SERVER['argv'][0])." <system root> [schema ID]...\n";
	exit();
}

if (!is_dir($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT.'/core/include/init.inc')) {
	echo "ERROR: Path provided doesn't point to a Matrix installation's System Root. Please provide correct path and try again.\n";
	echo 'Usage: '.basename($_SERVER['argv'][0])." <system root> [schema ID]...\n";
	exit();
}

define('SQ_SYSTEM_ROOT', realpath($SYSTEM_ROOT));
define('BATCH_SIZE', 50);																	// The number of assets being processed in one thread.
define('MAX_CONCURRENCY',  3);	
define('SYNCH_FILE', $SYSTEM_ROOT.'/data/private/logs/regen_metadata_by_schema.assetid');


$pid_prepare    = pcntl_fork();
    switch ($pid_prepare) {
        case -1:
            break;
        case 0:
            
            require_once $SYSTEM_ROOT.'/core/include/init.inc';          
            // This ridiculousness allows us to workaround Oracle, forking and CLOBs
            // if a query is executed that returns more than 1 LOB before a fork occurs,
            // the Oracle DB connection will be lost inside the fork.
            // have to place all queries inside a forked process, and store back results to a physical file

            // If user has specified schema ID's manually, use those
            if (isset($_SERVER['argv'][2])) {
                    $metadata_schemas = $_SERVER['argv'];

                    // Remove the first two arguments
                    array_shift($metadata_schemas);
                    array_shift($metadata_schemas);
            } else {

                    // Otherwise, get all the metadata schemas in the system
                    $metadata_schemas = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('metadata_schema', TRUE);
            }


           $mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
           $total_assets = Array();
            foreach ($metadata_schemas as $schemaid) {
                $schema = $GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
                if (!$schema) {
                        trigger_error('Schema asset #'.$schemaid.' could not be found', E_USER_WARNING);
                        continue;
                }
                if (!($schema instanceof Metadata_Schema)) {
                        trigger_error('Asset #'.$schema->id.' is not a metadata schema', E_USER_WARNING);
                        continue;
                }
                $total_assets = array_merge($total_assets, $mm->getSchemaAssets($schemaid, TRUE));
            }
            
            // Save the list into a file so we can access the list from the parent process
            file_put_contents(SYNCH_FILE, implode(',', $total_assets));


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

    
 // restore the to-be-processed assetid from the physical file
$total_assets    = Array();
if (file_exists(SYNCH_FILE)) {
   $total_assets_str    = file_get_contents(SYNCH_FILE);
} else {
    trigger_error ("Unable to find Synch File, probably because the user executing this script does not have permission to write to this folder.", E_USER_WARNING);
    exit(0);
}//end else

$total_assets     = explode(',', $total_assets_str);
$total_assets    = array_unique($total_assets);
// Chunk them up so we can process each batch when forking
$chunk_assets    = array_chunk($total_assets, BATCH_SIZE);
$current_asset_list    = Array();


  printUpdateStatus ("Total ".count($total_assets)." found");
   $fork_num    = 0;		// Determine how many child process we have forked
        while (!empty($chunk_assets)) {
            // do 50 assets per process
            // the main reason we need to chunk and fork is because  memory leakage prevention
            // call hipo job directly will cause slowly increasing memory usage, and eventually explode
            $current_asset_list    = array_pop($chunk_assets);
      	    $pid	= pcntl_fork();
            $fork_num++;
            switch ($pid) {
                case -1: 
                    trigger_error('Process failed to fork while regenerating metadata', E_USER_ERROR);
                    exit(1);
                    break;
                case 0:
                    
                    require_once $SYSTEM_ROOT.'/core/include/init.inc';
  
                    $mm    = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
                    
                    foreach ($current_asset_list as $assetid) {
                        $asset    = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
                        $GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
                        if (!$mm->regenerateMetadata($assetid, 'all')) {
                        	trigger_error('Asset failed to regenrate metedata #'.$assetid.'', E_USER_WARNING);
                            continue;
                        }//end if
                        else {
                        	printUpdateStatus('Regenerated Metadata for assetid '.$assetid);
                        }
                    
                        $GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
                        unset($asset);
                        
                    }//end foreach

                    unset($current_asset_list);
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

                    if (empty($chunk_assets)) {
                        // We wait for all the fork child to finish
                        while ($fork_num > 0) {
                            $status = null;
                            pcntl_waitpid(-1, $status);
                            $fork_num--;
                        }//end
                    }//end if

                    break;

        }//end switch & thread
       }//end while
       
       
     if (file_exists(SYNCH_FILE)) {
            unlink(SYNCH_FILE);
    }//end if
    
    printUpdateStatus("Done");
    
    exit(0);




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


?>
