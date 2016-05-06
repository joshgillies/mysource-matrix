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
* This script finds all Rest Resource JS asset in system and bulk change the JS engine attribute setting either to v8 or spidermonkey
*
*/

// Usage: php upgrade_change_rest_resource_js.php <SYSTEM_ROOT> [JS_ENGINE]

if (ini_get('memory_limit') != '-1') ini_set('memory_limit', -1);
error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) trigger_error("You can only run this script from the command line\n", E_USER_ERROR);

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
$js_engine = isset($_SERVER['argv'][2]) ? $_SERVER['argv'][2] : '';

if (empty($SYSTEM_ROOT)) {
    echo "ERROR: You need to supply the path to the System Root as the first argument\n";
    exit();
}


if (empty($js_engine) || ($js_engine !== 'v8' && $js_engine !== 'spidermonkey')) {
    echo "ERROR: You need to specify a JS engine to change to. v8 or spidermonkey.\n";
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

// Get all the bodycopy divs
$assetids = array_keys($GLOBALS['SQ_SYSTEM']->am->getChildren(1, 'page_rest_resource_js'));
$count = 0;
foreach($assetids as $assetid) {
    $rest_js = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
    $current_engine = $rest_js->attr('js_engine');
    $GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
                $rest_js->setAttrValue('js_engine', $js_engine);
                $rest_js->saveAttributes(TRUE);
    $GLOBALS['SQ_SYSTEM']->restoreRunLevel();
    $count++;
}//end foreach

echo $count. " asset(s) were changed";
echo "\n";
?>
