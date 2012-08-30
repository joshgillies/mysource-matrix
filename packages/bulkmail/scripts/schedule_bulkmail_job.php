#!/usr/bin/php
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
* $Id: schedule_bulkmail_job.php,v 1.5 2012/08/30 00:57:13 ewang Exp $
*
*/

/**
* Add a bulkmail job to the bulkmail queue
*
* Usage: php add.php /path/to/system/root/ [assetid of job]
*		This script adds a bulkmail job to the bulkmail queue.
*		Used to allow cron systems to handle bulkmail jobs.
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix
*/

// Check for environment/arguments
error_reporting(E_ALL);
if (ini_get('memory_limit') != '-1') ini_set('memory_limit', -1);
if ((php_sapi_name() != 'cli')) {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

$system_root = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($system_root) || !is_dir($system_root)) {
    trigger_error("You need to supply the path to the System Root as the first argument\n", E_USER_ERROR);
}

$asset_id = (isset($_SERVER['argv'][2])) ? $_SERVER['argv'][2] : '';
if (empty($asset_id)) {
    trigger_error("You need to supply the asset id of the bulkmail job as the second argument\n", E_USER_ERROR);
}

require_once $system_root.'/core/include/init.inc';

$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user);

// Check for valid asset
$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($asset_id);
if (is_null($asset) || (!($asset instanceof Bulkmail_Job))) {
    trigger_error("You need to provide a valid bulkmail job to process\n", E_USER_ERROR);
}//end if

// Process
$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
$vars = Array(
			'assetid'			=> $asset_id,
			'new_status'		=> SQ_STATUS_LIVE,
			'dependants_only'	=> TRUE
		);
if ($asset->status == SQ_STATUS_UNDER_CONSTRUCTION) {
	$errors = $hh->freestyleHipo('hipo_job_edit_status', $vars);
}

// Check for errors, and show if found
if (!empty($errors)) {
	echo print_r($errors, TRUE);
}

?>
