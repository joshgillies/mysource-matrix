<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: cron_manager.inc,v 1.21 2003/10/02 05:45:41 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/system/cron/cron_job/cron_job.inc';

/**
* Cron_Manager
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Cron_Manager extends Asset
{
	/**
	* @var string suffix to data dir where the error log should be stored when we are run()ning
	*/
	var $error_log_path = 'private/logs/cron_errors.log';


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Cron_Manager($assetid=0)
	{
		$this->Asset($assetid);
	}


	/**
	* Create this asset
	*
	* @param array()	&$link	information used to create the initial link
	*
	* @see Asset::create()
	* @return mixed int or false
	* @access public
	*/
	function create(&$link)
	{

		$this->_tmp[__CLASS__.'_in_create'] = true;

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$where = 'type_code = '.$db->quote($this->type());
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);
		$sql = 'SELECT COUNT(*) FROM '.SQ_TABLE_RUNNING_PREFIX.'asset '.$where;
		$exists = $db->getOne($sql);
		if (DB::isError($exists)) trigger_error($exists->getMessage().'<br/>'.$exists->getUserInfo(), E_USER_ERROR);

		if ($exists) {
			trigger_error('Unable to create another '.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name').', you are only allowed one', E_USER_WARNING);
			return false;
		}

		$ret_val = parent::create($link);
		unset($this->_tmp[__CLASS__.'_in_create']);
		return $ret_val;

	}//end create()


	/**
	* Returns name of the asset
	*
	* @param bool	$short_name	whether or not we are after the shortname or the full name
	*
	* @see Asset::_getName()
	* @access public
	*/
	function _getName($short_name=false)
	{
		return $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name');
	}//end _getName();


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks()
	{
		return Array(SQ_LINK_TYPE_1 => Array(),
					 SQ_LINK_TYPE_2 => Array(),
					 SQ_LINK_TYPE_3 => Array('cron_job' => Array('card' => 'M', 'exclusive' => true)),
					 SQ_LINK_NOTICE => Array()
					);

	}//end _getAllowedLinks()


	/**
	* Creates a duplicate of this asset. It DOES NOT duplicate the links that this asset has either to it or from it
	* Returns a reference to the new object or if an error occured FALSE
	*
	* @param bool	$dupe_directory	whether or not to duplicate thia assets data directory
	*
	* @return mixed object Asset or NULL
	* @access public
	* @see Asset::duplicate()
	*/
	function &duplicate($dupe_directory=true)
	{
		trigger_error('Unable to duplicate '.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name').', you are only allowed one', E_USER_WARNING);
		$null = NULL; // 'cause we need to return by reference :)
		return $null;
	}//end duplicate()


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
		trigger_error('Unable to morph '.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name'), E_USER_WARNING);
		return false;

	}//end morph()


	/**
	* Moves a link from one place in this assets links order to another position
	*
	* @param int	$linkid		the link id of the link to move about
	* @param string	$sort_order	the position in the links list that this link should take,
	*							if less than zero places at end of list
	*
	* @see Asset::moveLinkPos()
	*
	* @return boolean
	* @access public
	*/
	function moveLinkPos($linkid, $sort_order=-1)
	{
		trigger_error('You cannot move assets\'s around in '.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name'), E_USER_WARNING);
		return false;

	}//end moveLinkPos()


	/**
	* Set the value for the passed variable to the passed value
	*
	* @param string		$name				the name of the attribute
	* @param string		$value				the new value of the attribute
	*
	* @return boolean
	* @access public
	*/
	function setAttrValue($name, $value)
	{
		if (!$GLOBALS['SQ_SYSTEM']->userRoot()) {
			trigger_error('Unable to set "'.$name.'". You can only set the attributes of the '.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name').' if you are the Root User', E_USER_WARNING);
			return false;
		}// end if

		// the refresh time has some restrictions on it, because otherwise painting the when box would be a pain
		if ($name == 'refresh_time') {
			$value = (int) $value;

			$one_hour = 60 * 60;
			$one_day  = 24 * $one_hour;

			if ($value >= $one_day) {
				// if we are over a day we must be a multiple of one day
				if ($value % $one_day) {
					trigger_error('You can only have a refresh time of over one day if it\'s a multiple of a day ('.$one_day.' seconds)', E_USER_WARNING);
					return false;
				}
			} else if ($value >= $one_hour) {
				// if we are over an hour we must be a multiple of one hour
				if ($value % $one_hour || $one_day % $value) {
					trigger_error('You can only have a refresh time of over one hour if it\'s a multiple of an hour ('.$one_hour.' seconds) and the number of hours divides evenly into one day', E_USER_WARNING);
					return false;
				}

			// if we less than an hour we must divide evenly into an hour
			} else if ($one_hour % $value) {
				trigger_error('You can only have a refresh time of under one hour if it divides evenly into one hour ('.$one_hour.' seconds)', E_USER_WARNING);
				return false;
			}
		}// endi f

		return parent::setAttrValue($name, $value);

	}// end setAttrValue()


	/**
	* Adds a job to the cron manager
	*
	* @param object Cron_Job	$job		a reference to the cron job to add to add to the system
	* @param object User		$user		a reference to the user that this job will run as
	*
	* @return boolean
	* @access public
	*/
	function addJob(&$job, &$user)
	{
		if (!is_a($job, 'cron_job')) {
			trigger_error('"'.$job->type().'" is not an asset type that inherites from "cron_job"', E_USER_WARNING);
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$link = Array('asset' => &$this, 'link_type' => SQ_LINK_TYPE_3, 'exclusive' => '1');
		if ($job->create($link)) {
			if ($GLOBALS['SQ_SYSTEM']->am->acquireLock($job->id) && $job->setRunningUser($user)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
				return true;
			}
			$GLOBALS['SQ_SYSTEM']->am->releaseLock($job->id);
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		return false;

	}// end addJob()


	/**
	* Removes a job form the cron manager
	*
	* @param object Cron_Job	$job		a reference to the cron job to add to add to the system
	*
	* @return boolean
	* @access public
	*/
	function removeJob(&$job)
	{
		if (!is_a($job, 'cron_job')) {
			trigger_error('"'.$job->type().'" is not an asset type that inherites from "cron_job"', E_USER_WARNING);
			return false;
		}

		$link = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($this->id, $job->id);
		if (empty($link)) return false;

		return $this->deleteLink($link['linkid']);

	}// end removeJob()


	/**
	* Returns an array of cron jobs that are active under cron manager
	*
	* @param string|array	$type_code			the type of cron job, if an array returns link if matches any
	*											of the array values
	* @param bool			$strict_type_code	whether we are finding assets that are just a $type_code
	*											or $type_code and any of it's sub-classes
	*
	* @return Array() returns an array of references to the selected cron job type
	* @see Asset::getLinks()
	* @access public
	*/
	function getJobs($type_code='cron_job', $strict_type_code=true)
	{
		$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_3, $type_code, $strict_type_code, 'major', null, null, true);
		$jobs = Array();
		foreach($links as $link) {
			$job = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
			if (is_null($job)) continue;
			$jobs[] = &$job;
		}// end foreach

		return $jobs;

	}// end getJobs()


	/**
	* Returns the time of the next run of the cron manager
	*
	* @return int timestamp
	* @access public
	*/
	function timeOfNextRun()
	{
		return max((int) $this->attr('last_run'), (int) $this->attr('epoch')) + (int) $this->attr('refresh_time');
	}// end timeOfNextRun()


	/**
	* Returns a string with the readable version of how often the cron runs
	*
	* @return string
	* @access public
	*/
	function readableRefreshTime()
	{
		$refresh_time = (int) $this->attr('refresh_time');
		if ($refresh_time >= 86400) {
			$num = ($refresh_time / 86400);
			return ($num > 1) ? $num.' days' : 'day';
		}
		if ($refresh_time >= 3600) {
			$num = ($refresh_time / 3600);
			return ($num > 1) ? $num.' hours' : 'hour';
		}
		if ($refresh_time >= 60) {
			$num = ($refresh_time / 60);
			return ($num > 1) ? $num.' minutes' : 'minute';
		}
		return $refresh_time.' seconds';

	}// end timeOfNextRun()


	/**
	* Run's the jobs that need executing in the system
	*
	* @return void
	* @access public
	*/
	function run()
	{
		$start_time = time();

		$old_log_errors = ini_set('log_errors', '1');
		$old_error_log  = ini_set('error_log',  SQ_DATA_PATH.'/'.$this->error_log_path);
		set_error_handler(Array(&$this, '_errorHandler'));

		if (!empty($GLOBALS['SQ_SYSTEM']->tm) && $GLOBALS['SQ_SYSTEM']->tm->inTransaction('db')) {
			trigger_error('The Cron Manager should not be run() while nested within any transactions.<br>This stops each job from being independant of each other', E_USER_WARNING);
			return;
		}

		if ($start_time < (int) $this->attr('epoch')) {
			trigger_error('The Cron Manager run() aborted because it\'s Epoch is still in the future', E_USER_WARNING);
			return;
		}

		// if we are currently running then we should just leave this alone
		if ($this->attr('running')) {

			// if we have been stopped too many times without the while loop below
			// looping over
			if ((int) $this->attr('run_check') >= (int) $this->attr('warn_after_num_run_checks')) {

				$root_user  = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
				$sys_admins = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('system_user_group');

				$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
				$msg = $ms->newMessage();
				$msg->subject = 'POTENTIAL CRON MANAGER DEAD LOCK ISSUE';
				$msg->body	  = 'The cron manager for the system "'.SQ_CONF_SYSTEM_NAME.'" has attempted to run '.$this->attr('warn_after_num_run_checks').' times, but has not been able to for one of two reasons' ."\n"
								.'1) An existing cron manager is currently running an extremely long running job.'."\n"
								.'2) Some thing has caused the cron manager to exit uncleanly and the lock wasn\'t able to be removed.'."\n"
								.'Check the cron error log (in [DATA DIR]/'.$this->error_log_path.') for any information'."\n"
								."\n"
								.'(System Root Urls : '.str_replace("\n", ',', SQ_CONF_SYSTEM_ROOT_URLS).')'."\n";

				// userid zero sends a msg to SQ_CONF_DEFAULT_EMAIL and SQ_CONF_TECH_EMAIL
				$msg->to   = Array(0, $root_user->id, $sys_admins->id);
				$msg->from = 0; // a system message

				$msg->send();

			}// end if run_check


			// now update this last run time, so that we know for next time
			if ($GLOBALS['SQ_SYSTEM']->am->acquireLock($this->id, '', 0, true)) {
				$this->setAttrValue('run_check', (int) $this->attr('run_check') + 1);
				$this->saveAttributes();
				$GLOBALS['SQ_SYSTEM']->am->releaseLock($this->id);

			} else {
				trigger_error('Unable to acquire lock of "'.$this->name.'"', E_USER_WARNING);

			}

		// we aren't already running
		} else {

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
				}// end if
			}// end if

			// now get the lock, set the running flag and reset the run_check
			if ($GLOBALS['SQ_SYSTEM']->am->acquireLock($this->id, '', 0, true) && $this->setAttrValue('running', true) && $this->setAttrValue('run_check', 0) && $this->saveAttributes()) {

				$GLOBALS['SQ_SYSTEM']->am->releaseLock($this->id);

				$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
				$am = &$GLOBALS['SQ_SYSTEM']->am;

				// first get some times
				$last_run = (int) $this->attr('last_run');
				$next_run = $this_run + $refresh_time;

#pre_echo("Get Jobs : ".date('d/m/Y H:i:s', $last_run).", ".date('d/m/Y H:i:s', $this_run));
				$jobs = $this->getJobsToRun($last_run, $this_run);
#pre_echo("JOBS : ".var_export($jobs, true));

				// holds the number of times that a job has failed
				// Array([assetid] => [number of failed runs]);
				$failed_runs = Array();
				// holds the assetids for jobs to ignore - because they have failed to many times
				// Array([assetid]);
				$ignore_jobs = Array();

				while (!empty($jobs)) {

					$ms->openQueue();
					$remove_jobs = Array();
					foreach($jobs as $job_info) {

						$cron_job = &$am->getAsset($job_info['assetid'], $job_info['type_code']);
						if (is_null($cron_job)) continue;

echo "RUN    : ".$cron_job->id.' .... '.$cron_job->name."\n";

						$result = $cron_job->run();

						require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
echo "RESULT : ".$result.' .... '.implode(', ', get_bit_names('SQ_CRON_JOB_', $result, true))."\n\n";

						// We have been asked to remove this cron job
						if ($result & SQ_CRON_JOB_REMOVE) {
							$remove_jobs[] = $job_info['linkid'];
							$ignore_jobs[] = $cron_job->id;

						// If they want to have another go
						} elseif ($result & SQ_CRON_JOB_RETRY) {

							// if they resulted in an error, we need to check this hasn't happened before
							if ($result & SQ_CRON_JOB_ERROR) {

								if (isset($failed_runs[$cron_job->id])) {
									$failed_runs[$cron_job->id]++;
								} else {
									$failed_runs[$cron_job->id] = 1;
								}

								// if they have had their fair share, get rid of them
								if ($failed_runs[$cron_job->id] >= (int) $this->attr('num_failed_attempts')) {
									$remove_jobs[] = $job_info['linkid'];
									$ignore_jobs[] = $cron_job->id;

									$user       = &$cron_job->getRunningUser();
									$root_user  = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
									$sys_admins = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('system_user_group');

									$msg = $ms->newMessage();
									$msg->subject = 'FORCED FAILURE of '.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($cron_job->type(), 'name');
									$msg->body	  = 'The job "'.$cron_job->name.'" (#'.$cron_job->id.') has failed to run '.$this->attr('num_failed_attempts').' times.'."\n"
													.'It will be removed from the '.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name').' and will NOT be run again';
									$msg->to   = Array($user->id, $root_user->id, $sys_admins->id);
									$msg->from = 0; // a system message

									$ms->enqueueMessage($msg);

								}// end if

							}// end if error

						// they don't want another go but they don't want to be removed either ? just ignore them
						} else {
							$ignore_jobs[] = $cron_job->id;

						}// end if

						$am->forgetAsset($cron_job);

					}// end foreach

					// send all the messages for this run
					$ms->closeQueue();

					// give it three seconds to get the lock
					$lock_count = 0;
					while(1) {
						if ($acquired = $GLOBALS['SQ_SYSTEM']->am->acquireLock($this->id, '', 0, true)) {
							break;
						} elseif ($lock_count < 3) {
							$lock_count++;
							sleep(1);
						} else {
							break;
						}
					};

					// now update this last run time, so that we know for next time
					if ($acquired) {

						$this->setAttrValue('run_check', 0);
						if (!empty($remove_jobs)) {
							foreach($remove_jobs as $linkid) {
								$this->deleteLink($linkid);
							}
						}// end if

						$this->saveAttributes();

						$GLOBALS['SQ_SYSTEM']->am->releaseLock($this->id);

					} else {
						trigger_error('Unable to acquire lock of "'.$this->name.'", aborting run', E_USER_WARNING);
						return;
					}

					// if we have gone into the next run's time then we need to do it's work as well
					if (time() > $next_run) {
						$this_run = $next_run;
						$next_run += $refresh_time;
					}

#pre_echo("Get Jobs : ".date('d/m/Y H:i:s', $last_run).", ".date('d/m/Y H:i:s', $this_run));
					$jobs = $this->getJobsToRun($last_run, $this_run, $ignore_jobs);
#pre_echo("JOBS : ".var_export($jobs, true));

				}// end while

				// now update this last run time, so that we know for next time
				if ($GLOBALS['SQ_SYSTEM']->am->acquireLock($this->id, '', 0, true)) {
					if ($this->setAttrValue('last_run', $this_run) && $this->setAttrValue('running', false) && $this->saveAttributes()) {
						$GLOBALS['SQ_SYSTEM']->am->releaseLock($this->id);
					} else {
						trigger_error('Unable to unset the last run and/or running flag of "'.$this->name.'", aborting run', E_USER_WARNING);
					}
				} else {
					trigger_error('Unable to acquire lock of "'.$this->name.'", aborting run', E_USER_WARNING);
				}

			// else aquire or set running failed
			} else {
				trigger_error('Unable to acquire lock and set running flag of "'.$this->name.'", aborting run', E_USER_WARNING);

			}// end if aquire lock

		}// end if already running

		ini_set('log_errors', $old_log_errors);
		ini_set('error_log',  $old_error_log);
		restore_error_handler();

	}// end run()


	/**
	* Get the jobs that need to be run for the passed time period
	*
	* @param int		$start_ts		the starting timestamp of the time period
	* @param int		$end_ts			the ending timestamp of the time period
	* @param Array(int)	$ignore_jobs	a list of assetids to ignore
	*
	* @return Array()
	* @access public
	*/
	function getJobsToRun($start_ts, $end_ts, $ignore_jobs=Array())
	{
		require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';
		require_once SQ_FUDGE_PATH.'/general/datetime.inc';

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$start_sql_date = '2003-07-19 00:00';
		$end_sql_date   = '2003-07-21 23:59';

#		pre_echo("START : ".date('d M Y H:i',$start_ts)."\nEND   : ".date('d M Y H:i',$end_ts)."\n");

		if ($end_ts < $start_ts) {
			trigger_error('The start time is after the end time', E_USER_WARNING);
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

#		pre_echo("over_one_hour_ago  : $over_one_hour_ago");
#		pre_echo("over_one_day_ago   : $over_one_day_ago");
#		pre_echo("over_one_week_ago  : $over_one_week_ago");
#		pre_echo("over_one_month_ago : $over_one_month_ago");
#		pre_echo("over_one_year_ago  : $over_one_year_ago");

		// formatted some values of the date vars
		$end_f  = Array();
		$start_f = Array();
		foreach(Array('minutes', 'hours', 'wday', 'mday', 'mon', 'year') as $field) {
			switch($field) {
				case 'wday': $places = 1; break;
				case 'year': $places = 4; break;
				default    : $places = 2; break;
			}
			$end_f[$field]  = str_pad($end[$field], $places, '0', STR_PAD_LEFT);
			$start_f[$field] = str_pad($start[$field], $places, '0', STR_PAD_LEFT);
		}


		$build_up = Array();

		$build_up['hour'] = Array(
			'bridge' => 'av.custom_value BETWEEN '.$db->quote('%A%'.$start_f['minutes']).' AND '.$db->quote('%B%59').' OR
						 av.custom_value BETWEEN '.$db->quote('%C%00').' AND '.$db->quote('%D%'.$end_f['minutes']),
			'all'    => 'av.custom_value BETWEEN '.$db->quote('%A%'.$start_f['minutes']).' AND '.$db->quote('%B%'.$end_f['minutes']),
		);


		$build_up['day'] = Array(
			'bridge' => str_replace(Array('%A%', '%B%', '%C%', '%D%'),
									Array('%A%'.$start_f['hours'].':', '%B%23:', '%C%00:', '%D%'.$end_f['hours'].':'),
									$build_up['hour']['bridge']),
			'all'    => str_replace(Array('%A%', '%B%'),
									Array('%A%'.$start_f['hours'].':', '%B%'.$end_f['hours'].':'),
									$build_up['hour']['all']),
		);

		$build_up['week'] = Array(
			'bridge' => str_replace(Array('%A%', '%B%', '%C%', '%D%'),
									Array('%A%'.$start_f['wday'].' ', '%B%6 ', '%C%0 ', '%D%'.$end_f['wday'].' '),
									$build_up['day']['bridge']),
			'all'    => str_replace(Array('%A%', '%B%'),
									Array('%A%'.$start_f['wday'].' ', '%B%'.$end_f['wday'].' '),
									$build_up['day']['all']),
		);

		$build_up['month'] = Array(
			'bridge' => str_replace(Array('%A%', '%B%', '%C%', '%D%'),
									Array('%A%'.$start_f['mday'].' ', '%B%'.days_in_month($start['mon'], $start['year']).' ', '%C%00 ', '%D%'.$end_f['mday'].' '),
									$build_up['day']['bridge']),
			'all'    => str_replace(Array('%A%', '%B%'),
									Array('%A%'.$start_f['mday'].' ', '%B%'.$end_f['mday'].' '),
									$build_up['day']['all']),
		);

		$build_up['year'] = Array(
			'bridge' => str_replace(Array('%A%', '%B%', '%C%', '%D%'),
									Array('%A%'.$start_f['mon'].'-', '%B%12-', '%C%00-', '%D%'.$end_f['mon'].'-'),
									$build_up['month']['bridge']),
			'all'    => str_replace(Array('%A%', '%B%'),
									Array('%A%'.$start_f['mon'].'-', '%B%'.$end_f['mon'].'-'),
									$build_up['month']['all']),
		);

		$build_up['one_off'] = Array(
			'all'    => str_replace(Array('%A%', '%B%'),
									Array('%A%'.$start_f['year'].'-', '%B%'.$end_f['year'].'-'),
									$build_up['year']['all']),
		);

		#pre_echo(var_export($build_up, true));

		$type = 'all';

		if ($over_one_hour_ago) {
			$hourly = 'av.custom_value LIKE '.$db->quote('HR=%');
		} else {
			// if it has ticked over the hour since the last call
			$type = ($end['minutes'] < $start['minutes']) ? 'bridge' : 'all';
			$hourly = preg_replace('/%[ABCD]%/', 'HR=', $build_up['hour'][$type]);
		}


		if ($over_one_day_ago) {
			$daily = 'av.custom_value LIKE '.$db->quote('DL=%');
		} else {
			// if it has ticked over the day since the last call
			$type = ($end['hours'] < $start['hours']) ? 'bridge' : 'all';
			$daily = preg_replace('/%[ABCD]%/', 'DL=', $build_up['day'][$type]);
		}


		if ($over_one_week_ago) {
			$weekly = 'av.custom_value LIKE '.$db->quote('WK=%');
		} else {
			// if it has ticked over the week since the last call
			$type = ($end['wday'] < $start['wday']) ? 'bridge' : 'all';
			$weekly = preg_replace('/%[ABCD]%/', 'WK=', $build_up['week'][$type]);
		}


		if ($over_one_month_ago) {
			$monthly = 'av.custom_value LIKE '.$db->quote('MT=%');
		} else {
			// if it has ticked over the month since the last call
			$type = ($end['mday'] < $start['mday']) ? 'bridge' : 'all';
			$monthly = preg_replace('/%[ABCD]%/', 'MT=', $build_up['month'][$type]);
		}


		if ($over_one_year_ago) {
			$yearly = 'av.custom_value LIKE '.$db->quote('YR=%');
		} else {
			// if it has ticked over the year since the last call
			$type = ($end['mon'] < $start['mon']) ? 'bridge' : 'all';
			$yearly = preg_replace('/%[ABCD]%/', 'YR=', $build_up['year'][$type]);
		}

		$one_off = preg_replace('/%[ABCD]%/', 'OO=', $build_up['one_off']['all']);

		$where = 'l.majorid = '.$db->quote($this->id).'
				  AND a.type_code IN (~SQ0~)
				  AND at.name         = '.$db->quote('when').'
				  AND ('.$hourly.'
					   OR '.$daily.'
					   OR '.$weekly.'
					   OR '.$monthly.'
					   OR '.$yearly.'
					   OR '.$one_off.'
					   )';

		if (!empty($ignore_jobs)) {
			for($i = 0; $i < count($ignore_jobs); $i++) $ignore_jobs[$i] = $db->quote($ignore_jobs[$i]);
			$where .= ' AND a.assetid NOT IN ('.implode(',', $ignore_jobs).')';
		}// endif

		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'av');
		$sql = 'SELECT l.linkid, a.assetid, a.type_code, av.custom_value as when
				FROM sq_asset_link l
						INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'asset a ON l.minorid = a.assetid
						INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'asset_attribute_value av ON a.assetid = av.assetid
						INNER JOIN '.SQ_TABLE_PREFIX.'asset_attribute at ON av.attributeid = at.attributeid
				'.$where;

		$subs = Array( 'SELECT type_code
						FROM '.SQ_TABLE_PREFIX.'asset_type_inherited
						WHERE inherited_type_code = '.$db->quote('cron_job')
						);

		$sql = db_extras_subquery($db, $sql, $subs);
		if (DB::isError($sql)) {
			trigger_error($sql->getMessage().'<br/>'.$sql->getUserInfo(), E_USER_WARNING);
			return Array();
		}

		$result = $db->getAll($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return Array();
		}

		$GLOBALS['CRON_MANAGER_GET_JOBS_TO_RUN_START_TS'] = $start_ts;
		usort($result, Array('Cron_Manager', 'getJobsToRunCmp'));
		unset($GLOBALS['CRON_MANAGER_GET_JOBS_TO_RUN_START_TS']);

		return $result;

	}// end getJobsToRun()


	/**
	* Used by usort() in getJobsToRun() to sort the results in the proper order for execution
	*
	* @param Array()	$a	first value to compare
	* @param Array()	$b	second value to compare
	*
	* @return int	negative means $a is before $b, zero means $a == $b, positive means $b is before $a
	* @see getJobsToRun()
	* @access public
	*/
	function getJobsToRunCmp($a, $b)
	{
		$a_ts = Cron_Job::getNextRun($a['when'], $GLOBALS['CRON_MANAGER_GET_JOBS_TO_RUN_START_TS']);
		$b_ts = Cron_Job::getNextRun($b['when'], $GLOBALS['CRON_MANAGER_GET_JOBS_TO_RUN_START_TS']);
		if ($a_ts == $b_ts) return 0;
		return ($a_ts < $b_ts) ? -1 : 1;
	}// end getJobsToRunCmp()


	/**
	* This function captures all errors and warnings that occur during the execution of run()
	*
	* @param int	$err_no		The type of error (E_*)
	* @param string	$err_msg	The error message
	* @param string	$err_file	The file the error occured in
	* @param string	$err_line	The line the error occured on
	*
	* @returns void
	* @access private
	* @see run()
	*/
	function _errorHandler($err_no, $err_msg, $err_file, $err_line)
	{

		$terminate = ((E_USER_ERROR | E_ERROR) & $err_no);

		// if the function didn't have an '@' prepended OR if we are about to terminate
		// catch the error
		if (error_reporting() || $terminate) {

			// Strip out the file path begining
			$err_file = str_replace(Array("\r", SQ_SYSTEM_ROOT), Array('', '[SYSTEM_ROOT]'), $err_file);
			$err_msg  = str_replace(Array("\r", SQ_SYSTEM_ROOT), Array('', '[SYSTEM_ROOT]'), $err_msg);

			$err_name = '';
			switch($err_no) {
				case E_USER_ERROR	: $err_name = SQ_SYSTEM_SHORT_NAME.' Error';	break;
				case E_USER_WARNING	: $err_name = SQ_SYSTEM_SHORT_NAME.' Warning';	break;
				case E_USER_NOTICE	: $err_name = SQ_SYSTEM_SHORT_NAME.' Notice';	break;
				case E_ERROR		: $err_name = 'PHP Error';						break;
				case E_WARNING		: $err_name = 'PHP Warning';					break;
				case E_NOTICE		: $err_name = 'PHP Notice';						break;
				default				: $err_name = 'Unknown Error Type - '.$err_no;
			}//end switch

			// send a report to the system error log
			if (ini_get('log_errors')) {
				$text_msg = strip_tags(preg_replace(Array('/<br\\/?>/i', '/<p[^>]*>/i'), Array("\n", "\n\n"), $err_msg));
				$log_msg  = date('D d/m/Y H:i:s').' ['.strtolower($err_name).'] ('.$err_file.':'.$err_line.') - '.$text_msg;
				$log_msg  = str_replace('[SYSTEM_ROOT]', '', $log_msg);
				error_log($log_msg);
			}

			// if they haven't put tags in the err msg assume it to be plain text
			$err_msg = strip_tags(preg_replace(Array('/<br\\/?>/i', '/<p[^>]*>/i'), Array("\n", "\n\n"), $err_msg));
			$lines = explode("\n", $err_msg);
			$len = 7 + strlen($err_file);
			$len = max($len, 7 + strlen($err_line));
			foreach($lines as $line) $len = max($len, strlen($line));
			$len += 2;
			$str =	'+'.str_repeat('-', $len)."+\n".
					'| '.$err_name.str_repeat(' ', $len - 2 - strlen($err_name))." |\n".
					'|'.str_repeat('-', $len)."|\n".
					'| File : '.$err_file.str_repeat(' ', $len - 9 - strlen($err_file))." |\n".
					'| Line : '.$err_line.str_repeat(' ', $len - 9 - strlen($err_line))." |\n".
					'|'.str_repeat('-', $len)."|\n";
			foreach($lines as $line) {
				$str .=  '| '.$line.str_repeat(' ', $len - 2 - strlen($line))." |\n";
			}

			$str .= '+'.str_repeat('-', $len)."+\n";

			if (!isset($this->_tmp['running_errors'])) $this->_tmp['running_errors'] = '';
			$this->_tmp['running_errors'] .= $str;

		}// end error_reporting

		// if this is an irrercoverable error, send a distress email, delete ourselves and die
		if ($terminate && empty($this->_tmp['terminate_error'])) {
			$this->_tmp['terminate_error'] = true; // stop possible recursion

			$root_user  = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
			$sys_admins = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('system_user_group');

			$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
			$msg = $ms->newMessage();
			$msg->subject = 'TERMINATION of '.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name');
			$msg->body	  = 'The cron manager for the system "'.SQ_CONF_SYSTEM_NAME.'" fatally triggered an error, terminating the cron manager\'s run.'."\n"
							.'Check the cron error log (in [DATA DIR]/'.$this->error_log_path.') for any information'."\n"
							."\n"
							.'(System Root Urls : '.str_replace("\n", ',', SQ_CONF_SYSTEM_ROOT_URLS).')'."\n";

			// userid zero sends a msg to SQ_CONF_DEFAULT_EMAIL and SQ_CONF_TECH_EMAIL
			$msg->to   = Array(0, $root_user->id, $sys_admins->id);
			$msg->from = 0; // a system message

			$msg->send();

			exit(1);

		}// end if

	}// end _errorHandler()

}//end class
?>
