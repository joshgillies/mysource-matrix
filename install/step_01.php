<?php
/**
* Install Step 1
*
* Purpose
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve
*/

define('SQ_SYSTEM_ROOT',   dirname(dirname(realpath(__FILE__))));
define('SQ_INCLUDE_PATH',  SQ_SYSTEM_ROOT.'/core/include');
define('SQ_LIB_PATH',      SQ_SYSTEM_ROOT.'/core/lib');
define('SQ_DATA_PATH',     SQ_SYSTEM_ROOT.'/data');
define('SQ_FUDGE_PATH',    SQ_SYSTEM_ROOT.'/fudge');

require_once SQ_INCLUDE_PATH.'/resolve_object.inc';
require_once SQ_INCLUDE_PATH.'/config.inc';

#define('SQ_CONF_DB_DSN', 'mysql://root@localhost/blair_resolve');
define('SQ_CONF_DB_DSN', 'pgsql://brobertson@unix+localhost/blair_resolve');
define('SQ_CONF_SYSTEM_ROOT_URLS', 'http://beta.squiz.net/blair');
define('SQ_CONF_PEAR_PATH', SQ_SYSTEM_ROOT.'/php_includes');
define('SQ_CONF_ASSET_TREE_BASE', 64);
define('SQ_CONF_ASSET_TREE_SIZE', 4);


// Need to chmod cache and data directories

$cfg = new Config();
$cfg->save(Array(), true);

?>
