<?php
error_reporting(E_ALL); 
if ((php_sapi_name() != 'cli')) {
        trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

function print_usage() {
	echo $_SERVER['argv'][0]." - Import a CSV File to a csv data source";
	echo "Usage:\n";
	echo "\n";
	echo $_SERVER['argv'][0]." <SYSTEM_ROOT> <ASSETID> <CSV_PATH> <FORCE_UPDATE_TIME>\n";
	echo "\n";
	echo "SYSTEM_ROOT: Path to the Squiz Matrix system.\n";
	echo "ASSETID: Assetid of a CSV Data Source asset.\n";
	echo "CSV_PATH: Path to the CSV file to import\n";
	echo "FORCE_UPDATE_TIME: Forcibly refresh the asset's updated time, otherwise asset will not refresh updated time for setting same content.(y/n)\n";
}


  
$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT)) {
	echo "ERROR: You need to supply the path to the System Root as the first argument\n";
	print_usage();
	exit();
}

if (!is_dir($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT.'/core/include/init.inc')) {
	echo "ERROR: Path provided doesn't point to a Matrix installation's System Root. Please provide correct path and try again.\n";
	print_usage();
	exit();
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';

$GLOBALS['SQ_SYSTEM']->setCurrentUser($GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user'));

$assetid = (isset($_SERVER['argv'][2])) ? $_SERVER['argv'][2] : 0;
$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, 'data_source_csv');
if (empty($asset)) {
  echo "You need to supply a CSV Data Source assetid as the second argument\n";
  print_usage();
  exit();
}

if ($_SERVER['argc'] < 4) {
  echo "You need to supply the path to CSV file as the third argument\n";
  print_usage();
  exit();
}
$filepath = $_SERVER['argv'][3];
require_once SQ_FUDGE_PATH.'/csv/csv.inc';

$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);  

$csv = new CSV($filepath);
$csv->import();

$asset->setAttrValue('cached_content', $csv->values);

// overwrite the cache with the new content
$asset->setResultSet(Array(), $asset->name);
$asset->getResultSet($asset->name);

$asset->saveAttributes();

// set last update time if forcibly required
$FORCE_UPDATE = (isset($_SERVER['argv'][4])) ? $_SERVER['argv'][4] : 'n';
if(strtolower($FORCE_UPDATE) == 'y') {
	$asset->_updated();
} 

$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
?>

