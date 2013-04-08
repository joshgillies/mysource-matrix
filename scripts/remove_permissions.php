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
* $Id: remove_permissions.php,v 1.1.2.3 2013/04/08 05:11:13 ewang Exp $
*
*/
error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) {
    trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

$SYSTEM_ROOT = getCLIArg('system');
if (!$SYSTEM_ROOT) {
    echo "ERROR: You need to supply the path to the System Root\n";
    print_usage();
    exit(1);
}

if (!is_dir($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT.'/core/include/init.inc')) {
    echo "ERROR: Path provided doesn't point to a Matrix installation's System Root. Please provide correct path and try again.\n";
    print_usage();
    exit(1);
}

if (ini_get('memory_limit') != '-1') ini_set('memory_limit', '-1');
require_once ($SYSTEM_ROOT.'/core/include/init.inc');


$root_nodes_string = getCLIArg('rootnode');
if (empty($root_nodes_string)) {
    echo "ERROR: You need to supply the Root Node to process\n";
    print_usage();
    exit(1);
}



$sql = 'DELETE FROM SQ_AST_PERM WHERE ';

// root node condition
$root_nodes = explode(',', $root_nodes_string);
$assets = Array();
foreach ($root_nodes as $root_node) {
 $assets = array_merge($assets, array_keys($GLOBALS['SQ_SYSTEM']->am->getChildren(trim($root_node))));
}

$in_clause = Array();
foreach (array_chunk($assets, 999) as $chunk) {
	foreach ($chunk as $key => $value) {
		$chunk[$key] = MatrixDAL::quote($chunk[$key]);
	}
	$in_clause[] = '(assetid IN ('.implode(', ', $chunk).'))';
}
$in_clause_sql = implode(' OR ', $in_clause);
$in_clause_sql = '( '.$in_clause_sql.')';


// include user condition
$include_user_string = getCLIArg('includeuser');
$include_user_sql = '';
if (!empty($include_user_string)) {
    $include_users = explode(',', $include_user_string);
    foreach ($include_users as $key => $value) {
	    $include_users[$key] = MatrixDAL::quote($include_users[$key]);
    }
    $include_user_sql = ' AND userid IN ('.  implode(', ', $include_users).')';
}

// exclude user condition
$exclude_user_string = getCLIArg('excludeuser');
$exclude_user_sql = '';
if (!empty($exclude_user_string)) {
    $exclude_users = explode(',', $exclude_user_string);
    foreach ($exclude_users as $key => $value) {
	    $exclude_users[$key] = MatrixDAL::quote($exclude_users[$key]);
    }
    $exclude_user_sql = ' AND userid NOT IN ('.  implode(', ', $exclude_users).')';
}


// permission type condition
$type_string = getCLIArg('type');
$type_sql = '';
if (!empty($type_string)) {
    $types = explode(',', $type_string);
    foreach ($types as $key => $value) {
	    $types[$key] = MatrixDAL::quote($types[$key]);
    }
    $type_sql = ' AND permission IN ('.  implode(', ', $types).')';
}

$sql = $sql.$in_clause_sql.$include_user_sql.$exclude_user_sql.$type_sql;


// confirmation to proceed
$message = "This script is about to remove ALL permissions for assets under root node:".$root_nodes_string;
if(!empty($include_user_string)) 
    $message .= " assigned to user:".$include_user_string;
if(!empty($exclude_user_string)) 
    $message .= " excluding user:".$exclude_user_string;
if(!empty($type_string)) 
    $message .= " with permission type:".$type_string;

echo $message."\n";
echo count($assets)." assets to check\n";
echo "Are you sure you want to proceed (Y/N)? \n";

$yes_no = rtrim(fgets(STDIN, 4094));
if (strtolower($yes_no) != 'y') {
    echo "\nScript aborted. \n";
    exit;
}
    
    
    

$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');

$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

try {
	$query = MatrixDAL::preparePdoQuery($sql);
	$rows = MatrixDAL::execPdoQuery($query);
} catch (Exception $e) {
	throw new Exception('Unable to attempt to delete permissions, due to database error: '.$e->getMessage());
}

$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();


echo "$rows permission rows have been deleted\n\n";
				

/**
 * Get CLI Argument
 * Check to see if the argument is set, if it has a value, return the value
 * otherwise return true if set, or false if not
 *
 * @params $arg string argument
 *
 * @return string/boolean
 * @author Matthew Spurrier
 */
function getCLIArg($arg)
{
	return (count($match = array_values(preg_grep("/--" . $arg . "(\=(.*)|)/i",$_SERVER['argv']))) > 0 === TRUE) ? ((preg_match('/--(.*)=(.*)/',$match[0],$reg)) ? $reg[2] : true) : false;

}//end getCLIArg()


/**
 * Print the usage of this script
 *
 * @return void
 */
function print_usage()
{
    echo "\nThis script removes all permissions for all children under a given root node.\n\n";

    echo "Usage: php ".basename(__FILE__)." --system=<SYSTEM_ROOT> --rootnode=<ROOTNODE> [--includeuser=<INLCUDE_USERID>] [--excludeuser=<EXCLUDE_USERID>] [--type=<PERMISSION_TYPE>]\n\n";
    echo "\t<SYSTEM_ROOT> : The root directory of Matrix system.\n";
    echo "\t<ROOTNODES>   : Assetid of the rootnodes to be processed. Comma seprated values.\n";
    echo "\t<INLCUDE_USERIDS> : Only remove permissions for speicified users. Comma seprated values. Optional.\n";
    echo "\t<EXCLUDE_USERIDS> : Do not remove permissions for specified users. Comma seprated values. Optional.\n";
    echo "\t<PERMISSION_TYPE> : Only remove specified types. Comma seprated values. Optional. 1 for Read, 2 for Write, 3 for Admin\n";
   

    echo "\nWARNING: BACKUP PERMISSION TABLE BEFORE RUNNING THIS SCRIPT. ROLLBACK SHOULD BE DISABLED\n\n";

}//end print_usage()




?>
