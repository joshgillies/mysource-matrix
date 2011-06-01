<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: index.php,v 1.32 2007/12/13 00:23:25 hnguyen Exp $
*
*/

/**
* Index File
*
* The one file through which everything runs
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.32 $
* @package MySource_Matrix
*/
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
