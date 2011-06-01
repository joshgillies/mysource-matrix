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
* $Id: remove_old_cron_jobs.php,v 1.5 2008/02/18 05:28:41 lwright Exp $
*
*/

/**
* Script to remove cron jobs more than a week out of date.  Useful when they've
* piled up so much that you can't delete them manually and can't run the cron script
*
* @author  Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.5 $
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

$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

$old_date = date('Y-m-d', strtotime('-1 week'));
$sql = "SELECT assetid
		FROM sq_ast_attr_val
		WHERE
			attrid IN
				(select attrid from sq_ast_attr where (type_code = 'cron_job' or owning_type_code = 'cron_job') and name='when')
				AND custom_val < :old_date";
$query = MatrixDAL::preparePdoQuery($sql);
MatrixDAL::bindValueToPdo($query, 'old_date', 'OO='.$old_date);
$assetids = MatrixDAL::executePdoAssoc($query, 0);
if (empty($assetids)) {
	echo "No old cron jobs found\n";
} else {
	echo 'Found '.count($assetids).' old crons'."\n";
	$assetids_list = '('.implode(', ', $assetids).')';
	$res = MatrixDAL::executeSql('DELETE FROM sq_ast WHERE assetid IN '.$assetids_list);
	$res = MatrixDAL::executeSql('DELETE FROM sq_ast_lnk WHERE minorid IN '.$assetids_list);
	$res = MatrixDAL::executeSql('DELETE FROM sq_ast_lnk_tree WHERE linkid NOT IN (SELECT linkid FROM sq_ast_lnk)');
	$res = MatrixDAL::executeSql('DELETE FROM sq_ast_attr_val WHERE assetid IN '.$assetids_list);
}
$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
echo "Done\n";
