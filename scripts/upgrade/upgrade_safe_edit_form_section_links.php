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
* Create the system version section links info file for form assets with sections in safe edit
* See SquizMap #6754
*
* @author Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: $
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

pre_echo('Upgrading Form assets ...');

require_once SQ_FUDGE_PATH.'/general/file_system.inc';
$assetid_list = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('form_email', FALSE);
$upgraded_assetids = Array();
$unsuccessful_assetids = Array();
foreach($assetid_list as $assetid) {
	$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
	// Form assets that are in safe edit
	if ($asset->status & SQ_SC_STATUS_SAFE_EDITING) {
		if (is_file($asset->data_path.'/.sq_system/before_section_links')) {
			continue;
		}

		echo '.';
		$sections = $asset->getAllSections();
		$relevant_assets = Array($asset->id => $asset);
		foreach($sections as $section) {
			$relevant_assets[$section->id] = $section;
		}

		// Create system version "section links" info file for Form asset and its sections
		foreach($relevant_assets as $relevant_asset) {
			if (!($relevant_asset->status & SQ_SC_STATUS_SAFE_EDITING)) {
				continue;
			}
			$section_links = $relevant_asset->getSectionLinks();
			foreach($section_links as $link_index => $section_link) {
				if (isset($section_link['minorid']) && isset($relevant_assets[$section_link['minorid']])) {
					$link_asset = $relevant_assets[$section_link['minorid']];
					if (!($link_asset->status & SQ_SC_STATUS_SAFE_EDITING)) {
						unset($section_links[$link_index]);
					}
				}
			}//end foreach

			$success = FALSE;
			if (!$report_only) {
				$section_links = array_values($section_links);
				$success = string_to_file(serialize($section_links), $relevant_asset->data_path.'/.sq_system/after_section_links') &&
					string_to_file(serialize($section_links), $relevant_asset->data_path.'/.sq_system/before_section_links');
			}

		}//end foreach sections

		// Fix the form's system version content file pointing to correct system version section content files
		if (!$report_only && is_file($asset->data_path.'/.sq_system/content_file.php')) {
			$content = file_get_contents($asset->data_path.'/.sq_system/content_file.php');

			// Change the form sections content file path to their respective system versions
			$form_sections = $asset->getSections();
			if (!empty($form_sections)) {
				foreach($form_sections as $form_section) {
					if (!is_null($form_section)) {
						$raw_data_path = str_replace(SQ_DATA_PATH.'/', 'SQ_DATA_PATH."/', $form_section->data_path);
						$content = str_replace($raw_data_path.'/content_file.php"', $raw_data_path.'/.sq_system/content_file.php"', $content);
					}//end if
				}//end foreach form sections
				$success = string_to_file($content, $asset->data_path.'/.sq_system/content_file.php');
			}//end if
		}//end if
					
		if ($success || $report_only) {
			$upgraded_assetids[] = $assetid;
		} else {
			$unsuccessful_assetids[] = $assetid;
		}
	}//end if
}//end foreach form assets

if (!empty($upgraded_assetids)) {
	$upgraded_assetids = array_unique($upgraded_assetids);
	pre_echo(count($upgraded_assetids).' Form asset(s) '.($report_only ? 'requires upgrading' : 'upgraded').': '.implode(", ", $upgraded_assetids));
}

if (!empty($unsuccessful_assetids)) {
	$unsuccessful_msg = count($unsuccessful_assetids). ' Form assets were not upgraded. Please review these assets: '.implode(", ", $unsuccessful_assetids);
	pre_echo($unsuccessful_msg);
	log_dump($unsuccessful_msg);
}

pre_echo('Done.');

?>