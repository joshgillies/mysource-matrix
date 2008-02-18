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
* $Id: run_cron_job.php,v 1.4 2008/02/18 05:28:41 lwright Exp $
*
*/

/**
* Runs a single cron job once...deliberatley does not do any testing for whether it SHOULD be run at this time
* Used to develop and test cron jobs
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.4 $
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
if (is_null($cron_job)) {
	trigger_error('Asset ID passed (#'.$ASSETID.') does not point to a valid asset', E_USER_ERROR);
}
if (!is_a($cron_job, 'cron_job')) {
	trigger_error('Asset ID passed (#'.$ASSETID.') does not point to a Cron Job asset', E_USER_ERROR);
}

$result = $cron_job->run();

pre_echo("Result: ".implode(', ',get_bit_names('SQ_CRON_JOB_', $result, true)));

?>
