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
* $Id: session_file_garbage_collection.php,v 1.1.2.1 2006/07/06 06:32:28 tbarrett Exp $
*
*/

/**
* Perform garbage collection on the PHP session files
*
* This is necessary because Matrix uses ini_set to control the session file location and so
* PHP's internal garbage collection mechanism doesn't run.
*
* @author  Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.1.2.1 $
* @package MySource_Matrix
*/

error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}
$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	echo "ERROR: You need to supply the path to the System Root as the first argument\n";
	exit();
}
include_once $SYSTEM_ROOT.'/data/private/conf/main.inc';
chdir($SYSTEM_ROOT.'/cache');
if (exec('find . -name \'sess_????????????????????????????????\'')) {
	system('find . -name \'sess_????????????????????????????????\' -mmin +'.(SQ_CONF_SESSION_GC_MAXLIFETIME/60).' | xargs rm');
}
?>