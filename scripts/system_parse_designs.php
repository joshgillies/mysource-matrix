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
* $Id: system_parse_designs.php,v 1.2 2006/12/06 05:39:51 bcaldwell Exp $
*
*/

/**
* Upgrade menu design areas
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix
*/
ini_set('memory_limit', '-1');
error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) trigger_error("You can only run this script from the command line\n", E_USER_ERROR);

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	echo "ERROR: You need to supply the path to the System Root as the first argument\n";
	exit();
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';

// check that the correct root password was entered
$root_user = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');

// log in as root
if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	trigger_error("Failed logging in as root user\n", E_USER_ERROR);
}


$fv = &$GLOBALS['SQ_SYSTEM']->getFileVersioning();
$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_OPEN);

$designs = &$GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('design', true);
bam($designs);

foreach ($designs as $designid) {

	$design = &$GLOBALS['SQ_SYSTEM']->am->getAsset($designid);
	if (is_null($design)) exit();
	if (!is_a($design, 'design')) {
		trigger_error('Asset #'.$design->id.' is not a design', E_USER_ERROR);
	}

	printName('Checking Parse files "'.$design->name.'"');

	$parse_file  = $design->data_path.'/parse.txt';
	// add a new version to the repository
	$file_status = $fv->upToDate($parse_file);
	if (FUDGE_FV_MODIFIED & $file_status) {
		if (!$fv->commit($parse_file, '', false)) {
			trigger_error('Failed committing file version', E_USER_ERROR);
			printUpdateStatus('UNABLE TO COMMIT PARSE FILE');
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($design);
			exit();
		}
	}

	printUpdateStatus('OK');

	printName('Reparse design "'.$design->name.'"');

	$edit_fns = $design->getEditFns();
	if (!$edit_fns->parseAndProcessFile($design)) {
		printUpdateStatus('FAILED');
		exit();
	}
	$design->generateDesignFile(false);

	printUpdateStatus('OK');


	$customisation_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($design->id, SQ_LINK_TYPE_2, 'design_customisation', true, 'major', 'customisation');
	foreach($customisation_links as $link) {
		$customisation = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
		if (is_null($customisation)) continue;
		printName('Reparse design customisation "'.$customisation->name.'"');

			if (!$customisation->updateFromParent($design)) {
				printUpdateStatus('FAILED');
				continue;
			} else {
				printUpdateStatus('OK');
			}
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($customisation);

	}// end foreach

	$GLOBALS['SQ_SYSTEM']->am->forgetAsset($design);

}//end foreach

$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

  ////////////////////////
 //  HELPER FUNCTIONS  //
////////////////////////
function printName($name)
{
	printf ('%s%'.(50 - strlen($name)).'s', $name, '');

}//end printName()


function printUpdateStatus($status)
{
	echo "[ $status ]\n";

}//end printUpdateStatus()

?>
