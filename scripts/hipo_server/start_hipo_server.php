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
* $Id: start_hipo_server.php,v 1.2 2004/01/14 05:28:30 ramato Exp $
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

require_once dirname(__FILE__).'/../code/hipo_server.inc';
$server	= new HIPO_Server('localhost', 9090, dirname(__FILE__).'/server.conf', dirname(__FILE__).'/server.log');
$server->start();

?>
