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
* $Id: cron_job_generate_log_reports.inc,v 1.3 2005/11/17 00:06:39 rong Exp $
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
* @version $Revision: 1.3 $
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
	function Cron_Job_Generate_Log_Reports($assetid=0)
	{
		$this->Cron_Job($assetid);

	}//end constructor


	/**
	* Create this asset
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return mixed int or false
	* @access public
	* @see Asset::create()
	*/
	function create(&$link)
	{
		require_once SQ_CORE_PACKAGE_PATH.'/system/system_asset_fns.inc';
		if (!system_asset_fns_create_pre_check($this)) {
			return false;
		}
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if ($linkid = parent::create($link)) {
			if (!system_asset_fns_create_cleanup($this)) {
				$linkid = false;
			}
		}

		if ($linkid) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		}

		return $linkid;

	}//end create()


	/**
	* Return name of the asset
	*
	* @param bool	$short_name	whether or not we are after the shortname or the full name
	*
	* @see Asset::_getName()
	* @return string
	* @access private
	*/
	function _getName($short_name=false)
	{
		return translate('cron_job_generate_log_reports');

	}//end _getName()


	/**
	* returns FALSE always because we don't want to be able to clone a cron job
	*
	* @return boolean
	* @access public
	*/
	function canClone()
	{
		return false;

	}//end canClone()


	/**
	* Morph's this asset to either one of it's parent type or one of it's children types
	* Only succeeds if $type_code asset's is allowed to link to all children that this asset has
	*
	* @param string	$new_type_code	the new type_code to attempt to cast this asset to
	*
	* @return bool
	* @access public
	* @see Asset::morph()
	*/
	function morph($new_type_code)
	{
		trigger_localised_error('CORE0091', E_USER_WARNING, $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name'));
		return false;

	}//end morph()


	/**
	* Can this asset have its last significant link removed (putting it in the trash)?
	*
	* For the expired workflow cron job, NO!  There must always be exactly one of these here.
	*
	* @return boolean
	* @access public
	*/
	function canDelete()
	{
		return false;

	}//end canDelete()


	/**
	* Run this cron job
	*
	* Loops through all the logs, rotates them, then tells all the reports to generate a new report from the most recently rotated logs
	*
	* @param string	$msg	accepted by reference, used to return any messages about completion etc from _exec
	*
	* @return int	bitmask of the SQ_CRON_JOB_* constants to tell the cron manager what to do
	* @see Cron_Job::_exec(), Cron_Job::run()
	* @access private
	*/
	function _exec(&$msg)
	{
		// set the UMASK to u=rw,g=rw,o=r (0002), so that the system can write to the the logfiles the cronjob creates
		if (substr(PHP_OS, 0, 3) != 'WIN') umask(0113);

		$lm = &$GLOBALS['SQ_SYSTEM']->getLogManager();

		$rotated_logs = $lm->rotateLogs();

		// nothing was rotated, so don't generate any reports and end prematurely
		if (empty($rotated_logs)) return SQ_CRON_JOB_COMPLETED;

		// get all the log_reports in the system before looping through, if they exist
		$status = $this->generateReports($rotated_logs);

		return SQ_CRON_JOB_COMPLETED;

	}//end _exec()


	/**
	* Tells a group of Log Reports in the system to generate new reports
	*
	* @param Array(string)	$target_logs	the logs to generate
	*
	* @access public
	* @return boolean
	*/
	function generateReports($target_logs)
	{
		if (empty($target_logs)) return false;

		if (!$GLOBALS['SQ_SYSTEM']->am->installed('log_report')) {
			return false;
		}

		$log_reports  = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('log_report', false, true);

		// a list of all the logs in the system
		$lm = &$GLOBALS['SQ_SYSTEM']->getLogManager();
		$logs = $lm->getLogs();
		foreach ($target_logs as $logname) {
			if (isset($logs[$logname])) {
				if (isset($logs[$logname]['rotation']['last_index'])) {
					$last_index = $logs[$logname]['rotation']['last_index'];
				} else {
					$last_index = null;
				}
				$this->generateReportFromLog($logname, $log_reports, $last_index);
			}
		}

		return true;
	}//end generateReports()


	/**
	* Tells a group of Log Reports that need a particular log type to generate new reports
	*
	* @param string					$logname			the log to generate from
	* @param Array(assetid=>type)	&$log_reports		the log reports to generate
	* @param int					$rotation_index		the index of the log to generate from
	*
	* @access public
	* @return boolean
	*/
	function generateReportFromLog($logname, &$log_reports, $rotation_index=null)
	{
		if (empty($log_reports)) return false;

		// start up a hipo for each report
		$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();

		foreach ($log_reports as $reportid => $report_type) {
			$report = &$GLOBALS['SQ_SYSTEM']->am->getAsset($reportid, $report_type);
			if ($report->logname == $logname) {
				$vars   = Array(
							'assetid'			=> $reportid,
							'report'			=> &$report,
							'rotation_index'	=> $rotation_index,
						  );
				$status = $hh->freestyleHIPO('hipo_job_generate_report', $vars);
			}
		}

		return true;

	}//end generateReportFromLog()


	/**
	* Runs this job
	*
	* This method is overridden because we want to make sure the cron manager never deletes the
	* generate_log_reports cron job
	*
	* @return int	bitmask of the SQ_CRON_JOB_*	constants to tell the cron manager what to do
	* @access public
	*/
	function run()
	{
		$res = parent::run();
		if ($res & SQ_CRON_JOB_REMOVE) {
			$res = $res & -SQ_CRON_JOB_REMOVE;
		}
		return $res;

	}//end run()


}//end class



?>
