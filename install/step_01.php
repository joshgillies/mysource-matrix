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
* $Id: step_01.php,v 1.35 2005/03/16 04:24:46 mnyeholt Exp $
*
*/

/**
* Install Step 1
*
* Purpose
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.35 $
* @package MySource_Matrix
* @subpackage install
*/

// First off, run the version check script
require_once dirname(__FILE__).'/versioncheck.php';

ini_set('memory_limit', -1);
error_reporting(E_ALL);
define('SQ_SYSTEM_ROOT',   dirname(dirname(realpath(__FILE__))));
define('SQ_INCLUDE_PATH',  SQ_SYSTEM_ROOT.'/core/include');
define('SQ_LIB_PATH',      SQ_SYSTEM_ROOT.'/core/lib');
define('SQ_DATA_PATH',     SQ_SYSTEM_ROOT.'/data');
define('SQ_FUDGE_PATH',    SQ_SYSTEM_ROOT.'/fudge');
define('SQ_PHP_CLI', (php_sapi_name() == 'cli'));
$GLOBALS['SQ_INSTALL'] = true;

require_once SQ_INCLUDE_PATH.'/mysource_object.inc';
require_once SQ_INCLUDE_PATH.'/system_config.inc';

echo "\n";
echo "\n";
echo "Please enter a value for SQ_CONF_DB_DSN [] : ";
$dsn1 = read_line();
echo "Please enter a value for SQ_CONF_DB2_DSN [] : ";
$dsn2 = read_line();
echo "Please enter a value for SQ_CONF_SYSTEM_ROOT_URLS [www.example.com] : ";
$root_url = read_line('www.example.com');
echo "Please enter a value for SQ_CONF_DEFAULT_EMAIL [] : ";
$mail1 = read_line();
echo "Please enter a value for SQ_CONF_TECH_EMAIL [] : ";
$mail2 = read_line();

$dsn1 = 
#define('SQ_CONF_DB_DSN',  'pgsql://username:password@unix()/database_name');
#define('SQ_CONF_DB2_DSN', 'pgsql://username_2:password@unix()/database_name');
define('SQ_CONF_DB_DSN', $dsn1);
define('SQ_CONF_DB2_DSN', $dsn2);
define('SQ_CONF_SYSTEM_ROOT_URLS', $root_url);
define('SQ_CONF_PEAR_PATH', SQ_SYSTEM_ROOT.'/php_includes');
define('SQ_CONF_ASSET_TREE_BASE', 36);
define('SQ_CONF_ASSET_TREE_SIZE', 4);
define('SQ_CONF_ROLLBACK_ENABLED', '0');
define('SQ_CONF_DEFAULT_EMAIL', $mail1);
define('SQ_CONF_TECH_EMAIL',    $mail2);

$cfg = new System_Config();
$cfg->save(Array(), true);
$GLOBALS['SQ_INSTALL'] = false;

// reminder for chmod
echo 'Remember to give your system\'s Apache user write access to'."\n";
echo 'the cache and data directories of your Matrix install...'."\n";

/**
* Reads a line from stdin
*
* @return string		The read in string
*/
function read_line($default='')
{
	$input = fgets(STDIN);
	$input = trim($input);
	if (!strlen($input)) $input = $default;
	return $input;
}// end read_line()

?>
