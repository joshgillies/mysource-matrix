<?php

error_reporting(E_ALL);

define('SQ_HIPO_CONF_PEAR_PATH', '/home/brobertson/resolve/php_includes');
$sep = (substr(PHP_OS, 0, 3) == 'WIN') ? ';' : ':';
$inc_dir = ini_get('include_path');
$inc_dir = (substr($inc_dir, 0, 2) == '.'.$sep) ? '.'.$sep.SQ_HIPO_CONF_PEAR_PATH.$sep.substr($inc_dir, 2): SQ_HIPO_CONF_PEAR_PATH.$sep.$inc_dir;
ini_set('include_path', $inc_dir);

ini_set('error_log', '');

require_once '/home/brobertson/resolve/core/hipo/server/hipo_server.inc';
$server	= new HIPO_Server('localhost', 9090, dirname(__FILE__).'/server.conf');
$server->start();

?>
