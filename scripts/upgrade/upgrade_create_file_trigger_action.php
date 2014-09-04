<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
*/

/**
* To prevent unauthorised access to the files in the server a new parameter SQ_TOOL_AUTHORISED_PATHS has been added
* to tools.inc config file. This parameter defines the list of paths allowed by Matrix to source the files for
* backend operations like "Create File Asset" trigger action
*
* This uprage script basically copies the 'file_paths' setting in the existing "Create File Asset" trigger actions
* to this new parameter SQ_TOOL_AUTHORISED_PATHS, to white list those files.
*
* @author Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: $
* @package MySource_Matrix
*/

// Usage: php upgrade_create_file_trigger_action.php


$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
require_once $SYSTEM_ROOT.'/core/include/init.inc';

$allowed_filepaths = Array();


// Get the file paths from "Create File Asset" trigger actions
$trigger_manager = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('trigger_manager');
$children = $trigger_manager->getTriggerList();
foreach($children as $trigger_data) {
	$trigger = $trigger_manager->getAsset($trigger_data['id']);
	$actions = $trigger->attr('actions');
	foreach ($actions as $index => $action) {
		if (isset($action['type']) && $action['type'] == 'trigger_action_create_file_asset') {
			if (!empty($action['data']['file_path'])) {
				$allowed_filepaths[] = trim($action['data']['file_path']);
			}
		}
	}//end foreach
}//end foreach

// Add those file paths to the white list
if (!empty($allowed_filepaths)) {
	echo "\nSetting 'SQ_TOOL_AUTHORISED_PATHS' based on the existing 'Create File Asset' trigger actions... ";
	// Save into the External tool config file
	$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
	$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user);
	$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
		include_once SQ_INCLUDE_PATH.'/external_tools_config.inc';
		$tools_config = new External_Tools_Config();
		$vars = Array(
					'SQ_TOOL_AUTHORISED_PATHS' => implode("\n", $allowed_filepaths),
				);
					
		$result = $tools_config->save($vars, TRUE, FALSE);
	$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
	$GLOBALS['SQ_SYSTEM']->restoreCurrentUser();

	echo $result ? 'Done.' : 'Failed';
	echo "\n";
}
?>
