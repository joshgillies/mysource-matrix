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
* As per security fixes done couple of trigger actions regarding unrestricted server file access and unauthorised PHP file execution,
* this upgrade script address following two parts:
*
* 1. File path allowed in following trigger actions are now restricted to SQ_TOOL_AUTHORISED_PATHS:
* 	- Replace File Asset
* 	- Set Thesaurus Term from XML
* 	- Set Design Associated Files
* 	- Set Design Parse File
*
* This upgrade script will update the SQ_TOOL_AUTHORISED_PATHS to include the currently existing file paths in these trigger actions
* so that the security fix does breaks existing implementation.
*
* 2. Also the path to IPB SDK installation in Log in/out Invision Power Board trigger actions cannot be configured from backend anymore.
* This config setting has been moved to new external tool config SQ_TOOL_IPB_SDK_PATH.
* So this upgrade step will move this setting from trigger action the to SQ_TOOL_IPB_SDK_PATH.
*
* @author Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: $
* @package MySource_Matrix
*/

// Usage: php upgrade_trigger_actions_security_fix.php


$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
require_once $SYSTEM_ROOT.'/core/include/init.inc';
require_once SQ_DATA_PATH.'/private/conf/tools.inc';

$allowed_filepaths = Array();
if (defined('SQ_TOOL_AUTHORISED_PATHS')) {
	$allowed_filepaths = explode("\n", SQ_TOOL_AUTHORISED_PATHS);
}


$target_types_1 = Array(
					'trigger_action_replace_file_asset',
					'trigger_action_set_thesaurus_terms',
					'trigger_action_set_design_parse_file',
					'trigger_action_set_design_associated_files',
				);
$target_types_2 = Array(
					'trigger_action_login_ipb',
					'trigger_action_logout_ipb',
				);
$ibs_sdk_path = '';

// Get the file paths from "Create File Asset" trigger actions
$trigger_manager = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('trigger_manager');
$children = $trigger_manager->getTriggerList();
foreach($children as $trigger_data) {
	$trigger = $trigger_manager->getAsset($trigger_data['id']);
	$actions = $trigger->attr('actions');
	foreach ($actions as $index => $action) {
		if (isset($action['type']) && in_array($action['type'], $target_types_1)) {
			if (!empty($action['data']['file_path'])) {
				$allowed_filepaths[] = trim($action['data']['file_path']);
			}
		}

		// Get very first valid path for IPB SDK installation
		if (empty($ibs_sdk_path) && isset($action['type']) && in_array($action['type'], $target_types_2)) {
			if (!empty($action['data']['ipbsdk_path'])) {
				$action['data']['ipbsdk_path'] = trim($action['data']['ipbsdk_path']);
				if (is_dir($action['data']['ipbsdk_path'])) {
					$ibs_sdk_path = $action['data']['ipbsdk_path'];
				}
			}
		}//end if ipb sdk trigger action

	}//end foreach
}//end foreach

$allowed_filepaths = array_unique($allowed_filepaths);

echo "Setting external tool config parameters SQ_TOOL_AUTHORISED_PATHS and SQ_TOOL_IPB_SDK_PATH as security changes in trigger actions ... ";
// Add those "allowed" file paths to the white list in external tool config file
// and also add path for IPB SDK installation
$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user);
$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
	include_once SQ_INCLUDE_PATH.'/external_tools_config.inc';
	$tools_config = new External_Tools_Config();
	$vars = Array(
				'SQ_TOOL_AUTHORISED_PATHS' => implode("\n", $allowed_filepaths),
				'SQ_TOOL_IPB_SDK_PATH' => $ibs_sdk_path,
			);

	$result = $tools_config->save($vars, TRUE, FALSE);
$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
$GLOBALS['SQ_SYSTEM']->restoreCurrentUser();

echo $result ? 'Done.' : 'Failed';
echo "\n";

?>
