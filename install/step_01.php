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
* $Id: step_01.php,v 1.53 2013/06/05 04:20:18 akarelia Exp $
*
*/

/**
* Install Step 1
*
* Purpose
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.53 $
* @package MySource_Matrix
* @subpackage install
*/
ini_set('memory_limit', -1);
error_reporting(E_ALL);
$SYSTEM_ROOT = '';

if ((php_sapi_name() == 'cli')) {
	if (isset($_SERVER['argv'][1])) {
		$SYSTEM_ROOT = $_SERVER['argv'][1];
	}

	$err_msg = "ERROR: You need to supply the path to the System Root as the first argument.\n";

} else {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

if (empty($SYSTEM_ROOT)) {
	$err_msg .= "Usage: php install/step_01.php <PATH_TO_MATRIX>\n";
	echo $err_msg;
	exit(1);
}

if (!is_dir($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT.'/core/include/init.inc')) {
	$err_msg = "ERROR: Path provided doesn't point to a Matrix installation's System Root. Please provide correct path and try again.\n";
	$err_msg .= "Usage: php install/step_01.php <PATH_TO_MATRIX>\n";
	echo $err_msg;
	exit(1);
}

define('SQ_SYSTEM_ROOT',  $SYSTEM_ROOT);
define('SQ_INCLUDE_PATH', SQ_SYSTEM_ROOT.'/core/include');
define('SQ_LIB_PATH',     SQ_SYSTEM_ROOT.'/core/lib');
define('SQ_DATA_PATH',    SQ_SYSTEM_ROOT.'/data');
define('SQ_FUDGE_PATH',   SQ_SYSTEM_ROOT.'/fudge');
define('SQ_PHP_CLI',      (php_sapi_name() == 'cli'));
define('SQ_LOG_PATH',     SQ_SYSTEM_ROOT.'/data/private/logs');

require_once SQ_INCLUDE_PATH.'/mysource_object.inc';
require_once SQ_INCLUDE_PATH.'/system_config.inc';


$cfg = new System_Config();
$cfg->save(Array(), FALSE);

// Copy the DB config sample to the data directory
// Only overwrite the file if there is no file exists already
if (!file_exists(SQ_DATA_PATH.'/private/conf/db.inc')) {
	copy(dirname(__FILE__).'/db-inc.sample', SQ_DATA_PATH.'/private/conf/db.inc');
}

// Do the same with the memcache config file
if (!file_exists(SQ_DATA_PATH.'/private/conf/memcache.inc')) {
	copy(dirname(__FILE__).'/memcache-inc.sample', SQ_DATA_PATH.'/private/conf/memcache.inc');
}

// Do the same with the Redis config file
if (!file_exists(SQ_DATA_PATH.'/private/conf/redis.inc')) {
	copy(dirname(__FILE__).'/redis-inc.sample', SQ_DATA_PATH.'/private/conf/redis.inc');
}

echo "\n";
echo "Step 1 completed successfully.\n";
echo "\n";

// reminder for chmod
echo "Remember to give your system's Apache user write access to\n";
echo "the cache and data directories of your Matrix install.\n";

echo "\n";
echo "Step 1 completed successfully.\n";
echo "\n";

?>
