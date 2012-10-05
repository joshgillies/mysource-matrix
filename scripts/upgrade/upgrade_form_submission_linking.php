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
* $Id: upgrade_form_submission_linking.php,v 1.4 2012/10/05 07:20:39 akarelia Exp $
*
*/

/**
*
* @author Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix
*/
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

$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

$am = $GLOBALS['SQ_SYSTEM']->am;

$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);

$ecommerce_formids = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('form_ecommerce', FALSE);

$formids = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('form', FALSE);
foreach ($formids as $formid) {
	$form = $am->getAsset($formid);

	if (in_array($formid, $ecommerce_formids)) continue;

	pre_echo('Moving submissions for '.$form->name.' ('.$form->id.') to submissions folder');
	$submissions_folder = $form->getSubmissionsFolder();
	if (is_null($submissions_folder)) {
		trigger_error('Submissions folder not found for form #'.$formid.'; upgrade may be required', E_USER_ERROR);
	}
	$submission_links = $am->getLinks($formid, SQ_SC_LINK_ALL, 'form_submission', FALSE);
	foreach ($submission_links as $link) {
		$am->moveLink($link['linkid'], $submissions_folder->id, $link['link_type'], $link['sort_order']);
	}
}

$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
