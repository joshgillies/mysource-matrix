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
* $Id: import_from_xml.php,v 1.30 2013/04/09 01:59:59 ewang Exp $
*
*/

/**
* Creates assets based on an xml file provided.
*
* See the accompanying file 'xml_example.xml' for an example structure
*
*
* @author  Darren McKee <dmckee@squiz.net>
* @version $Revision: 1.30 $
* @package MySource_Matrix
*/

define ('SQ_IN_IMPORT', 1);

error_reporting(E_ALL);
if (ini_get('memory_limit') != '-1') ini_set('memory_limit', '-1');

// enable gabage collection
if(function_exists ('gc_enable')) {
    gc_enable();
}

if ((php_sapi_name() != 'cli')) trigger_error("You can only run this script from the command line\n", E_USER_ERROR);

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT)) {
	echo "ERROR: You need to supply the path to the System Root as the first argument\n";
	usage();
	exit();
}

if (!is_dir($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT.'/core/include/init.inc')) {
	echo "ERROR: Path provided doesn't point to a Matrix installation's System Root. Please provide correct path and try again.\n";
	usage();
	exit();
}

$import_file = (isset($_SERVER['argv'][2])) ? $_SERVER['argv'][2] : '';
if (empty($import_file) || !is_file($import_file)) {
	echo "You need to supply the path to the import file as the second argument\n";
	usage();
	exit();
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';

if (isset($_SERVER['argv'][3]) && $_SERVER['argv'][3] == '--root-node' ) {
	$root_node_id = (isset($_SERVER['argv'][4])) ? $_SERVER['argv'][4] : '';
	if (empty($root_node_id)) {
		echo "you need to supply root node under which the assets are to be imported as fourth argument\n";
		usage();
		exit();
	}
}//end if


// should we skip virus check?
$skip_virus_check = getCLIArg('skip-virus-check');
if($skip_virus_check) $GLOBALS['SQ_SKIP_VIRUS_SCAN'] = TRUE;


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
		usage();
		exit;
	}
	//restore error reporting
	error_reporting(E_ALL);
	$import_actions['actions'][0]['action'][0]['parentid'][0] = $root_node_id;
}//end if

$force_create = FALSE;
if (isset($_SERVER['argv'][3]) && $_SERVER['argv'][3] == '--force-create-dependants' || isset($_SERVER['argv'][5]) && $_SERVER['argv'][5] == '--force-create-dependants') {
	$force_create = TRUE;
}//end if

$total_number = count($import_actions['actions'][0]['action']);

// overcom oracle 'end of communication' bug
_disconnectFromMatrixDatabase();

// temp file to store global data, so child process can access it
// also used as a progress file to resume previous progress
$import_file_name_hash = md5($import_file);
define('TEMP_FILE', SQ_TEMP_PATH.'/import_from_xml_'.$import_file_name_hash.'.tmp');
if(is_file(TEMP_FILE)) {
    echo "Previous progress file is detected, resuming to previous import.\n";
    echo "Using ".TEMP_FILE."\n\n";
    $actions_done = unserialize(file_get_contents(TEMP_FILE));
}


$import_action_outputs = Array();
$nest_content_to_fix = Array();
$designs_to_fix = Array();
// Loop through the Actions from the XML File
foreach ($import_actions['actions'][0]['action'] as $index => $action) {
	// skip actions that have done before
	// also try those previous failed actions
	if(isset($actions_done[$action['action_id'][0]])) {
	    continue;
	}


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
			if ($force_create) {
				$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_SECURITY_INTEGRITY);
			} else {
				$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_OPEN);
			}
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
				trigger_error('Action ID "'.$action['action_id'][0].'" could not be executed', E_USER_WARNING);
			} else {
				$count = $index + 1;
				printStatus($count.'/'.$total_number);
			}

			// blank out old value and new value returned from set attribute action.
			// recording the attribute value content will flood our temp data file and memory
			$new_entry = array_slice( $import_action_outputs, -1, 1, TRUE );
			$new_entry_key = key($new_entry);
			if(isset($new_entry[$new_entry_key]['old_value'])) unset($new_entry[$new_entry_key]['old_value']);
			if(isset($new_entry[$new_entry_key]['new_value'])) unset($new_entry[$new_entry_key]['new_value']);
			$import_action_outputs = array_merge($import_action_outputs, $new_entry);

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

_connectToMatrixDatabase();

// restore import action outputs from file
$import_action_outputs = unserialize(file_get_contents(TEMP_FILE));

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

// disable gabage collection
if(function_exists ('gc_disable')) {
    gc_disable();
}

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
    while (TRUE) {
	try {
	    $conn_id = MatrixDAL::getCurrentDbId();
	}
	catch (Exception $e) {
	    // run out of connections, that's it
	    break;
	}
	MatrixDAL::restoreDb();
	MatrixDAL::dbClose($conn_id);
    }

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
* Prints the usage details about the script on command line
*
* @return void
* @access public
*/
function usage()
{
	echo "\nUSAGE:\n".basename(__FILE__)." <system_root> <import_xml_file_to_read_from> [--root-node XX] [--force-create-dependants] [--skip-virus-check]\n\n";
	echo "system_root                   :The path to the Matrix install\n";
	echo "import_xml_file_to_read_from  :The XML file that contains the asset data\n";
	echo "--root-node                   :Optional argument used to let the script know the assetid to import the assets under.\n";
	echo "                               --root-node needs to be followed by assetid after a space\n";
	echo "--force-create-dependants     :Optional argument used to tell script to force create the dependant assets (like bodycopy under standard page)\n";
	echo "                               if the actions to create these isn't in the XML import file\n";
	echo "--skip-virus-check            :Optional argument. If global Virus Checker is enabled, immediate scan of created file will slow down the import process.\n";
	echo "                               You can disable this immediate check using this argument.\n";

}//end usage()


/**
 * Get CLI Argument
 * Check to see if the argument is set, if it has a value, return the value
 * otherwise return true if set, or false if not
 *
 * @params $arg string argument
 *
 * @return string/boolean
 * @author Matthew Spurrier
 */
function getCLIArg($arg)
{
	return (count($match = array_values(preg_grep("/--" . $arg . "(\=(.*)|)/i",$_SERVER['argv']))) > 0 === TRUE) ? ((preg_match('/--(.*)=(.*)/',$match[0],$reg)) ? $reg[2] : true) : false;

}//end getCLIArg()

?>
