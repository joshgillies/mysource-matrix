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
* $Id: remove_old_cron_jobs.php,v 1.1 2006/01/12 22:37:19 tbarrett Exp $
*
*/

/**
* Finds and links orphaned assets (ie. ones with no links to them, ie. ones without links where they are
* the minor) underneath a specified asset id, preferably a folder
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix
*/
error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) trigger_error("You can only run this script from the command line\n", E_USER_ERROR);

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	echo "ERROR: You need to supply the path to the System Root as the first argument\n";
	exit();
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';


// ask for the root password for the system
echo 'Enter the root password for "'.SQ_CONF_SYSTEM_NAME.'": ';
$root_password = rtrim(fgets(STDIN, 4094));

// check that the correct root password was entered
$root_user = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
if (!$root_user->comparePassword($root_password)) {
	echo "ERROR: The root password entered was incorrect\n";
	exit();
}

// log in as root
if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	trigger_error("Failed login in as root user\n", E_USER_ERROR);
}

$db =& $GLOBALS['SQ_SYSTEM']->db;
$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
$old_date = date('Y-m-d', strtotime('-1 week'));
$sql = "SELECT assetid 
		FROM sq_ast_attr_val 
		WHERE
			attrid IN 
				(select attrid from sq_ast_attr where (type_code = 'cron_job' or owning_type_code = 'cron_job') and name='when')
			AND custom_val < 'OO=$old_date'";
$assetids = $db->getCol($sql);
assert_valid_db_result($assetids);
if (empty($assetids)) {
	echo "No old cron jobs found\n";
}
echo 'Found '.count($assetids).' old crons'."\n";
$assetids_list = '('.implode(', ', $assetids).')';
$res = $db->query('DELETE FROM sq_ast WHERE assetid IN '.$assetids_list);
assert_valid_db_result($res);
$res = $db->query('DELETE FROM sq_ast_lnk WHERE minorid IN '.$assetids_list);
assert_valid_db_result($res);
$res = $db->query('DELETE FROM sq_ast_lnk_tree WHERE linkid NOT IN (SELECT linkid FROM sq_ast_lnk)');
assert_valid_db_result($res);
$res = $db->query('DELETE FROM sq_ast_attr_val WHERE assetid IN '.$assetids_list);
assert_valid_db_result($res);
$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
echo "Done\n";
