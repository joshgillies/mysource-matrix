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
* $Id: cron_job_queue_scheduled_job.inc,v 1.3 2012/08/30 00:57:13 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/system/cron/cron_job/cron_job.inc';
require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';

/**
*	Cron_Job_Queue_Scheduled_Job
*
* Purpose
*
*
* @author  Ash Karelia <akarelia@squiz.com.au>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage __bulkmail__
*/
class Cron_Job_Queue_Scheduled_Job extends Cron_Job
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
	function create(&$link)
	{
		require_once SQ_CORE_PACKAGE_PATH.'/system/system_asset_fns.inc';

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
	function _getName($short_name=FALSE)
	{
		return $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name');

	}//end _getName()


	/**
	* returns FALSE always because we don't want to be able to clone this cron job
	*
	* @return boolean
	* @access public
	*/
	function canClone()
	{
		return FALSE;

	}//end canClone()


	/**
	* Morphs this asset to either one of its parent types or one of its children types.
	*
	* Only succeeds if $new_type_code assets are allowed to link to all children that this asset has
	*
	* @param string	$new_type_code	the new type_code to attempt to cast this asset to
	*
	* @return boolean
	* @access public
	* @see Asset::morph()
	*/
	function morph($new_type_code)
	{
		trigger_localised_error('CRON0024', sprintf(translate('Unable to morph %s'), $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name')), E_USER_WARNING)
		return FALSE;

	}//end morph()


	/**
	* Can this asset have its last significant link removed (putting it in the trash)?
	*
	* For the Manage Pending Accounts cron job, NO!  There must always be exactly one of these here.
	*
	* @return boolean
	* @access public
	*/
	function canDelete()
	{
		return FALSE;

	}//end canDelete()


	/**
	* Run this cron job
	* Will get all the bulkmail job in the system and see if the scheduling is enabled.
	* If scheduling is enabled then will calculate if the Bulkmail job needs to be queued now
	*
	* Returns a bitmask of the SQ_CRON_JOB_* constants to tell the cron manager what to do
	*
	* @param string	&$msg	used to return any messages about completion etc from _exec
	*
	* @return int
	* @access private
	* @see Cron_Job::_exec(), Cron_Job::run()
	*/
	function _exec(&$msg)
	{
		$jobs = $this->getBulkmailJobs();
		if (empty($jobs)) return SQ_CRON_JOB_COMPLETED;

		$now = time();

		$am = $GLOBALS['SQ_SYSTEM']->am;
		$cm = $am->getSystemAsset('cron_manager');
		$bm = $am->getSystemAsset('bulkmail_manager');
		$last_run = $cm->attr('last_run');

		$multiplier = Array(
						'hourly'	=> (60 * 60),
						'daily'		=> (60 * 60 * 24),
						'weekly'	=> (60 * 60 * 24 * 7),
						'monthly'	=> (60 * 60 * 24 * 30),
						'yearly'	=> (60 * 60 * 24 * 365),
					  );

		foreach ($jobs as $job_id => $schedule) {
			$frequency = array_get_index(array_keys($schedule), 0);
			$date_time = $schedule[$frequency];
			$date_time_ts = strtotime($date_time);

			// if the starting time of the scheduling of jobs is in future ignore it
			if ($date_time_ts > $now) continue;

			// check to see if the jb is already queued up that way even
			// if it is our time to queue up a job we need not bother
			$queued_jobs = $bm->getQueuedJobs($job_id);
			if (!empty($queued_jobs)) continue;

			$bulkmail_job = $am->getAsset($job_id);
			$last_job_run = $bulkmail_job->attr('last_scheduled_run');

			if (!$last_job_run && $date_time_ts < $now) {
				// if the job isn't scheduled before the first time
				// it needs to be scheduled is its epoch time
				$bulkmail_job->processStatusChange(SQ_STATUS_LIVE);
				$am->acquireLock($bulkmail_job->id, 'all');
					$bulkmail_job->setAttrValue('last_scheduled_run', $now);
					$bulkmail_job->saveAttributes();
				$am->releaseLock($bulkmail_job->id, 'all');
				continue;
			}

			// we have the last time the bulkmail job was queue up time
			// and we also have the time interval for it to be queued up
			// calculate and check to see if the frecuency for next
			// queuing up match our past and current cron manager run
			$next_run_for_bulkmail = $last_job_run + $multiplier[$frequency];

			if ($next_run_for_bulkmail < $now && $next_run_for_bulkmail > $last_run) {
				// we have calculated the next time weneed to queue the bulkmail job
				// now queue it up if that time is after the last cron run but before now
				$bulkmail_job->processStatusChange(SQ_STATUS_LIVE);
				$am->acquireLock($bulkmail_job->id, 'all');
					$bulkmail_job->setAttrValue('last_scheduled_run', $now);
					$bulkmail_job->saveAttributes();
				$am->releaseLock($bulkmail_job->id, 'all');
				continue;
			}
		}

		return SQ_CRON_JOB_COMPLETED;

	}//end _exec()


	function getBulkmailJobs()
	{
		$res = Array();

		$all_bulkmail_jobs = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('bulkmail_job');
		foreach ($all_bulkmail_jobs as $job_id) {
			$bulkmail_job = $GLOBALS['SQ_SYSTEM']->am->getAsset($job_id);
			// check to see fi the scheduling is enabled
			if (!$bulkmail_job->attr('scheduling_status')) continue;
			$schedule = $bulkmail_job->attr('scheduled_time');
			// if there is no schedule configured we don't need this either
			if (empty($schedule)) continue;
			$res[$job_id] = $schedule;

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($bulkmail_job, TRUE);
			unset($bulkmail_job);
		}

		return $res;

	}//end getBulkmailJobs()


}//end class

?>
