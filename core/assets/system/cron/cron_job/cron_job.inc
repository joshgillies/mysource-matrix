<?php

require_once SQ_INCLUDE_PATH.'/asset.inc';

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
	* Text versions of the type names to used for the when attr
	* @var array
	*/
	var	$type_names = Array(
							'OO' => 'One Off',
							'YR' => 'Yearly',
							'MT' => 'Monthly',
							'WK' => 'Weekly',
							'DL' => 'Daily',
							'HR' => 'Hourly',
							);

	/**
	* Text versions of the week day names for the when attr
	* @var array
	*/
	var	$weekly_names = Array(
							'0' => 'Sunday',
							'1' => 'Monday',
							'2' => 'Tuesday',
							'3' => 'Wednesday',
							'4' => 'Thursday',
							'5' => 'Friday',
							'6' => 'Saturday',
							);


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
	function &getRunningUser(&$user)
	{
		$null = null; // because we need to return by ref

		$link = $this->getLink(SQ_LINK_NOTICE, 'user', false, 'running as');
		if (empty($link)) return $null;
		return $GLOBALS['SQ_SYSTEM']->getAsset($link['minorid'], $link['type_code']);

	}// end getRunningUser()


	/**
	* Readable version of the 'when' time
	*
	* @return string
	* @access public
	*/
	function readableWhen()
	{
		$when_arr = Cron_Job::getWhenArr($this->attr('when'));
		$ret_val = $this->type_names[$when_arr['type']].' at ';
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
				$ret_val .= $when_arr['hour'].':'.$when_arr['minute'].' on '.$this->weekly_names[$when_arr['day']].'\'s';
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
	* @return boolean	TRUE on success, FALSE on failure
	* @access public
	*/
	function run()
	{
		// First let's get the lock
		if (!$this->acquireLock()) return false;

		$user = &$this->getRunningUser();
		if (is_null($user)) {
			trigger_error('Unable to run "'.$this->name.'" (#'.$this->id.'), couldn\'t find user to run as', E_USER_WARNING);
			return false;
		}

		if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($user)) {
			trigger_error('Unable to run "'.$this->name.'" (#'.$this->id.'), couldn\'t set the current system user to "'.$user->name.'" (#'.$user->id.')', E_USER_WARNING);
			return false;
		}

		$ret_val = $this->_exec();

		$this->releaseLock();

		return $ret_val;

	}// end run()


	/**
	* Actually executes the functionality for this job
	*
	* @return boolean	TRUE on success, FALSE on failure
	* @access private
	*/
	function _exec()
	{
		trigger_error('Unable to run "'.$this->name.'" (#'.$this->id.'), exec not done', E_USER_WARNING);
		return false;
	}// end _exec()


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
