<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: step_01.php,v 1.21 2003/10/21 04:07:59 brobertson Exp $
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
*/
ini_set('memory_limit', -1);
error_reporting(E_ALL);
define('SQ_SYSTEM_ROOT',   dirname(dirname(realpath(__FILE__))));
define('SQ_INCLUDE_PATH',  SQ_SYSTEM_ROOT.'/core/include');
define('SQ_LIB_PATH',      SQ_SYSTEM_ROOT.'/core/lib');
define('SQ_DATA_PATH',     SQ_SYSTEM_ROOT.'/data');
define('SQ_FUDGE_PATH',    SQ_SYSTEM_ROOT.'/fudge');
$GLOBALS['SQ_INSTALL'] = true;

require_once SQ_INCLUDE_PATH.'/mysource_object.inc';
require_once SQ_INCLUDE_PATH.'/system_config.inc';

#define('SQ_CONF_DB_DSN', 'mysql://root@localhost/dom_resolve');
define('SQ_CONF_DB_DSN', 'pgsql://dwong@unix+localhost/dom_resolve');
define('SQ_CONF_DB2_DSN', 'pgsql://dwong@unix+localhost/dom_resolve');
define('SQ_CONF_SYSTEM_ROOT_URLS', "dev.squiz.net/dom_resolvefx\nbeta.squiz.net/dom_resolvefx");
define('SQ_CONF_PEAR_PATH', SQ_SYSTEM_ROOT.'/php_includes');
define('SQ_CONF_ASSET_TREE_BASE', 36);
define('SQ_CONF_ASSET_TREE_SIZE', 4);
define('SQ_CONF_ROLLBACK_ENABLED', '0');
define('SQ_CONF_INDEXING_ENABLED', '0');
define('SQ_CONF_DEFAULT_EMAIL', 'dwong@squiz.net');
define('SQ_CONF_TECH_EMAIL',    'dwong@squiz.net');

trigger_error('Need to chmod cache and data directories', E_USER_NOTICE);

$cfg = new System_Config();
$cfg->save(Array(), true);
$GLOBALS['SQ_INSTALL'] = false;

?>
