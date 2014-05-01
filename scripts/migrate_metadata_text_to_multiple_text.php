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
* $Id: migrate_metadata_text_to_multiple_text.php,v 1.5 2012/08/30 01:04:53 ewang Exp $
*
*/

/**
*
* @author Scott Kim <skim@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix
*/
if ((php_sapi_name() != 'cli')) {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

if (defined('E_STRICT') && (E_ALL & E_STRICT)) {
	error_reporting(E_ALL ^ E_DEPRECATED ^ E_STRICT);
} else {
	if (defined('E_DEPRECATED')) {
		error_reporting(E_ALL ^ E_DEPRECATED);
	} else {
		error_reporting(E_ALL);
	}
}

require_once 'Console/Getopt.php';

$shortopt = 's:';
$longopt = Array('from=', 'to=', 'delimiter=');

$con = new Console_Getopt();
$args = $con->readPHPArgv();
array_shift($args);
$options = $con->getopt($args, $shortopt, $longopt);
if (empty($options[0])) usage();

$REPORT_ONLY = FALSE;
$FROM_ASSETID = FALSE;
$TO_ASSETID = FALSE;
$DELIMITER = ' ';
$SYSTEM_ROOT = '';
foreach ($options[0] as $option) {
	switch ($option[0]) {
		case 's':
			if (empty($option[1])) usage();
			if (!is_dir($option[1])) usage();
			$SYSTEM_ROOT = $option[1];
		break;
		case '--from':
			$FROM_ASSETID = $option[1];
		break;
		case '--to':
			$TO_ASSETID = $option[1];
		break;
		case '--delimiter':
			$DELIMITER = $option[1];
		break;
	}

}
if (empty($SYSTEM_ROOT)) {
	echo "ERROR: You need to supply the path to the System Root as the first argument\n";
	usage();
}

if (!is_dir($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT.'/core/include/init.inc')) {
	echo "ERROR: Path provided doesn't point to a Matrix installation's System Root. Please provide correct path and try again.\n";
	usage();
}

if (empty($FROM_ASSETID)) usage();
if (empty($TO_ASSETID)) usage();


if (ini_get('memory_limit') != '-1') ini_set('memory_limit', '-1');
define('SQ_SYSTEM_ROOT', $SYSTEM_ROOT);
include_once $SYSTEM_ROOT.'/core/include/init.inc';

$from_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($FROM_ASSETID);
if (empty($from_asset) || !is_a($from_asset, 'metadata_field_text')) {
	echo 'ERROR: Asset #"'.$FROM_ASSETID.'" is not of type Metadata Text Field'."\n";
	exit(1);
}

$to_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($TO_ASSETID);
if (empty($to_asset) || !is_a($to_asset, 'metadata_field_multiple_text')) {
	echo 'ERROR: Asset #"'.$TO_ASSETID.'" is not of type Metadata Multiple Text Field'."\n";
	exit(1);
}
$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();

// Find which assets contain both fields
$from_schema_link = $GLOBALS['SQ_SYSTEM']->am->getParents($from_asset->id, 'metadata_schema');
$from_schema_applied = Array();

foreach ($from_schema_link as $from_schema_assetid => $schema_type) {
	$from_schema_applied = array_merge($from_schema_applied, $mm->getSchemaAssets($from_schema_assetid, TRUE));
}

$to_schema_link = $GLOBALS['SQ_SYSTEM']->am->getParents($to_asset->id, 'metadata_schema');
$to_schema_applied = Array();

foreach ($to_schema_link as $to_schema_assetid => $schema_type) {
	$to_schema_applied = array_merge($to_schema_applied, $mm->getSchemaAssets($to_schema_assetid, TRUE));
}

$from_names = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($from_schema_applied, 'asset', FALSE, 'name');

foreach ($from_schema_applied as $assetid) {
	if (array_search($assetid, $to_schema_applied) === FALSE) {
		continue;
	}

	$asset_metadata = $mm->getMetadata($assetid);

	$field_value = $mm->getMetadataFieldValues($assetid);
	$field_value = $field_value[$from_asset->name];

	if (!empty($field_value)) {
		$field_list = explode($DELIMITER, $field_value);
		$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
		$asset_metadata[$to_asset->id]['value'] = implode('; ', $field_list);
		echo str_pad('#'.$assetid.' '.$from_names[$assetid], 60);
		if ($mm->setMetadata($assetid, $asset_metadata)) {
			echo ' [  OK  ]'."\n";
		} else {
			echo ' [FAILED]'."\n";
		}
		$mm->regenerateMetadata($assetid);
		$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

	}

}

exit(0);

function usage()
{
	echo 'Usage: '.basename($_SERVER['argv'][0]).' -s <system root> --from=<from asset ID> --to=<to asset ID> --delimiter=<delimiter>'."\n";
	echo "\n";
	echo '   -s          : the Matrix system root path'."\n";
	echo '   --from      : the asset ID of the Metadata Text Field to migrate from'."\n";
	echo '   --to        : the asset ID of the Metadata Multiple Text Field to migrate to'."\n";
	echo '   --delimiter : delimiter between items (default is a single space) - multiple'."\n";
	echo '                 characters are allowed'."\n";
	echo "\n";
	echo 'NOTES:'."\n";
	echo '* If your delimiter contains a space, you may need to place your delimiter'."\n";
	echo '  string in quotation marks - eg. --delimiter=", "'."\n";
	echo '* It is recommended to use the fully qualified path to your Matrix root to'."\n";
	echo '  avoid warnings relating to file versioning in some assets (ie. not ".").'."\n";

	exit(1);
}//end usage();
?>
