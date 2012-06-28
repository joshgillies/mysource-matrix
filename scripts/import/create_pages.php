<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: create_pages.php,v 1.7 2012/06/28 02:03:20 akarelia Exp $
*
*/

/**
* Creates page standard assets based on a CSV file provided
* The CSV file format is:
* asset_name, type_code, parent_assetid, link_type
*
*
* @author  Avi Miller <avim@netspace.net.au>
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.7 $
* @package MySource_Matrix
*/
error_reporting(E_ALL);
if (ini_get('memory_limit') != '-1') ini_set('memory_limit', '-1');

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

$import_file = (isset($_SERVER['argv'][2])) ? $_SERVER['argv'][2] : '';
if (empty($import_file) || !is_file($import_file)) {
	echo "You need to supply the path to the import file as the second argument\n";
	exit();
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';

// get the import file
require_once SQ_FUDGE_PATH.'/general/file_system.inc';

$pages = file($import_file);
$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
$csv_fd = fopen($import_file, 'r');
$line_number = 1;
while (($data = fgetcsv($csv_fd, 1024, ',')) !== FALSE) {

	if (count($data) != 4) {
		echo "Wrong number of arguments passed on line #$line_number : ".implode(', ', $data)."\n";
		$line_number++;
		continue;
	}

	$GLOBALS['SQ_SYSTEM']->am->includeAsset(trim($data[1]));

	$parent_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset(trim($data[2]), '', TRUE);
	if (is_null($parent_asset)) {
		echo "New parent asset #{$data[2]} does not exist on line #$line_number\n";
		$line_number++;
		continue;
	}

	$import_link = Array('asset' => &$parent_asset, 'link_type' => $data[3]);

	$new_asset_type = trim($data[1]);

	$new_page = new $new_asset_type();
	$new_page->setAttrValue('name', trim($data[0]));

	if (!$new_page->create($import_link)) {
		echo 'Failed to import '.$new_asset_type.' '.trim($data[0]);
		$line_number++;
		continue;
	} else {
		bam('New '.$new_page->type().' asset created for '.trim($data[0]).' - asset ID #'.$new_page->id);
	}
	$line_number++;
}

$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

?>
