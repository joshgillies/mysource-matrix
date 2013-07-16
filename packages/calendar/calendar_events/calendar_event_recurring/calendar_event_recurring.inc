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
* $Id: calendar_event_recurring.inc,v 1.108.2.1 2013/07/16 04:16:12 cupreti Exp $
*
*/

require_once  dirname(__FILE__).'/../../calendar_event/calendar_event.inc';
require_once  dirname(__FILE__).'/../../lib/calendar_common.inc';
require_once SQ_FUDGE_PATH.'/datetime_field/datetime_field.inc';
require_once SQ_FUDGE_PATH.'/general/general.inc';
require_once SQ_FUDGE_PATH.'/general/datetime.inc';

/**
* Calendar_Event_Recurring
*
* An event which recurs at a specified frequency after its first instance
*
* @author  Luke Wright <lwright@squiz.net>
* @euthor  Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.108.2.1 $
* @package MySource_Matrix_Packages
* @subpackage calendar
*/
class Calendar_Event_Recurring extends Calendar_Event
{
	var $parent_links = Array();


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Calendar_Event_Recurring($assetid=0)
	{
		$this->Calendar_Event($assetid);

	}//end constructor


	/**
	* Load the asset represented by the passed asset ID
	*
	* @param int	$assetid	the asset ID to be loaded
	*
	* @return void
	* @access public
	*/
	function load($assetid)
	{
		parent::load($assetid);

		$this->parent_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($assetid, SQ_SC_LINK_ALL, 'calendar_event_multi_date', FALSE, 'minor');

	}//end load()


	/**
	* Perform any pre-validation required during the creation of this asset
	*
	* For example, checking that attributes are set should bee performed in this method
	*
	* @param array	&$link	information used to create the initial link<br/>
	* <pre>
	* Array (
	*   'asset'        => [ref major asset to create link under],
	*   'link_type'    => SQ_LINK_?,
	*   'value'        => [link value],
	*   'sort_order'   => [link sort order],
	*   'is_dependant' => [0|1],
	*   'is_exclusive' => [0|1],
	* )
	* </pre>
	*
	* @return boolean
	* @access private
	*/
	function _preCreateCheck(&$link)
	{
		$freq = $this->attr('frequency');
		if (empty($freq)) {
			trigger_localised_error('CAL0047', E_USER_WARNING);
			return FALSE;
		}
		return parent::_preCreateCheck($link);

	}//end _preCreateCheck()


	/**
	* Returns the number of seconds since the start of the day
	*
	* @param int	$timestamp	the date to test
	*
	* @return int
	* @access private
	*/
	function _secondsToday($timestamp)
	{
		return $timestamp - strtotime(gmdate('Y-m-d 00:00:00', $timestamp).' GMT');

	}//end _secondsToday()


	/**
	* Determines the next occurrence of an event on or after a certain time
	*
	* This function can be called on an object (with just the first parameter) or
	* statically (with all the parameters). If only $after is specified the object's
	* attributes are used.
	* If $after falls ON the date of one of the event's instances, that instance will be
	* returned. If $after falls after the first day of an instance, the next instance will be
	* returned.
	* Returns the date (YYYY-MM-DD) of the next occurrence
	*
	* @param string	$after		the earliest date (YYYY-MM-DD) we are interested in
	* @param string	$start		The date (YYYY-MM-DD) on which the event's first instance started
	* @param string	$freq_type	3-letter frequency code
	* @param int	$interval	The frequency interval
	* @param string	$stop		The date on which the event stops recurring
	*
	* @return int
	* @access public
	* @static
	*/
	function getFirstOccurrenceAfter($after=NULL, $start=NULL, $freq_type=NULL, $interval=NULL, $stop=NULL)
	{
		if (is_null($after)) $after = date('Y-m-d');

		// if we are calling statically make sure that we have all our arguments
		if ((!(isset($this) && is_a($this, 'Calendar_Event_Recurring'))) && (func_num_args() < 5)) {
			trigger_localised_error('CAL0030', E_USER_ERROR);
		}

		if (is_null($start)) {
			// called on an object
			$freq_type = substr($this->attr('frequency'), 0, 3);
			$interval = substr($this->attr('frequency'), 3);
			if (!$interval)	$interval = 1;
			$start = substr($this->attr('start_date'),0,10);
			$stop = $this->attr('stop_date');
			// null stop date attribute?
			if ($stop == '---------- --:--:--') $stop = NULL;
		}

		$seconds_per_day = 60 * 60 * 24;
		if ($after <= $start) return $start;
		$result = '';

		while ($result == '') {
			list($res_year,$res_month,$res_day) = sscanf($after, '%04d-%02d-%02d');
			list($after_year,$after_month,$after_day) = sscanf($after, '%04d-%02d-%02d');
			list($start_year,$start_month,$start_day) = sscanf($start, '%04d-%02d-%02d');
			$month_diff = (12 * ($after_year - $start_year)) + $after_month - $start_month;

			switch ($freq_type) {
				case 'DED':
					$periods_so_far = ceil(days_between_isos($after, $start) / $interval);
					$result = add_days_to_iso($start, $interval * $periods_so_far);
				break;

				case 'DWD':
					$periods_so_far = ceil(days_between_isos($after, $start) / $interval);
					$result = add_days_to_iso($start, $interval * $periods_so_far);
					while (in_array(date('D', iso8601_ts($result)), Array('Sat', 'Sun'))) {
						$result = add_days_to_iso($result, 1);
					}
				break;

				case 'DWE':
					$periods_so_far = ceil(days_between_isos($after, $start) / $interval);
					$result = add_days_to_iso($start, $interval * $periods_so_far);
					while (!in_array(date('D', iso8601_ts($result)), Array('Sat', 'Sun'))) {
						$result = add_days_to_iso($result, 1);
					}
				break;

				case 'WEW':
					$periods_so_far = ceil(days_between_isos($after, $start) / ($interval*7));
					$result = add_days_to_iso($start, $interval * 7 * $periods_so_far);
				break;

				// Every 'x' months on the 'y'th day of the month
				case 'MFN':
					if (($month_diff % $interval) != 0) {
						increment_month($res_month, $res_year, ($interval - ($month_diff % $interval)));
					} else if ($res_day > $start_day) {
						increment_month($res_month, $res_year, $interval);
					}
					while ($start_day > days_in_month($res_month, $res_year)) {
						increment_month($res_month, $res_year, $interval);
					}
					$result = sprintf('%04d-%02d-%02d',$res_year,$res_month,$start_day);
				break;

				// Every 'x' months on the 'y'th weekday of the month (eg. 2nd Thursday)
				case 'MFW':
					$target_week_of_month = (int)(($start_day - 1) / 7);
					$target_day_of_week = date('w', iso8601_ts($start));
					$res_day = ($target_week_of_month * 7) + 1;

					if (($month_diff % $interval) != 0) {
						// go to the next valid month
						increment_month($res_month, $res_year, $interval - ($month_diff % $interval));
					} else if ((int)($after_day / 7) > $target_week_of_month) {
						// we're in a valid month, but too late in it, so go to next valid month
						increment_month($res_month, $res_year, $interval);
					} else if ((int)($after_day / 7) == $target_week_of_month) {
						$res_day = max($res_day, $after_day);
					}

					$month_length = days_in_month($res_month, $res_year);

					$result = sprintf('%04d-%02d-%02d',$res_year,$res_month,$res_day);
					while (date('w', iso8601_ts($result)) != $target_day_of_week) {
						$res_day += (7 + $target_day_of_week - date('w', iso8601_ts($result)) - 1)  % 7 + 1;

						$result = sprintf('%04d-%02d-%02d',$res_year,$res_month,$res_day);
						if (($res_day > $month_length) || ($res_day > (($target_week_of_month+1) * 7))) {

							// run out of days, so we'll have to start again in the next valid month
							increment_month($res_month, $res_year, $interval);
							$month_length = days_in_month($res_month, $res_year);
							$res_day = ($target_week_of_month * 7)+1;
						}
						$result = sprintf('%04d-%02d-%02d',$res_year,$res_month,$res_day);
					}
				break;

				// Every 'x' months on the 'y'th last day of the month
				case 'MRN':
					$target_days_from_end = days_in_month($start_month, $start_year) - $start_day;

					if (($month_diff % $interval) != 0) {
						increment_month($res_month, $res_year, $interval - ($month_diff % $interval));
					} else if ((days_in_month($res_month, $res_year) - $after_day) < $target_days_from_end) {
						increment_month($res_month, $res_year, $interval);
					}

					$result = sprintf('%04d-%02d-%02d',$res_year,$res_month,days_in_month($res_month, $res_year) - $target_days_from_end);

					while ((days_in_month($res_month, $res_year) - $target_days_from_end) < 0) {
						increment_month($res_month, $res_year, $interval);

						$result = sprintf('%04d-%02d-%02d',$res_year,$res_month,days_in_month($res_month, $res_year) - $target_days_from_end);
					}
				break;

				// Every 'x' months on the 'y'th last weekday of the month
				case 'MRW':
					$min_day = 1;
					$target_weeks_from_end = (int)((days_in_month($start_month, $start_year) - $start_day) / 7);
					$target_day_of_week = (date('w', iso8601_ts($start)));

					if (($month_diff % $interval) != 0) {
						// go to the next valid month
						increment_month($res_month, $res_year, $interval - ($month_diff % $interval));
					} else if ((int)((days_in_month($after_month, $after_year) - $after_day) / 7) < $target_weeks_from_end) {
						// we're too far into this month, so go to the next valid month
						increment_month($res_month, $res_year, $interval);
					} else if ((int)((days_in_month($after_month, $after_year) - $after_day) / 7) == $target_weeks_from_end) {
						// we're in the right week so start from 'now', not the begining of the week,
					}
					$min_day = $after_day;

					$res_day = days_in_month($res_month, $res_year) - ($target_weeks_from_end * 7);
					$result = sprintf('%04d-%02d-%02d',$res_year,$res_month,$res_day);

					while (date('w', iso8601_ts($result)) != $target_day_of_week) {
						// if ($res_month - $start_month > 2) die;
						$res_day -= (7 + date('w', iso8601_ts($result)) - $target_day_of_week)  % 7;

						if (($res_day < $min_day) || ($result < $after)) {
							// can't get the nth-last xday in this month, either because
							// it's already passed or because it doesn't exist, so goto next month
							increment_month($res_month, $res_year, $interval);
							$res_day = days_in_month($res_month, $res_year) - ($target_weeks_from_end * 7);
							$min_day = 1;

						}
						$result = sprintf('%04d-%02d-%02d',$res_year,$res_month,$res_day);
					}
				break;
			}//end switch

			if (isset($this) && is_a($this, 'Calendar_Event_Recurring') && $exception_link = $this->getExceptionLink($result, $this->id)) {
				$after  = add_days_to_iso($result);
				$result = '';
			}
		}//end while

		if (!empty($stop) && ($result > $stop)) {
			return FALSE;
		} else {
			return $result;
		}

	}//end getFirstOccurrenceAfter()


