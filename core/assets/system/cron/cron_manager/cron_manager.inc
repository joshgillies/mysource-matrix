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
* $Id: cron_manager.inc,v 1.88 2013/10/18 03:08:30 cupreti Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset.inc';
require_once SQ_CORE_PACKAGE_PATH.'/system/cron/cron_job/cron_job.inc';

/**
* Cron_Manager
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.88 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Cron_Manager extends Asset
{

	/**
	* @var string suffix to data dir where the error log should be stored when we are run()ning
	*/
	var $error_log_file_name = 'cron_errors';


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		parent::__construct($assetid);

		// including these assets here as they are required to send an error
		// message in the case that a cron job run() fails. necessary because
		// getSystemAsset() will fail because it cannot includeAsset() inside
		// an aborted transaction
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('root_user');
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('system_user_group');

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
	* Return a name to use to describe this instance of the asset
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	protected function _getName($short_name=FALSE)
	{
		return $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name');

	}//end _getName()


	/**
	* Return an array of all the permitted links type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	public function _getAllowedLinks()
	{
		return Array(
				SQ_LINK_TYPE_1	=> Array('cron_job' => Array('card' => 'M', 'exclusive' => TRUE)),
				SQ_LINK_TYPE_2	=> Array('cron_job' => Array('card' => 'M', 'exclusive' => TRUE)),
				SQ_LINK_TYPE_3	=> Array('cron_job' => Array('card' => 'M', 'exclusive' => TRUE)),
				SQ_LINK_NOTICE	=> Array(),
			   );

	}//end _getAllowedLinks()


	/**
	* Return a list of lock types available for this asset type
	*
	* @return array
	* @access public
	*/
	public function lockTypes()
	{
		$lock_types = parent::lockTypes();
		$lock_types['attr_links'] = ($lock_types['attributes'] | $lock_types['links']);
		return $lock_types;

	}//end lockTypes()


	/**
	* Return FALSE always because we don't want to be able to clone a cron manager
	*
	* @return boolean
	* @access public
	*/
	public function canClone()
	{
		return FALSE;

	}//end canClone()


	/**
	* Morph this asset to either one of it's parent type or one of it's children types
	* Only succeeds if $type_code asset's is allowed to link to all children that this asset has
	*
	* @param string	$new_type_code	the new type_code to attempt to cast this asset to
	*
	* @return boolean
	* @access public
	* @see Asset::morph()
	*/
	public function morph($new_type_code)
	{
		trigger_localised_error('CRON0024', E_USER_WARNING, $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name'));
		return FALSE;

	}//end morph()


	/**
	* Moves a link from one place in this assets links order to another position
	*
	* @param int	$linkid		the link id of the link to move about
	* @param string	$sort_order	the position in the links list that this link should take,
	*							if less than zero places at end of list
	*
	* @return boolean
	* @access public
	* @see Asset::moveLinkPos()
	*/
	public function moveLinkPos($linkid, $sort_order=-1)
	{
		trigger_localised_error('CRON0041', E_USER_WARNING, $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name'));
		return FALSE;

	}//end moveLinkPos()


	/**
	* Remove a link where this asset is the major player
	*
	* @param int		$linkid			the link id of the link to remove
	* @param boolean	$check_locked	Check if the link is locked or not
	*
	* @return boolean
	* @access public
	* @see Asset_Manager::deleteAssetLink()
	*/
	public function deleteLink($linkid, $check_locked=TRUE)
	{
		$link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($linkid, $this->id);
		if (empty($link)) {
			trigger_localised_error('SYS0243', E_USER_NOTICE, $linkid);
			return FALSE;
		}

		if ($GLOBALS['SQ_SYSTEM']->am->isSystemAssetType($link['minor_type_code'])){
			trigger_localised_error('CORE0118', E_USER_WARNING, $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name'));
			return FALSE;
		}

		return parent::deleteLink($linkid, $check_locked);

	}//end deleteLink()


	/**
	* Set the value for the passed variable to the passed value
	*
	* @param string	$name	the name of the attribute
	* @param string	$value	the new value of the attribute
	*
	* @return boolean
	* @access public
	*/
	public function setAttrValue($name, $value)
	{
		if (!$GLOBALS['SQ_SYSTEM']->userRoot()) {
			trigger_localised_error('CRON0025', E_USER_WARNING, $name, $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name'));
			return FALSE;
		}

		// the refresh time has some restrictions on it, because otherwise painting the when box would be a pain
		if ($name == 'refresh_time') {
			$value = (int) $value;

			$one_hour = 60 * 60;
			$one_day  = 24 * $one_hour;

			if ($value >= $one_day) {
				// if we are over a day we must be a multiple of one day
				if ($value % $one_day) {
					trigger_localised_error('CRON0035', E_USER_WARNING, $one_day);
					return FALSE;
				}
			} else if ($value >= $one_hour) {
				// if we are over an hour we must be a multiple of one hour
				if ($value % $one_hour || $one_day % $value) {
					trigger_localised_error('CRON0036', E_USER_WARNING, $one_hour);
					return FALSE;
				}
			} else {
				// if we less than an hour we must divide evenly into an hour
				if ($one_hour % $value) {
					trigger_localised_error('CRON0037', E_USER_WARNING, $one_hour);
					return FALSE;
				}
			}
		}//end if setting refresh_time

		// if we are setting the last run time and the epoc has not been set yet,
		// assume the last run was the first run and set the epoc
		if ($name == 'last_run' && $this->attr('epoch') == 0) {
			$this->setAttrValue('epoch', $value);
		}

		return parent::setAttrValue($name, $value);

	}//end setAttrValue()


	/**
	* Adds a job to the cron manager
	*
	* @param Cron_Job	$job		a reference to the cron job to add to the system
	* @param User		$user		a reference to the user that this job will run as
	* @param string		$link_type	type of the link between the cron manager and the job
	* @param boolean	$read_only	if TRUE, the cron job can only be removed by the system
	*
	* @return boolean
	* @access public
	*/
	public function addJob(Cron_Job $job, User $user, $link_type=SQ_LINK_TYPE_3, $read_only=FALSE)
	{
		$job->setAttrValue('running_as', $user->id);
		$job->setAttrValue('read_only', $read_only);

		$link = Array('asset' => $this, 'link_type' => $link_type, 'is_exclusive' => '1');

		$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
		$linkid = $job->create($link);
		$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

		return ($linkid > 0);

	}//end addJob()


	/**
	* Removes a job form the cron manager
	*
	* @param Cron_Job	$job	a reference to the cron job to remove
	*
	* @return boolean
	* @access public
	*/
	public function removeJob(Cron_Job $job)
	{
		// If job is not associated (Type 3 linked) with the Cron Manager, don't bother
		$crn_mgr_link = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($this->id, $job->id, SQ_LINK_TYPE_3);
		if (empty($crn_mgr_link)) {
			return FALSE;
		}

		// If its system cron job (Type 2 linked under the manager), don't worry
		$link = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($this->id, $job->id, SQ_LINK_TYPE_2);
		if (!empty($link)) {
			return FALSE;
		}

		// Wrap the whole thing in the transaction
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// Ignore the permissions checks/locks
		$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);

			// Delete all the links in which job is the major party
			$major_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($job->id, SQ_SC_LINK_ALL, '', TRUE, 'major');
			foreach($major_links as $major_link) {
				$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($major_link['linkid']);
			}

			// Link the job to the Trash
			$trash = $GLOBALS['SQ_SYSTEM']->am->getAsset($GLOBALS['SQ_SYSTEM']->am->getSystemAssetid('trash_folder'));
			$trash_linkid = $trash->createLink($job, SQ_LINK_TYPE_1);

			$success = !empty($trash_linkid);
			if ($success) {
				// Delete the link (type 3) from the Cron Manager
				$success = $this->deleteLink($crn_mgr_link['linkid']);

				// Finally purge the job from the system
				if ($success) {
					$vars = Array('purge_root_linkid' => $trash_linkid);
					$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
					$errors = $hh->freestyleHipo('hipo_job_purge_trash', $vars);
					$success = empty($errors);
				}
			}//end if trashed

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($trash);

		$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

		$GLOBALS['SQ_SYSTEM']->doTransaction($success ? 'COMMIT' : 'ROLLBACK');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return $success;

	}//end removeJob()


	/**
	* Returns an array of cron jobs that are active under cron manager
	*
	* Returns an array of references to the selected cron job type
	*
	* @param string|array	$type_code			the type of cron job, if an array returns link if matches any
	*											of the array values
	* @param boolean		$strict_type_code	whether we are finding assets that are just a $type_code
	*											or $type_code and any of it's sub-classes
	*
	* @return array
	* @access public
	* @see Asset::getLinks()
	*/
	public function getJobs($type_code='cron_job', $strict_type_code=TRUE)
	{
		$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_SC_LINK_SIGNIFICANT, $type_code, $strict_type_code, 'major', NULL, NULL, TRUE);
		$jobs = Array();
		foreach ($links as $link) {
			$job = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
			if (is_null($job)) continue;
			$jobs[] = $job;
		}

		return $jobs;

	}//end getJobs()


	/**
	* Returns the time of the next run of the cron manager
	*
	* @return int
	* @access public
	*/
	public function timeOfNextRun()
	{
		return max((int) $this->attr('last_run'), (int) $this->attr('epoch')) + (int) $this->attr('refresh_time');

	}//end timeOfNextRun()


	/**
	* Returns a string with the readable version of how often the cron runs
	*
	* @return string
	* @access public
	*/
	public function readableRefreshTime()
	{
		$refresh_time = (int) $this->attr('refresh_time');
		if ($refresh_time >= 86400) {
			$num = ($refresh_time / 86400);
			return ($num > 1) ? $num.' '.translate('days') : translate('day');
		}
		if ($refresh_time >= 3600) {
			$num = ($refresh_time / 3600);
			return ($num > 1) ? $num.' '.translate('hours') : translate('hour');
		}
		if ($refresh_time >= 60) {
			$num = ($refresh_time / 60);
			return ($num > 1) ? $num.' '.translate('minutes') : translate('minute');
		}
		return $refresh_time.' '.translate('seconds');

	}//end readableRefreshTime()


	/**
	* Check if current time is within excluded time
	*
	* @param int	$start_time	Current time
	*
	* @return boolean
	* @access public
	*/
	public function isExcludedTime($start_time)
	{

		$days_array = Array(
						0	=> 'Sun',
						1	=> 'Mon',
						2	=> 'Tue',
						3	=> 'Wed',
						4	=> 'Thu',
						5	=> 'Fri',
						6	=> 'Sat',
					  );
		$excluded_times = $this->attr('exclude_times');

		foreach ($excluded_times as $exclude_time) {
			$excluded_days = Array();
			foreach ($exclude_time['days'] as $excluded_day) {
				$excluded_days[] = $days_array[$excluded_day];
			}

			$now_day = date('D', $start_time); // A textual representation of a day, three letters, eg. 'Mon'
			$now_time = date('G', $start_time); // 24-hour format of an hour without leading zeros, eg. 8, 15
			if (in_array($now_day, $excluded_days)) {
				// range: [from, to)
				if ($now_time >= $exclude_time['from'] && (($exclude_time['to'] < $exclude_time['from']) || ($now_time < $exclude_time['to']))) {
					return TRUE;
				}
			} else if ($exclude_time['from'] > $exclude_time['to']) {
				// exclude from previous day X hour to today Y hour
				$yesterday_day = date('D', time()-86400);
				if (in_array($yesterday_day, $excluded_days)) {
					if ($now_time < $exclude_time['to']) return TRUE;
				}
			}
		}
		return FALSE;

	}//end isExcludedTime()


	/**
	* Run the jobs that need executing in the system
	*
	* @return void
	* @access public
	*/
	public function run()
	{
		$start_time = time();

		$old_log_errors = ini_set('log_errors', '1');
		$old_error_log  = ini_set('error_log',  SQ_LOG_PATH.'/'.$this->error_log_file_name.SQ_CONF_LOG_EXTENSION);
		set_error_handler(Array($this, '_errorHandler'));

		if ($this->isExcludedTime($start_time)) {
			trigger_localised_error('CRON0047', E_USER_NOTICE, $this->name);
			return;
		}

		if (!empty($GLOBALS['SQ_SYSTEM']->tm) && $GLOBALS['SQ_SYSTEM']->tm->inTransaction(MatrixDAL::getCurrentDbId())) {
			trigger_localised_error('CRON0012', E_USER_WARNING);
			return;
		}

		if ($start_time < (int) $this->attr('epoch')) {
			trigger_localised_error('CRON0011', E_USER_WARNING);
			return;
		}

		// holds the assetids for potential long jobs
		// running flag for cron manager is set to false while running these jobs
		// we use the running flag in the cron job to control the logic
		$long_jobs = Array();

		// if we are currently running then we should just leave this alone
		if ($this->attr('running')) {
			// if we have been stopped too many times without the while loop below
			// looping over
			if ((int) $this->attr('run_check') >= (int) $this->attr('warn_after_num_run_checks')) {

				$root_user	= $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
				$sys_admins	= $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('system_user_group');

				$ms  = $GLOBALS['SQ_SYSTEM']->getMessagingService();
				$msg = $ms->newMessage();
				$msg_reps = Array(
								'system_name'		=> SQ_CONF_SYSTEM_NAME,
								'num_attempts'		=> $this->attr('warn_after_num_run_checks'),
								'error_log_path'	=> SQ_LOG_PATH.'/'.$this->error_log_file_name.SQ_CONF_LOG_EXTENSION,
								'root_urls'			=> str_replace("\n", ',', SQ_CONF_SYSTEM_ROOT_URLS),
							);

				// SQ_CONF_DEFAULT_EMAIL, SQ_CONF_TECH_EMAIL, and root user as default we send the email to
				// get the other users configured to send the email to
				$config_user 	   = $this->attr('dead_lock_email_to');

				// userid zero sends a msg to SQ_CONF_DEFAULT_EMAIL and SQ_CONF_TECH_EMAIL
				$msg->to           = array_merge(Array(0, $root_user->id), $config_user);
				$msg->from         = 0; // a system message
				$msg->type         = 'cron.deadlock';
				$msg->replacements = $msg_reps;

				$msg->send();

			}//end if run_check


			// now update this last run time, so that we know for next time
			if ($GLOBALS['SQ_SYSTEM']->am->acquireLock($this->id, 'attributes', 0, TRUE)) {
				$this->setAttrValue('run_check', (int) $this->attr('run_check') + 1);
				$this->saveAttributes();
				$GLOBALS['SQ_SYSTEM']->am->releaseLock($this->id, 'attributes');

			} else {
				trigger_localised_error('CRON0016', E_USER_WARNING, $this->name);
			}

		} else {
			// we aren't already running
			$this->setAttrValue('current_job', NULL);

			// OK using the epoch time and the the refresh time figure when the this period "really" started
			$epoch        = (int) $this->attr('epoch');
			$refresh_time = (int) $this->attr('refresh_time');

			$secs = $start_time - $epoch;
			$periods = floor($secs / $refresh_time);
			$this_run = $epoch + ($refresh_time * $periods);

			// if we only every day (or higher) we need to take into account the effects of
			// daylight savings
			if ($refresh_time >= (60 * 60 * 24)) {
				$epoch_in_dst = (date('I', $epoch)      == '1');
				$start_in_dst = (date('I', $start_time) == '1');

				// if the epoch and start time are in different stages of daylight savings we need to adjust
				if ($epoch_in_dst != $start_in_dst) {
					// because the epoch is before the start time, if it is in daylight savings
					// then we need to add one hour to this run's time
					// otherwise we need to drop an hour
					$this_run += ($epoch_in_dst) ? +3600 : -3600;
				}
			}

			// now get the lock, set the running flag and reset the run_check
			if ($GLOBALS['SQ_SYSTEM']->am->acquireLock($this->id, 'attributes', 0, TRUE) && $this->setAttrValue('running', TRUE) && $this->setAttrValue('run_check', 0) && $this->saveAttributes()) {

				$GLOBALS['SQ_SYSTEM']->am->releaseLock($this->id, 'attributes');

				$ms	= $GLOBALS['SQ_SYSTEM']->getMessagingService();
				$am = $GLOBALS['SQ_SYSTEM']->am;

				// first get some times
				$last_run = (int) $this->attr('last_run');
				$next_run = $this_run + $refresh_time;

				$jobs = $this->getJobsToRun($last_run, $this_run);

				// holds the number of times that a job has failed
				// in the form: Array([assetid] => [number of failed runs]);
				$failed_runs = Array();

				// holds the assetids for jobs to ignore - because they have failed to many times
				// in the form: Array([assetid]);
				$ignore_jobs = Array();

				while (!empty($jobs)) {

					$ms->openQueue();
					foreach ($jobs as $job_info) {

						$cron_job	= $am->getAsset($job_info['assetid'], $job_info['type_code']);
						if (is_null($cron_job)) continue;

						if ($cron_job->attr('long')) {
							// ignore potential long jobs, and run them later
							// with cron manager running flag as false
							// with cron job running flag as true
							if (!$cron_job->attr('running')) {
								// only include the long job if it is not already running
								$long_jobs[] = $job_info;
							}
							$ignore_jobs[] = $cron_job->id;
							$am->forgetAsset($cron_job);
							continue;

						} else {

							$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
							$this->setAttrValue('current_job', $cron_job->id);
							$this->saveAttributes();
							$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

							$result = $cron_job->run();

							$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
							$this->setAttrValue('current_job', NULL);
							$this->saveAttributes();
							$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
						}

						require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

						// we have been asked to remove this cron job
						if ($result & SQ_CRON_JOB_REMOVE) {
							$this->removeJob($cron_job);
							$ignore_jobs[] = $cron_job->id;

						} else if ($result & SQ_CRON_JOB_RETRY) {
							// they want to have another go

							// if they resulted in an error, we need to check this hasn't happened before
							if ($result & SQ_CRON_JOB_ERROR) {

								if (isset($failed_runs[$cron_job->id])) {
									$failed_runs[$cron_job->id]++;
								} else {
									$failed_runs[$cron_job->id] = 1;
								}

								// if they have had their fair share, get rid of them
								if ($failed_runs[$cron_job->id] >= (int) $this->attr('num_failed_attempts')) {
									$this->removeJob($cron_job);
									$ignore_jobs[] = $cron_job->id;

									$user       = $cron_job->getRunningUser();
									$root_user  = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
									$sys_admins = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('system_user_group');

									$msg = $ms->newMessage();
									$msg_reps = Array(
													'cron_job_code'	=> $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($cron_job->type(), 'name'),
													'asset_name'	=> $cron_job->name,
													'assetid'		=> $cron_job->id,
												);
									$msg->to           = Array($user->id, $root_user->id, $sys_admins->id);
									$msg->from         = 0; // a system message
									$msg->type         = 'cron.forced_fail';
									$msg->replacements = $msg_reps;

									$ms->enqueueMessage($msg);

								}//end if

							}//end if error

						} else {
							// they don't want another go but they don't want to be removed either ? just ignore them
							$ignore_jobs[] = $cron_job->id;

						}//end if

						$am->forgetAsset($cron_job);

					}//end foreach

					// send all the messages for this run
					$ms->closeQueue();

					// give it three seconds to get the lock
					$lock_count = 0;
					while (1) {
						if ($acquired = $GLOBALS['SQ_SYSTEM']->am->acquireLock($this->id, 'attr_links', 0, TRUE)) {
							break;
						} else if ($lock_count < 3) {
							$lock_count++;
							sleep(1);
						} else {
							break;
						}
					}

					// now update this last run time, so that we know for next time
					if ($acquired) {

						$this->setAttrValue('run_check', 0);
						$this->saveAttributes();

						$GLOBALS['SQ_SYSTEM']->am->releaseLock($this->id, 'attr_links');

					} else {
						trigger_localised_error('CRON0015', E_USER_WARNING, $this->name);
						return;
					}

					// if we have gone into the next run's time then we need to do its work as well
					if (time() > $next_run) {
						$this_run = $next_run;
						$next_run += $refresh_time;
					}

					$jobs = $this->getJobsToRun($last_run, $this_run, $ignore_jobs);
				}//end while

				// now update this last run time, so that we know for next time
				if ($GLOBALS['SQ_SYSTEM']->am->acquireLock($this->id, 'attributes', 0, TRUE)) {
					if ($this->setAttrValue('last_run', $this_run) && $this->setAttrValue('running', FALSE) && $this->saveAttributes()) {
						$GLOBALS['SQ_SYSTEM']->am->releaseLock($this->id, 'attributes');
					} else {
						trigger_localised_error('CRON0032', E_USER_WARNING, $this->name);
					}
				} else {
					trigger_localised_error('CRON0016', E_USER_WARNING, $this->name);
				}

				// process the potential long running jobs
				foreach ($long_jobs as $job_info) {
					$cron_job	= $am->getAsset($job_info['assetid'], $job_info['type_code']);
					if ($cron_job->attr('long') && !$cron_job->attr('running')) {
						$result = $cron_job->run();
						if ($result & SQ_CRON_JOB_REMOVE) {
							$this->removeJob($cron_job);
						} else if ($result & SQ_CRON_JOB_ERROR) {
							trigger_localised_error('CRON0049', E_USER_WARNING, $cron_job->id);
						}
					}
					$am->forgetAsset($cron_job);
				}

			} else {
				// aquire or set running failed
				trigger_localised_error('CRON0015', E_USER_WARNING, $this->name);

			}//end else - if acquire lock

		}//end else already running

		ini_set('log_errors', $old_log_errors);
		ini_set('error_log',  $old_error_log);
		restore_error_handler();

	}//end run()


	/**
	* Converts a CLOB expression to a varchar for comarisons with other varchar fields
	*
	* Returns the sql to convert to a varchar, or the expression unchanged if no conversion is needed
	*
	* @param string	$expression	the expression to convert to a varchar
	*
	* @return string
	* @access private
	*/
	protected function _clob2Str($expression)
	{
		if (MatrixDAL::getDbType() === 'oci') {
			return 'DBMS_LOB.SUBSTR('.$expression.', DBMS_LOB.GETLENGTH('.$expression.'), 1)';
		} else {
			return $expression;
		}

	}//end _clob2Str()


	/**
	* Get the jobs that need to be run for the passed time period
	*
	* @param int	$start_ts		the starting timestamp of the time period
	* @param int	$end_ts			the ending timestamp of the time period
	* @param array	$ignore_jobs	a list of assetids to ignore
	*
	* @return array
	* @access public
	*/
	public function getJobsToRun($start_ts, $end_ts, $ignore_jobs=Array())
	{
		require_once SQ_FUDGE_PATH.'/general/datetime.inc';

		if ($end_ts < $start_ts) {
			trigger_localised_error('CRON0014', E_USER_WARNING);
			return Array();
		}

		$end   = getdate($end_ts);
		$start = getdate($start_ts);

		$time_since_last_run = $end_ts - $start_ts;

		$one_hour = 60 * 60;
		$one_day  = 24 * $one_hour;
		$one_week =  7 * $one_day;

		$over_one_hour_ago = ($time_since_last_run >= $one_hour);
		$over_one_day_ago  = ($time_since_last_run >= $one_day);
		$over_one_week_ago = ($time_since_last_run >= $one_week);


		if ($end['mon'] == 1) {
			$days_last_month = days_in_month(12, $end['year'] - 1);
		} else {
			$days_last_month = days_in_month($end['mon'] - 1, $end['year']);
		}
		// if it is over the number of days last month, then it is over a month ago
		$over_one_month_ago = ($time_since_last_run >= ($days_last_month * $one_day));

		$days_last_year = (is_leap_year($end['year'] - 1)) ? 366 : 365;
		// if it is over the number of days last year, then it is over a year ago
		$over_one_year_ago = ($time_since_last_run >= ($days_last_year * $one_day));

		// formatted some values of the date vars
		$end_f  = Array();
		$start_f = Array();
		foreach (Array('minutes', 'hours', 'wday', 'mday', 'mon', 'year') as $field) {
			switch ($field) {
				case 'wday':
					$places = 1;
				break;
				case 'year':
					$places = 4;
				break;
				default :
					$places = 2;
				break;
			}
			$end_f[$field]  = str_pad($end[$field], $places, '0', STR_PAD_LEFT);
			$start_f[$field] = str_pad($start[$field], $places, '0', STR_PAD_LEFT);
		}


		$build_up = Array();

		$build_up['hour']['bridge'] = '('.$this->_clob2Str('COALESCE(av.custom_val, at.default_val)').')
										> '.MatrixDAL::quote('%A%'.$start_f['minutes']).'
										AND ('.$this->_clob2Str('COALESCE(av.custom_val, at.default_val)').') <= '.MatrixDAL::quote('%B%59').'
										OR ('.$this->_clob2Str('COALESCE(av.custom_val, at.default_val)').')
										BETWEEN '.MatrixDAL::quote('%C%00').' AND '.MatrixDAL::quote('%D%'.$end_f['minutes']);

		$build_up['hour']['all'] = '('.$this->_clob2Str('COALESCE(av.custom_val, at.default_val)').') > '.MatrixDAL::quote('%A%'.$start_f['minutes']).'
									AND ('.$this->_clob2Str('COALESCE(av.custom_val, at.default_val)').') <= '.MatrixDAL::quote('%B%'.$end_f['minutes']);

		$build_up['day']['bridge'] = str_replace(Array('%A%', '%B%', '%C%', '%D%'), Array('%A%'.$start_f['hours'].':', '%B%23:', '%C%00:', '%D%'.$end_f['hours'].':'), $build_up['hour']['bridge']);
		$build_up['day']['all'] = str_replace(Array('%A%', '%B%'), Array('%A%'.$start_f['hours'].':', '%B%'.$end_f['hours'].':'), $build_up['hour']['all']);

		$build_up['week']['bridge'] = str_replace(Array('%A%', '%B%', '%C%', '%D%'),
												  Array('%A%'.$start_f['wday'].' ', '%B%6 ', '%C%0 ', '%D%'.$end_f['wday'].' '),
												  $build_up['day']['bridge']);

		$build_up['week']['all'] = str_replace(Array('%A%', '%B%'),
											   Array('%A%'.$start_f['wday'].' ', '%B%'.$end_f['wday'].' '),
											   $build_up['day']['all']);

		$build_up['month']['bridge'] = str_replace(Array('%A%', '%B%', '%C%', '%D%'),
												   Array('%A%'.$start_f['mday'].' ', '%B%'.days_in_month($start['mon'], $start['year']).' ', '%C%00 ', '%D%'.$end_f['mday'].' '),
												   $build_up['day']['bridge']);

		$build_up['month']['all'] = str_replace(Array('%A%', '%B%'),
												Array('%A%'.$start_f['mday'].' ', '%B%'.$end_f['mday'].' '),
												$build_up['day']['all']);

		$build_up['year']['bridge'] = str_replace(Array('%A%', '%B%', '%C%', '%D%'),
												  Array('%A%'.$start_f['mon'].'-', '%B%12-', '%C%00-', '%D%'.$end_f['mon'].'-'),
												  $build_up['month']['bridge']);

		$build_up['year']['all'] = str_replace(Array('%A%', '%B%'),
											   Array('%A%'.$start_f['mon'].'-', '%B%'.$end_f['mon'].'-'),
											   $build_up['month']['all']);

		$build_up['one_off']['all'] = str_replace(Array('%A%', '%B%'),
												  Array('%A%'.$start_f['year'].'-', '%B%'.$end_f['year'].'-'),
												  $build_up['year']['all']);

		$type = 'all';

		if ($over_one_hour_ago) {
			$hourly = '('.$this->_clob2Str('COALESCE(av.custom_val, at.default_val)').') LIKE '.MatrixDAL::quote('HR=%');
		} else {
			// if it has ticked over the hour since the last call
			$type = ($end['minutes'] < $start['minutes']) ? 'bridge' : 'all';
			$hourly = preg_replace('/%[ABCD]%/', 'HR=', $build_up['hour'][$type]);
		}


		if ($over_one_day_ago) {
			$daily = '('.$this->_clob2Str('COALESCE(av.custom_val, at.default_val)').') LIKE '.MatrixDAL::quote('DL=%');
		} else {
			// if it has ticked over the day since the last call
			$type = ($end['hours'] < $start['hours']) ? 'bridge' : 'all';
			$daily = preg_replace('/%[ABCD]%/', 'DL=', $build_up['day'][$type]);
		}


		if ($over_one_week_ago) {
			$weekly = '('.$this->_clob2Str('COALESCE(av.custom_val, at.default_val)').') LIKE '.MatrixDAL::quote('WK=%');
		} else {
			// if it has ticked over the week since the last call
			$type = ($end['wday'] < $start['wday']) ? 'bridge' : 'all';
			$weekly = preg_replace('/%[ABCD]%/', 'WK=', $build_up['week'][$type]);
		}


		if ($over_one_month_ago) {
			$monthly = '('.$this->_clob2Str('COALESCE(av.custom_val, at.default_val)').') LIKE '.MatrixDAL::quote('MT=%');
		} else {
			// if it has ticked over the month since the last call
			$type = ($end['mday'] < $start['mday']) ? 'bridge' : 'all';
			$monthly = preg_replace('/%[ABCD]%/', 'MT=', $build_up['month'][$type]);
		}


		if ($over_one_year_ago) {
			$yearly = '('.$this->_clob2Str('COALESCE(av.custom_val, at.default_val)').') LIKE '.MatrixDAL::quote('YR=%');
		} else {
			// if it has ticked over the year since the last call
			$type = ($end['mon'] < $start['mon']) ? 'bridge' : 'all';
			$yearly = preg_replace('/%[ABCD]%/', 'YR=', $build_up['year'][$type]);
		}

		$one_off = preg_replace('/%[ABCD]%/', 'OO=', $build_up['one_off']['all']);

		$every_time = '('.$this->_clob2Str('COALESCE(av.custom_val, at.default_val)').') = '.MatrixDAL::quote('ET=');

		// repeating cron job with time interval option added to the query str
		$repeating_time_interval = 'UPPER(av.custom_val) LIKE '.MatrixDAL::quote('%TI=%');

		$sql   = 'SELECT l.linkid, a.assetid, a.type_code
				  FROM ('.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l
						INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast a ON l.minorid = a.assetid)
						INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_attr at ON at.type_code = a.type_code
						LEFT OUTER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_attr_val av ON (av.assetid = a.assetid AND av.attrid = at.attrid)';
		$where = 'l.majorid = '.MatrixDAL::quote($this->id).'
				  AND a.type_code IN (
									SELECT type_code
									FROM sq_ast_typ_inhd
									WHERE inhd_type_code = '.MatrixDAL::quote('cron_job').'
									)
				  AND at.name = '.MatrixDAL::quote('when').'
				  AND ('.$hourly.'
					OR '.$daily.'
					OR '.$weekly.'
					OR '.$monthly.'
					OR '.$yearly.'
					OR '.$one_off.'
					OR '.$every_time.'
					OR '.$repeating_time_interval.'
					)';

		if (!empty($ignore_jobs)) {
			for ($i = 0; $i < count($ignore_jobs); $i++) {
				$ignore_jobs[$i] = MatrixDAL::quote($ignore_jobs[$i]);
			}
			$where .= ' AND a.assetid NOT IN ('.implode(',', $ignore_jobs).')';
		}

		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'av');

		try {
			$result = MatrixDAL::executeSqlAll($sql.$where);
		} catch (DALException $e) {
			return Array();
		}

		$GLOBALS['CRON_MANAGER_GET_JOBS_TO_RUN_START_TS'] = $start_ts;
		unset($GLOBALS['CRON_MANAGER_GET_JOBS_TO_RUN_START_TS']);

		return $result;

	}//end getJobsToRun()


	/**
	* This function captures all errors and warnings that occur during the execution of run()
	*
	* @param int	$err_no		The type of error (E_*)
	* @param string	$err_msg	The error message
	* @param string	$err_file	The file the error occured in
	* @param string	$err_line	The line the error occured on
	*
	* @return void
	* @access private
	* @see run()
	*/
	public function _errorHandler($err_no, $err_msg, $err_file, $err_line)
	{

		$terminate = ((E_USER_ERROR | E_ERROR) & $err_no);

		// if the function didn't have an '@' prepended OR if we are about to terminate
		// catch the error
		if ((error_reporting() & $err_no) || $terminate) {

			// Strip out the file path begining
			$err_file = hide_system_root($err_file);
			$err_msg  = hide_system_root($err_msg);

			$err_name = get_error_name($err_no);

			// send a report to the system error log
			if (ini_get('log_errors')) {
				$text_msg = strip_tags(preg_replace(Array('/<br\\/?>/i', '/<p[^>]*>/i'), Array("\n", "\n\n"), $err_msg));
				log_error($text_msg, $err_no, $err_file, $err_line, $this->error_log_file_name);
			}

			// if they haven't put tags in the err msg assume it to be plain text
			$err_msg = strip_tags(preg_replace(Array('/<br\\/?>/i', '/<p[^>]*>/i'), Array("\n", "\n\n"), $err_msg));
			$lines = explode("\n", $err_msg);
			$len = 7 + strlen($err_file);
			$len = max($len, 7 + strlen($err_line));
			foreach ($lines as $line) {
				$len = max($len, strlen($line));
			}
			$len += 2;
			$str =	'+'.str_repeat('-', $len)."+\n".
					'| '.$err_name.str_repeat(' ', $len - 2 - strlen($err_name))." |\n".
					'|'.str_repeat('-', $len)."|\n".
					'| File : '.$err_file.str_repeat(' ', $len - 9 - strlen($err_file))." |\n".
					'| Line : '.$err_line.str_repeat(' ', $len - 9 - strlen($err_line))." |\n".
					'|'.str_repeat('-', $len)."|\n";
			foreach ($lines as $line) {
				$str .=  '| '.$line.str_repeat(' ', $len - 2 - strlen($line))." |\n";
			}

			$str .= '+'.str_repeat('-', $len)."+\n";

			if (!isset($this->_tmp['running_errors'])) {
				$this->_tmp['running_errors'] = '';
			}
			$this->_tmp['running_errors'] .= $str;

		}//end if error_reporting

		// if this is an irrercoverable error, send a distress email, delete ourselves and die
		if ($terminate && empty($this->_tmp['terminate_error'])) {
			$this->_tmp['terminate_error'] = TRUE; // stop possible recursion

			$root_user	= $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
			$sys_admins	= $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('system_user_group');

			$ms  = $GLOBALS['SQ_SYSTEM']->getMessagingService();
			$msg = $ms->newMessage();
			$msg_reps = Array(
							'system_name'		=> SQ_CONF_SYSTEM_NAME,
							'error_log_path'	=> SQ_LOG_PATH.'/'.$this->error_log_file_name.SQ_CONF_LOG_EXTENSION,
							'root_urls'			=> str_replace("\n", ',', SQ_CONF_SYSTEM_ROOT_URLS),
						);
			$msg->replacements = $msg_reps;

			// userid zero sends a msg to SQ_CONF_DEFAULT_EMAIL and SQ_CONF_TECH_EMAIL
			$msg->to   = Array(0, $root_user->id, $sys_admins->id);
			$msg->from = 0; // a system message
			$msg->type = 'cron.term';

			$msg->send();

			exit(1);

		}//end if

	}//end _errorHandler()


	/**
	* get an integer value in Last Run attribute
	*
	* @return int
	* @access public
	*/
	function getLastRun() {
		return (int) $this->attr('last_run');
	}

}//end class

?>
