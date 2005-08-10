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
* $Id: run_cron_job.php,v 1.1.2.1 2005/08/10 16:05:11 brobertson Exp $
*
*/

/**
* Runs a single cron job once...deliberatley does not do any testing for whether it SHOULD be run at this time
* Used to develop and test cron jobs
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.1.2.1 $
* @package MySource_Matrix
*/
error_reporting(E_ALL);
if (php_sapi_name() != 'cli') trigger_error("You can only run this script from the command line\n", E_USER_ERROR);

if (count($_SERVER['argv']) != 3) {
	echo "Usage:\n\n";
	echo "\tphp ".basename($_SERVER['argv'][0])." <SYSTEM_ROOT> <cron job assetid>\n\n";
	die();
}

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	trigger_error('The directory you specified as the system root does not exist, or is not a directory', E_USER_ERROR);
}

$ASSETID = (isset($_SERVER['argv'][2])) ? $_SERVER['argv'][2] : '';

require_once $SYSTEM_ROOT.'/core/include/init.inc';
require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

$root_user = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
if (is_null($root_user)) {
	trigger_error('Unable to get Root User', E_USER_ERROR);
}

if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	trigger_error('Unable to set Root User as current user', E_USER_ERROR);
}

$cron_job = &$GLOBALS['SQ_SYSTEM']->am->getAsset($ASSETID);
if (is_null($cron_job)) die('Not a valid assetid');
if (!is_a($cron_job, 'cron_job')) die('Not a cron job');

$result = $cron_job->run();

pre_echo("Result: ".implode(', ',get_bit_names('SQ_CRON_JOB_', $result, true)));

?>
