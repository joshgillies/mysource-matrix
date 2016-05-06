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
* If the assets of type Design, Metadata Schema and Workflow Schema are in safe edit status, cancel the safe edit
* See SquizMap #6213
*
* @author Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: $
* @package MySource_Matrix
*/

// Usage: php upgrade_safe_edit_not_supported_assets.php <SYSTEM_ROOT> [y]

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

$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user);

pre_echo('Cancelling safe edit status on Design, Metadata Schema and Workflow Schema assets ...');

require_once SQ_FUDGE_PATH.'/general/file_system.inc';
$assetid_list = array_merge(
					$GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('design', FALSE),
					$GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('metadata_schema', FALSE),
					$GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('workflow_schema', FALSE)
				);

// We will use hipo job to change the status as it will handle the dependant children
$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();

$upgraded_assetids = Array();
$unsuccessful_assetids = Array();
foreach($assetid_list as $assetid) {
	$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);

	// We need to process all design assets and its decendents assets
	// However these are dependants 'design' assets as well decendents of design asset, so we can ignore these as they get processed along with the parent
	if ($asset->type() == 'design_customisation' || $asset->type() == 'design_css_customisation') {
		continue;
	}

	// Assets that are in safe edit status
	// We don't cancel safe edit for assets that are already approved i.e. in SQ_STATUS_EDITING_APPROVED status
	if ($asset->status != SQ_STATUS_EDITING && $asset->status != SQ_STATUS_EDITING_APPROVAL) {
		continue;
	}

	echo '.';

	if ($report_only) {
		$upgraded_assetids[] = $assetid;
	} else {
		$status_errors = Array();

		$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_OPEN);

			// If in "safe edit pending approval" status, change it to safe edit first to make go through workflow process smoothly
			if ($asset->status == SQ_STATUS_EDITING_APPROVAL) {
				$vars = Array(
							'assetid' => $asset->id,
							'new_status' => SQ_STATUS_EDITING,
						);
				$status_errors = $hh->freestyleHipo('hipo_job_edit_status', $vars);
			}

			// If in "safe edit" status, change it to "live". This will cancel the safe edit mode
			if (empty($status_errors) && $asset->status == SQ_STATUS_EDITING) {
				$vars = Array(
							'assetid' => $asset->id,
							'new_status' => SQ_STATUS_LIVE,
						);
				$status_errors = $hh->freestyleHipo('hipo_job_edit_status', $vars);
			}

		$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

		if (empty($status_errors)) {
			$upgraded_assetids[] = $assetid;
		} else {
			$unsuccessful_assetids[] = $assetid;
		}
	}//end else

}//end foreach

$GLOBALS['SQ_SYSTEM']->restoreCurrentUser();

if (!empty($upgraded_assetids)) {
	$upgraded_assetids = array_unique($upgraded_assetids);
	pre_echo(count($upgraded_assetids).' Asset(s) '.($report_only ? 'requires upgrading (cancel' : 'upgraded (cancelled').' safe edit): '.implode(", ", $upgraded_assetids));
}

if (!empty($unsuccessful_assetids)) {
	$unsuccessful_msg = count($unsuccessful_assetids). ' Assets had issues when cancelling safe edit. Please review these assets: '.implode(", ", $unsuccessful_assetids);
	pre_echo($unsuccessful_msg);
	log_dump($unsuccessful_msg);
}

pre_echo('Done.');

?>
