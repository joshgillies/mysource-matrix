<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: start_hipo_server.php,v 1.6 2003/10/22 03:26:48 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

/**
* Start HIPO Server
*
*   Example script to start the HIPO Server
*
* @author  Blair Robertson <brobertson@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/



error_reporting(E_ALL);
ini_set('memory_limit', '8M');

define('SQ_HIPO_CONF_PEAR_PATH', dirname(__FILE__).'/../php_includes');
$sep = (substr(PHP_OS, 0, 3) == 'WIN') ? ';' : ':';
$inc_dir = ini_get('include_path');
$inc_dir = (substr($inc_dir, 0, 2) == '.'.$sep) ? '.'.$sep.SQ_HIPO_CONF_PEAR_PATH.$sep.substr($inc_dir, 2): SQ_HIPO_CONF_PEAR_PATH.$sep.$inc_dir;
ini_set('include_path', $inc_dir);

ini_set('error_log', '');

require_once dirname(__FILE__).'/../core/hipo/server/hipo_server.inc';
$server	= new HIPO_Server('localhost', 9090, dirname(__FILE__).'/server.conf', dirname(__FILE__).'/server.log');
$server->start();

?>
