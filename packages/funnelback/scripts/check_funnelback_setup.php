<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: check_funnelback_setup.php,v 1.3 2012/08/30 00:58:43 ewang Exp $
*
*/

/**
* Check the setup of a funnelback system
*
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.3 $
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
