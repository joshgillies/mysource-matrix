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
* $Id: step_01.php,v 1.46 2008/01/03 22:08:39 hnguyen Exp $
*
*/

/**
* Install Step 1
*
* Purpose
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.46 $
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

	$err_msg = "You need to supply the path to the System Root as the first argument\n";

} else {
	if (isset($_GET['SYSTEM_ROOT'])) {
		$SYSTEM_ROOT = $_GET['SYSTEM_ROOT'];
	}

	$err_msg = '
	<div style="background-color: red; color: white; font-weight: bold;">
		You need to supply the path to the System Root as a query string variable called SYSTEM_ROOT
	</div>
	';
}

if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	trigger_error($err_msg, E_USER_ERROR);
}

define('SQ_SYSTEM_ROOT',  $SYSTEM_ROOT);
define('SQ_INCLUDE_PATH', SQ_SYSTEM_ROOT.'/core/include');
define('SQ_LIB_PATH',     SQ_SYSTEM_ROOT.'/core/lib');
define('SQ_DATA_PATH',    SQ_SYSTEM_ROOT.'/data');
define('SQ_FUDGE_PATH',   SQ_SYSTEM_ROOT.'/fudge');
define('SQ_PHP_CLI',      (php_sapi_name() == 'cli'));

require_once SQ_INCLUDE_PATH.'/mysource_object.inc';
require_once SQ_INCLUDE_PATH.'/system_config.inc';
require_once SQ_INCLUDE_PATH.'/licence_config.inc';

// override some of the default config values
define('SQ_CONF_PEAR_PATH', SQ_SYSTEM_ROOT.'/php_includes');

$cfg =& new System_Config();
$cfg->save(Array(), TRUE);

$cfg =& new Licence_Config();
$cfg->save(Array(), TRUE);

// Copy the DB config sample to the data directory
// Only overwrite the file if there is no file exists already
if (!file_exists(SQ_DATA_PATH.'/private/conf/db.inc')) {
	copy(dirname(__FILE__).'/db-inc.sample', SQ_DATA_PATH.'/private/conf/db.inc');
}

// reminder for chmod
echo 'Remember to give your system\'s Apache user write access to'."\n";
echo 'the cache and data directories of your Matrix install...'."\n";

?>
