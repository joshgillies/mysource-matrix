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
* $Id: step_01.php,v 1.29 2004/08/26 05:58:52 mnyeholt Exp $
* $Name: not supported by cvs2svn $
*/

/**
* Install Step 1
*
* Purpose
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
* @subpackage install
*/
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

#define('SQ_CONF_DB_DSN',  'mysql://username:password@localhost/database_name');
#define('SQ_CONF_DB2_DSN', 'mysql://username_2:password@localhost/database_name');
#define('SQ_CONF_DB_DSN',  'pgsql://username:password@unix()/database_name');
#define('SQ_CONF_DB2_DSN', 'pgsql://username_2:password@unix()/database_name');
define('SQ_CONF_SYSTEM_ROOT_URLS', 'www.example.com');
define('SQ_CONF_PEAR_PATH', SQ_SYSTEM_ROOT.'/php_includes');
define('SQ_CONF_ASSET_TREE_BASE', 36);
define('SQ_CONF_ASSET_TREE_SIZE', 4);
define('SQ_CONF_ROLLBACK_ENABLED', '0');
define('SQ_CONF_DEFAULT_EMAIL', 'matrix-team@squiz.net');
define('SQ_CONF_TECH_EMAIL',    'matrix-team@squiz.net');

trigger_error('Need to chmod cache and data directories', E_USER_NOTICE);

$cfg = new System_Config();
$cfg->save(Array(), true);
$GLOBALS['SQ_INSTALL'] = false;

?>
