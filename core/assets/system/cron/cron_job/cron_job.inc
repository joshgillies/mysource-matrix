<?php

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
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
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
	function Cron_Job($assetid=0)
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
		if (!$this->attr('type') || !$this->attr('when')) {
			trigger_error('A type and a when time need to be set for a cron job to be created', E_USER_ERROR);
			return false;
		}

		return parent::create($link);

	}//end create()


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
					 SQ_LINK_TYPE_3 => Array(),
					 SQ_LINK_NOTICE => Array('user' => Array('card' => 1))
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
		trigger_error('Unable to duplicate '.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name'), E_USER_WARNING);
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
	* Can the current user forceably acquire the current lock on this asset?
	*
	* @return boolean
	* @access public
	*/
	function canForceablyAcquireLock()
	{
		return false;
	}


	/**
	* Set the value for the passed variable to the passed value
	*
	* @param string		$name				the name of the attribute
	* @param string		$value				the new value of the attribute
	* @param boolean	$dont_run_updated	if true this->_updated() won't be run
	*										(THIS IS ONLY EVER CALLED FROM Asset::create())
	*
	* @return boolean
	* @access public
	*/
	function setAttrValue($name, $value, $dont_run_updated=false)
	{
		if ($name == 'when') {

			$value = (string) $value;
			if ($this->attr('type') != 'one_off' && substr($value, 0, 3) == 'OO=') {
				trigger_error('Only one_off type cron jobs can have their "when" attributes set to a "OO=" prefixed value', E_USER_NOTICE);
				return false;
			}
			if ($this->attr('type') == 'one_off' && substr($value, 0, 3) != 'OO=') {
				trigger_error('one_off type cron jobs MUST have their "when" attributes set to a "OO=" prefixed value', E_USER_NOTICE);
				return false;
			}


			// if this is a one off job
			// then we need to make sure that it is happening on or before the
			// next time that the cron job is getting run
			if ($this->attr('type') == 'one_off') {

				$cron_mgr = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
				if (is_null($cron_mgr)) return false;

				$when = Cron_Job::getWhenArr($value);

				$when_ts = mktime($when['hour'], $when['minute'], 0, $when['month'], $when['day'], $when['year']);

				if ($when_ts < $cron_mgr->timeOfNextRun()) {
					require_once SQ_FUDGE_PATH.'/general/datetime.inc';
					trigger_error('You cannot set a One Off '.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name').' to be run before the next job run is scheduled (which is at '.easy_datetime($cron_mgr->timeOfNextRun()).')', E_USER_WARNING);
					return false;
				}
			}

		}// end if

		return parent::setAttrValue($name, $value, $dont_run_updated);

	}// end setAttrValue()


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
	* Can this asset have its last significant link removed (putting it in the trash)?
	* For Cron Jobs this can only be done by the running user or a sys admin
	*
	* @return boolean
	* @access public
	*/
	function canDelete()
	{
		$user = &$this->getRunningUser();
		if (!is_null($user) && $GLOBALS['SQ_SYSTEM']->currentUser($user)) return true;

		// if this is to be run as the root user, it can only be deleted by the root user
		if (!is_null($user) && $GLOBALS['SQ_SYSTEM']->userRoot($user) && !$GLOBALS['SQ_SYSTEM']->userRoot()) return false;

		// else root user or sys admins can delete
		if ($GLOBALS['SQ_SYSTEM']->userRoot() || $GLOBALS['SQ_SYSTEM']->userSystemAdmin()) return true;

		return false;

	}//end canDelete()


	/**
	* Sets the user that this cron job is to run as while executing
	*
	* @param object User	$user		a reference to the user that this job will run as
	*
	* @return boolean
	* @access public
	*/
	function setRunningUser(&$user)
	{
		if (!is_a($user, 'user')) {
			trigger_error('Unable to set running user, the user must be a user object', E_USER_WARNING);
			return false;
		}

		return (bool) $this->createLink($user, SQ_LINK_NOTICE, 'running as');

	}// end setRunningUser()


	/**
	* Gets the user that this cron job is to run as while executing
	*
	* @return mixed object User or NULL on error
	* @access public
	*/
	function &getRunningUser()
	{
		$null = null; // because we need to return by ref

		$link = $this->getLink(SQ_LINK_NOTICE, 'user', false, 'running as');
		if (empty($link)) return $null;
		return $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['type_code']);

	}// end getRunningUser()


	/**
	* Returns a text version of the type names to used for the when attr
	*
	* @param string	$type	two char type name from the when attr
	*
	* @return string
	* @access public
	* @static
	*/
	function whenTypeName($type)
	{
		switch($type) {
			case 'OO' : return 'One Off';
			case 'HR' : return 'Hourly';
			case 'DL' : return 'Daily';
			case 'WK' : return 'Weekly';
			case 'MT' : return 'Monthly';
			case 'YR' : return 'Yearly';
			default :
				trigger_error('Cron Job when type "'.$type.'" not known', E_USER_NOTICE);
			return '';
		}
	}// end whenTypeName()


	/**
	* Returns a text version of the week day
	*
	* @param int	$wday	the wday value (0-6)
	*
	* @return string
	* @access public
	* @static
	*/
	function whenWeekDayName($wday)
	{
		switch((int) $wday) {
			case 0 : return 'Sunday';
			case 1 : return 'Monday';
			case 2 : return 'Tuesday';
			case 3 : return 'Wednesday';
			case 4 : return 'Thursday';
			case 5 : return 'Friday';
			case 6 : return 'Saturday';
			default :
				trigger_error('Week Day "'.$wday.'" not known', E_USER_NOTICE);
				return '';
		}// end switch

	}// end whenWeekDayName()


	/**
	* Readable version of the 'when' time
	*
	* @param boolean	$inc_type	whether or not to include the job type with the string
	*
	* @return string
	* @access public
	*/
	function readableWhen($inc_type=true)
	{
		$when_arr = Cron_Job::getWhenArr($this->attr('when'));
		$ret_val = ($inc_type) ? Cron_Job::whenTypeName($when_arr['type']).' at ' : '';
		switch($when_arr['type']) {
			case 'OO' :
				$ret_val .= $when_arr['hour'].':'.$when_arr['minute'].' on '.$when_arr['day'].'/'.$when_arr['month'].'/'.$when_arr['year'];
				break;

			case 'YR' :
				$ret_val .= $when_arr['hour'].':'.$when_arr['minute'].' on '.$when_arr['day'].' '.date('M', mktime(0, 0, 0, (int) $when_arr['month'], 1, 2000));
				break;

			case 'MT' :
				$ret_val .= $when_arr['hour'].':'.$when_arr['minute'].' on the '.date('jS', mktime(0, 0, 0, 1, (int) $when_arr['day'], 2000));
				break;

			case 'WK' :
				$ret_val .= $when_arr['hour'].':'.$when_arr['minute'].' on '.Cron_Job::whenWeekDayName($when_arr['day']).'\'s';
				break;

			case 'DL' :
				$ret_val .= $when_arr['hour'].':'.$when_arr['minute'];
				break;

			case 'HR' :
				$ret_val .= $when_arr['minute'].' minute past the hour';
				break;

		}// end switch

		return $ret_val;

	}// end readableWhen()


	/**
	* Returns the When String attribute split up into the different types
	*
	* @return array()
	* @access public
	* @static
	*/
	function getWhenArr($when_str)
	{
		$when_arr = Array('type' => '', 'year' => '', 'month' => '', 'day' => '', 'hour' => '', 'minute' => '');
		if ($when_str) {
			$when_arr['type'] = substr($when_str, 0, 2);
			$offset = 3;
			switch($when_arr['type']) {
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
					break;

			}// end switch

		}// end if

		return $when_arr;

	}// end getWhenArr()


	/**
	* Runs this job
	*
	* @return int	bitmask of the SQ_CRON_JOB_* constants to tell the cron manager what to do
	*				eg: SQ_CRON_JOB_ERROR | SQ_CRON_JOB_RETRY means an error occured, but can I please be re-tried,
	*					SQ_CRON_JOB_COMPLETED means everything is done correctly, but I can't be removed,
	*					SQ_CRON_JOB_COMPLETED | SQ_CRON_JOB_REMOVE means everything is done correctly and I can be removed
	* @access public
	*/
	function run()
	{
		$this->_tmp['running_errors'] = '';
		set_error_handler(Array(&$this, '_errorHandler'));

		$exec_result = 0;
		$exec_msg = '';

		// First let's see if we can find someone to run as
		$user = &$this->getRunningUser();
		if (!is_null($user)) {

			// Now let's get the lock
			if ($GLOBALS['SQ_SYSTEM']->am->acquireLock($this->id)) {

				if ($GLOBALS['SQ_SYSTEM']->setCurrentUser($user)) {

					$exec_result = $this->_exec($exec_msg);

					// reset the system user to who it was originally
					$GLOBALS['SQ_SYSTEM']->restoreCurrentUser();

				// else couldn't set the current user
				} else {
					trigger_error('Unable to run "'.$this->name.'" (#'.$this->id.'), couldn\'t set the current system user to "'.$user->name.'" (#'.$user->id.')', E_USER_WARNING);
					// we can retry because the user may have just been being edited, or something
					$exec_result = SQ_CRON_JOB_ERROR | SQ_CRON_JOB_RETRY;
				}

				$GLOBALS['SQ_SYSTEM']->am->releaseLock($this->id);

			// else acquireLock failed
			} else {
				trigger_error('Unable to run "'.$this->name.'" (#'.$this->id.'), couldn\'t acquire lock', E_USER_WARNING);
				// we can retry because the lock might be free by the time that we try again
				$exec_result = SQ_CRON_JOB_ERROR | SQ_CRON_JOB_RETRY;

			}// end if

		// else no running user
		} else {
			trigger_error('Unable to run "'.$this->name.'" (#'.$this->id.'), couldn\'t find user to run as', E_USER_WARNING);
			// we can be removed because without the running user we can't do anything
			$exec_result = SQ_CRON_JOB_ERROR | SQ_CRON_JOB_REMOVE;

		}// end if

		restore_error_handler();

		$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
		$ms->openQueue();
		$msg = $ms->newMessage();

		$msg->subject = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name').' ';
		$msg->from = 0; // a system message

		// if no user was found resort to sys admins
		if (is_null($user)) {
			$root_user  = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
			$sys_admins = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('system_user_group');
			$msg->to = Array($root_user->id, $sys_admins->id);
		} else {
			$msg->to = Array($user->id);
		}

		$msg->body = $this->name."\n"
					.'Scheduled : '.$this->readableWhen()."\n";

		// Exec failed
		if ($exec_result & SQ_CRON_JOB_ERROR) {
			$msg->subject .= 'Failure';
			$msg->body    .= 'Failed';

		// Exec successful and completed
		} elseif ($exec_result & SQ_CRON_JOB_COMPLETED) {
			$msg->subject .= 'Success';
			$msg->body    .= 'Completed Successfully';

			// If we are a one off type and we aren't yet being removed do so
			if ($this->attr('type') == 'one_off' && !($exec_result & SQ_CRON_JOB_REMOVE)) {
				if ($exec_result & SQ_CRON_JOB_RETRY) $exec_result &= ~SQ_CRON_JOB_RETRY; // delete any retry attempts
				$exec_result |= SQ_CRON_JOB_REMOVE;
			}

		// Exec successful and but still more to do
		} elseif ($exec_result & SQ_CRON_JOB_NOT_COMPLETE) {
			$msg->subject .= 'Success';
			$msg->body    .= 'Successful, but there is still more work to do';

		// Exec stuffed up, email the sys admins - SHOULD NEVER HAPPEN ;)
		} else {
			require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
			$msg->subject .= 'ERROR';
			$msg->body    .= '###################################################################'."\n"
							.'#  RETURNED AN INVALID RESULT FROM _exec()'."\n"
							.'#  Returned Value : '.$exec_result."\n"
							.'#  (Transated : '.implode(', ', get_bit_names('SQ_CRON_JOB_', $exec_result, true)).')'."\n"
							.'###################################################################';

			$root_user  = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
			$sys_admins = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('system_user_group');
			$msg->to[] = 0; // default and tech email
			$msg->to[] = $root_user->id;
			$msg->to[] = $sys_admins->id;

		}// end if

		if (!empty($exec_msg)) {
			$msg->body .= "\n".$exec_msg."\n";
		}

		if (!empty($this->_tmp['running_errors'])) {
			$msg->body .= "\nError Messages :\n".$this->_tmp['running_errors'];
		}

		$ms->enqueueMessage($msg);
		$ms->closeQueue();


		return $exec_result;

	}// end run()


	/**
	* This function captures all errors and warnings that occur during the execution of a job
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

			$cm = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
			$cm->removeJob($this);

			$user       = &$this->getRunningUser();
			$root_user  = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
			$sys_admins = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('system_user_group');

			$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
			$msg = $ms->newMessage();
			$msg->subject = 'TERMINATION of '.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name');
			$msg->body	  = "The job \"".$this->name."\" (#".$this->id.") fatally triggered an error and terminated the cron script.\n"
							."It has been removed from the cron manager\n\nError Messages :\n"
							.$this->_tmp['running_errors'];

			// userid zero sends a msg to SQ_CONF_DEFAULT_EMAIL and SQ_CONF_TECH_EMAIL
			$msg->to   = Array(0, $user->id, $root_user->id, $sys_admins->id);
			$msg->from = 0; // a system message

			$msg->send();

			exit(1);

		}// end if

	}// end _errorHandler()


	/**
	* Actually executes the functionality for this job, called by run()
	*
	* @param string	$msg	accepted by reference, used to return any messages about completion etc from _exec
	*
	* @return int	bitmask of the SQ_CRON_JOB_* constants to tell the cron manager what to do
	*				eg: SQ_CRON_JOB_ERROR | SQ_CRON_JOB_RETRY means an error occured, but can I please be re-tried,
	*					SQ_CRON_JOB_COMPLETED means everything is done correctly, but I can't be removed,
	*					SQ_CRON_JOB_COMPLETED | SQ_CRON_JOB_REMOVE means everything is done correctly and I can be removed
	* @see run()
	* @access private
	*/
	function _exec(&$msg)
	{
		trigger_error('Unable to run "'.$this->name.'" (#'.$this->id.'), _exec() not done', E_USER_WARNING);
		// we can be removed because we haven't been done
		return SQ_CRON_JOB_ERROR | SQ_CRON_JOB_REMOVE;

	}// end _exec()


	/**
	* When given a when string value and a start time returns the timestamp for the first time after the
	* start time that this when string represents
	*
	* @return int	timestamp of when the next run will occur for this when string
	* @access public
	* @static
	*/
	function getNextRun($when_str, $start_ts=null)
	{
		$when = Cron_Job::getWhenArr($when_str);

		// One Off is simple
		if ($when['type'] == 'OO') {
			return mktime($when['hour'], $when['minute'], 0, $when['month'], $when['day'], $when['year']);
		}

		if (is_null($start_ts)) $start_ts = time();
		$start = getdate($start_ts);

		$add = Array('minutes' => 0, 'hours' => 0, 'days' => 0);

		$hour_loops  = ($when['minute'] < $start['minutes']);
		$day_loops   = ($when['hour']   < $start['hours']  || ($when['hour']  == $start['hours'] && $hour_loops));
		$week_loops  = ($when['day']    < $start['wday']   || ($when['day']   == $start['wday']  && $day_loops));
		$month_loops = ($when['day']    < $start['mday']   || ($when['day']   == $start['mday']  && $day_loops));
		$year_loops  = ($when['month']  < $start['mon']    || ($when['month'] == $start['mon']   && $month_loops));

		switch($when['type']) {
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
						for($i = $start['mon']; $i <= 12; $i++) {
							$add['days'] += days_in_month($i, $start['year']);
						}
						// add the months for next year
						for($i = 1; $i < $when['month']; $i++) {
							$add['days'] += days_in_month($i, $start['year'] + 1);
						}

					} else {

						// add the months between
						for($i = $start['mon']; $i < $when['month']; $i++) {
							$add['days'] += days_in_month($i, $start['year']);
						}

					}// end if

					$add['days']    += $when['day']    - $start['mday'];
					$add['hours']   += $when['hour']   - $start['hours'];
					$add['minutes'] += $when['minute'] - $start['minutes'];


				break;

		}// end switch

		$add['hours']   += $add['days']  * 24;
		$add['minutes'] += $add['hours'] * 60;

		return $start_ts + ($add['minutes'] * 60);

	}// end getNextRun()


}//end class
?>
