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
* $Id: remove_old_cron_jobs.php,v 1.9 2012/10/05 07:20:38 akarelia Exp $
*
*/

/**
* Script to remove cron jobs more than a week out of date.  Useful when they've
* piled up so much that you can't delete them manually and can't run the cron script
*
* @author  Tom Barrett <tbarrett@squiz.net>
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

$root_user = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');

// log in as root
if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	echo "ERROR: Failed login in as root user\n";
	exit();
}

$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

$old_date = date('Y-m-d', strtotime('-1 week'));
$sql = "SELECT assetid
		FROM sq_ast_attr_val
		WHERE
			attrid IN
				(select attrid from sq_ast_attr where (type_code = 'cron_job' or owning_type_code = 'cron_job') and name='when')
				AND CAST(custom_val AS varchar2(255)) < :old_date";
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
