<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
*/

/**
* Add or Remove a url from a site/asset. This script will go and update the
* sq_ast_url, sq_ast_lookup, sq_ast_lookup_value table. It assume that the site being edited is already have
* a URL applied to it.
*
* @author  Huan Nguyen <hnguyen@squiz.net>
* @version $Revision: 1.12 $
* @package MySource_Matrix
*/

error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT)) {
	echo "ERROR: You need to supply the path to the System Root as the first argument\n";
	exit();
}

if (!is_dir($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT.'/core/include/init.inc')) {
	echo "ERROR: Path provided doesn't point to a Matrix installation's System Root. Please provide correct path and try again.\n";
	exit();
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';

$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user);

echo "\n";

$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
$db =& $GLOBALS['SQ_SYSTEM']->db;


$action = NULL;
while ($action != 'add' && $action != 'remove') {
	$action = get_line('Please specify whether you want to \'add\' or \'remove\' a URL: ');
}

require_once $SYSTEM_ROOT.'/scripts/url_manager.inc';

if ($action == 'add') {
	$inputs		= URL_Manager::cliInterfaceAddUrl();
	$queries	= URL_Manager::addUrl($inputs['http'], $inputs['https'], $inputs['new_url'], $inputs['existing_url'], $inputs['siteid'], $inputs['update_file_public_live_assets'], $inputs['existing_urlid'], $SYSTEM_ROOT);
} else if ($action == 'remove') {
	$remove_url_info	= URL_Manager::cliInterfaceRemoveUrl();
	$queries	= URL_Manager::removeUrl($remove_url_info['remove_urlid'], $remove_url_info['remove_assetid'], $remove_url_info['remove_url'], FALSE, $SYSTEM_ROOT);
}//end else if


	$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
	$GLOBALS['SQ_SYSTEM']->restoreCurrentUser();

	exit(0);


/**
* Prints the specified prompt message and returns the line from stdin
*
* @param string $prompt the message to display to the user
*
* @return string
* @access public
*/
function get_line($prompt='')
{
	echo $prompt;
	// now get their entry and remove the trailing new line
	return rtrim(fgets(STDIN, 4096));

}//end get_line()


?>