	/**
	* Tells us whether this event has an occurrence on the specified date
	*
	* This function can be called on an object (with just the first parameter) or
	* statically (with all the parameters). If only $after is specified the object's
	* attributes are used.
	*
	* @param string	$date		the date we're interested in, in YYYY-MM-DD format - default
	*							is today's date (time can be passed in too but it will be
	*							stripped)
	* @param string	$start		The iso datetime at which the event's 1st instance started
	* @param string	$freq_type	3-letter frequency code
	* @param int	$interval	The frequency interval
	* @param string	$stop		The date after which recurrences should stop
	*
	* @return boolean
	* @access public
	* @static
	*/
	function hasOccurrenceOnDate($date=NULL, $start=NULL, $freq_type=NULL, $interval=NULL, $stop=NULL)
	{
		// if we are calling statically make sure that we have all our arguments
		if (is_null($this) && (func_num_args() < 5)) {
			trigger_localised_error('CAL0031', E_USER_ERROR);
		}

		// fill in defaults if we haven't been provided with all values
		if (is_null($start)) {
			// called on an object
			$freq_type = substr($this->attr('frequency'), 0, 3);
			$interval = substr($this->attr('frequency'), 3);
			$start = substr($this->attr('start_date'),0,10);
			$stop = $this->attr('stop_date');
		}
		if (is_null($date)) $date = date('Y-m-d');
		if (strpos($stop, '----------') !== FALSE) {
			$stop = NULL;
		}

		// dispose of trivial cases
		if ($start > $date) return FALSE;
		if ((!is_null($stop)) && ($stop < $date)) {
			return FALSE;
		}

		// get components and get into it
		$date_components = Calendar_Common::getDateComponents($date);
		$start_components = Calendar_Common::getDateComponents($start);

		if (!$interval)	$interval = 1;
		if ($interval > 1) $freq_type .= 'x';

		switch ($freq_type) {
			case 'DED':
				// definitely occurs
				return TRUE;
			break;

			case 'DEDx':
				// days between start_date and date must be a multiple of $interval
				return (($date_components['ds'] - $start_components['ds']) % $interval == 0);
			break;

			case 'DWD':
				// definitely occurs unless this is a weekend day
				return !in_array($date_components['wday'], Array(0, 6));
			break;

			case 'DWE':
				// definitely occurs if this is a weekend day
				return in_array($date_components['wday'], Array(0, 6));
			break;

			case 'WEW':
				// occurs if start_date and date are the same day of the week
				return ($date_components['wday'] == $start_components['wday']);
			break;

			case 'WEWx':
				// occurs if start_date and date are the same day of the week
				// AND the number of weeks between start_date and date is a multiple of $interval
				return ($date_components['wday'] == $start_components['wday'])
					&& (((($date_components['ds'] - $start_components['ds']) / 7) % $interval) == 0);
			break;

			case 'MFN':
				// occurs if date and start_date are the same day of the month
				return ($date_components['mday'] == $start_components['mday']);
			break;

			case 'MFNx':
				// occurs if date and start_date are the same day of the month
				// AND the number of months in between is a multiple of $interval
				return ($date_components['mday'] == $start_components['mday'])
					&& ((($date_components['ms'] - $start_components['ms']) % $interval) == 0);
			break;

			case 'MFW':
				// occurs if date and start_date are the same day of the week
				// AND they are in the same week of the month
				return ($date_components['wday'] == $start_components['wday'])
					&& ($date_components['wdom'] == $start_components['wdom']);
			break;

			case 'MFWx':
				// occurs if date and start_date are the same day of the week
				// AND they are in the same week of the month
				// AND the number of months in between is a multiple of $interval
				return ($date_components['wday'] == $start_components['wday'])
					&& ($date_components['wdom'] == $start_components['wdom'])
					&& ((($date_components['ms'] - $start_components['ms']) % $interval) == 0);
			break;

			case 'MRN':
				// occurs if monthlength - day_of_month is equal for date and start_date
				return ($start_components['rmday'] == $date_components['rmday']);
			break;

			case 'MRNx':
				// occurs if monthlength - day_of_month is equal for date and start_date
				// AND number of months between start_date and date is a multiple of $interval
				return ($start_components['rmday'] == $date_components['rmday'])
					&& ((($date_components['ms'] - $start_components['ms']) % $interval) == 0);
			break;

			case 'MRW':
				// occurs if date and start_date are the same day of the week...
				// AND they are in the same week from the end of the month
				return ($date_components['wday'] == $start_components['wday'])
					&& ($date_components['rwdom'] == $start_components['rwdom']);
			break;

			case 'MRWx':
				// occurs if date and start_date are the same day of the week...
				// AND they are in the same week from the end of the month
				// AND the number of months between is a multiple of $interval
				return ($date_components['wday'] == $start_components['wday'])
					&& ($date_components['rwdom'] == $start_components['rwdom'])
					&& ((($date_components['ms'] - $start_components['ms']) % $interval) == 0);
			break;
		}//end switch
		return FALSE;

	}//end hasOccurrenceOnDate()


