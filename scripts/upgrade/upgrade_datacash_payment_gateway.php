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
* Move the Datacash Payment Gateway attributes 'datacash_api_path' and 'cardinfo_dir_path' to
* the new external tool config parameters
* See #6647
*
* @author Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: 1.5 $
* @package MySource_Matrix
*/

// Usage: php upgrade_datacash_payment_gateway.php <SYSTEM_ROOT>

if (ini_get('memory_limit') != '-1') ini_set('memory_limit', -1);
error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) trigger_error("You can only run this script from the command line\n", E_USER_ERROR);

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
$report_only = (isset($_SERVER['argv'][2]) && $_SERVER['argv'][2] == 'y') ? FALSE : TRUE;

if (empty($SYSTEM_ROOT)) {
	echo "ERROR: You need to supply the path to the System Root as the first argument\n";
	exit();
}

if (!is_dir($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT.'/core/include/init.inc')) {
	echo "ERROR: Path provided doesn't point to a Matrix installation's System Root. Please provide correct path and try again.\n";
	exit();
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';

$root_user = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user);

pre_echo('Upgrading Datacash payment gateway assets');
$assetid_list = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('payment_gateway_datacash', FALSE);
foreach ($assetid_list as $assetid) {
	$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
	$datacsh_api_path = $asset->attr('datacash_api_path');
	$cardinfo_dir_path = $asset->attr('cardinfo_dir_path');
	if (!empty($datacsh_api_path) && !empty($cardinfo_dir_path)) {
		if (is_file($datacsh_api_path) && is_dir($cardinfo_dir_path)) {
			// Use Datacase API settting from the very first Datacash gateway asset with the valid values
			include_once SQ_INCLUDE_PATH.'/external_tools_config.inc';
			$tools_config = new External_Tools_Config();
			$vars = Array(
						'SQ_TOOL_DATACASH_API_PATH' => $datacsh_api_path,
						'SQ_TOOL_DATACASH_CARDINFO_DIR_PATH' => $cardinfo_dir_path,
					);
			$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
			$tools_config->save($vars, TRUE, FALSE);
			$GLOBALS['SQ_SYSTEM']->restoreRunLevel(SQ_RUN_LEVEL_FORCED);
			pre_echo('Datacash API external tool config parameters set using Datacash Payment Gateway asset #'.$asset->id.'.');
			break;
		}
	}
	$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
}//end foreach
pre_echo('Done.');

?>
