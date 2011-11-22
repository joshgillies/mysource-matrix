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
* $Id: upgrade_form_submission_linking.php,v 1.1 2011/11/22 06:13:29 ewang Exp $
*
*/

/**
*
* @author Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix
*/
error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	trigger_error("You need to supply the path to the System Root as the first argument\n", E_USER_ERROR);
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';

// ask for the root password for the system
echo 'Enter the root password for "'.SQ_CONF_SYSTEM_NAME.'": ';
$root_password = rtrim(fgets(STDIN, 4094));

// check that the correct root password was entered
$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
if (!$root_user->comparePassword($root_password)) {
	trigger_error("The root password entered was incorrect\n", E_USER_ERROR);
}

// log in as root
if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	trigger_error("Failed logging in as root user\n", E_USER_ERROR);
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
