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
* $Id: calendar_event_recurring.inc,v 1.24 2004/08/31 04:54:51 lwright Exp $
* $Name: not supported by cvs2svn $
*/

require_once  dirname(__FILE__).'/../../calendar_event/calendar_event.inc';
require_once SQ_FUDGE_PATH.'/datetime_field/datetime_field.inc';
require_once SQ_FUDGE_PATH.'/general/datetime.inc';

/**
* Calendar_Event_Recurring
*
* An event which recurs at a specified frequency after its first instance
*
* @author  Luke Wright <lwright@squiz.net>
* @euthor  Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.24 $ - 1.0
* @package MySource_Matrix_Packages
* @subpackage calendar
*/
class Calendar_Event_Recurring extends Calendar_Event
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Calendar_Event_Recurring($assetid=0)
	{
		$this->Calendar_Event($assetid);

	}//end Calendar_Event_Recurring()


	/**
	* Returns the number of seconds since the start of the day
	*
	* @param int	$timestamp	the date to test
	*
	* @return int	number of seconds since midnight
	* @access private
	*/
	function _secondsToday($timestamp)
	{
		return $timestamp - strtotime(gmdate('Y-m-d 00:00:00', $timestamp).' GMT');
	}


	/**
	* Determines the next occurrence of an event after a certain time.
	*
	* This function can be called on an object (with just the first parameter) or
	* statically (with all the parameters). If only $after is specified the object's
	* attributes are used.
	*
	* @param string	$after		the time for which we want the first occurrence after
	*							if NULL, reverts to current server time
	* @param string	$start		The timestamp at which the event's 1st instance started 
	* @param string	$freq_type	3-letter frequency code
	* @param int	$interval	The frequency interval
	* @param string	$stop		The date after which recurrences should stop
	*
	* @static
	* @return int	a UNIX timestamp representing the next occurrence
	* @access public
	*/
	function getFirstOccurrenceAfter($after=null, $start=null, $freq_type=null, $interval=null, $stop=null)
	{
		if (is_null($after)) $after = date('Y-m-d');
		
		// if we are calling statically make sure that we have all our arguments
		if (is_null($this) && (func_num_args() < 5)) {
			trigger_error('If Calendar_Event_Recurring::getFirstOccurrenceAfter() is called statically it must be called using all 5 arguments', E_USER_ERROR);
		}
		
		if (is_null($start)) {
			// called on an object
			$freq_type = substr($this->attr('frequency'), 0, 3);
			$interval = substr($this->attr('frequency'), 3);
			if (!$interval)	$interval = 1;
			$start = substr($this->attr('start_date'),0,10);
			$stop = $this->attr('stop_date');
		}

		$seconds_per_day = 60 * 60 * 24;
		if ($after <= $start)
			return $start;
		
		list($res_year,$res_month,$res_day) = sscanf($after, '%04d-%02d-%02d');
		list($after_year,$after_month,$after_day) = sscanf($after, '%04d-%02d-%02d');
		list($start_year,$start_month,$start_day) = sscanf($start, '%04d-%02d-%02d');
		$month_diff = (12 * ($after_year - $start_year)) + $after_month - $start_month;			
		
		switch ($freq_type) {
			case 'DED':				
				$periods_so_far = ceil(days_between_isos($after, $start) / $interval);
				$result = increment_iso_by_days($start, $interval * $periods_so_far);
			break;

			case 'DWD':				
				$periods_so_far = ceil(days_between_isos($after, $start) / $interval);
				$result = increment_iso_by_days($start, $interval * $periods_so_far);
				while (in_array(date('D', iso8601_ts($result)), Array('Sat', 'Sun'))) {
					$result = increment_iso_by_days($result, 1);
				}
			break;

			case 'WEW':
				$periods_so_far = ceil(days_between_isos($after, $start) / ($interval*7));
				$result = increment_iso_by_days($start, $interval * 7 * $periods_so_far);
			break;

			// Every 'x' months on the 'y'th day of the month
			case 'MFN':					
				if (($month_diff % $interval) != 0) {
					increment_month($res_month, $res_year, ($interval - ($month_diff % $interval)));
				} elseif ($res_day > $start_day) {
					increment_month($res_month, $res_year, $interval);
				}
				while ($start_day > days_in_month($res_month, $res_year))
					increment_month($res_month, $res_year, $interval);
				$result = sprintf('%04d-%02d-%02d',$res_year,$res_month,$start_day);
			break;

			// Every 'x' months on the 'y'th weekday of the month (eg. 2nd Thursday)
			case 'MFW':
				$target_week_of_month = (int)($start_day / 7);
				$target_day_of_week = date('w', iso8601_ts($start));
				$res_day = ($target_week_of_month * 7) + 1;

				if (($month_diff % $interval) != 0) {
					// go to the next valid month
					increment_month($res_month, $res_year, $interval - ($month_diff % $interval));
				} elseif ((int)($after_day / 7) > $target_week_of_month) {
					// we're in a valid month, but too late in it, so go to next valid month
					increment_month($res_month, $res_year, $interval);
				} elseif ((int)($after_day / 7) == $target_week_of_month) {
					$res_day = $after_day;
				}

				$month_length = days_in_month($res_month, $res_year);
				
				$result = sprintf('%04d-%02d-%02d',$res_year,$res_month,$res_day);
				
				while (date('w', iso8601_ts($result)) != $target_day_of_week) {				
					$res_day += (7 + $target_day_of_week - date('w', iso8601_ts($result)))  % 7;
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
				} elseif ((days_in_month($res_month, $res_year) - $after_day) < $target_days_from_end) {
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
				} elseif ((int)((days_in_month($after_month, $after_year) - $after_day) / 7) < $target_weeks_from_end) {
					// we're too far into this month, so go to the next valid month
					increment_month($res_month, $res_year, $interval);
				} elseif ((int)((days_in_month($after_month, $after_year) - $after_day) / 7) == $target_weeks_from_end) {
					// we're in the right week so start from 'now', not the begining of the week,
					//$min_day = $start_day;
				}
				$min_day = $after_day;
					
				$res_day = days_in_month($res_month, $res_year) - ($target_weeks_from_end * 7);
				$result = sprintf('%04d-%02d-%02d',$res_year,$res_month,$res_day);
				
				while (date('w', iso8601_ts($result)) != $target_day_of_week) {					
					//if ($res_month - $start_month > 2) die;
					$res_day -= (7 + date('w', iso8601_ts($result)) - $target_day_of_week)  % 7;
					if (($res_day <= $min_day) || ($result < $after)) {
						// can't get the nth-last xday in this month, either because
						// it's already passed or because it doesn't exist, so goto next month
						increment_month($res_month, $res_year, $interval);
						$res_day = days_in_month($res_month, $res_year) - ($target_weeks_from_end * 7);
						$min_day = 1;
					}
					$result = sprintf('%04d-%02d-%02d',$res_year,$res_month,$res_day);
				}
			break;
		}
				
		if (!empty($stop) && ($result > $stop)) {
			return false;
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
	* @param string	$start		The timestamp at which the event's 1st instance started 
	* @param string	$freq_type	3-letter frequency code
	* @param int	$interval	The frequency interval
	* @param string	$stop		The date after which recurrences should stop
	*
	* @static
	* @return boolean	true if this event occurs on the specified date
	* @access public
	*/
	function hasOccurrenceOnDate($date=null, $start=null, $freq_type=null, $interval=null, $stop=null)
	{
		// if we are calling statically make sure that we have all our arguments
		if (is_null($this) && (func_num_args() < 5)) {
			trigger_error('If Calendar_Event_Recurring::hasOccurrenceOnDate() is called statically it must be called using all 5 arguments', E_USER_ERROR);
		}
		
		if (is_null($start)) {
			// called on an object
			$freq_type = substr($this->attr('frequency'), 0, 3);
			$interval = substr($this->attr('frequency'), 3);
			if (!$interval)	$interval = 1;
			$start = substr($this->attr('start_date'),0,10);
			$stop = $this->attr('stop_date');
		}
		
		if (is_null($date)) $date = date('Y-m-d'); 
		
		$seconds_per_day = 60 * 60 * 24;
		$start_ts = iso8601_ts(iso8601_date_component($start));

		$query_ts = iso8601_ts(iso8601_date_component($date));	
		$stop_ts = iso8601_ts($stop);
		if (($stop_ts != -1) && ($query_ts > ($stop_ts + $seconds_per_day))) return false;
		if ($query_ts < $start_ts) return false;
		
		//$freq_type = substr($this->attr('frequency'), 0, 3);
		if ($interval > 1) $freq_type .= 'x';

		switch ($freq_type) {
			case 'DED':
				// definitely occurs
				return true;
			break;

			case 'DEDx':
				// days between start_date and date must be a multiple of $interval
				return ((($query_ts - $start_ts) / $seconds_per_day) % $interval == 0);
			break;

			case 'DWD':
				// definitely occurs unless this is a weekend day
				return !in_array(date('D', $query_ts), Array('Sat', 'Sun'));
			break;

			case 'WEW':
				// occurs if start_date and date are the same day of the week
				return (date('D', $start_ts) == date('D', $query_ts));
			break;

			case 'WEWx':
				// occurs if start_date and date are the same day of the week
				// AND the number of weeks between start_date and date is a multiple of $interval
				return ((date('D', $start_ts) == date('D', $query_ts)) 
					&& (((($query_ts - $start_ts) / ($seconds_per_day * 7)) % $interval) == 0));
			break;

			case 'MFN':
				// occurs if date and start_date are the same day of the month
				return substr($start_date, 8, 2) == substr($date, 8, 2);
			break;

			case 'MFNx':
				// occurs if date and start_date are the same day of the month
				// AND the number of months in between is a multiple of $interval
				return ((substr($start_date, 8, 2) == substr($date, 8, 2)) 
					&& ((((((int)substr($date, 0, 4) - (int)substr($start_date, 0, 4)) * 12 ) + substr($date, 5, 2) - substr($start_date, 5, 2)) % $interval) == 0));
			break;

			case 'MFW':
				// occurs if date and start_date are the same day of the week
				// AND they are in the same week of the month
				return (date('D', $start_ts) == date('D', $query_ts)) 
					&& ((int)((date('j', $start_ts) - 1)/ 7) == (int)((date('j', $query_ts) - 1) / 7));
			break;

			case 'MFWx':
				// occurs if date and start_date are the same day of the week
				// AND they are in the same week of the month
				// AND the number of months in between is a multiple of $interval
				return ((date('D', $start_ts) == date('D', $query_ts)) 
					&& ((int)(date('j', $start_ts) / 7) == (int)(date('j', $query_ts) / 7))) 
					&& ((((((int)substr($date, 0, 4) - (int)substr($start_date, 0, 4)) * 12 ) + substr($date, 5, 2) - substr($start_date, 5, 2)) % $interval) == 0);
			break;

			case 'MRN':
				// occurs if monthlength - day_of_month is equal for date and start_date
				return days_in_month(substr($start_date, 5, 2), substr($start_date, 0, 4)) - substr($start_date, 8, 2) == days_in_month(substr($date, 5, 2), substr($date, 0, 4)) - substr($date, 8, 2);
			break;

			case 'MRNx':
				// occurs if monthlength - day_of_month is equal for date and start_date
				// AND number of months between start_date and date is a multiple of $interval
				return ((((((int)substr($date, 0, 4) - (int)substr($start_date, 0, 4)) * 12 ) + substr($date, 5, 2) - substr($start_date, 5, 2)) % $interval) == 0) 
					&& (days_in_month(substr($start_date, 5, 2), substr($start_date, 0, 4)) - substr($start_date, 8, 2) == days_in_month(substr($date, 5, 2), substr($date, 0, 4)) - substr($date, 8, 2));
			break;

			case 'MRW':
				// occurs if date and start_date are the same day of the week...
				if (date('D', $start_ts) != date('D', $query_ts)) return false;
				
				// ...AND they are in the same week from the end of the month
				$start_back_week = (int)((days_in_month(date('m', $start_ts), date('Y', $start_ts)) - date('d', $start_ts)) / 7);
				
				$query_back_week = (int)((days_in_month(date('m', $query_ts), date('Y', $query_ts)) - date('d', $query_ts)) / 7);
				
				return $start_back_week == $query_back_week;
				
			break;

			case 'MRWx':
				// occurs if date and start_date are the same day of the week...
				if (date('D', $start_ts) != date('D', $query_ts)) return false;
				
				// ...AND the number of months in between start_date and date is a multiple of $interval...
				if ((((((int)substr($date, 0, 4) - (int)substr($start_date, 0, 4)) * 12 ) + substr($date, 5, 2) - substr($start_date, 5, 2)) % $interval) != 0) return false;
				
				// ...AND they are in the same week from the end of the month
				$start_back_week = (int)((days_in_month(date('m', $start_ts), date('Y', $start_ts)) - date('d', $start_ts)) / 7);
				
				$query_back_week = (int)((days_in_month(date('m', $query_ts), date('Y', $query_ts)) - date('d', $query_ts)) / 7);
				
				return $start_back_week == $query_back_week;
				
			break;
		}

		return false;

	}//end hasOccurrenceOnDate()
	

	/**
	* Returns this event's frequency in an easy-to-understand format
	*
	* @return string
	* @access public
	*/
	function getFriendlyFrequency() {
		$freq_type = substr($this->attr('frequency'), 0, 3);
		$interval = substr($this->attr('frequency'), 3);
		$monthly_type = '';
		switch ($freq_type) {
			// every day (or x days)
			case 'DED':
				if ($interval) {
					$res = "every $interval days";
				} else {
					$res = 'every day';
				}
			break;

			// every weekday
			case 'DWD':
				$res = 'every weekday';
			break;

			// every week (or x weeks)
			case 'WEW':
				if ($interval) {
					$res = "every $interval weeks";
				} else {
					$res = 'every week';
				}
			break;

			// every month (or x months) on the yth day of the month
			case 'MFN':
				$day = (int)substr($this->attr('start_date'), 8, 2);
				$monthly_type = 'on the '.add_ordinal_suffix($day).' day of the month';
			break;

			// every month (or x months) on the yth weekday of the month
			case 'MFW':
				$day = (int)substr($this->attr('start_date'), 8, 2);
				$day_of_week = date('l', iso8601_ts($this->attr('start_date')));
				$monthly_type = 'on the '.add_ordinal_suffix((int)((($day-1) / 7) + 1)).' '.$day_of_week.' of the month';
			break;

			// every month (or x months) on the yth LAST day of the month
			case 'MRN':
				$day = (int)substr($this->attr('start_date'), 8, 2);
				$month = (int)substr($this->attr('start_date'), 5, 2);
				$year = (int)substr($this->attr('start_date'), 0, 4);
				$month_length = date('j', strtotime('yesterday', iso8601_ts($year.'-'.($month+1).'-01')));
				$monthly_type = 'on the '.add_reverse_ordinal_suffix($month_length - $day + 1).' last day of the month';
			break;

			// every month (or x months) on the yth LAST weekday of the month
			case 'MRW':
				$day = (int)substr($this->attr('start_date'), 8, 2);
				$day_of_week = date('l', iso8601_ts($this->attr('start_date')));
				$month = (int)substr($this->attr('start_date'), 5, 2);
				$year = (int)substr($this->attr('start_date'), 0, 4);
				$month_length = date('j', strtotime('yesterday', iso8601_ts($year.'-'.($month+1).'-01')));
				$monthly_type = 'on the '.add_reverse_ordinal_suffix((int)((($month_length - $day + 1) / 7) + 1)).' last '.$day_of_week.' of the month';
			break;

			// unknown frequency
			default:
				$res = 'Unknown frequency type "'.$this->attr('frequency').'"';
		}
		
		// set up final display string for monthly types 
		if ($monthly_type) {
			if ($interval) {
				$res = "every $interval months, $monthly_type";
			} else {
				$res = "every month, $monthly_type";
			}
		}
		
		$stop_date_attr = $this->getAttribute('stop_date');
		if (($stop_timestamp = $stop_date_attr->getTimestamp()) == -1) {
			return "$res, until further notice";
		} else {
			return "$res, until ".readable_datetime($stop_timestamp);
		}

	}//end getFriendlyFrequency()


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
				<td><b>Event Name</b></td>
				<td><?php echo $this->attr('name'); ?></td>
			</tr>
			<tr>
				<td><b>Start Date of first instance</b></td>
				<td><?php echo $this->getFriendlyStart(); ?></td>
			</tr>
			<tr>
				<td><b>End Date of first instance</b></td>
				<td><?php echo $this->getFriendlyEnd(); ?></td>
			</tr>
			<tr>
				<td><b>Duration of each instance</b></td>
				<td><?php
						// give useful info if no duration - easy_time_total returns empty string
						$total_time = easy_time_total($this->getDuration(), true);
						echo (empty($total_time) ? 'N/A' : $total_time);
					?></td>
			</tr>
			<tr>
				<td><b>Frequency</b></td>
				<td>Recurs <?php echo $this->getFriendlyFrequency() ?></td>
			</tr>
			<tr>
				<td><b>Description</b></td>
				<td><?php echo $this->attr('description'); ?></td>
			</tr>
		</table>
		<?php

	}//end printBody()


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @return Array()
	* @access private
	*/
	function _getAllowedLinks()
	{
		$links = parent::_getAllowedLinks();
		$links[SQ_LINK_TYPE_2]['calendar_event_modification'] = Array('card' => 'M', 'exclusive' => false);
		$links[SQ_LINK_TYPE_2]['calendar_event_cancellation'] = Array('card' => 'M', 'exclusive' => false);
		return $links;

	}//end _getAllowedLinks()

	
	/**
	* Return the keywords with their replacements
	*
	* The return value is in the form:
	* <pre>
	* Array(
	*     'name' => Array (
	*          'value' => 'page',
	*          'description' => 'the asset name',
	*     ),
	* )
	* </pre>
	*
	* @param boolean $descriptions if TRUE descriptions will be included in the array
	*
	* @access public
	* @return Array(mixed)
	*/
	function getAssetKeywords($descriptions=false)
	{
		$keywords = parent::getAssetKeywords($descriptions);
		$keywords['event_frequency'] = Array('value'=> $this->getFriendlyFrequency());
		if ($descriptions) {
			$keywords['event_frequency']['description'] = 'The frequency with which this event recurs';
		}
		return $keywords;
	}//end getAssetKeywords()
	
	
	/*
	* Cache the calendar data's stuff
	*
	* @param boolean	$updating	are we updating (TRUE) or inserting a new record
	* 								(FALSE - set only in create() call)
	*
	* @return boolean	indicating success
	* @access private
	*/
	function cacheCalendarData($updating=true)
	{
		$db = $GLOBALS['SQ_SYSTEM']->db;
	
		$date_values = Array();
		foreach(Array('start_date', 'end_date') as $date_comp) {
			$date_values += $this->getDateComponents($this->attr($date_comp), $date_comp.'_');
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
	
		// if updating attribute then we use
		// rollbackUpdate('...', $date_values, $where)
		if (!$updating) {
			// set the asset ids
			$date_values['assetid'] = $this->id;
		
			if (!$GLOBALS['SQ_SYSTEM']->rollbackInsert('calendar_date_value', $date_values)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}
		} else {
			// now quote everything except null values
			foreach(array_keys($date_values) as $key) {
				$date_values[$key] = (is_null($date_values[$key]) ? 'null' : 	$db->quote($date_values[$key]));						
			}
		
			// asset id becomes a where condition
			$where = ' assetid = '.$db->quote($this->id);
		
			if (!$GLOBALS['SQ_SYSTEM']->rollbackUpdate('calendar_date_value', $date_values, $where)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}
		}
		
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;
			
	}//end cacheCalendarData()
	

}//end class

?>