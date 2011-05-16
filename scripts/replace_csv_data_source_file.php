<?php
error_reporting(E_ALL); 
if ((php_sapi_name() != 'cli')) {
        trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}               
                        
  function print_usage() {
    echo $_SERVER['argv'][0]." - Import a CSV File to a csv data source";
    echo "Usage:\n";
    echo "\n";
    echo $_SERVER['argv'][0]." <SYSTEM_ROOT> <ASSETID> <CSV_PATH>\n";
    echo "\n";
    echo "SYSTEM_ROOT: Path to the Squiz Matrix system.\n";
    echo "ASSETID: Assetid of a CSV Data Source asset.\n";
    echo "CSV_PATH: Path to the CSV file to import\n";
  }

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
  print_usage();
  trigger_error("You need to supply the path to the System Root as the first argument\n", E_USER_ERROR);
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';

$GLOBALS['SQ_SYSTEM']->setCurrentUser($GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user'));

$assetid = (isset($_SERVER['argv'][2])) ? $_SERVER['argv'][2] : 0;
$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, 'data_source_csv');
if (empty($asset)) {
  print_usage();
  trigger_error("You need to supply a CSV Data Source assetid as the second argument\n", E_USER_ERROR);
}

if ($_SERVER['argc'] < 4) {
  trigger_error("You need to supply the path to CSV file as the third argument\n", E_USER_ERROR);
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

$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
?>

