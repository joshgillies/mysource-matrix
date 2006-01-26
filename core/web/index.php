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
* $Id: index.php,v 1.24 2006/01/26 22:34:08 lwright Exp $
*
*/

/**
* Index File
*
* The one file through which everything runs
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.24 $
* @package MySource_Matrix
*/

ini_set('memory_limit', '32M');
define('SQ_SYSTEM_ROOT', dirname(dirname(dirname($_SERVER['SCRIPT_FILENAME']))));
require_once dirname(dirname(__FILE__)).'/include/init.inc';
$GLOBALS['SQ_SYSTEM']->start();
$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

// make sure nobody has set run levels without restoring
$run_level = $GLOBALS['SQ_SYSTEM']->getRunLevel();
if (!is_null($run_level)) {
	trigger_error('A run level has been set without restoring', E_USER_ERROR);
}

?>
