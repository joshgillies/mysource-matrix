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
* $Id: step_01.php,v 1.39 2006/01/26 22:34:09 lwright Exp $
*
*/

/**
* Install Step 1
*
* Purpose
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.39 $
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

// override some of the default config values
define('SQ_CONF_PEAR_PATH', SQ_SYSTEM_ROOT.'/php_includes');
define('SQ_CONF_DEFAULT_EMAIL', 'matrix-team@squiz.net');
define('SQ_CONF_TECH_EMAIL',    'matrix-team@squiz.net');

$cfg =& new System_Config();
$cfg->save(Array(), TRUE);

// reminder for chmod
echo 'Remember to give your system\'s Apache user write access to'."\n";
echo 'the cache and data directories of your Matrix install...'."\n";

?>
