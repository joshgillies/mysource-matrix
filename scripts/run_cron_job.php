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
* $Id: run_cron_job.php,v 1.6 2012/08/30 01:04:53 ewang Exp $
*
*/

/**
* Runs a single cron job once...deliberatley does not do any testing for whether it SHOULD be run at this time
* Used to develop and test cron jobs
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.6 $
* @package MySource_Matrix
*/
error_reporting(E_ALL);
if (php_sapi_name() != 'cli') trigger_error("You can only run this script from the command line\n", E_USER_ERROR);

if (count($_SERVER['argv']) != 3) {
	echo "Usage:\n\n";
	echo "\tphp ".basename($_SERVER['argv'][0])." <SYSTEM_ROOT> <cron job assetid>\n\n";
	exit();
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
$ASSETID = (isset($_SERVER['argv'][2])) ? $_SERVER['argv'][2] : '';

require_once $SYSTEM_ROOT.'/core/include/init.inc';
require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

$root_user = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
if (is_null($root_user)) {
	echo "Unable to get Root User\n";
	exit();
}

if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	echo "Unable to set Root User as current user\n";
	exit();
}

$cron_job = &$GLOBALS['SQ_SYSTEM']->am->getAsset($ASSETID);
if (is_null($cron_job)) {
	echo "Asset ID passed (#'.$ASSETID.') does not point to a valid asset\n";
	exit();
}
if (!is_a($cron_job, 'cron_job')) {
	echo "Asset ID passed (#'.$ASSETID.') does not point to a Cron Job asset";
	exit();
}

$result = $cron_job->run();

pre_echo("Result: ".implode(', ',get_bit_names('SQ_CRON_JOB_', $result, true)));

?>