	/**
	* Returns the start and end dates/times of the instance containing a specified ISO date
	* If the $date falls within the range of a certain event, it will return that date
	* range. If $date does NOT fall within a recurrence it will return the dates of the NEXT
	* instance that starts after $date.
	*
	* Return value is Array('start' => start date/time of instance,
	*						'end' => end date/time of instance)
	*
	* @param string	$date	The date to find an instance with
	*
	* @return array
	* @access public
	*/
	function getInstanceDates($date)
	{
		if (empty($this->_tmp['instance_dates'][$date])) {
			$res = Array();
			if ($this->hasOccurrenceOnDate($date)) {
				$res['start'] = $date;
			} else if (!$this->getEndDate()) {
				$res['start'] = $this->getFirstOccurrenceAfter($date);
			} else {
				$period = substr($this->attr('frequency'), 3);
				if (!$period) $period = 1;
				switch (substr($this->attr('frequency'), 0, 1)) {
					case 'D':
						$first_occur = add_days_to_iso($date, -$period);
					break;

					case 'W':
						$first_occur = add_days_to_iso($date, -$period*7);
					break;

					case 'M':
						$first_occur = add_months_to_iso($date, -$period).'-'.date('d', iso8601_ts($date));
					break;

				}
				$res['start'] = $this->getFirstOccurrenceAfter($first_occur);
			}
			
			if ($this->getStartTime()) $res['start'] .= ' '.$this->getStartTime();
			
			$duration = $this->getDuration();
			$res['end'] = date('Y-m-d', iso8601_ts($res['start']) + $duration);
			
			if ($date > $res['end']){
				$res['start'] = $this->getFirstOccurrenceAfter($date);
				if ($this->getStartTime()) $res['start'] .= ' '.$this->getStartTime();
				$res['end'] = date('Y-m-d', iso8601_ts($res['start']) + $duration);
			}
			
			if ($this->getEndTime()) $res['end'] .= ' '.$this->getEndTime();
			
			$this->_tmp['instance_dates'][$date] = $res;
		}
		return $this->_tmp['instance_dates'][$date];

	}//end getInstanceDates()


	/**
	* Returns this event's frequency in iCal Recurring Rule format
	*
	* @return string
	* @access public
	*/
	function getiCalFrequency()
	{
		$freq_type = substr($this->attr('frequency'), 0, 3);
		$interval = substr($this->attr('frequency'), 3);
		$rule = '';
		switch ($freq_type) {
			// every day (or x days)
			case 'DED':
				if ($interval) {
					$rule .= 'FREQ=DAILY;INTERVAL='.$interval;
				} else {
					$rule .= 'FREQ=DAILY;INTERVAL=1';
				}
			break;

			// every weekday
			case 'DWD':
				if ($interval) {
					$rule .= 'FREQ=WEEKLY;BYDAY=MO,TU,WE,TH,FR;INTERVAL='.$interval;
				} else {
					$rule .= 'FREQ=WEEKLY;BYDAY=MO,TU,WE,TH,FR;INTERVAL=1';
				}
			break;

			// every weekday
			case 'DWE':
				if ($interval) {
					$rule .= 'FREQ=WEEKLY;BYDAY=SA,SU;INTERVAL='.$interval;
				} else {
					$rule .= 'FREQ=WEEKLY;BYDAY=SA,SU;INTERVAL=1';
				}
			break;

			// every week (or x weeks)
			case 'WEW':
				if ($interval) {
					$rule .= 'FREQ=WEEKLY;INTERVAL='.$interval;
				} else {
					$rule .= 'FREQ=WEEKLY;INTERVAL=1';
				}

			break;

			// every month (or x months) on the yth day of the month
			case 'MFN':
				$day = (int)substr($this->attr('start_date'), 8, 2);
				if ($interval) {
					$rule .= 'FREQ=MONTHLY;INTERVAL='.$interval;
				} else {
					$rule .= 'FREQ=MONTHLY;INTERVAL=1';
				}
			break;

			// every month (or x months) on the yth weekday of the month
			case 'MFW':
				$day = (int)substr($this->attr('start_date'), 8, 2);
				$counter = (int)(($day-1) / 7) + 1;
				$day_of_week = strtoupper(substr(date('l', iso8601_ts($this->attr('start_date'))), 0, 2));

				if ($interval) {
					$rule .= 'FREQ=MONTHLY;BYDAY='.$counter.$day_of_week.';INTERVAL='.$interval;
				} else {
					$rule .= 'FREQ=MONTHLY;BYDAY='.$counter.$day_of_week.';INTERVAL=1';
				}
			break;

			// every month (or x months) on the yth LAST day of the month
			case 'MRN':
				$day = (int)substr($this->attr('start_date'), 8, 2);
				$month = (int)substr($this->attr('start_date'), 5, 2);
				$year = (int)substr($this->attr('start_date'), 0, 4);
				$month_length = date('j', strtotime('yesterday', iso8601_ts($year.'-'.($month+1).'-01')));
				$day_of_month = (int) $month_length - $day + 1;

				if ($interval) {
					$rule .= 'FREQ=MONTHLY;INTERVAL='.$interval.';BYMONTHDAY=-'.$day_of_month;
				} else {
					$rule .= 'FREQ=MONTHLY;BYMONTHDAY=-'.$day_of_month;
				}
			break;

			// every month (or x months) on the yth LAST weekday of the month
			case 'MRW':
				$day = (int)substr($this->attr('start_date'), 8, 2);
				$day_of_week = strtoupper(substr(date('l', iso8601_ts($this->attr('start_date'))), 0, 2));
				$month = (int)substr($this->attr('start_date'), 5, 2);
				$year = (int)substr($this->attr('start_date'), 0, 4);
				$month_length = date('j', strtotime('yesterday', iso8601_ts($year.'-'.($month+1).'-01')));
				$day_of_month = (int)((($month_length - $day + 1) / 7) + 1);

				if ($interval) {
					$rule .= 'FREQ=MONTHLY;INTERVAL='.$interval.';BYDAY=-'.$day_of_month.$day_of_week;
				} else {
					$rule .= 'FREQ=MONTHLY;INTERVAL=1;BYDAY=-'.$day_of_month.$day_of_week;
				}
			break;

		}//end switch

		// recurs until (only include 'UNTIL' part if the stop date is defined)
		$stop_date_attr = $this->getAttribute('stop_date');
		if ($stop_date_attr->value != '---------- --:--:--') {
			$rule .= ';UNTIL='.date('Ymd\THis', iso8601_ts($stop_date_attr->value));
		}
		return $rule;

	}//end getiCalFrequency()


