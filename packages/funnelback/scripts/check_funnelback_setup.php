<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +--------------------------------------------------------------------+
*
* $Id: check_funnelback_setup.php,v 1.1 2010/03/01 04:47:37 bpearson Exp $
*
*/

/**
* Check the setup of a funnelback system
*
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage funnelback
*/

if (php_sapi_name() != 'cli') {
	trigger_error('This script can only be run from the command line', E_USER_ERROR);
	exit();
}//end if

$SYSTEM_ROOT = ((isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '');
if (empty($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT)) {
	trigger_error('You need to specify a path to Matrix as the first parameter', E_USER_ERROR);
	exit();
}//end if

define('SQ_SYSTEM_ROOT', $SYSTEM_ROOT);
require_once SQ_SYSTEM_ROOT.'/core/include/init.inc';

// Check the script is being run from same user as apache
$apache_user  = fileowner(SQ_SYSTEM_ROOT.'/data/private/conf/main.inc');
$current_user = array_get_index($_ENV, 'USER', '');

if ($apache_user != $current_user) {
//	trigger_error('This script needs to run as the apache user', E_USER_ERROR);
//	exit();
}//end if

$funnelback_manager = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('funnelback_manager');

// Check the base install for paths and permissions
$valid = $funnelback_manager->checkInstalled();

if ($valid) {
	echo "Looks ok from here\n";
} else {
	echo "Funnelback has not been installed correctly\n";
}//end if

exit();
?>
