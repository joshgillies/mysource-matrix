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
* $Id: import_from_xml.php,v 1.24.2.2 2012/12/13 01:40:59 ewang Exp $
*
*/

/**
* Creates assets based on an xml file provided.
*
* See the accompanying file 'xml_example.xml' for an example structure
*
*
* @author  Darren McKee <dmckee@squiz.net>
* @version $Revision: 1.24.2.2 $
* @package MySource_Matrix
*/

define ('SQ_IN_IMPORT', 1);

error_reporting(E_ALL);
if (ini_get('memory_limit') != '-1') ini_set('memory_limit', '-1');

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

$import_file = (isset($_SERVER['argv'][2])) ? $_SERVER['argv'][2] : '';
if (empty($import_file) || !is_file($import_file)) {
	echo "You need to supply the path to the import file as the second argument\n";
	exit();
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';

if (isset($_SERVER['argv'][3]) && $_SERVER['argv'][3] == '--root-node' ) {
	$root_node_id = (isset($_SERVER['argv'][4])) ? $_SERVER['argv'][4] : '';
	if (empty($root_node_id)) {
		echo "you need to supply root node under which the assets are to be imported as fourth argument\n";
		exit();
	}
}

$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');

require_once SQ_LIB_PATH.'/import_export/import.inc';
$import_actions = get_import_actions($import_file);

//done checking authenticity of the system, first thing we check is the valid asset id for root node
if (isset($_SERVER['argv'][3]) && $_SERVER['argv'][3] == '--root-node' ) {
	//we are going to try and get the asset, we dont wanna be throwing errors here, so mute them
	error_reporting(E_NOTICE);
	$root_node = $GLOBALS['SQ_SYSTEM']->am->getAsset($root_node_id);
	if (is_null($root_node)) {
		echo "\nProvided assetid is not valid for given system, Script will stop execution\n";
		exit;
	}
	//restore error reporting
	error_reporting(E_ALL);

	$import_actions['actions'][0]['action'][0]['parentid'][0] = $root_node_id;

}

// overcom oracle 'end of communication' bug
_disconnectFromMatrixDatabase();

// temp file to store global data, so child process can access it
define('TEMP_FILE', SQ_TEMP_PATH.'/import_from_xml.tmp');	
if(is_file(TEMP_FILE)) unlink(TEMP_FILE);

$import_action_outputs = Array();
$nest_content_to_fix = Array();
$designs_to_fix = Array();
// Loop through the Actions from the XML File
foreach ($import_actions['actions'][0]['action'] as $action) {
	// remember nest content to fix
	if($action['action_type'][0] === 'create_asset' && $action['type_code'][0] === 'Content_Type_Nest_Content') {	
		$nest_content_to_fix[] = $action['action_id'][0];
	}
	if($action['action_type'][0] === 'create_asset' && $action['type_code'][0] === 'Design') {	
		$designs_to_fix[] = $action['action_id'][0];
	}
	

 
	// Use forked process to make sure memory usage is stable
	$pid = pcntl_fork();
	switch ($pid) {
		case -1: 
			trigger_error('Process failed to fork', E_USER_ERROR);
			exit(1);
			break;
		case 0:
			// process action in forked child process
			// connect to DB within the child process to overcome oracle stupidness
			_connectToMatrixDatabase();          
			$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_OPEN);
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
			// restore the temp data from file
			if(is_file(TEMP_FILE))
			    $import_action_outputs = unserialize(file_get_contents(TEMP_FILE));
			
			// check if there is hard coded assetid reference which doesn't exist in the target system
			if(!checkAssetExists($action, 'parentid') || !checkAssetExists($action, 'assetid') || !checkAssetExists($action, 'asset')) {
				trigger_error('Action ID "'.$action['action_id'][0].'" contains non-exist assetid reference. Action skipped.', E_USER_WARNING);
				_disconnectFromMatrixDatabase();
				exit(0);
				break;
			}
			
			// Execute the action
			printActionId($action['action_id'][0]);
			if (!execute_import_action($action, $import_action_outputs)) {
				printStatus('--');
				trigger_error('Action ID "'.$action['action_id'][0].'" could not be executed', E_USER_WARNING);
			} else {
				printStatus('OK');
			}
			
			// save global temp data to file
			$temp_string = serialize($import_action_outputs);
			string_to_file($temp_string, TEMP_FILE);
			
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
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
	
}


// fix nest content type, regenerate the bodycopy
foreach ($nest_content_to_fix as $actionid) {
	if(isset($import_action_outputs[$actionid])) {
		$nest_content_id = $import_action_outputs[$actionid]['assetid'];
		$nest_content = $GLOBALS['SQ_SYSTEM']->am->getAsset($nest_content_id);
		$nest_content->_tmp['edit_fns'] = NULL;
		$nest_content->linksUpdated();
	}
}

// we have imported a few design , lets fix them
foreach ($designs_to_fix as $design) {
	if(isset($import_action_outputs[$design])) {
		$design_id = $import_action_outputs[$design]['assetid'];
		$vars = Array('assetid' => $design_id);
		$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$hh->freestyleHipo('hipo_job_regenerate_design', $vars);
	}
}

// remove the temp file
if(is_file(TEMP_FILE)) unlink(TEMP_FILE);



/**
* Prints the Action ID as a padded string
*
* @param string	$action_id	the id of the action
*
* @return void
* @access public
*/
function printActionId($action_id)
{
	if (strlen($action_id) > 66) {
		$action_id = substr($action_id, 0, 66).'...';
	}
	printf ('%s%'.(70 - strlen($action_id)).'s', $action_id,'');

}//end printActionId()


/**
* Prints the status of the import action
*
* @param string	$status	the status of the action
*
* @return void
* @access public
*/
function printStatus($status)
{
	echo "[ $status ]\n";

}//end printStatus()


/**
* Check if hard coded assetid reference is a valid assetid in target system
*
* @param array	$action	the action xml
* @param string	$type	the type of asset id field asset | assetid | parentid
*
* @return void
* @access public
*/
function checkAssetExists($action, $type='asset')
{
    	if(isset($action[$type][0]) && preg_match('/^[0-9]+$/', $action[$type][0])){
	    return ($GLOBALS['SQ_SYSTEM']->am->assetExists ($action[$type][0]));
	}	
	return TRUE;
}


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


?>