	/**
	* Returns this event's frequency in an easy-to-understand format
	*
	* @return string
	* @access public
	*/
	function getFriendlyFrequency()
	{
		$freq_type = substr($this->attr('frequency'), 0, 3);
		$interval = substr($this->attr('frequency'), 3);
		$monthly_type = '';
		switch ($freq_type) {
			// every day (or x days)
			case 'DED':
				if ($interval) {
					$res = translate('cal_event_every_x_days', $interval);
				} else {
					$res = translate('cal_event_every_day');
				}
			break;

			// every weekday
			case 'DWD':
				$res = translate('cal_event_every_weekday');
			break;

			// every weekend day
			case 'DWE':
				$res = translate('cal_event_every_weekend');
			break;

			// every week (or x weeks)
			case 'WEW':
				$dow = date('l', iso8601_ts($this->attr('start_date')));
				if ($interval) {
					$res = translate('cal_event_every_x_weeks', $interval, $dow);
				} else {
					$res = translate('cal_event_every_week', $dow);
				}
			break;

			// every month (or x months) on the yth day of the month
			case 'MFN':
				$day = (int)substr($this->attr('start_date'), 8, 2);
				if ($interval) {
					$res = translate('cal_event_every_x_months_first_day', $interval, add_ordinal_suffix($day));
				} else {
					$res = translate('cal_event_every_month_first_day', add_ordinal_suffix($day));
				}
			break;

			// every month (or x months) on the yth weekday of the month
			case 'MFW':
				$day = (int)substr($this->attr('start_date'), 8, 2);
				$day_of_week = date('l', iso8601_ts($this->attr('start_date')));
				if ($interval) {
					$res = translate('cal_event_every_x_months_first_weekday', $interval, add_ordinal_suffix((int)((($day-1) / 7) + 1)), $day_of_week);
				} else {
					$res = translate('cal_event_every_month_first_weekday', add_ordinal_suffix((int)((($day-1) / 7) + 1)), $day_of_week);
				}
			break;

			// every month (or x months) on the yth LAST day of the month
			case 'MRN':
				$day = (int)substr($this->attr('start_date'), 8, 2);
				$month = (int)substr($this->attr('start_date'), 5, 2);
				$year = (int)substr($this->attr('start_date'), 0, 4);
				$month_length = date('j', strtotime('yesterday', iso8601_ts($year.'-'.($month+1).'-01')));
				if ($interval) {
					$res = translate('cal_event_every_x_months_last_day', $interval, add_reverse_ordinal_suffix($month_length - $day + 1));
				} else {
					$res = translate('cal_event_every_month_last_day', add_reverse_ordinal_suffix($month_length - $day + 1));
				}
			break;

			// every month (or x months) on the yth LAST weekday of the month
			case 'MRW':
				$day = (int)substr($this->attr('start_date'), 8, 2);
				$day_of_week = date('l', iso8601_ts($this->attr('start_date')));
				$month = (int)substr($this->attr('start_date'), 5, 2);
				$year = (int)substr($this->attr('start_date'), 0, 4);
				$month_length = date('j', strtotime('yesterday', iso8601_ts($year.'-'.($month+1).'-01')));
				if ($interval) {
					$res = translate('cal_event_every_x_months_last_weekday', $interval, add_reverse_ordinal_suffix((int)((($month_length - $day + 1) / 7) + 1)), $day_of_week);
				} else {
					$res = translate('cal_event_every_month_last_weekday', add_reverse_ordinal_suffix((int)((($month_length - $day + 1) / 7) + 1)), $day_of_week);
				}
			break;

			// unknown frequency
			default:
				$res = translate('cal_event_unknown_frequency', $this->attr('frequency'));
		}//end switch

		$stop_date_attr = $this->getAttribute('stop_date');
		if (($stop_timestamp = $stop_date_attr->getTimestamp()) == -1) {
			return $res.', '.translate('cal_event_recur_until_further_notice');
		} else {
			return $res.', '.translate('cal_event_recur_until_date', date('jS M Y', $stop_timestamp));
		}

	}//end getFriendlyFrequency()

	
	/**
	* Returns recurring event's frequency in days
	*
	* @param array $event	Event data (needs to supplied when called statically)
	*
	* @return int|boolean
	* @access public
	*/	
	function getEventFrequency($event=NULL)
	{	
		// If called statically, make sure we have relevant arguments
		if (isset($this) && is_a($this, 'Calendar_Event_Recurring')) {
			$freq_type = substr($this->attr('frequency'), 0, 3);
			$interval = substr($this->attr('frequency'), 3);			
		} else {
			if (is_null($event)) {
				return FALSE;
			} else {
				$freq_type = array_get_index($event, 'frequency', '');;
				$interval = array_get_index($event, 'period', '');
			}		
		}
		
		if (empty($interval)) {
			$interval = 1;
		}
		
		$frequency = FALSE;		
		switch ($freq_type) {
			case 'DED':
				$frequency = 1*$interval; // Daily, 1 day gap between the events
			break;

			case 'DWD':
				$frequency = 2*$interval; // Weekdays, 2 days gap on weekends
			break;

			case 'DWE':
				$frequency = 5*$interval; // Weekends, 5 days gap on weekdays
			break;

			case 'WEW':
				$frequency = 7*$interval; // Weekly, 7 days gap
			break;

			case 'MFN':
			case 'MFW':
			case 'MRN':
			case 'MRW':
				$frequency = 30*$interval; // Monthly, 30 days gap
			break;
		
		}//end switch
		
		return $frequency;
	
	}//end getEventFrequency()
	

