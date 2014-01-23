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
* Shows the number of users using each hash.
*
* @author  Benjamin Pearson <bpearson@squiz.com.au>
* @version $Revision: 1.9 $
* @package MySource_Matrix
*/
error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) trigger_error("You can only run this script from the command line\n", E_USER_ERROR);

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
require_once SQ_FUDGE_PATH.'/general/security.inc';

$algos   = array('1','2y','6','6o');
$sql     = "SELECT v.custom_val FROM sq_ast_attr_val v INNER JOIN sq_ast_attr n ON n.attrid=v.attrid WHERE n.name='password' AND n.owning_type_code='user' AND v.contextid = 0";
$output  = MatrixDAL::executeSqlAssoc($sql);
$results = array(
            '1'  => 0,
            '6o' => 0,
            '6'  => 0,
            '2y' => 0,
           );
foreach ($output as $o) {
    $hash = $o['custom_val'];
    if (strpos($hash, '$2y$') === 0) {
        $results['2y']++;
    } else if (strpos($hash, '$6$') === 0) {
        $results['6']++;
    } else if (strpos($hash, '$6o$') === 0) {
        $results['6o']++;
    } else {
        $results['1']++;
    }
}//end foreach

echo 'Currently, there are '.count($output).' Users with passwords on this system.'."\n";
echo '------------------------------------------------------------------'."\n";
echo "\t".$results['1'].' Users with a MD5 hash'."\n";
echo "\t".$results['6o'].' Users with an older SHA512 hash'."\n";
echo "\t".$results['6'].' Users with a SHA512 hash'."\n";
echo "\t".$results['2y'].' Users with a bcrypt hash'."\n";
echo '------------------------------------------------------------------'."\n";
echo "\n";
if ($results['1'] > 0 || $results['6o'] > 0) {
    echo 'There is potential on this system for compromise with insecure hashes.'."\n";
    echo 'Please update PHP as soon as possible.'."\n";
}
