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
* $Id: cron_job.inc,v 1.64.6.1 2013/09/05 00:51:26 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';

define('SQ_CRON_JOB_ERROR',			 1);// run()/_exec() failed
define('SQ_CRON_JOB_COMPLETED',		 2);// run()/_exec() successful and complete
define('SQ_CRON_JOB_NOT_COMPLETE',	 4);// run()/_exec() successful and but still more to do
define('SQ_CRON_JOB_REMOVE',		 8);// cron job can be removed from cron manager
define('SQ_CRON_JOB_RETRY',			16);// cron job requests to be retried (only valid with ERROR and NOT_COMPLETE)


/**
* Cron_Job
*
* Purpose
*	Base class that all the cron jobs extend. Cron manager executes cron jobs at required times.
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.64.6.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Cron_Job extends Asset
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
	* Perform any pre-validation required during the creation of this asset
	*
	* Cron jobs ensure a type and time has been set before they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	protected function _preCreateCheck(Array &$link)
	{
		if (!parent::_preCreateCheck($link)) return FALSE;

		if (!$this->attr('type') || !$this->attr('when')) {
			trigger_localised_error('CRON0002', E_USER_ERROR);
			return FALSE;
		}

		return TRUE;

	}//end _preCreateCheck()


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	public function _getAllowedLinks()
	{
		return Array(
				SQ_LINK_TYPE_1	=> Array(),
				SQ_LINK_TYPE_2	=> Array(),
				SQ_LINK_TYPE_3	=> Array(),
				SQ_LINK_NOTICE	=> Array('user' => Array('card' => 1)),
			   );

	}//end _getAllowedLinks()


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
	* Morph's this asset to either one of it's parent type or one of it's children types
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
	* Can the current user forceably acquire the current lock on this asset?
	*
	* @param string	$lock_type	any valid lock type for this asset's type,
	*							plus 'all' which will attempt to acquire all locks that this asset has
	*
	* @return boolean
	* @access public
	*/
	public function canForceablyAcquireLock($lock_type)
	{
		return FALSE;

	}//end canForceablyAcquireLock()


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
		if ($name == 'when') {

			$value = (string) $value;
			if ($this->attr('type') != 'one_off' && substr($value, 0, 2) == 'OO') {
				trigger_localised_error('CRON0009', E_USER_NOTICE);
				return FALSE;
			}//end if
			if ($this->attr('type') == 'one_off' && substr($value, 0, 2) != 'OO') {
				trigger_localised_error('CRON0043', E_USER_NOTICE);
				return FALSE;
			}// end if

			/*
				time interval can only be set in a repeating cron job
				when str format: "TI=nextrun timestamp,time interval,time interval unit
				time in seconds between each run = time interval * time interval unit
				for example, repeating time interval cron job that runs
					1. every 15 minutes
						TI=1147416120,15,60
					2. every hour
						TI=1147416120,1,3600
			*/
			if ($this->attr('type') != 'repeating' && substr($value, 0, 2) == 'TI') {
				trigger_localised_error('CRON0045', E_USER_NOTICE);
				return FALSE;
			}//end if

			// if this is a one off job
			// then we need to make sure that it is happening on or before the
			// next time that the cron job is getting run
			if ($this->attr('type') == 'one_off') {

				$when_str = $value;

				$cron_mgr = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
				if (is_null($cron_mgr)) return FALSE;

				// is there a duration set? if so, recalculate the when time
				if (strpos($when_str, '!')) {
					// user chose a duration instead of a fixed date, so lets work out what that is
					$now = time();
					$duration = substr($value, strpos($value, '!') + 2);
					$period = substr($value, strpos($value, '!') + 1, 1);

					switch ($period) {
						case 'i':
							$then = $now + $duration*60;
						break;
						case 'h':
							$then = $now + $duration*3600;
						break;
						case 'd':
							$then = $now + $duration*86400;
						break;
						case 'w':
							$then = $now + $duration*604800;
						break;
						case 'm':
							$then = strtotime('+'.$duration.' months');
						break;
						case 'y':
							$then = strtotime('+'.$duration.' years');
						break;
					}//end switch

					// If the job is scheduled to run immediately, make sure the current time is not as same as
					// the Cron Manager's last run, otherwise this job will never be picked (see bug #5943)
					if ($now == $then && $cron_mgr->attr('last_run') == strtotime(date('Y-m-d H:i', $now))) {
						// Push it to the next Cron run
						$then += $cron_mgr->attr('refresh_time');
					}

					$when_str = 'OO=';

					foreach (Array('year' => 'Y', 'month' => 'm', 'day' => 'd', 'hour' => 'H', 'minute' => 'i') as $field => $format_string) {
						$fields[$field] = date($format_string, $then);
						switch ($field) {
							case 'year' :
								$when_str .= $fields[$field].'-';
							break;
							case 'month' :
								$when_str .= $fields[$field].'-';
							break;
							case 'day' :
								$when_str .= $fields[$field].' ';
							break;
							case 'hour' :
								$when_str .= $fields[$field].':';
							break;
							case 'minute' :
								$minutes = date($format_string, $then);
								$interval = $cron_mgr->attr('refresh_time') / 60;
								while ($minutes % $interval != 0) {
									$minutes++;
								}
								// we have removed an "if" block from here
								// we need not worry about the minutes greater than 60
								// PHP mktime() will take care if we pass minutes like
								// 60 or greater, 11:60 and 12:00 OR 11:66 and 12:06 
								// will give us same results as timestamp outta mktime
								$fields[$field] = $minutes;
								$when_str .= $fields[$field];
							break;
						}// end switch

					}//end foreach

				}//end if

				$when = Cron_Job::getWhenArr($when_str);

				// the next run time is stored in the encoded when str, for a repeating time interval cron job (TI)
				$when_ts = mktime($when['hour'], $when['minute'], 0, $when['month'], $when['day'], $when['year']);
				if ($this->attr('type') == 'repeating' && substr($value, 0, 2) == 'TI') {
					$when_ts = $when['duration'];
				}

				if ($when_ts < (int) $cron_mgr->attr('last_run')) {
					require_once SQ_FUDGE_PATH.'/general/datetime.inc';
					trigger_localised_error('CRON0042', E_USER_WARNING, $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name'), easy_datetime((int) $cron_mgr->attr('last_run')));
					return FALSE;
				}//end if

				// instead of using $when_str, convert back the date 
				// from when_ts this will ensure us that the date if
				// passed something like 11:64 to mktime will be
				// converted back to 12:04 below
				// for proper displaying on backend
				$value = 'OO='.date('Y-m-d H:i', $when_ts);

			}//end if

			// Make sure exact date/times have the right precision, because later DB queries depend on this
			if (0 === strpos($value, 'OO=')) {
				$value = substr($value, 0, 19);
			}

		}//end if


		return parent::setAttrValue($name, $value);

	}//end setAttrValue()


	/**
	* Can this asset have its last significant link removed (putting it in the trash)?
	* For Cron Jobs this can only be done by the running user or a sys admin, and
	* only if the Cron Job is not read-only
	*
	* @return boolean
	* @access public
	*/
	public function canDelete()
	{
		// read-only?
		if ($this->attr('read_only')) return FALSE;

		$user = $this->getRunningUser();
		if (!is_null($user) && $GLOBALS['SQ_SYSTEM']->currentUser($user)) {
			return TRUE;
		}

		// if this is to be run as the root user, it can only be deleted by the root user
		if (!is_null($user) && $GLOBALS['SQ_SYSTEM']->userRoot($user) && !$GLOBALS['SQ_SYSTEM']->userRoot()) {
			return FALSE;
		}

		// else root user or sys admins can delete
		if ($GLOBALS['SQ_SYSTEM']->userRoot() || $GLOBALS['SQ_SYSTEM']->userSystemAdmin()) {
			return TRUE;
		}

		return FALSE;

	}//end canDelete()


	/**
	* Gets the user that this cron job is to run as while executing
	*
	* @return mixed object|NULL
	* @access public
	*/
	public function getRunningUser()
	{
		$null = NULL; // because we need to return by ref
		$userid = $this->attr('running_as');
		if (!$userid) return $null;
		$user = $GLOBALS['SQ_SYSTEM']->am->getAsset($userid, '', TRUE);
		if (is_null($user)) return $null;
		return $user;

	}//end getRunningUser()


	/**
	* Returns a text version of the type names to used for the when attr
	*
	* @param string	$type	two char type name from the when attr
	*
	* @return string
	* @access public
	* @static
	*/
	public static function whenTypeName($type)
	{
		switch ($type) {
			case 'ET' : return translate('every_time');
			case 'OO' : return translate('one_off');
			case 'HR' : return translate('hourly');
			case 'DL' : return translate('daily');
			case 'WK' : return translate('weekly');
			case 'MT' : return translate('monthly');
			case 'YR' : return translate('yearly');
			case 'TI' : return translate('repeating');
			default :
				trigger_localised_error('CRON0005', $type, E_USER_NOTICE);
			return '';
		}

	}//end whenTypeName()


	/**
	* Returns a text version of the week day
	*
	* @param int	$wday	the wday value (0-6)
	*
	* @return string
	* @access public
	* @static
	*/
	public static function whenWeekDayName($wday)
	{
		switch ((int) $wday) {
			case 0 : return translate('sunday');
			case 1 : return translate('monday');
			case 2 : return translate('tuesday');
			case 3 : return translate('wednesday');
			case 4 : return translate('thursday');
			case 5 : return translate('friday');
			case 6 : return translate('saturday');
			default :
				trigger_localised_error('CRON0034', E_USER_NOTICE, $wday);
				return '';
		}//end switch

	}//end whenWeekDayName()


	/**
	* Readable version of the 'when' time
	*
	* @param boolean	$inc_type	whether or not to include the job type with the string
	*
	* @return string
	* @access public
	*/
	public function readableWhen($inc_type=TRUE)
	{
		$when_arr = Cron_Job::getWhenArr($this->attr('when'));
		$ret_val = ($inc_type) ? Cron_Job::whenTypeName($when_arr['type']).' '.strtolower(translate('at')).' ' : '';
		if ($when_arr['type'] != 'TI') {
			// do not do padding for repeating time interval cron job  (TI)
			if (intval($when_arr['minute']) < 10 && strlen($when_arr['minute']) == 1) {
				$when_arr['minute'] = '0'.$when_arr['minute'];
			}
		}
		$when_arr['hour'] = str_replace(':', '', $when_arr['hour']);
		switch ($when_arr['type']) {
			case 'OO' :
				$ret_val .= translate('cron_run_other', $when_arr['hour'].':'.$when_arr['minute'], ltrim($when_arr['day'], '0').' '.date('M', mktime(0, 0, 0, (int) $when_arr['month'], 1, 2000)).', '.$when_arr['year']);
			break;

			case 'YR' :
				$ret_val .= translate('cron_run_other', $when_arr['hour'].':'.$when_arr['minute'], $when_arr['day'].' '.date('M', mktime(0, 0, 0, (int) $when_arr['month'], 1, 2000)));
			break;

			case 'MT' :
				$ret_val .= translate('cron_run_month' ,$when_arr['hour'].':'.$when_arr['minute'], date('jS', mktime(0, 0, 0, 1, (int) $when_arr['day'], 2000)));
			break;

			case 'WK' :
				$ret_val .= translate('cron_run_other', $when_arr['hour'].':'.$when_arr['minute'], Cron_Job::whenWeekDayName($when_arr['day']).'\'s');
			break;

			case 'DL' :
				$ret_val .= $when_arr['hour'].':'.$when_arr['minute'];
			break;

			case 'HR' :
				$ret_val .= translate('cron_run_hourly', $when_arr['minute']);
			break;

			case 'ET' :
				$ret_val = translate('cron_run_always');
			break;

			case 'TI' :
				if ($when_arr['minute'] != '') {
					$interval_unit = 'minute';
				} else if ($when_arr['hour'] != '') {
					$interval_unit = 'hour';
				} else if ($when_arr['day'] != '') {
					$interval_unit = 'day';
				}
				// format date, eg. 02:00 on 21 Jan, 2006
				$next_run = date('H:i \o\n j M, Y',$when_arr['duration']);
				$ret_val = translate('cron_run_repeat',$next_run, $when_arr[$interval_unit], translate($interval_unit.'s'));
			break;

		}//end switch

		return $ret_val;

	}//end readableWhen()


	/**
	* Returns the 'when' string attribute split up into the different types
	*
	* @param string	$when_str	the encoded 'when' str
	*
	* @return array
	* @access public
	* @static
	*/
	public static function getWhenArr($when_str)
	{
		$when_arr = Array('type' => '', 'year' => '', 'month' => '', 'day' => '', 'hour' => '', 'minute' => '', 'duration' => '', 'period' => '');

		if ($when_str) {
			if (strpos($when_str, '!')) {
				$when_arr['type'] = substr($when_str, 0, 2);
				$when_arr['duration'] = substr($when_str, strpos($when_str, '!') + 2);
				$when_arr['period'] = substr($when_str, strpos($when_str, '!') + 1, 1);
			} else {
				$when_arr['type'] = substr($when_str, 0, 2);
				$offset = 3;
				switch ($when_arr['type']) {
					case 'TI' :
						$time_unit = Array(
										60		=> 'minute',
										3600	=> 'hour',
										86400	=> 'day',
									 );

						$when = substr($when_str, $offset);
						$val = explode(',', $when);
						$next_run = $val[0];
						$interval = $val[1];
						$interval_unit = (int)$val[2];
						// store the next run time and interval info of this repeating job
						$when_arr['duration'] = $next_run;
						$when_arr[$time_unit[$interval_unit]] = $interval;
					break;
					case 'OO' :
						$when_arr['year'] = substr($when_str, $offset, 4);
						$offset += 5;
						// deliberately fall through
					case 'YR' :
						$when_arr['month'] = substr($when_str, $offset, 2);
						$offset += 3;
						// deliberately fall through
					case 'MT' :
					case 'WK' :
						// because the weekly type uses only one char, we do this
						if ($when_arr['type'] == 'WK') {
							$when_arr['day'] = substr($when_str, $offset, 1);
							$offset += 2;
						// else the monthly, yearly and one off use 2 chars
						} else {
							$when_arr['day'] = substr($when_str, $offset, 2);
							$offset += 3;
						}
						// deliberately fall through
					case 'DL' :
						$when_arr['hour'] = substr($when_str, $offset, 2);
						$offset += 3;
						// deliberately fall through
					case 'HR' :
						$when_arr['minute'] = substr($when_str, $offset, 2);
						$offset += 3;
						// deliberately fall through
					case 'PE' :
						$when_arr['period'] = substr($when_str, $offset, 1);
						$offset += 1;
						// deliberately fall through
					case 'DU' :
						$when_arr['duration'] = substr($when_str, $offset);
					break;

				}//end switch

			}//end else

		}//end if

		return $when_arr;

	}//end getWhenArr()


	/**
	* Runs this job and returns bitmask of the constants to tell cron manager what to do
	* eg:	SQ_CRON_JOB_ERROR | SQ_CRON_JOB_RETRY means an error occured, but can I please be re-tried,
	*		SQ_CRON_JOB_COMPLETED means everything is done correctly, but I can't be removed,
	*		SQ_CRON_JOB_COMPLETED | SQ_CRON_JOB_REMOVE means everything is done correctly and I can be removed
	*
	* @return int
	* @access public
	*/
	public function run()
	{
		$this->_tmp['running_errors'] = '';
		set_error_handler(Array(&$this, '_errorHandler'));

		$exec_result = 0;
		$exec_msg = '';

		// First let's see if we can find someone to run as
		$user = $this->getRunningUser();
		if (!is_null($user)) {

			// Now let's get the total lock
			if ($GLOBALS['SQ_SYSTEM']->am->acquireLock($this->id, 'all')) {

				if ($GLOBALS['SQ_SYSTEM']->setCurrentUser($user)) {

					// check if we should run the repeating cron job (TI) this time
					$when_str = $this->attr('when');
					$start_time_ok =  time() > $this->getNextRun($when_str);
					if (($this->attr('type') == 'repeating' && substr($when_str, 0, 2) == 'TI') && !$start_time_ok) {
						// not running yet, skip this run
						$exec_result = SQ_CRON_JOB_NOT_COMPLETE;
					} else {
						$exec_result = $this->_exec($exec_msg);
					}

					// reset the system user to who it was originally
					$GLOBALS['SQ_SYSTEM']->restoreCurrentUser();

				// else couldn't set the current user
				} else {
					trigger_localised_error('CRON0027', E_USER_WARNING, $this->name, $this->id, $user->name, $user->id);
					// we can retry because the user may have just been being edited, or something
					$exec_result = SQ_CRON_JOB_ERROR | SQ_CRON_JOB_RETRY;
				}

				$GLOBALS['SQ_SYSTEM']->am->releaseLock($this->id, 'all');

			// else acquireLock failed
			} else {
				trigger_localised_error('CRON0025', E_USER_WARNING, $this->name, $this->id);
				// we can retry because the lock might be free by the time that we try again
				$exec_result = SQ_CRON_JOB_ERROR | SQ_CRON_JOB_RETRY;

			}//end if

		// else no running user
		} else {
			trigger_localised_error('CRON0026', E_USER_WARNING, $this->name, $this->id);
			// we can be removed because without the running user we can't do anything
			$exec_result = SQ_CRON_JOB_ERROR | SQ_CRON_JOB_REMOVE;

		}//end if

		restore_error_handler();

		$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
		$ms->openQueue();
		$msg = $ms->newMessage();

		$msg_reps = Array(
						'cron_type_code'	=> $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name'),
						'cron_job_name'		=> $this->name,
						'cron_job_when'		=> $this->readableWhen(),
					);

		$msg->from = 0; // a system message

		// if no user was found resort to sys admins
		if (is_null($user)) {
			$root_user  = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
			$sys_admins = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('system_user_group');
			$msg->to = Array($root_user->id, $sys_admins->id);
		} else {
			$msg->to = Array($user->id);
		}

		// Exec failed
		if ($exec_result & SQ_CRON_JOB_ERROR) {
			$msg->type 	   = 'cron.job.fail';

		// Exec successful and completed
		} else if ($exec_result & SQ_CRON_JOB_COMPLETED) {
			$msg->type 	   = 'cron.job.success';

			// If we are a one off type and we aren't yet being removed do so
			if ($this->attr('type') == 'one_off' && !($exec_result & SQ_CRON_JOB_REMOVE)) {
				if ($exec_result & SQ_CRON_JOB_RETRY) {
					// delete any retry attempts
					$exec_result &= ~SQ_CRON_JOB_RETRY;
				}
				$exec_result |= SQ_CRON_JOB_REMOVE;
			}

			// If this is a repeating cron job with a time interval, update the next run time
			if ($this->attr('type') == 'repeating') {
				$when_str = $this->attr('when');
				$type = substr($when_str, 0, 2);
				if ($type == 'TI') $this->setNextRun();
			}

		// Exec successful and but still more to do
		} else if ($exec_result & SQ_CRON_JOB_NOT_COMPLETE) {
			$msg->type 	   = 'cron.job.success.incomplete';

		// Exec stuffed up, email the sys admins - SHOULD NEVER HAPPEN ;)
		} else {
			require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
			$msg_reps['result']      = $exec_result;
			$msg_reps['result_bits'] = implode(', ', get_bit_names('SQ_CRON_JOB_', $exec_result, TRUE));
			$msg->type 	             = 'cron.job.error';

			$root_user  = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
			$sys_admins = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('system_user_group');
			$msg->to[] = 0; // default and tech email
			$msg->to[] = $root_user->id;
			$msg->to[] = $sys_admins->id;

		}//end if

		$msg_reps['exec_msg'] = $exec_msg;

		if (!empty($this->_tmp['running_errors'])) {
			$msg_reps['errors'] = $this->_tmp['running_errors'];
		} else {
			$msg_reps['errors'] = 'Nil';
		}

		$msg->replacements = $msg_reps;
		$ms->enqueueMessage($msg);
		$ms->closeQueue();

		return $exec_result;

	}//end run()


	/**
	* This function captures all errors and warnings that occur during the execution of a job
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

			$err_msg = strip_tags(preg_replace(Array('/<br\\/?>/i', '/<p[^>]*>/i'), Array("\n", "\n\n"), $err_msg));

			// send a report to the system error log
			if (ini_get('log_errors')) {
				$cron_mgr = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
				log_error($err_msg, $err_no, $err_file, $err_line, $cron_mgr->error_log_file_name);
			}

			// if they haven't put tags in the err msg assume it to be plain text
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

			$cm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
			$cm->removeJob($this);

			$user       = $this->getRunningUser();
			$root_user  = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
			$sys_admins = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('system_user_group');

			$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
			$msg = $ms->newMessage();

			$msg_reps = Array(
							'cron_type_code'	=> $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name'),
							'asset_name'		=> $this->name,
							'assetid'			=> $this->id,
							'errors'			=> $this->_tmp['running_errors'],
						);

			// userid zero sends a msg to SQ_CONF_DEFAULT_EMAIL and SQ_CONF_TECH_EMAIL
			$msg->to           = Array(0, $user->id, $root_user->id, $sys_admins->id);
			$msg->from         = 0; // a system message
			$msg->type         = 'cron.job.term';
			$msg->replacements = $msg_reps;

			$msg->send();

			exit(1);

		}//end if

	}//end _errorHandler()


	/**
	* Actually executes the functionality for this job, called by run()
	*
	* Returns a bitmask of the SQ_CRON_JOB_* constants to tell the cron manager what to do
	*				eg: SQ_CRON_JOB_ERROR | SQ_CRON_JOB_RETRY means an error occured, but can I please be re-tried,
	*					SQ_CRON_JOB_COMPLETED means everything is done correctly, but I can't be removed,
	*					SQ_CRON_JOB_COMPLETED | SQ_CRON_JOB_REMOVE means everything is done correctly and I can be removed
	*
	* @param string	&$msg	used to return any messages about completion etc from _exec
	*
	* @return int
	* @access private
	* @see run()
	*/
	protected function _exec(&$msg)
	{
		trigger_localised_error('CRON0028', E_USER_WARNING, $this->name, $this->id);
		// we can be removed because we haven't been done
		return SQ_CRON_JOB_ERROR | SQ_CRON_JOB_REMOVE;

	}//end _exec()


	/**
	* When given a when string value and a start time returns the timestamp for the first time after the
	* start time that this when string represents
	*
	* @param string	$when_str	the encoded 'when' str
	* @param int	$start_ts	the starting timestamp
	*
	* @return int
	* @access public
	*/
	public function getNextRun($when_str, $start_ts=NULL)
	{
		$when = Cron_Job::getWhenArr($when_str);

		// One Off is simple
		if ($when['type'] == 'OO') {
			return mktime($when['hour'], $when['minute'], 0, $when['month'], $when['day'], $when['year']);
		}

		if (is_null($start_ts)) $start_ts = time();
		$start = getdate($start_ts);

		// EveryTime is even simpler
		if ($when['type'] == 'ET') return $start_ts + 1;

		// Repeating Job with time interval
		if ($when['type'] == 'TI') return $when['duration'];

		$add = Array('minutes' => 0, 'hours' => 0, 'days' => 0);

		$hour_loops  = ($when['minute'] < $start['minutes']);
		$day_loops   = ($when['hour']   < $start['hours']  || ($when['hour']  == $start['hours'] && $hour_loops));
		$week_loops  = ($when['day']    < $start['wday']   || ($when['day']   == $start['wday']  && $day_loops));
		$month_loops = ($when['day']    < $start['mday']   || ($when['day']   == $start['mday']  && $day_loops));
		$year_loops  = ($when['month']  < $start['mon']    || ($when['month'] == $start['mon']   && $month_loops));

		switch ($when['type']) {
			// Hourly
			case 'HR' :
					// if we need to loop over until the next hour
					if ($hour_loops) {
						$add['minutes'] += (60 - $start['minutes']) + $when['minute'];
					} else {
						$add['minutes'] += $when['minute'] - $start['minutes'];
					}

			break;

			// Daily
			case 'DL' :
					// if we need to loop over until the next day
					if ($day_loops) {
						$add['hours'] += (24 - $start['hours']) + $when['hour'];
					} else {
						$add['hours'] += $when['hour'] - $start['hours'];
					}

					$add['minutes'] += $when['minute'] - $start['minutes'];

			break;

			// Weekly
			case 'WK' :
					// if we need to loop over until the next week
					if ($week_loops) {
						$add['days'] += (7 - $start['wday']) + $when['day'];
					} else {
						$add['days'] += $when['day'] - $start['wday'];
					}

					$add['hours']   += $when['hour']   - $start['hours'];
					$add['minutes'] += $when['minute'] - $start['minutes'];

			break;

			// Monthly
			case 'MT' :
					// if we need to loop over until the next month
					if ($month_loops) {
						require_once SQ_FUDGE_PATH.'/general/datetime.inc';
						$add['days'] += (days_in_month($start['mon'], $start['year']) - $start['mday']) + $when['day'];
					} else {
						$add['days'] += $when['day'] - $start['mday'];
					}

					$add['hours']   += $when['hour']   - $start['hours'];
					$add['minutes'] += $when['minute'] - $start['minutes'];

			break;

			// Yearly
			case 'YR' :

					require_once SQ_FUDGE_PATH.'/general/datetime.inc';

					// if we need to loop over until the next year
					if ($year_loops) {

						// add the rest of the months this year
						for ($i = $start['mon']; $i <= 12; $i++) {
							$add['days'] += days_in_month($i, $start['year']);
						}
						// add the months for next year
						for ($i = 1; $i < $when['month']; $i++) {
							$add['days'] += days_in_month($i, $start['year'] + 1);
						}

					} else {

						// add the months between
						for ($i = $start['mon']; $i < $when['month']; $i++) {
							$add['days'] += days_in_month($i, $start['year']);
						}

					}//end if

					$add['days']    += $when['day']    - $start['mday'];
					$add['hours']   += $when['hour']   - $start['hours'];
					$add['minutes'] += $when['minute'] - $start['minutes'];


			break;

		}//end switch

		$add['hours']   += $add['days']  * 24;
		$add['minutes'] += $add['hours'] * 60;

		return $start_ts + ($add['minutes'] * 60);

	}//end getNextRun()


	/**
	* Set the next run time in the encoded 'when' str
	*
	* Only for a repeating cron job with time interval (TI)
	*
	* @return void
	* @access public
	*/
	public function setNextRun()
	{
		if ($this->attr('type') == 'repeating') {
			$when_str = $this->attr('when');
			$type = substr($when_str, 0, 2);
			if ($type == 'TI') {
				$when = substr($when_str, 3);
				$val = explode(',', $when);
				$old_run = $val[0];
				$interval = $val[1];
				$interval_unit = $val[2];
				$next_run = $old_run + ($interval * $interval_unit);
				if($next_run <= $old_run) {
				    $ti = $next_run - $old_run;
				    trigger_error('Invalid Repeat Time Interval ('.$ti.'s) set on this cron job #'.$this->id);
				    return;
				}
				while ($next_run < time()) {
					// if this is a long running job, and we cannot start because it is still running
					// then we have to skip a few turns to make sure next run time is in the future

					$next_run = $old_run + ($interval * $interval_unit);
					$old_run = $next_run;


					if ($next_run < time()) {
						// send a system email to inform admins about this,
						// so that they can increase the time interval between each run
						$root_user  = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
						$sys_admins = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('system_user_group');

						$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
						$ms->openQueue();
						$msg = $ms->newMessage();
						$msg_reps = Array(
										'cron_type_code'	=> $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name'),
										'asset_name'		=> $this->name,
										'assetid'			=> $this->id,
										'run_time'			=> easy_datetime($next_run),
									);
						$msg->to           = Array($root_user->id, $sys_admins->id);
						$msg->from         = 0; // a system message
						$msg->type         = 'cron.job.startfail';
						$msg->replacements = $msg_reps;

						$ms->enqueueMessage($msg);
						$ms->closeQueue();
					}
				}//end while

				$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED + SQ_SECURITY_LINK_INTEGRITY);
					$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
					if ($GLOBALS['SQ_SYSTEM']->am->acquireLock($this->id, 'links')) {
						$this->setAttrValue('when', "TI=$next_run,$interval,$interval_unit");
						$this->saveAttributes();
						$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
						$GLOBALS['SQ_SYSTEM']->am->releaseLock($this->id, 'links');
					}
				$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
			}//end if
		}//end if

	}//end setNextRun()


	/**
	* Called when a link to this job is removed. If that was it's last significant
	* link, the job should be moved to the trash.
	*
	* @return boolean
	* @access public
	*/
	public function removeJob()
	{
		$am = $GLOBALS['SQ_SYSTEM']->am;
		// if we are type2 linked under the manager, don't worry.
		// if we are only type3 or notice linked under the manager, trash ourselves
		$cron_manager_links = $am->getLinks($this->id, SQ_LINK_TYPE_2, 'cron_manager', TRUE, 'minor');
		if (empty($cron_manager_links)) {
			// clean up any notice links in which we are the major
			$notice_links = $am->getLinks($this->id, SQ_LINK_NOTICE, NULL, NULL, 'major');

			foreach ($notice_links as $notice_link) {
				$am->deleteAssetLink($notice_link['linkid']);
			}

			$trash = $am->getAsset($am->getSystemAssetid('trash_folder'));
			$trash->createLink($this, SQ_LINK_TYPE_1);
		}

	}//end removeJob()


}//end class
?>
