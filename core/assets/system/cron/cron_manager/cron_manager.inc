<?php

require_once SQ_CORE_PACKAGE_PATH.'/system/cron/cron_job/cron_job.inc';

/**
* Cron_Manager
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
* @subpackage __core__
*/
class Cron_Manager extends Asset
{
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
		$sql = 'SELECT count(*) FROM '.SQ_TABLE_RUNNING_PREFIX.'asset '.$where;
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
	* Sets a permission for the passed user or user_group
	*
	* @param int		$userid		the id of the user or user_group to add the permission for
	* @param string		$permission	the permission code you are adding
	* @param string		$access		whether this permission grants access or not ('1' or '0')
	*
	* @return boolean
	* @access public
	*/
	function setPermission($userid, $permission, $access)
	{
		trigger_error('You cannot set permissions on the '.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name'), E_USER_WARNING);
		return false;
	}// end setPermission()


	/**
	* Sets a permission for the passed user or user_group
	*
	* @param object Cron_Job	$job		a reference to the cron job to add to add to the system
	* @param object User		$user		a reference to the user that this job will run as
	*
	* @return boolean
	* @access public
	*/
	function &addJob(&$job, &$user)
	{
		if (!is_a($job, 'cron_job')) {
			trigger_error('"'.$job->type().'" is not an asset type that inherites from "cron_job"', E_USER_WARNING);
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$link = Array('asset' => &$this, 'link_type' => SQ_LINK_TYPE_3, 'exclusive' => '1');
		if ($job->create($link)) {
			if ($job->setRunningUser($user)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
				return true;
			}
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		return false;

	}// end addJob()


	/**
	* Returns the time of the next run of the cron manager
	*
	* @return int timestamp
	* @access public
	*/
	function timeOfNextRun()
	{
		return (int) $this->attr('last_run') + (int) $this->attr('refresh_time');
	}// end


	/**
	* Run's the jobs that need executing in the system
	*
	* @access public
	*/
	function run()
	{
		// if we are currently running then we should just leave this alone
		if ($this->attr('running')) return;

		// now update this last run time, so that we know for next time
		if (!$this->acquireLock(0, true)) {
			trigger_error('Unable to acquire lock of "'.$this->name.'", aborting run', E_USER_WARNING);
			return;
		}
		$this->setAttrValue('running', true);
		$this->releaseLock();

		$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
		$am = &$GLOBALS['SQ_SYSTEM']->am;

		$set_last_run = true;

		// first get some times
		$last_run = (int) $this->attr('last_run');
		$this_run = time();
		$next_run = $this_run + (int) $this->attr('refresh_time');

		$jobs = $this->getJobsToRun($last_run, $this_run);
		pre_echo("JOBS : ".var_export($jobs, true));


		$failed_runs = Array(); // Array([assetid] => [number of failed runs]);

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

				// If they want to have another go
				} elseif ($result & SQ_CRON_JOB_RETRY) {

					// if they resulted in an error, we need to check this hasn't happened before
					if ($result & SQ_CRON_JOB_ERROR) {

						if (isset($failed_runs[$cron_job->id])) {
							$failed_runs[$cron_job->id]++;
						} else {
							$failed_runs[$cron_job->id] = 1;
						}

						// if they have had their fair share get rid of them
						if ($failed_runs[$cron_job->id] < (int) $this->attr('num_failed_attempts')) {
							$remove_jobs[] = $job_info['linkid'];
						}// end if

					}// end if error

				}// end if

				$am->unregisterAsset($cron_job);

			}// end foreach

			// send all the messages for this run
			$ms->closeQueue();


break;

			if (!empty($remove_jobs)) {
				// now update this last run time, so that we know for next time
				if (!$this->acquireLock(0, true)) {
					trigger_error('Unable to acquire lock of "'.$this->name.'", aborting run', E_USER_WARNING);
					return;
				}
				foreach($remove_jobs as $linkid) {
					$this->deleteLink($linkid);
				}

				$this->releaseLock();

			}// end if

			// if we have gone into the next run's time then we need to do it's work as well
			if (time() > $next_run) {
				$this_run = $next_run;
				$next_run += (int) $this->attr('refresh_time');
			}

			$jobs = $this->getJobsToRun($last_run, $this_run);
			pre_echo("JOBS : ".var_export($jobs, true));

		}// end while


		// now update this last run time, so that we know for next time
		if (!$this->acquireLock(0, true)) {
			trigger_error('Unable to acquire lock of "'.$this->name.'", aborting run', E_USER_WARNING);
			return;
		}
		$this->setAttrValue('running', false);
		$set_last_run = false;
		if ($set_last_run) $this->setAttrValue('last_run', time());
		$this->releaseLock();

	}// end run()


	/**
	* Get the jobs that need to be run for the passed time period
	*
	* @param int	$start_ts	the starting timestamp of the time period
	* @param int	$end_ts		the ending timestamp of the time period
	*
	* @return Array()
	* @access public
	*/
	function getJobsToRun($start_ts, $end_ts)
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

}//end class
?>
