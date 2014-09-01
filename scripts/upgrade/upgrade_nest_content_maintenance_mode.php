<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
*
*/

/**
* After #5318 maintenance mode feature, all nested content type assets will need to be regenerated to pick up the new printBody() wrapper
* printAssetBody()
*
*/

// Usage: php upgrade_nest_content_maintenance_mode.php <SYSTEM_ROOT>

if (ini_get('memory_limit') != '-1') ini_set('memory_limit', -1);
error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) trigger_error("You can only run this script from the command line\n", E_USER_ERROR);

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';

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
if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
        echo "ERROR: Failed logging in as root user\n";
        exit();
}

$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);

// Get all the bodycopy divs
$assetids = array_keys($GLOBALS['SQ_SYSTEM']->am->getChildren(1, 'content_type_nest_content'));

$count = 0;
foreach($assetids as $assetid) {

    $temp_parent_array = $GLOBALS['SQ_SYSTEM']->am->getDependantParents($assetid, 'bodycopy_div' , TRUE, TRUE);
    if(!isset($temp_parent_array[0])) continue;
    $bodycopy_div_id = $temp_parent_array[0];
    $bodycopy_div = $GLOBALS['SQ_SYSTEM']->am->getAsset($bodycopy_div_id);
    if(empty($bodycopy_div)) continue;

        // Keyword in the content file contains non-safe keywords, so regenerate the content file       
    $bodycopy_div = $GLOBALS['SQ_SYSTEM']->am->getAsset($bodycopy_div_id);
    $bodycopy_div_edit_fns = $bodycopy_div->getEditFns();
    
    $all_context = $GLOBALS['SQ_SYSTEM']->getAllContexts();
    foreach ($all_context as $context_id => $info) {
        $GLOBALS['SQ_SYSTEM']->changeContext($context_id);
        $bodycopy_div_edit_fns->generateContentFile($bodycopy_div);
        $GLOBALS['SQ_SYSTEM']->restoreContext();
    }

    $GLOBALS['SQ_SYSTEM']->am->forgetAsset($bodycopy_div);

        echo "#".$bodycopy_div_id." updated\n";
    $count++;   

    $GLOBALS['SQ_SYSTEM']->am->forgetAsset($bodycopy_div->id, TRUE);
}//end foreach


$GLOBALS['SQ_SYSTEM']->restoreRunLevel();


echo $count. " asset(s) fixed";
echo "\n";
?>
