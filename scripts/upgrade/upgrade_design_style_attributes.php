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
* Upgrade the design and design cusotmisation's 'wysiwyg_classes' and 'div_classes' attribute values as per SquizMap #6472
*
* 'wysiwyg_classes' attribute value is upgraded from following format:
* Array(
*	'[class name 1]' => '[class friendly name 1]',
*	)
* to
* Array(
*	'[friendly_name 1]' => Array(
*							'class_name' => '[class friendly name 1]',
*							'show_for' => '[show for value]',
*							'hide_for' => '[hide for value]',
*						),
*	)
*
* While 'div_classes' attribute value is:
* Array(
*	'[class name 1]' => '[class friendly name 1]',
*	)
* to
* Array(
*	'[friendly_name 1]' => Array(
*							'class_name' => '[class friendly name 1]',
*						),
*	)
*
* Usage: php upgrade_design_style_attributes.php [MATRIX_ROOT] <--update>
* Omitting "--update" will run the script in report mode.
* 
*/


error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
$report_only = (isset($_SERVER['argv'][2]) && $_SERVER['argv'][2] == '--update') ? FALSE : TRUE;

if (empty($SYSTEM_ROOT)) {
	echo "ERROR: You need to supply the path to the System Root as the first argument\n";
	exit(1);
}

if (!is_dir($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT.'/core/include/init.inc')) {
	echo "ERROR: Path provided doesn't point to a Matrix installation's System Root. Please provide correct path and try again.\n";
	exit(1);
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';

$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');

// log in as root
if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	echo "ERROR: Failed logging in as root user\n";
	exit(1);
}

// Upgrade the style definition attributes on Design and Design Customisation assets
$design_ids = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('design', TRUE, TRUE) + $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('design_customisation', TRUE, TRUE);

echo "Updating design/design customisation asset's style definition attributes...\n";
foreach ($design_ids as $design_id => $type_code) {
	$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($design_id);

	$value = $asset->vars['wysiwyg_classes']['value'];
	$new_value = Array();
	$asset_updated = FALSE;
	if (!empty($value)) {
		$update_required = FALSE;
		foreach($value as $friendly_name => $class_name) {
			if (!is_array($class_name)) {
				$update_required = TRUE;
				$new_value[$friendly_name] = Array(
										'classNames' => $class_name,
										'showFor' => '',
										'hideFor' => '',
									);
			}
		}//end foreach
		if ($update_required && !empty($new_value)) {
			$asset->setAttrValue('wysiwyg_classes', $new_value);
			$asset_updated = TRUE;
		}
	}//end if

	$value = $asset->vars['div_classes']['value'];
	$new_value = Array();
	if (!empty($value)) {
		$update_required = FALSE;
		foreach($value as $friendly_name => $class_name) {
			if (!is_array($class_name)) {
				$update_required = TRUE;
				$new_value[$friendly_name] = Array(
										'classNames' => $class_name,
									);
			}
		}//end foreach
		if ($update_required && !empty($new_value)) {
			$asset->setAttrValue('div_classes', $new_value);
			$asset_updated = TRUE;
		}
	}//end if
	
	$success = TRUE;
	if ($asset_updated && !$report_only) {
		$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
			$success = $asset->saveAttributes();
		$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
	}

	echo '#'.$design_id.' ... '.($asset_updated ? (' updating ... '.($success ? 'done.' : 'failed.')) : ' update not required.')."\n";

}//end foreach

echo "\n";
echo "Finished updating design and design customisation asset's attributes.\n";
if ($report_only) {
	echo "NOTE: The report ran is 'repot mode'. Database was not updated.\n";
}
exit();

?>