	/**
	* Checks for cancellations and creates the iCalendar exception string for each one
	*
	* @param int	$id	Id
	*
	* @return string
	* @access public
	*/
	function getiCalExceptions($id=NULL)
	{
		$exception_string = '';
		if ($this->id <> 0) {
			$cancellations = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_SC_LINK_ALL, Array('calendar_event_cancellation'), FALSE, 'major');

			foreach ($cancellations as $cancelled_event) {
				$event = $GLOBALS['SQ_SYSTEM']->am->getAsset($cancelled_event['minorid'], $cancelled_event['minor_type_code']);
				if ($event->readAccess()) {
					$exception_string .= date('Ymd\THis', iso8601_ts($event->attr('start_date'))).',';
				}
			}
			if (substr($exception_string, -1) == ',') {
				$exception_string = substr($exception_string, 0, -1);
			}
		}
		return $exception_string;

	}//end getiCalExceptions()


	/**
	* Called by the design to print the body of this asset
	*
	* @return void
	* @access public
	*/
	function printBody()
	{
		?>
		<table border="0" cellspacing="2" cellpadding="2">
			<tr>
				<td><b><?php echo translate('name'); ?></b></td>
				<td><?php echo $this->attr('name'); ?></td>
			</tr>
			<tr>
				<td><b><?php echo translate('cal_event_first_start'); ?></b></td>
				<td><?php echo $this->getFriendlyStart(); ?></td>
			</tr>
			<tr>
				<td><b><?php echo translate('cal_event_first_end'); ?></b></td>
				<td><?php echo $this->getFriendlyEnd(); ?></td>
			</tr>
			<tr>
				<td><b><?php echo translate('cal_event_instance_duration'); ?></b></td>
				<td><?php
						// give useful info if no duration - easy_time_total returns empty string
						$total_time = easy_time_total($this->getDuration(), TRUE);
						echo (empty($total_time) ? 'N/A' : $total_time);
					?></td>
			</tr>
			<tr>
				<td><b><?php echo translate('frequency'); ?></b></td>
				<td><?php echo $this->getFriendlyFrequency() ?></td>
			</tr>
			<tr>
				<td><b><?php echo translate('description'); ?></b></td>
				<td><?php echo $this->getKeywordReplacement('asset_attribute_description'); ?></td>
			</tr>
		</table>
		<?php

	}//end printBody()


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @return array
	* @access private
	*/
	function _getAllowedLinks()
	{
		$links = parent::_getAllowedLinks();
		$links[SQ_LINK_TYPE_2]['calendar_event_modification'] = Array('card' => 'M', 'exclusive' => FALSE);
		$links[SQ_LINK_TYPE_2]['calendar_event_cancellation'] = Array('card' => 'M', 'exclusive' => FALSE);
		return $links;

	}//end _getAllowedLinks()


	/**
	* Prepares for linking by checking that the link values are valid for this link
	*
	* This function will be called if this asset is the major or minor party in the link, so the side_of_link flag
	* should be checked to work out what side of the link this asset is on. The return value should indicate
	* if any of the link fields were changed.
	*
	* @param object	&$asset			the major or minor asset that we are linking to
	* @param string	$side_of_link	the side of the link we are on (major or minor)
	* @param string	&$link_type		the type of link we are creating
	* @param string	&$value			the value that is to be associated with the link
	* @param string	&$sort_order	the position in the links list that this link should take,
	* @param string	&$dependant		'0' / '1' on whether the this asset is dependant on the
	*								asset that will be linked by the new link
	* @param string	&$exclusive		'0' / '1' on whether the this asset is linked exclusivly
	*								to the asset that will be linked by the new link
	*
	* @return boolean
	* @access public
	*/
	function prepareLink(&$asset, $side_of_link, &$link_type, &$value, &$sort_order, &$dependant, &$exclusive)
	{
		if (is_a($asset, 'calendar_event_modification') || is_a($asset, 'calendar_event_cancellation')) {
			$link_type = SQ_LINK_TYPE_2;
		}
		return TRUE;

	}//end prepareLink()


	/**
	* Get bodycopy keyword replacement
	*
	* Provides a mechanism for providing keyword replacement functions by
	* looking up the function named 'getKeywordNameKeywordReplacement'
	* so an asset wishing to add support for 'best_ever' custom keyword
	* should implement a function getBestEverKeywordReplacement()
	* which returns a replacement string
	*
	* @param string	$keyword	Keyword to find a replacement for
	*
	* @return string
	* @access public
	*/
	function getKeywordReplacement($keyword)
	{
		$full_keyword = $keyword;
		$keyword = parse_keyword($keyword, $modifiers);

		switch ($keyword) {
			case 'edit_link':
				$replacement = $this->writeAccess('') ? '<a href="'.htmlspecialchars(replace_query_string_vars(Array('SQ_CALENDAR_VIEW'=>'edit', 'SQ_CALENDAR_EVENT_ID'=>$this->id))).'">'.translate('cal_event_recurring_edit_link_text').'</a>' : '';
				break;

			case 'event_frequency':
				$replacement = $this->getFriendlyFrequency();
				break;

			case 'event_frequency_summary':
				$frequency_threshold = $GLOBALS['SQ_SYSTEM']->getUserPrefs('calendar_event_recurring', 'SQ_CALENDAR_EVENTS_FREQUENCY');
				$event_frequency = $this->getEventFrequency();
				// If the event's frequency less than the predefined frequency threshold, display the event's recurrance summary
				$replacement = ($event_frequency < $frequency_threshold) ? $this->getFriendlyFrequency() : '';
				break;

			case 'event_ical_frequency':
				$replacement = $this->getiCalFrequency();
				break;

			case 'event_ical_exceptions':
				$replacement = $this->getiCalExceptions();
				break;

			default:
				if (in_array($keyword, Array('instance_start_datetime', 'instance_end_datetime', 'cancel_link', 'modify_link'))) {
					if (isset($_REQUEST['SQ_CALENDAR_DATE'])) {
						if ($exception_link = $this->getExceptionLink($_REQUEST['SQ_CALENDAR_DATE'])) {
							// exception today
							$replacement = '';
						} else {
							// we are showing a normal instance of this event
							$instance_dates = $this->getInstanceDates($_REQUEST['SQ_CALENDAR_DATE']);
							switch ($keyword) {
								case 'instance_start_datetime':								
									if (!isset($this->vars['start_date_ts'])) {
										$replacement = $instance_dates['start'];
									} else {
										// If the event is the shadow asset, then instance data is available as asset attribute
										$replacement = parent::getKeywordReplacement($keyword);
									}
									break;

								case 'instance_end_datetime':
									if (!isset($this->vars['start_end_ts'])) {
										$replacement = $instance_dates['end'];
									} else {
										// If the event is the shadow asset, then instance data is available as asset attribute
										$replacement = parent::getKeywordReplacement($keyword);
									}
									break;

								case 'cancel_link':
									$write_access = $this->writeAccess('');
									$replacement = $write_access ? $this->_getCancelLink() : translate('cal_event_cancel_link_default');
									break;

								case 'modify_link':
									$write_access = $this->writeAccess('');
									$replacement = $write_access ? $this->_getModifyLink() : translate('cal_event_modify_link_default');
									break;
							}
						}
					} else if ($keyword == 'instance_start_datetime' || $keyword == 'instance_end_datetime') {
						// If the event is the shadow asset, then instance data is available as asset attribute
						$replacement = parent::getKeywordReplacement($keyword);
					} else {

						// somehow we're just viewing the event by itself, so set the instance-specific
						// keywords to blank for tidiness's sake
						$replacement = '';
					}

					// break outta switch block
					break;
				}

				$matches = Array();
				preg_match('/^instance_start_datetime_(.+)/', $keyword, $matches);
				if (!empty($matches)) {
					if (!empty($_REQUEST['SQ_CALENDAR_DATE'])) {
						if ($exception_link = $this->getExceptionLink($_REQUEST['SQ_CALENDAR_DATE'])) {
							// exception today
							$replacement = '';
						} else {
							// we are showing a normal instance of this event
							$instance_dates = $this->getInstanceDates($_REQUEST['SQ_CALENDAR_DATE']);
							switch ($matches[1]){
								case 'ical':
									// If theres no time bit, then only use the date portion to get the correct iCal timestamp  
									if (preg_match("/^(.*?) (--):(--):(--)$/", $instance_dates['start'])) {
										$replacement = date('Ymd', iso8601_ts($instance_dates['start']));
									} else {
										$replacement = date('Ymd\THis', iso8601_ts($instance_dates['start']));
									}
									break;
								default:
									$replacement = date($matches[1], iso8601_ts($instance_dates['start']));
							}							
						}
					} else {
						// If the event is the shadow asset, then instance data is available as asset attribute
						$replacement = parent::getKeywordReplacement($keyword);
					}
					// break outta switch block
					break;
				}

				$matches = Array();
				preg_match('/^instance_start_(.+)/', $keyword, $matches);
				if (!empty($matches)) {
					if (!empty($_REQUEST['SQ_CALENDAR_DATE'])) {
						if ($exception_link = $this->getExceptionLink($_REQUEST['SQ_CALENDAR_DATE'])) {
							// exception today
							$replacement = '';
						} else {
							// we are showing a normal instance of this event
							$instance_dates = $this->getInstanceDates($_REQUEST['SQ_CALENDAR_DATE']);
							switch ($matches[1]){
								case 'date':
									$replacement = date('jS M Y', iso8601_ts($instance_dates['start']));
									break;

								case 'time':
									$iso_formated = iso8601_ts($instance_dates['start']);
									$mins = date('i', $iso_formated);
									if ($mins == '00') {
										$replacement = date('ga', $iso_formated);
									} else {
										$replacement = date('g:ia', $iso_formated);
									}
									break;

								case 'time_12h':
									$replacement = date('h:i a', iso8601_ts($instance_dates['start']));
									break;

								case 'time_24h':
									$replacement = date('H:i', iso8601_ts($instance_dates['start']));
									break;

								default:
									$replacement = '';
									break;
							}							
						}
					} else {
						// If the event is the shadow asset, then instance data is available as asset attribute
						$replacement = parent::getKeywordReplacement($keyword);
					}
					// break outta switch block
					break;
				}

				$matches = Array();
				preg_match('/^instance_end_datetime_(.+)/', $keyword, $matches);
				if (!empty($matches)) { 
					if (!empty($_REQUEST['SQ_CALENDAR_DATE'])) {
						if ($exception_link = $this->getExceptionLink($_REQUEST['SQ_CALENDAR_DATE'])) {
							// exception today
							$replacement = '';
						} else {
							// we are showing a normal instance of this event
							$instance_dates = $this->getInstanceDates($_REQUEST['SQ_CALENDAR_DATE']);
							switch ($matches[1]){
								case 'ical':
									if (preg_match("/^(.*?) (--):(--):(--)$/", $instance_dates['end'])) {
										$replacement = date('Ymd', iso8601_ts($instance_dates['end']));
									} else {
										$replacement = date('Ymd\THis', iso8601_ts($instance_dates['end']));
									}
									break;
								default:
									$replacement = date($matches[1], iso8601_ts($instance_dates['end']));
							}
						}
					} else {
						// If the event is the shadow asset, then instance data is available as asset attribute
						$replacement = parent::getKeywordReplacement($keyword);					
					}
					// break outta switch block
					break;
				}

				$matches = Array();
				preg_match('/^instance_end_(.+)/', $keyword, $matches);
				if (!empty($matches)) {
					if (!empty($_REQUEST['SQ_CALENDAR_DATE'])) {
						if ($exception_link = $this->getExceptionLink($_REQUEST['SQ_CALENDAR_DATE'])) {
							// exception today
							$replacement = '';
						} else {
							// we are showing a normal instance of this event
							$instance_dates = $this->getInstanceDates($_REQUEST['SQ_CALENDAR_DATE']);
							switch ($matches[1]){
								case 'date':
									$replacement = date('jS M Y', iso8601_ts($instance_dates['end']));
									break;

								case 'time':
									$iso_formated = iso8601_ts($instance_dates['end']);
									$mins = date('i', $iso_formated);
									if ($mins == '00') {
										$replacement = date('ga', $iso_formated);
									} else {
										$replacement = date('g:ia', $iso_formated);
									}
									break;

								case 'time_12h':
									$replacement = date('h:i a', iso8601_ts($instance_dates['end']));
									break;

								case 'time_24h':
									$replacement = date('H:i', iso8601_ts($instance_dates['end']));
									break;

								default:
									$replacement = '';
									break;
							}							
						}
					} else {
						// If the event is the shadow asset, then instance data is available as asset attribute
						$replacement = parent::getKeywordReplacement($keyword);
					}
					// break outta switch block
					break;
				}


				$matches = Array();
				preg_match('/^event_stop_date_(.+)/', $keyword, $matches);
				$stop_date = $this->attr('stop_date');
				if ((!empty($matches)) && !empty($stop_date) && (FALSE === strpos('----', $stop_date))) {
					$replacement = date($matches[1], iso8601_ts($stop_date));

					// break outta switch block
					break;
				}
				if (in_array($keyword, Array('asset_url', 'asset_href'))) {
					$res = parent::getKeywordReplacement($keyword);
					if (!empty($_REQUEST['SQ_CALENDAR_DATE'])) {
						$res = replace_query_string_vars(Array('SQ_CALENDAR_DATE' => $_REQUEST['SQ_CALENDAR_DATE']), $res);
					}

					// Produce some valid XHTML
					$replacement = htmlentities($res);

					// break outta switch block
					break;
				}

				$replacement = parent::getKeywordReplacement($keyword);
				$empty_replacement = (((string)$replacement) == '%'.$keyword.'%');

				// If we have not found an appropriate replacement, ask the parent, if it is a Calendar Multi Date Event asset
				if ($empty_replacement && (count($this->parent_links) > 0)) {
					// Only expecting one relevant link
					$parent_link = reset($this->parent_links);

					$parent_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($parent_link['majorid']);
					$replacement = $parent_asset->getKeywordReplacement($keyword);
					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($parent_asset);
				}

		}//end switch

		if ($replacement != '%'.$keyword.'%') {
			apply_keyword_modifiers($replacement, $modifiers, Array('assetid' => $this->id));
		} else {
			$replacement = '%'.$full_keyword.'%';
		}

		return $replacement;

	}//end getKeywordReplacement()


	/**
	* Gets the link details of the exception (cancellation or modification) event linked under
	* this event for the date given
	*
	* This function can be called statically (with both parameters) or on an object (with just
	* the first parameter)
	*
	* @param int	$date	YYYY-MM-DD date to consider
	* @param int	$id		The ID of the recurring event to test, or NULL to use the current object
	*
	* @return mixed array|boolean
	* @access public
	* @static (optionally)
	*/
	function getExceptionLink($date, $id=NULL)
	{
		if (is_null($id) && isset($this->_tmp['exception_link'])) {
			return $this->_tmp['exception_link'];
		}
		$search_id = is_null($id) ? $this->id : $id;
		$child_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($search_id, SQ_SC_LINK_ALL, Array('calendar_event_cancellation', 'calendar_event_modification'), FALSE, 'major', $date);
		$res = current($child_links);
		if ($res['minor_type_code'] == 'calendar_event_cancellation') {
			$replacement_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($res['minorid'], SQ_SC_LINK_ALL, 'calendar_event_single', FALSE);
			if (!empty($replacement_links)) {
				$res = current($replacement_links);
			}
		}
		if (is_null($id)) {
			$this->_tmp['exception_link'] = $res;
		}
		return $res;

	}//end getExceptionLink()


	/**
	* Return the link that the user can click to add a cancellation of this event from the event view
	*
	* @return string
	* @access private
	*/
	function _getCancelLink()
	{
		if (isset($_REQUEST['SQ_CALENDAR_DATE'])) {
			return '<a href="?SQ_CALENDAR_VIEW=event&amp;SQ_CALENDAR_EVENT_ID='.$this->id.'&amp;SQ_CALENDAR_EVENT_ACTION=cance11&amp;SQ_CALENDAR_DATE='.$_REQUEST['SQ_CALENDAR_DATE'].'" onclick="return confirm(\''.translate('cal_event_cancel_this_instance_confirm').'\');">'.translate('cal_event_cancel_this_instance').'</a>';
		} else {
			return translate('cal_event_cancel_link_default');
		}

	}//end _getCancelLink()


	/**
	* Return the link that the user can click to modify an instance of this event from the event view
	*
	* @return string
	* @access private
	*/
	function _getModifyLink()
	{
		if (isset($_REQUEST['SQ_CALENDAR_DATE'])) {
			return '<a href="?SQ_CALENDAR_VIEW=event&amp;SQ_CALENDAR_EVENT_ID='.$this->id.'&amp;SQ_CALENDAR_EVENT_ACTION=modify1&amp;SQ_CALENDAR_DATE='.$_REQUEST['SQ_CALENDAR_DATE'].'">'.translate('cal_event_modify_this_instance').'</a>';
		} else {
			return translate('cal_event_modify_link_default');
		}

	}//end _getModifyLink()


	/**
	* Process the specified action on this event, and return whatever html the action wants to show
	*
	* @param string	$action_name	The action to perform, eg 'cancel'
	*
	* @return string
	* @access public
	*/
	function processEventAction($action_name)
	{
		$res = '';
		switch ($action_name) {
			case 'cancel1':
				if (!isset($_REQUEST['SQ_CALENDAR_DATE'])) {
					trigger_localised_error('CAL0032', E_USER_WARNING);
					return '';
				}
				$res = '<form method="POST" action="?SQ_CALENDAR_VIEW=event&amp;SQ_CALENDAR_EVENT_ID='.$this->id.'&amp;SQ_CALENDAR_EVENT_ACTION=cancel2&amp;SQ_CALENDAR_DATE='.$_REQUEST['SQ_CALENDAR_DATE'].'"><table cellpadding="4" border="0">';
				$res .= '<tr><td style="vertical-align: top">'.translate('cal_event_reason_for_cancellation').':</td><td><textarea rows="4" cols="40" name="event_cancellation_reason"></textarea></td></tr>';
				$res .= '<tr><td colspan="2"><input type="submit" value="'.translate('cal_event_add_cancellation').'" />&nbsp;&nbsp;<input type="button" value="'.translate('go_back').'" onclick="document.location=\''.$_SERVER['HTTP_REFERER'].'\'" /></td></tr></table></form>';
			break;

			case 'cancel2':
				if ((!isset($_REQUEST['SQ_CALENDAR_DATE'])) || (!isset($_REQUEST['event_cancellation_reason']))) {
					trigger_localised_error('CAL0033', E_USER_WARNING);
					return '';
				}

				$cancel_event = $this->addCancellation($_REQUEST['SQ_CALENDAR_DATE'], $_REQUEST['event_cancellation_reason']);

				$res = '<p>'.translate('cal_event_cancel_rep_invite_message_1', $this->attr('name'), date('jS M Y', strtotime($_REQUEST['SQ_CALENDAR_DATE'].' 12:00:00 GMT'))).'</p>';
				$res .= '<p style="margin: 0px; ">'.translate('cal_event_cancel_rep_invite_message_2').'<p>';
				$res .= '<form style="margin: 0px; float: left" method="POST" action="?SQ_CALENDAR_VIEW=event&amp;SQ_CALENDAR_EVENT_ID='.$cancel_event->id.'&amp;SQ_CALENDAR_EVENT_ACTION=addreplacement&amp;SQ_CALENDAR_DATE='.$_REQUEST['SQ_CALENDAR_DATE'].'"><input type="submit" value="'.translate('yes').'" /></form><form style="margin: 0px 1ex; float: left" method="POST" action="?SQ_CALENDAR_VIEW=event&amp;SQ_CALENDAR_EVENT_ID='.$this->id.'&amp;SQ_CALENDAR_DATE='.$_REQUEST['SQ_CALENDAR_DATE'].'"><input type="submit" value="'.translate('no').'" /></form>';
			break;

			case 'modify1':
				if (!isset($_REQUEST['SQ_CALENDAR_DATE'])) {
					trigger_localised_error('CAL0034', E_USER_WARNING);
					return '';
				}
				$res = '<form method="POST" action="?SQ_CALENDAR_VIEW=event&amp;SQ_CALENDAR_EVENT_ID='.$this->id.'&amp;SQ_CALENDAR_EVENT_ACTION=modify2&amp;SQ_CALENDAR_DATE='.$_REQUEST['SQ_CALENDAR_DATE'].'"><table cellpadding="4" border="0">';

				$res .= '<tr><td style="vertical-align: top">'.translate('cal_event_start_and_end').'</td><td>';
				ob_start();
					$edit_fns = $this->getEditFns();
					$edit_fns->paintStartEndChooser($this, 'event');
					$res .= ob_get_contents();
				ob_end_clean();
				$res .= '</td></tr>';

				$res .= '<tr><td style="vertical-align: top">'.translate('description').':</td><td><textarea name="event_description">';
				$res .= $this->attr('description');
				$res .= '</textarea></td></tr>';

				$res .= '<tr><td colspan="2"><input type="submit" value="'.translate('cal_event_add_modification').'" />&nbsp;&nbsp;<input type="button" value="'.translate('cancel').'" onclick="document.location=\''.$_SERVER['HTTP_REFERER'].'\'" /></td></tr></table></form>';

			break;

			case 'modify2':
				// Figure out the original details of the occurrence we're modifying
				$occurrence_start = '';
				$occurrence_end = '---------- --:--:--';
				if ($this->hasOccurrenceOnDate($_REQUEST['SQ_CALENDAR_DATE'])) {
					$occurrence_start_date = $_REQUEST['SQ_CALENDAR_DATE'];
				} else {
					$occurrence_start_date = $this->getFirstOccurrenceAfter(date('Y-m-d', iso8601_ts($_REQUEST['SQ_CALENDAR_DATE']) - $this->getDuration()));
				}
				$occurrence_start = $occurrence_start_date.' ';
				$occurrence_start .= ($this->getStartTime() ? $this->getStartTime().':00' : '--:--:--');

				if ($duration = $this->getDuration()) {
					$occurrence_end = date('Y-m-d', iso8601_ts($occurrence_start) + $duration).' ';
					$occurrence_end .= ($this->getEndTime() ? $this->getEndTime().':00' : '--:--:--');
				}

				// Get the new details
				$edit_fns = $this->getEditFns();
				$new_details = $edit_fns->getStartEndChooserResult('event');

				// Compare old and new details
				if (($new_details['start'] == $occurrence_start) && ($new_details['end'] == $occurrence_end)) {
					// because the dates and times have not changed, we can just add a modification
					// with the new description
					$this->addModification($occurrence_start_date, $_REQUEST['event_description']);
					$res = '<p>'.translate('cal_event_instance_modified').'</p>';
				} else {
					// dates/times have changed, so we have to cancel and then reschedule
					$cancellation_event =& $this->addCancellation($occurrence_start_date, translate('cal_event_rescheduled_note'));

					$replacement =& $cancellation_event->addReplacement($new_details['start'], $new_details['end'] ,  $_REQUEST['event_description']);

					$res = '<p>Event rescheduled</p>';
					$new_query_string_vars = Array(
												'SQ_CALENDAR_EVENT_ID'		=> $replacement->id,
												'SQ_CALENDAR_DATE'			=> $replacement->getStartDate(),
												'SQ_CALENDAR_EVENT_ACTION'	=> NULL,
											 );
					$res .= '<p><a href="'.htmlspecialchars(replace_query_string_vars($new_query_string_vars)).'">'.translate('cal_event_view_replacement_event').'</a></p>';
				}
			break;

			default:
				$res = parent::processEventAction($action_name);
			break;
		}//end switch
		return $res;

	}//end processEventAction()


	/**
	* Add a cancellation event linked under this recurring event
	*
	* @param string	$date	the date of the occurrence to cancel (YYYY-MM-DD)
	* @param string	$reason	text to put in the description field of the cancellation event
	*
	* @return mixed object|boolean
	* @access public
	*/
	function &addCancellation($date, $reason)
	{
		if (!$this->writeAccess('')) {
			trigger_localised_error('CAL0035', E_USER_WARNING);
			return FALSE;
		}
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('calendar_event_cancellation');
		if ($existing_link = $this->getExceptionLink($date)) {
			if ($existing_link['minor_type_code'] == 'calendar_event_cancellation') {
				$cancellation_event = $GLOBALS['SQ_SYSTEM']->am->getAsset($existing_link['minorid']);
			} else {
				$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($existing_link['linkid']);
				$cancellation_event = new Calendar_Event_Cancellation();
			}
		} else {
			$cancellation_event = new Calendar_Event_Cancellation();
		}
		$cancellation_event->setAttrValue('start_date', $date.' '.($this->getStartTime() ? $this->getStartTime().':00' : '--:--:--'));
		$end_date = date('Y-m-d', iso8601_ts($cancellation_event->attr('start_date')) + $this->getDuration());
		$cancellation_event->setAttrValue('end_date', $end_date.' '.($this->getEndTime() ? $this->getEndTime().':00' : '--:--:--'));
		$cancellation_event->setAttrValue('description', $reason);
		if (!$cancellation_event->id) {
			$link = Array('asset' => &$this, 'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 0, 'value' => $date);
			$cancellation_event->create($link);
		} else {
			if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($cancellation_event->id, 'attributes')) {
				trigger_localised_error('CAL0036', E_USER_WARNING, $cancellation_event->name);
				return 0;
			}
			$cancellation_event->saveAttributes();
			$GLOBALS['SQ_SYSTEM']->am->releaseLock($cancellation_event->id, 'attributes');
		}
		$cancellation_event->processStatusChange($this->status);
		return $cancellation_event;

	}//end addCancellation()


	/**
	* Add a modification event linked under this recurring event
	*
	* @param string	$date			the date of the occurrence to cancel (YYYY-MM-DD)
	* @param string	$description	text to put in the description field of the cancellation event
	*
	* @return mixed object|boolean
	* @access public
	*/
	function &addModification($date, $description)
	{
		if (!$this->writeAccess('')) {
			trigger_localised_error('CAL0037', E_USER_WARNING);
			return FALSE;
		}
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('calendar_event_modification');

		if ($existing_link = $this->getExceptionLink($date)) {
			if ($existing_link['minor_type_code'] == 'calendar_event_modification') {
				$modification_event = $GLOBALS['SQ_SYSTEM']->am->getAsset($existing_link['minorid']);
			} else {
				$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($existing_link['linkid']);
				$modification_event = new Calendar_Event_Modification();
			}
		} else {
			$modification_event = new Calendar_Event_Modification();
		}
		$modification_event->setAttrValue('start_date', $date.' --:--:--');
		$modification_event->setAttrValue('description', $description);
		if (!$modification_event->id) {
			$link = Array('asset' => &$this, 'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 0, 'value' => $date);
			$modification_event->create($link);
		} else {
			if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($modification_event->id, 'attributes')) {
				trigger_localised_error('CAL0038', E_USER_WARNING, $modification_event->name);
				return 0;
			}
			$modification_event->saveAttributes();
			$GLOBALS['SQ_SYSTEM']->am->releaseLock($modification_event->id, 'attributes');
		}
		return $modification_event;

	}//end addModification()


	/**
	* Cache the calendar data's stuff
	*
	* @param boolean	$updating	are we updating (TRUE) or inserting a new record
	*								(FALSE - set only in create() call)
	*
	* @return boolean
	* @access private
	*/
	function cacheCalendarData($updating=TRUE)
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$db = MatrixDAL::getDb();

		$date_values = Array();
		foreach (Array('start_date', 'end_date') as $date_comp) {
			$date_values += Calendar_Common::getDateComponents($this->attr($date_comp), $date_comp.'_');
		}

		// frequency field => as thy will be set in the standard attribute value table
		$date_values['frequency'] = substr($this->attr('frequency'),0,3);
		if (strlen($this->attr('frequency')) > 3) {
			$date_values['period'] = intval(substr($this->attr('frequency'),3));
		} else {
			$date_values['period'] = 1;
		}

		// stop date
		if (strtotime(substr($this->attr('stop_date'),0,10)) != -1) {
			$date_values['stop_date'] = substr($this->attr('stop_date'),0,10);
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if (!$updating) {
			// set the asset ids
			$date_values['assetid'] = $this->id;

			for (reset($date_values); NULL !== ($key = key($date_values)); next($date_values)) {
				if (is_null($date_values[$key]))
					$date_values[$key] = 'NULL';
				else
					$date_values[$key] = MatrixDAL::quote($date_values[$key]);
			}
			$val_string = implode(',', $date_values);

			$sql = 'INSERT INTO
						sq_cal_date_val
						(
							'.implode(',', array_keys($date_values)).'
						)
						VALUES
						(
							'.$val_string.'
						)';

			try {
				$query = MatrixDAL::preparePdoQuery($sql);
				MatrixDAL::execPdoQuery($query);
			} catch (Exception $e) {
				throw new Exception($e->getMessage());
			}

		} else {
			// asset id becomes a where condition
			$sql = 'UPDATE
						sq_cal_date_val
					SET ';

			$set_array = Array();
			foreach ($date_values as $key => $value) {
				$set_array[] = $key.' = '.((is_null($value)) ? 'null' : MatrixDAL::quote($value));
			}
			$sql .= implode(',', $set_array);
			$sql .=' WHERE
						assetid = :assetid';

			try {
				$query = MatrixDAL::preparePdoQuery($sql);
				MatrixDAL::bindValueToPdo($query, 'assetid', $this->id);
				MatrixDAL::execPdoQuery($query);
			} catch (Exception $e) {
				throw new Exception($e->getMessage());
			}
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return TRUE;

	}//end cacheCalendarData()


	/**
	* Return the keywords for this asset type.
	*
	* This function is called in asset-listing situations like the asset listing, search page etc
	* The return value is in the form:
	* <PRE>
	* Array(
	*   'name' => 'description' => 'the asset name',
	* );
	* </PRE>
	*
	* @return array
	* @access public
	*/
	function getAvailableKeywords()
	{
		$res = parent::getAvailableKeywords();

		$our_keywords[] = 'asset_attribute_start_date'; // = translate('start_date_iso'); //'Start date (iso8601)';
		$our_keywords[] = 'asset_attribute_end_date'; // = translate('end_date_iso'); //'End date (iso8601)';
		$our_keywords[] = 'event_start_date'; // = translate('start_date_only_readable'); //'Event Start Date';
		$our_keywords[] = 'event_end_date'; // = tralsate('end_date_only_readable'); //'Event End Date';
		$our_keywords[] = 'event_start_datetime'; // = translate('Event start date/time';
		$our_keywords[] = 'event_end_datetime'; // = 'Event end date/time';
		$our_keywords[] = 'event_start_time_12h'; // = 'Start time (12 hour)';
		$our_keywords[] = 'event_end_time_12h'; // = 'End time (12 hour)';
		$our_keywords[] = 'event_start_time_24h'; // = 'Start time (24 hour)';
		$our_keywords[] = 'event_end_time_24h'; // = 'End time (24 hour)';
		$our_keywords[] = 'event_start_time_ical'; // = 'Start time (iCalendar)';
		$our_keywords[] = 'event_end_time_ical'; // = 'End time (iCalendar)';
		$our_keywords[] = 'event_duration'; // = 'Duration of the Event';
		$our_keywords[] = 'event_start_datetime_'; // = dynamic keyword to print start date and time
		$our_keywords[] = 'event_end_datetime_'; // = dynamic keyword to print end date and time
		$our_keywords[] = 'event_frequency';
		$our_keywords[] = 'instance_end_datetime';
		$our_keywords[] = 'instance_end_datetime_ical';
		$our_keywords[] = 'instance_end_datetime_';
		$our_keywords[] = 'instance_end_date';
		$our_keywords[] = 'instance_end_time';
		$our_keywords[] = 'instance_end_time_12h';
		$our_keywords[] = 'instance_end_time_24h';
		$our_keywords[] = 'instance_start_datetime';
		$our_keywords[] = 'instance_start_datetime_ical';
		$our_keywords[] = 'instance_start_datetime_';
		$our_keywords[] = 'instance_start_date';
		$our_keywords[] = 'instance_start_time';
		$our_keywords[] = 'instance_start_time_12h';
		$our_keywords[] = 'instance_start_time_24h';
		$our_keywords[] = 'event_stop_date_';
		$our_keywords[] = 'event_ical_frequency';
		$our_keywords[] = 'event_ical_exceptions';
		$our_keywords[] = 'event_frequency_summary';

		foreach ($our_keywords as $keyword) {
			$res[$keyword] = translate('cal_evt_recur_keyword_'.$keyword);
		}
		ksort($res);
		return $res;

	}//end getAvailableKeywords()


}//end class

?>
