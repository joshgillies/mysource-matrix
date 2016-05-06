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
*
*/


define('SQ_SYSTEM_ROOT', dirname(dirname(dirname(__FILE__))));
require_once SQ_SYSTEM_ROOT.'/core/include/init.inc';
require_once SQ_SYSTEM_ROOT.'/core/include/locale_manager.inc';

$packages = $GLOBALS['SQ_SYSTEM']->getInstalledPackages();
$lm = new Locale_Manager();

$total = 0;
foreach ($packages as $package) {
	$package_name = $package['code_name'];

	echo str_pad($package_name, 30, '.').' ';

	$strings      = $lm->extractPoFile($package_name, 'pl');
	$string_index = array_unique($lm->extractSourcesFromPoFile($package_name, 'pl'), SORT_REGULAR);
	$new_strings  = $lm->getLocalisableStrings($package_name);
	$new_list     = Array();
	$plural_sources = Array();

	foreach ($new_strings as $new_string) {
		if (is_array($new_string) === TRUE) {
			$index = $new_string[0];
			$plural_sources[$index] = $new_string[1];
		} else {
			$index = $new_string;	
		}
		
		if (array_key_exists($index, $strings) === TRUE) {
			// Existing plural.
			$new_list[$index] = $strings[$index];
		} else if (is_array($new_string) === TRUE) {
			// New plural.
			$new_list[$index] = Array();
		} else {
			$new_list[$index] = '';
		}
	}

	if ($package_name === '__core__') {
		$target_file = dirname(__FILE__).'/core.xliff';
	} else {
		$target_file = dirname(__FILE__).'/package.'.$package_name.'.xliff';
	}

	$contents = $lm->buildXliffFile($new_list, $plural_sources, 3);
	file_put_contents($target_file, $contents);
	echo '['.str_pad(count($new_list), 6, ' ', STR_PAD_LEFT).']';
	echo "\n";

	$total += count($new_list);
}

echo 'total '.$total."\n";
?>
