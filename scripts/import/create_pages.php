<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: create_pages.php,v 1.1.2.1 2004/11/04 04:03:54 mnyeholt Exp $
* $Name: not supported by cvs2svn $
*/

/**
* Creates page standard assets based on a CSV file provided
* The CSV file format is:
* asset_name, type_code, parent_assetid, link_type
*
*
* @author  Avi Miller <avim@netspace.net.au>
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/
error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) trigger_error("You can only run this script from the command line\n", E_USER_ERROR);

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	trigger_error("You need to supply the path to the System Root as the first argument\n", E_USER_ERROR);
}

$import_file = (isset($_SERVER['argv'][2])) ? $_SERVER['argv'][2] : '';
if (empty($import_file) || !is_file($import_file)) {
	trigger_error("You need to supply the path to the import file as the second argument\n", E_USER_ERROR);
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';

// get the import file
require_once SQ_FUDGE_PATH.'/general/file_system.inc';
$pages = file($import_file);
$GLOBALS['SQ_INSTALL'] = true;
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

unset($GLOBALS['SQ_INSTALL']);

?>
