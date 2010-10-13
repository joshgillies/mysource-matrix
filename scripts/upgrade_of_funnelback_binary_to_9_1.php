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
* $Id: upgrade_of_funnelback_binary_to_9_1.php,v 1.1.2.2 2010/10/13 03:10:17 akarelia Exp $
*
*/

/**
 * ths script will copy across the file to their new names which is required (due to 
 * changes made) by funnelback 9.1 binary. This script WILL NOT delete old .cfg files.
 * 
 * USAGE : php scripts/upgrade_of_funnelback_binary_to_9_1.php <SYSTEM_ROOT> 
 */

/**
* @author  Ash Karelia <akarelia@squiz.com.au>
* @version $Revision: 1.1.2.2 $
* @package MySource_Matrix
* @subpackage scripts 
*/

ini_set('memory_limit', -1);
error_reporting(E_ALL);

if (php_sapi_name() != 'cli') {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

if (count($argv) < 1) {
    echo "USAGE : php scripts/script_to_update_xml_for_submission.php <SYSTEM_ROOT> ROOT_NODE<ASSETID> [--regen]\n";
    exit();
}//end if

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	echo "ERROR : You need to supply the path to the source System Root as the First argument\n";
	exit();
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';

// ask for the root password for the system
echo 'Enter the root password for "'.SQ_CONF_SYSTEM_NAME.'": ';
$root_password = rtrim(fgets(STDIN, 4094));

$root_user	= $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
// Check that the correct root password was entered.
// Yes this is checked for each site because even if this individual forked process is killed
// the parent process still runs and continues to fork more processes.
if (!$root_user->comparePassword($root_password)) {
	// only show the error once
	trigger_error("The root password entered was incorrect\n", E_USER_ERROR);
	exit;
}

// log in as root
if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	trigger_error("Failed logging in as root user\n", E_USER_ERROR);
	exit(1);
}//end if

$fbm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('funnelback_manager');
$collections = $fbm->getCollections();

foreach($collections as $index => $collection ) {
	$file_paths = Array();
	$all_good = TRUE;
	printCollection($index, $collection['name']);
	$data_path = $fbm->getCollectionDataPath($index).'/conf/';
	$file_paths['contextual_navigation.cfg'] = 'fluster.cfg';
	$file_paths['query_expansion.cfg'] = 'synonyms.cfg';
	$file_paths['best_bets.cfg'] = 'featured_pages.cfg';

	foreach ($file_paths as $new_path => $file_path) {
		if(file_exists($data_path.$file_path)) {
			if(copy($data_path.$file_path, $data_path.$new_path)) {bam($data_path.$file_path);
				$all_good = FALSE;
			}
		}
	}

	if ($all_good) {
		printUpdateStatus('OK');
	} else {
		printUpdateStatus('FAILED');
	}
}



/**
* Prints the name of the collection as a padded string
*
* Pads name to 40 columns
*
* @param string	$id		id of the collection
* @param string $name	name of the collection
*
* @return void
* @access public
*/
function printCollection($id, $name)
{
	$str = '[ #'.$id.' ]'.$name;
	if (strlen($str) > 66) {
		$str = substr($str, 0, 66).'...';
	}
	printf ('%s%'.(70 - strlen($str)).'s', $str,'');

}//end printCollection()


/**
* Prints the status of the container integrity check
*
* @param string	$status	the status of the check
*
* @return void
* @access public
*/
function printUpdateStatus($status)
{
	echo "[ $status ]\n";

}//end printUpdateStatus()

?>
