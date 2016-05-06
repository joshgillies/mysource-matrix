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
* $Id: hipo_management.php,v 1.6 2012/10/05 07:20:38 akarelia Exp $
*
*/

/**
* Hipo Management 
*
* @author  Benjamin Pearson <bpearson@squiz.com.au>
* @version $Revision: 1.6 $
* @package MySource_Matrix
*/
error_reporting(E_ALL);
if (ini_get('memory_limit') != '-1') ini_set('memory_limit', '-1');

if ((php_sapi_name() != 'cli')) {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

if (count($_SERVER['argv']) < 2) {
	echo "USAGE : php hipo_management.php MATRIX_ROOT [-remove_all_jobs]\n";
	exit();
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

$truncate_table = FALSE;
if (isset($_SERVER['argv'][2]) && $_SERVER['argv'][2] == '-remove_all_jobs') {
	$truncate_table = TRUE;
} else if (isset($_SERVER['argv'][2]) && $_SERVER['argv'][2] != '-remove_all_jobs') {
	echo "USAGE : php hipo_management.php MATRIX_ROOT [-remove_all_jobs]\n";
	exit();
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';
require_once SQ_FUDGE_PATH.'/general/datetime.inc';

$am = $GLOBALS['SQ_SYSTEM']->am;
$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();

$root_user = $am->getSystemAsset('root_user');
$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user);

$source_jobs = Array();
$now = time();

$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db3');
$db = MatrixDAL::getDb();
$sql = 'SELECT source_code_name, code_name, job_type
		FROM sq_hipo_job';
try {
	$query = MatrixDAL::preparePdoQuery($sql);
	$results = MatrixDAL::executePdoAssoc($query);
} catch (Exception $e) {
	throw new Exception('Unable to get HIPO jobs due to database error: '.$e->getMessage());
}
$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

if ($truncate_table) {
	echo "Found ".count($results)." jobs.";
	if (count($results) > 0) {
		echo "\nMake sure no HIPO jobs are running currently. Selecting 'yes' will remove those HIPO jobs too.\n";
		echo "Are you sure you want to continue removing all HIPO jobs? (y/n)\n";
		$wish = rtrim(fgets(STDIN, 4094));

		if (strtolower($wish) == 'y') {
			echo "Now truncating Hipo job table\t";
			$sql = 'TRUNCATE TABLE sq_hipo_job';
			try {
				$ok = MatrixDAL::executeSql($sql);
				if ($ok !== FALSE) {
					echo "[ OK ]\n";
				} else {
					echo "[ FAILED ]\n";
				}
			}catch (Exception $e) {
				throw new Exception('Unable to truncate table due to : '.$e->getMessage());
			}
		}
	} else {
		echo "\n";
	}
	exit();
}
// Filter out dependants
foreach ($results as $result) {
	$source_name = array_get_index($result, 'source_code_name', '');
	$job_name = array_get_index($result, 'code_name', '');

	if (!empty($job_name) && $job_name == $source_name) {
		$source_jobs[] = $result;
	}//end if
}//end foreach

if (empty($source_jobs)) {
	echo translate('There are currently no HIPO Jobs on the system')."\n";
	exit;
}//end if

foreach ($source_jobs as $index => $job) {
	$source_code_name = array_get_index($job, 'source_code_name', '');
	$source_job_type = array_get_index($job, 'job_type', '');
	if (empty($source_code_name)) continue;
	$source_job = $hh->getJob($source_code_name);
	if (is_null($source_job)) continue;
	echo ($index+1).': ';
	echo ucwords(str_replace('_', ' ', $source_job_type));
	echo '( '.$source_job->percentDone().'% )';
	echo "\tLast Updated: ".easy_time_total($source_job->last_updated - $now, TRUE)."\n";
	unset($source_job);
}//end foreach

echo 'Enter the number of the job to change: (Press q to quit)';
$choice = rtrim(fgets(STDIN, 4094));

if (strtolower($choice) == 'q') exit;
$actual_choice = ($choice-1);
if (!isset($source_jobs[$actual_choice])) {
	echo "Incorrect entry\n";
	exit;
}//end if

echo "Options\n\tr - resume\n\tk - kill\n\tq - quit\nChoice:";
$action = rtrim(fgets(STDIN, 4094));
$action = strtolower($action);

$action = (string) $action;
switch ($action) {
	case 'r':
		// Recover/resume
		$source_code_name = array_get_index($source_jobs[$actual_choice], 'source_code_name', '');
		$source_job_type  = array_get_index($source_jobs[$actual_choice], 'job_type', '');
		if (empty($source_code_name)) exit;
		$source_job = $hh->getJob($source_code_name);
		if (is_null($source_job)) exit;
		
		echo 'Resuming HIPO Job ';
		echo ucwords(str_replace('_', ' ', $source_job_type))."\t";

		// Do itttttt
		$status = $source_job->process();
		if ($status) {
			echo '[  OK  ]';
		} else {
			echo '[  !!  ]';
		}//end if
		echo "\n";
	break;

	case 'k':
		// Kill kill kill
		$source_code_name = array_get_index($source_jobs[$actual_choice], 'source_code_name', '');
		$source_job_type  = array_get_index($source_jobs[$actual_choice], 'job_type', '');
		if (empty($source_code_name)) exit;
		$source_job = $hh->getJob($source_code_name);
		if (is_null($source_job)) exit;

		echo 'Aborting HIPO Job ';
		echo ucwords(str_replace('_', ' ', $source_job_type))."\n";
		$source_job->abort();
	break;

	case 'q':
	default:
		exit;
}//end switch

?>
