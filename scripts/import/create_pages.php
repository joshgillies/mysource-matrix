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
* $Id: create_pages.php,v 1.6 2012/06/05 06:26:10 akarelia Exp $
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
* @version $Revision: 1.6 $
* @package MySource_Matrix
*/
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
foreach ($pages as $pageline) {

	// create an asset under the new parent of the correct type
	list($pagename, $page_type, $parent_assetid, $link_type) = explode(",", $pageline);
	$GLOBALS['SQ_SYSTEM']->am->includeAsset(trim($page_type));

	$parent_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset(trim($parent_assetid));
	if (is_null($parent_asset)) trigger_error("New parent asset #$parent_assetid does not exist\n", E_USER_ERROR);
	$import_link = Array('asset' => &$parent_asset, 'link_type' => $link_type);

	$new_asset_type = trim($page_type);

	$new_page = new $new_asset_type();
	$new_page->setAttrValue('name', trim($pagename));

	if (!$new_page->create($import_link)) {
		trigger_error('Failed to import '.$new_asset_type.' '.trim($pagename), E_USER_WARNING);
	} else {
		bam('New '.$new_page->type().' asset created for '.trim($pagename).' - asset ID #'.$new_page->id);
	}
}

$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

?>
