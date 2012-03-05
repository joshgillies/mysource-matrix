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
* $Id: system_parse_designs.php,v 1.7.4.1 2012/03/05 00:33:34 akarelia Exp $
*
*/

/**
* Reparses all of the designs in the system
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.7.4.1 $
* @package MySource_Matrix
*/
if (ini_get('memory_limit') != '-1') ini_set('memory_limit', '-1');
error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) trigger_error("You can only run this script from the command line\n", E_USER_ERROR);

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	echo "ERROR: You need to supply the path to the System Root as the first argument\n";
	exit();
}

define('SQ_SYSTEM_ROOT', realpath($SYSTEM_ROOT));
require_once $SYSTEM_ROOT.'/core/include/init.inc';

$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');

// log in as root
if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	trigger_error("Failed logging in as root user\n", E_USER_ERROR);
}


$fv = $GLOBALS['SQ_SYSTEM']->getFileVersioning();
$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_OPEN);

$parse_asset_types = Array(
						'design'		=> TRUE,
						'design_css'	=> TRUE,
					 );
$designs = Array();
foreach ($parse_asset_types as $type_code => $strict) {
	$designs = array_merge($designs, $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids($type_code, $strict));
}

foreach ($designs as $designid) {

	$design = $GLOBALS['SQ_SYSTEM']->am->getAsset($designid);
	if (is_null($design)) exit();
	if (!($design instanceof Design)) {
		trigger_error('Asset #'.$design->id.' is not a design', E_USER_ERROR);
	}

	printName('Checking Parse files "'.$design->name.'" (#'.$design->id.')');

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

	printName('Reparse '.$design->type().' "'.$design->name.'" (#'.$design->id.')');

	$edit_fns = $design->getEditFns();
	if (!$edit_fns->parseAndProcessFile($design)) {
		printUpdateStatus('FAILED');
		exit();
	}
	$design->generateDesignFile(false);

	printUpdateStatus('OK');


	$customisation_links = $GLOBALS['SQ_SYSTEM']->am->getChildren($design->id, 'design_customisation', TRUE, NULL, NULL, NULL, TRUE, NULL, NULL, TRUE, 'customisation', Array(SQ_LINK_TYPE_2));

	foreach($customisation_links as $asset_id => $info) {
		$customisation = $GLOBALS['SQ_SYSTEM']->am->getAsset($asset_id, $info[0]['type_code']);
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
