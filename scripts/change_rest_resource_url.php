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
* This script finds all Rest Resource(JS) assets in system and replace its URLs from old URL to new URL.
*
*/

// Usage: php change_rest_resource_url.php <SYSTEM_ROOT> [OLD_URL] [NEW_URL]

if (ini_get('memory_limit') != '-1') ini_set('memory_limit', -1);
error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) trigger_error("You can only run this script from the command line\n", E_USER_ERROR);

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
$old_url = isset($_SERVER['argv'][2]) ? $_SERVER['argv'][2] : '';
$new_url = isset($_SERVER['argv'][3]) ? $_SERVER['argv'][3] : '';

if (empty($SYSTEM_ROOT)) {
    echo "ERROR: You need to supply the path to the System Root as the first argument\n";
    exit();
}


if (empty($old_url) || empty($new_url)) {
    echo "ERROR: You need to supply the old url to replace and the new url.\n";
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

// Get all the rest assets
$assetids = array_keys($GLOBALS['SQ_SYSTEM']->am->getChildren(1, 'page_rest_resource', FALSE));
$count = 0;
foreach($assetids as $assetid) {
    $rest_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
    $http_request = unserialize($rest_asset->attr('http_request'));
    $urls = isset($http_request['urls']) ? $http_request['urls'] : array();
    $need_replace = false;
    if(!empty($urls)) {
        foreach($urls as $index => $url) {
            if($url === $old_url) {
                $urls[$index] = $new_url;
                $need_replace = true;
            }
        }
    }
    if($need_replace) {
        $GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
                $http_request['urls'] = $urls;
                $rest_asset->setAttrValue('http_request', serialize($http_request));
                $rest_asset->saveAttributes(TRUE);
        $GLOBALS['SQ_SYSTEM']->restoreRunLevel();
        $count++;
    }
}//end foreach

echo count($assetids). " Rest assets were found in system. Asset IDs are :";
echo "\n";
echo implode(", ", $assetids);
echo "\n";
echo $count. " asset(s) were changed";
echo "\n";
?>
