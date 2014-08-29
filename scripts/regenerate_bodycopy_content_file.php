<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
*/

/**
* This script regenerates the bodycopy content file
*
* @author Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: 1.5 $
* @package MySource_Matrix
*/

// Usage: php regenerate_bodycopy_content_file.php -s [SYSTEM_ROOT] <--root-node=[ROOT_NODE_ASSETID]>

if (ini_get('memory_limit') != '-1') ini_set('memory_limit', -1);
error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) trigger_error("You can only run this script from the command line\n", E_USER_ERROR);


$SYSTEM_ROOT = getCLIArg('system');
if (empty($SYSTEM_ROOT)) {
	echo "ERROR: You need to supply the path to the System Root";
	print_usage();
	exit(1);
}

if (!is_dir($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT.'/core/include/init.inc')) {
	echo "ERROR: Path provided doesn't point to a Matrix installation's System Root. Please provide correct path and try again.\n";
	print_usage();
	exit(1);
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';

$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
        echo "ERROR: Failed logging in as root user\n";
        exit();
}

$ROOT_NODE = getCLIArg('rootnode');
if (!$ROOT_NODE) {
	$ROOT_NODE = 1;
}

// Get all the bodycopy divs
$assetids = array_keys($GLOBALS['SQ_SYSTEM']->am->getChildren($ROOT_NODE, 'bodycopy_div'));
echo "Regenerating content file for ".count($assetids)." assets\n";


$count = 0;
foreach($assetids as $assetid) {
	$bodycopy_div = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
	$bodycopy_div_edit_fns = $bodycopy_div->getEditFns();
	$bodycopy_div_edit_fns->generateContentFile($bodycopy_div);
	$GLOBALS['SQ_SYSTEM']->am->forgetAsset($bodycopy_div, TRUE);
	$count++;

	echo ($count%10) ? '.' : '';
	
}//end foreach

echo " Done.\n";


/**
* Print script usage
*
* @return void
*/
function print_usage()
{
	echo "\nRegenerates the bodycopy content file of bodycopy container assets.\n";
	echo "\nUsage:  php ".basename(__FILE__)." --system=<SYSTEM_ROOT> [--rootnode=<ROOT_NODE>]\n";
	echo "\t<SYSTEM_ROOT> : The root directory of Matrix system.\n";
	echo "\t<ROOT_NODE>  : Assetid of the root node to action on.\n";
	echo "\n";

}//end print_usage()


/**
 * Get CLI Argument
 * Check to see if the argument is set, if it has a value, return the value
 * otherwise return true if set, or false if not
 *
 * @params $arg string argument
 *
 * @return string/boolean
 */
function getCLIArg($arg)
{
	return (count($match = array_values(preg_grep("/--" . $arg . "(\=(.*)|)/i",$_SERVER['argv']))) > 0 === TRUE) ? ((preg_match('/--(.*)=(.*)/',$match[0],$reg)) ? $reg[2] : true) : false;

}//end getCLIArg()

?>
