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
* $Id: system_parse_design.php,v 1.13 2012/08/30 01:04:53 ewang Exp $
*
*/

/**
* Reparses a specified design
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.13 $
* @package MySource_Matrix
*/
if (ini_get('memory_limit') != '-1') ini_set('memory_limit', '-1');
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

// log in as root
if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	echo "ERROR: Failed logging in as root user\n";
	exit();
}


$DESIGNID = (isset($_SERVER['argv'][2])) ? $_SERVER['argv'][2] : '';
if (empty($DESIGNID)) {
	echo "ERROR: You need to supply the assetid for the design to reparse as the second argument \n";
	exit();
}

$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
$fv = $GLOBALS['SQ_SYSTEM']->getFileVersioning();
$design = $GLOBALS['SQ_SYSTEM']->am->getAsset($DESIGNID);
if (is_null($design)) exit();
if (!($design instanceof Design)) {
	trigger_error('Asset #'.$design->id.' is not a design', E_USER_ERROR);
}

printName('Acquiring Locks for design "'.$design->name.'"');

// try to lock the design
$vars = Array('assetids' => Array($design->id), 'lock_type' => 'all', 'forceably_acquire' => false);
$errors = $hh->freestyleHipo('hipo_job_acquire_locks', $vars);
if (!empty($errors)) {
	printUpdateStatus('LOCK FAILED');
	$GLOBALS['SQ_SYSTEM']->am->forgetAsset($design);
	exit();
}

printUpdateStatus('OK');

printName('Checking Parse files "'.$design->name.'"');

$parse_file  = $design->data_path.'/parse.txt';
// add a new version to the repository
$file_status = $fv->upToDate($parse_file);
if (FUDGE_FV_MODIFIED & $file_status) {
	if (!$fv->commit($parse_file, '')) {
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
	$customisation = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
	if (is_null($customisation)) continue;
	printName('Reparse design customisation "'.$customisation->name.'"');
	$vars = Array('assetids' => Array($customisation->id), 'lock_type' => 'all', 'forceably_acquire' => false);
	$errors = $hh->freestyleHipo('hipo_job_acquire_locks', $vars);
	if (!empty($errors)) {
		printUpdateStatus('LOCK FAILED');
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($design);
		exit();
	}
	if ($acquired = $GLOBALS['SQ_SYSTEM']->am->acquireLock($customisation->id, 'all')) {
		if (!$customisation->updateFromParent($design)) {
			printUpdateStatus('FAILED');
			continue;
		}
		if ($acquired != 2) $GLOBALS['SQ_SYSTEM']->am->releaseLock($customisation->id, 'all');
		printUpdateStatus('OK');
	} else {
		printUpdateStatus('LOCK FAILED');
		continue;
	}
	$GLOBALS['SQ_SYSTEM']->am->forgetAsset($customisation);

}// end foreach


// try to unlock the design
if (!$GLOBALS['SQ_SYSTEM']->am->releaseLock($design->id, 'all')) {
	printUpdateStatus('RELEASE LOCK FAILED');
	$GLOBALS['SQ_SYSTEM']->am->forgetAsset($design);
}
$GLOBALS['SQ_SYSTEM']->am->forgetAsset($design);


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

