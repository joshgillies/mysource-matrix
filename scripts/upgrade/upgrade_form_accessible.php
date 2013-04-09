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
* $Id: upgrade_form_accessible.php,v 1.1.2.2 2013/04/09 07:56:54 ewang Exp $
*
*/

/**
*
* @version $Revision: 1.1.2.2 $
* @package MySource_Matrix
*/
if (ini_get('memory_limit') != '-1') ini_set('memory_limit', '-1');
error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

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


$am = $GLOBALS['SQ_SYSTEM']->am;

$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);

$formids = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('form_ecommerce', FALSE);
$formids += $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('form', FALSE);
$count = 0;
foreach (array_unique($formids) as $formid) {
	$form = $am->getAsset($formid);
	if (is_null($form)) continue;

	if (!$form->setAttrValue('use_accessible_format',   FALSE)) {
		echo 'Asset #', $form->id, ' (', $form->name,') Unable to disable accessible format', "\n";
		continue;
	}


	if (!$form->saveAttributes()) {
		echo 'Asset #', $form->id, ' (', $form->name,') Unable to save attributes', "\n";
		continue;
	}
	
	$GLOBALS['SQ_SYSTEM']->am->forgetAsset($form, TRUE);
	unset($form);
	$count++;
	
}

echo $count, ' forms is processed', "\n";

$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

