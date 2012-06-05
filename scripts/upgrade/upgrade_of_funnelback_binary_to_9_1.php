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
* $Id: upgrade_of_funnelback_binary_to_9_1.php,v 1.2 2012/06/05 06:26:10 akarelia Exp $
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
* @version $Revision: 1.2 $
* @package MySource_Matrix
* @subpackage scripts 
*/

if (ini_get('memory_limit') != '-1') ini_set('memory_limit', -1);
error_reporting(E_ALL);

if (php_sapi_name() != 'cli') {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

if (count($argv) < 1) {
    echo "USAGE : php scripts/upgrade_of_funnelback_binary_to_9_1.php <SYSTEM_ROOT>\n";
    exit();
}//end if

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
$root_user	= $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');

// log in as root
if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	echo "ERROR: Failed logging in as root user\n";
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
			if(!copy($data_path.$file_path, $data_path.$new_path)) {
				bam($data_path.$file_path);
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
