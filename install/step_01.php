<?php

define('SQ_SYSTEM_ROOT',   dirname(dirname(realpath(__FILE__))));
define('SQ_INCLUDE_PATH',  SQ_SYSTEM_ROOT.'/core/include');
define('SQ_LIB_PATH',      SQ_SYSTEM_ROOT.'/core/lib');
define('SQ_DATA_PATH',     SQ_SYSTEM_ROOT.'/data');

require_once SQ_INCLUDE_PATH.'/resolve_object.inc';
require_once SQ_INCLUDE_PATH.'/config.inc';
require_once SQ_INCLUDE_PATH.'/resolve_object.inc';

define('SQ_CONF_DB_DSN',    'mysql://root@localhost/blair_resolve');
#define('SQ_CONF_PEAR_PATH', '/usr/local/pear/share/pear');

$cfg = new Config();
$cfg->save(Array(), true);

?>