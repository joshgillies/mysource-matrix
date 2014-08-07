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
* $Id: cron_job_generate_log_reports.inc,v 1.15 2012/08/30 01:09:11 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/system/cron/cron_job/cron_job.inc';
require_once SQ_INCLUDE_PATH.'/workflow_manager.inc';
require_once SQ_FUDGE_PATH.'/general/datetime.inc';

/**
* Cron_Job_Generate_Log_Reports
*
* Purpose
*   Rotates logs when required, then subsequently tells the log_reports in the system to regenerate
*   using the last rotated log
*
*   Note that this is a singleton asset - when the asset type is installed it creates a single
*   instance of itself under the cron manager, which should remain there always.
*
* @version $Revision: 1.15 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Cron_Job_Generate_Log_Reports extends Cron_Job
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Create this asset
	*
	* Return either the linkid or FALSE on failure
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return mixed int|boolean
	* @access public
	* @see Asset::create()
	*/
	public function create(Array &$link)
	{
		require_once SQ_CORE_PACKAGE_PATH.'/system/system_asset_fns.inc';
		if (!system_asset_fns_create_pre_check($this)) {
			return FALSE;
		}
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if ($linkid = parent::create($link)) {
			if (!system_asset_fns_create_cleanup($this)) {
				$linkid = FALSE;
			}
		}

		if ($linkid) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		}

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return $linkid;

	}//end create()


	/**
	* Return name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	protected function _getName($short_name=FALSE)
	{
		return translate('cron_job_generate_log_reports');

	}//end _getName()


	/**
	* returns FALSE always because we don't want to be able to clone a cron job
	*
	* @return boolean
	* @access public
	*/
	public function canClone()
	{
		return FALSE;

	}//end canClone()


	/**
	* Can this asset have its last significant link removed (putting it in the trash)?
	*
	* For the expired workflow cron job, NO!  There must always be exactly one of these here.
	*
	* @return boolean
	* @access public
	*/
	public function canDelete()
	{
		return FALSE;

	}//end canDelete()
	
	
	/**
	* Run this cron job
	*
	* Loops through all the logs, rotates them, then tells all the reports to generate a new report from the most recently rotated logs
	* Returns a bitmask of the SQ_CRON_JOB_* constants to tell the cron manager what to do
	*
	* @param string	&$msg	used to return any messages about completion etc from _exec
	*
	* @return int
	* @access private
	* @see Cron_Job::_exec(), Cron_Job::run()
	*/
	protected function _exec(&$msg)
	{
		// set the UMASK to u=rw,g=rw,o=r (0002), so that the system can write to the the logfiles the cronjob creates
		if (substr(PHP_OS, 0, 3) != 'WIN') $old_mask = umask(0113);

		$lm = $GLOBALS['SQ_SYSTEM']->getLogManager();

		$rotated_logs = $lm->rotateLogs();

		// nothing was rotated, so don't generate any reports and end prematurely
		if (empty($rotated_logs)) return SQ_CRON_JOB_COMPLETED;

		// get all the log_reports in the system before looping through, if they exist
		$status = $this->generateReports($rotated_logs);

		if (isset($old_mask)) umask($old_mask);

		return SQ_CRON_JOB_COMPLETED;

	}//end _exec()


	/**
	* Tells a group of Log Reports in the system to generate new reports
	*
	* @param array	$target_logs	the logs to generate
	*
	* @return boolean
	* @access public
	*/
	public function generateReports(Array $target_logs)
	{
		if (empty($target_logs)) return FALSE;

		if (!$GLOBALS['SQ_SYSTEM']->am->installed('log_report')) {
			return FALSE;
		}

		$log_reports  = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('log_report', FALSE, TRUE);

		// a list of all the logs in the system
		$lm = $GLOBALS['SQ_SYSTEM']->getLogManager();
		$logs = $lm->getLogs();
		foreach ($target_logs as $logname) {
			if (isset($logs[$logname])) {
				$this->generateReportFromLog($logname, $log_reports);
			}
		}

		return TRUE;

	}//end generateReports()


	/**
	* Tells a group of Log Reports that need a particular log type to generate new reports
	*
	* @param string	$logname		the log to generate from
	* @param array	&$log_reports	the log reports to generate (assetid=>type)
	*
	* @return boolean
	* @access public
	*/
	public function generateReportFromLog($logname, Array &$log_reports)
	{
		if (empty($log_reports)) return FALSE;

		// start up a hipo for each report
		$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();

		foreach ($log_reports as $reportid => $info) {
			$report_type = $info['type_code'];
			$report = $GLOBALS['SQ_SYSTEM']->am->getAsset($reportid, $report_type);
			if ($report->logname == $logname) {
				$vars   = Array(
							'assetid'			=> $reportid,
							'report'			=> $report,
						  );
				$status = $hh->freestyleHIPO('hipo_job_generate_report', $vars);
			}
		}

		return TRUE;

	}//end generateReportFromLog()


	/**
	* Runs this job
	*
	* This method is overridden because we want to make sure the cron manager never deletes the
	* generate_log_reports cron job
	* Returns a bitmask of the SQ_CRON_JOB_* constants to tell the cron manager what to do
	*
	* @return int
	* @access public
	*/
	public function run()
	{
		$res = parent::run();
		if ($res & SQ_CRON_JOB_REMOVE) {
			$res = $res & -SQ_CRON_JOB_REMOVE;
		}
		return $res;

	}//end run()


}//end class



?>
