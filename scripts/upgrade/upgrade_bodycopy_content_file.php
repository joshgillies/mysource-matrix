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
* $Id: upgrade_bodycopy_content_file.php,v 1.5 2013/02/12 05:20:33 ewang Exp $
*
*/

/**
* After #5802 bug fix, the keywords in the keyword replacement array 
* in the bodycopy contnet file are htmlencoded to take care of the "unsafe" chars
*
* This script makes sure that the older bodycopy content files are in the line with this change.
*
* @author Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: 1.5 $
* @package MySource_Matrix
*/

// Usage: php upgrade_bodycopy_content_file.php <SYSTEM_ROOT> [REPORT_ONLY]
// [REPORT_ONLY] = {y, n}

if (ini_get('memory_limit') != '-1') ini_set('memory_limit', -1);
error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) trigger_error("You can only run this script from the command line\n", E_USER_ERROR);

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
$report_only = (isset($_SERVER['argv'][2]) && $_SERVER['argv'][2]!='y') ? FALSE : TRUE;

if (empty($SYSTEM_ROOT)) {
	echo "ERROR: You need to supply the path to the System Root as the first argument\n";
	exit();
}

if (!is_dir($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT.'/core/include/init.inc')) {
	echo "ERROR: Path provided doesn't point to a Matrix installation's System Root. Please provide correct path and try again.\n";
	exit();
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';

if ($report_only) {
	echo "Following Bodycopy Div asset(s) contains unsafe keywords in the content file:\n\n";
} else {
	echo "Fixing the unsafe keyword(s) in content file for following Bodycopy Div asset(s):\n\n";
}

$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
        echo "ERROR: Failed logging in as root user\n";
        exit();
}

// Get all the bodycopy divs
$assetids = array_keys($GLOBALS['SQ_SYSTEM']->am->getChildren(1, 'bodycopy_div'));
$count = 0;
foreach($assetids as $assetid) {
	
	$data_dir = $SYSTEM_ROOT.'/data/private/'.asset_data_path_suffix('bodycopy_div', $assetid).'/content_file.php';
	if (!is_file($data_dir)) {		
		continue;
	}
	
	$file_content = file_get_contents($data_dir);
	// Extract the keyword replacements in the content file
	preg_match_all('|echo \(isset\(\$keyword_replacements\["(.*?)\]\)\) \?|mis', $file_content, $matches);
	if (empty($matches[1])) {
		// No keywords in the content
		continue;
	}
	
	$keywords_str = '';
	foreach($matches[1] as $keyword) {
		$keywords_str .= trim($keyword, '"');
	
	}
	
	if ($keywords_str != htmlentities(html_entity_decode($keywords_str))) {	
		echo "#".$assetid."\n";
		// Keyword in the content file contains non-safe keywords, so regenerate the content file		
		if (!$report_only) {
			$bodycopy_div = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			$bodycopy_div_edit_fns = $bodycopy_div->getEditFns();
			$bodycopy_div_edit_fns->generateContentFile($bodycopy_div);
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($bodycopy_div);
		}		
		$count++;		
	}
	
}//end foreach

echo $report_only ? $count. " asset(s) requires fixing" : $count. " asset(s) were fixed";
echo "\n";
?>
