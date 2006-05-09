<?php

/**
* Creates assets based on an xml file provided.
*
* See the accompanying file 'xml_example.xml' for an example structure
*
*
* @author  Darren McKee <dmckee@squiz.net>
* @version $Revision: 1.2.2.1 $
* @package MySource_Matrix
*/

error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) trigger_error("You can only run this script from the command line\n", E_USER_ERROR);

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	trigger_error("You need to supply the path to the System Root as the first argument\n", E_USER_ERROR);
}

$import_file = (isset($_SERVER['argv'][2])) ? $_SERVER['argv'][2] : '';
if (empty($import_file) || !is_file($import_file)) {
	trigger_error("You need to supply the path to the import file as the second argument\n", E_USER_ERROR);
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';

// ask for the root password for the system
echo 'Enter the root password for "'.SQ_CONF_SYSTEM_NAME.'": ';
$root_password = rtrim(fgets(STDIN, 4094));

// check that the correct root password was entered
$root_user = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
if (!$root_user->comparePassword($root_password)) {
	echo "ERROR: The root password entered was incorrect\n";
	exit();
}

require_once SQ_LIB_PATH.'/import_export/import.inc';

$import_actions = get_import_actions($import_file);

$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_OPEN);
$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

$import_action_outputs = Array();
// Loop through the Actions from the XML File
foreach ($import_actions['actions'][0]['action'] as $action) {

	// Execute the action
	if (!execute_import_action($action, $import_action_outputs)) {
		$action_id = $action['action_id'][0];
		trigger_error('Action ID, "'.$action_id.'" could not be executed', E_USER_ERROR);
	}
}

$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

?>
