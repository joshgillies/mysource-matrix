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
* $Id: calendar_event_recurring.inc,v 1.9 2004/08/02 00:53:24 tbarrett Exp $
* $Name: not supported by cvs2svn $
*/

require_once  dirname(__FILE__).'/../../calendar_event/calendar_event.inc';
require_once SQ_FUDGE_PATH.'/datetime_field/datetime_field.inc';

/**
* Calendar_Event_Recurring
*
* An event which recurs at a specified frequency after its first instance
*
* @author  Luke Wright <lwright@squiz.net>
* @euthor  Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.9 $ - 1.0
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
	* Adds the specified number of months to the date supplied
	*
	* @param int	&$month	The month to add to
	* @param int	&$year	The year to add to
	* @param int	&$num	The number of months to add
	*
	* @access private
	*/
	function _addMonths(&$month, &$year, $num)
	{
		$month += $num;
		if ($month > 12) {
			$year++;
			$month = 1;
		}
	}


	/**
	* Determines the next occurrence of an event after a certain time
	*
	* @param int	$after	the time for which we want the first occurrence after
	*						if NULL, reverts to current server time
	*
	* @return int	a UNIX timestamp representing the next occurrence
	* @access public
	*/
	function getFirstOccurrenceAfter($after=null)
	{
		if (is_null($after)) $after = time();
		$freq_type = substr($this->attr('frequency'), 0, 3);
		$interval = substr($this->attr('frequency'), 3);
		if (!$interval)	$interval = 1;
		$seconds_per_day = 60 * 60 * 24;
		$start_ts = iso8601_ts($this->attr('start_time'));

		switch ($freq_type) {
			case 'DED':
				$interval_length = $seconds_per_day * $interval;
				$periods_so_far = (int)(($after - $start_ts) / ($interval_length));
				return $start_ts + ($interval_length * ($periods_so_far + 1));
			break;

			case 'DWD':
				$res = $after + $seconds_per_day;
				while (in_array(date('D', $res), Array('Sat', 'Sun'))) {
					$res += $seconds_per_day;
				}
				return $res;
			break;

			case 'WEW':
				$interval_length = 7 * $seconds_per_day * $interval;
				$periods_so_far = (int)(($after - $start_ts) / ($interval_length));
				return $start_ts + ($interval_length * ($periods_so_far + 1));
			break;

			case 'MFN':
				$res_month = date('n', $after);
				$res_year = date('Y', $after);
				$month_diff = (12 * (date('Y', $after) - date('Y', $start_ts))) + $res_month - date('n', $start_ts);
				if (($month_diff % $interval) != 0)
					$res_month = date('n', $after) + $interval - ($month_diff % $interval);
				elseif ((int)date('j', $after) > (int)date('j', $start_ts))
					$this->_addMonths($res_month, $res_year, $interval);
				while (date('j', $start_ts) > days_in_month($res_month, $res_year))
					$this->_addMonths($res_month, $res_year, $interval);
				return strtotime($res_year . '-' . sprintf('%02d', $res_month) . date('-d', $start_ts));
			break;

			case 'MFW':
				$res_month = date('n', $after);
				$res_year = date('Y', $after);
				$month_diff = (12 * (date('Y', $after) - date('Y', $start_ts))) + $res_month - date('n', $start_ts);
				$target_week_of_month = (int)((date('j', $start_ts) - 1) / 7);
				if (($month_diff % $interval) != 0)
					$res_month = date('n', $after) + $interval - ($month_diff % $interval);
				elseif (((int)(date('j', $after) - 1) / 7) > $target_week_of_month)
					$this->_addMonths($res_month, $res_year, $interval);

				$target_day_of_week = (date('w', $start_ts));
				$res_day = ($target_week_of_month * 7) + 1;

				$month_length = days_in_month($res_month, $res_year);
				while ((date('w', strtotime($res_year . '-' . sprintf('%02d', $res_month) . '-' . $res_day))) != $day_of_week) {
					$res_day++;
					if ($res_day > $month_length) {
						// run out of days, so we'll have to start again in the next valid month
						$this->_addMonths($res_month, $res_year, $interval);
						$month_length = days_in_month($res_month, $res_year);
						$res_day = ($target_week_of_month * 7) + 1;
					}
				}
				return strtotime($res_year . '-' . sprintf('%02d', $res_month) . date('-d', $start_ts));
			break;

			case 'MRN':
				$res_month = date('m', $after);
				$res_year = date('Y', $after);
				$target_days_from_end = days_in_month(date('n', $start_ts)) - date('j', $start_ts);

				$month_diff = (12 * (date('Y', $after) - date('Y', $start_ts))) + $res_month - date('n', $start_ts);
				if (($month_diff % $interval) != 0)
					$res_month = date('n', $after) + $interval - ($month_diff % $interval);
				elseif ((days_in_month($res_month, $res_year) - (int)date('j', $after)) < $target_days_from_end)
					$this->_addMonths($res_month, $res_year, $interval);
				while ((days_in_month($res_month, $res_year) - $target_days_from_end) < 0)
					$this->_addMonths($res_month, $res_year, $interval);

				return strtotime($res_year . '-' . $res_month . sprintf('%02d', days_in_month($res_month, $res_year) - $target_days_from_end));
			break;

			case 'MRW':
				$res_month = date('m', $after);
				$res_year = date('Y', $after);
				$target_week_from_end = (int)((days_in_month(date('n', $start_ts), date('Y', $start_ts)) - date('j', $start_ts)) / 7);
				$month_diff = (12 * (date('Y', $after) - date('Y', $start_ts))) + $res_month - date('n', $start_ts);
				if (($month_diff % $interval) != 0)
					$res_month = date('n', $after) + $interval - ($month_diff % $interval);
				elseif ((int)((days_in_month(date('n', $after), date('Y', $after)) - date('j', $after)) / 7) < $target_week_from_end)
					$this->_addMonths($res_month, $res_year, $interval);

				$target_day_of_week = (date('w', $start_ts));
				$res_day = days_in_month($res_month, $res_year) - ($target_weeks_from_end * 7);

				while ((date('w', strtotime($res_year . '-' . $res_month . '-' . $res_day))) != $target_day_of_week) {
					$res_day--;
					if ($res_day < 1) {
						// this month doesn't have a nth-last xday, so go to the next valid month
						$this->_addMonths($res_month, $res_year, $interval);
						$res_day = days_in_month($res_month, $res_year) - ($target_weeks_from_end * 7);
					}
				}
				return strtotime($res_year . '-' . $res_month . date('-d', $start_ts));
			break;
		}
		return false;
		
	}//end getFirstOccurrenceAfter()


	/**
	* Tells us whether this event has an occurrence on the specified date
	*
	* @param string	$date	the date we're interested in, in YYYY-MM-DD format - default is
	*						today's date (time can be passed in too but it will be stripped)
	*
	* @return boolean	true if this event occurs on the specified date
	* @access public
	*/
	function hasOccurrenceOnDate($date=null) {
		if (is_null($date)) $date = date('Y-m-d'); 
		
		$start_date = $this->getStartDate();
		$start_ts = strtotime($start_date);
		$query_ts = strtotime(substr($date,0,10));		
		$seconds_per_day = 60 * 60 * 24;
		
		if ($query_ts < $start_ts) return false;
		$freq_type = substr($this->attr('frequency'), 0, 3);
		if ($interval = substr($this->attr('frequency'), 3))
			$freq_type .= 'x';
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
				// occurs if date and start_date are the same day of the week 
				// AND they are in the same week from the end of the month
				return (date('D', $start_ts) == date('D', $end_ts)) 
					&& (((int)(days_in_month(substr($start_date, 5, 2), substr($start_date, 0, 4)) - substr($start_date, 8, 2)) / 7) == ((int)(days_in_month(substr($date, 5, 2), substr($date, 0, 4)) - substr($date, 8, 2)) / 7));
			break;

			case 'MRWx':
				// occurs if date and start_date are the same day of the week 
				// AND they are in the same week from the end of the month
				// AND the number of months in between start_date and date is a multiple of $interval
				return (date('D', $start_ts) == date('D', $end_ts)) 
					&& (((int)(days_in_month(substr($start_date, 5, 2), substr($start_date, 0, 4)) - substr($start_date, 8, 2)) / 7) == ((int)(days_in_month(substr($date, 5, 2), substr($date, 0, 4)) - substr($date, 8, 2)) / 7)) 
					&& ((((((int)substr($date, 0, 4) - (int)substr($start_date, 0, 4)) * 12 ) + substr($date, 5, 2) - substr($start_date, 5, 2)) % $interval) == 0);
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
				if ($interval) 
					$res = "every $interval days";
				else
					$res = 'every day';
			break;

			// every weekday
			case 'DWD':
				$res = 'every weekday';
			break;

			// every week (or x weeks)
			case 'WEW':
				if ($interval)
					$res = "every $interval weeks";
				else
					$res = 'every week';
			break;

			// every month (or x months) on the yth day of the month
			case 'MFN':
				$day = (int)substr($this->attr('start_date'), 8, 2);
				$monthly_type = 'on the ' . add_ordinal_suffix($day) . ' day of the month';
			break;

			// every month (or x months) on the yth weekday of the month
			case 'MFW':
				$day = (int)substr($this->attr('start_date'), 8, 2);
				$day_of_week = date('l', iso8601_ts($this->attr('start_date')));
				$monthly_type = 'on the ' . add_ordinal_suffix((int)((($day-1) / 7) + 1)) . ' ' . $day_of_week . ' of the month';
			break;

			// every month (or x months) on the yth LAST day of the month
			case 'MRN':
				$day = (int)substr($this->attr('start_date'), 8, 2);
				$month = (int)substr($this->attr('start_date'), 5, 2);
				$year = (int)substr($this->attr('start_date'), 0, 4);
				$month_length = date('j', strtotime('yesterday', iso8601_ts($year . '-' . ($month+1) . '-01')));
				$monthly_type = 'on the ' . add_reverse_ordinal_suffix($month_length - $day + 1) . ' last day of the month';
			break;

			// every month (or x months) on the yth LAST weekday of the month
			case 'MRW':
				$day = (int)substr($this->attr('start_date'), 8, 2);
				$day_of_week = date('l', iso8601_ts($this->attr('start_date')));
				$month = (int)substr($this->attr('start_date'), 5, 2);
				$year = (int)substr($this->attr('start_date'), 0, 4);
				$month_length = date('j', strtotime('yesterday', iso8601_ts($year . '-' . ($month+1) . '-01')));
				$monthly_type = 'on the ' . add_reverse_ordinal_suffix((int)((($month_length - $day + 1) / 7) + 1)) . ' last ' . $day_of_week . ' of the month';
			break;

			// unknown frequency
			default:
				$res = 'Unknown frequency type "' . $this->attr('frequency') . '"';
		}
		
		// set up final display string for monthly types 
		if ($monthly_type) {
			if ($interval)
				$res = "every $interval months, $monthly_type";
			else
				$res = "every month, $monthly_type";
		}
		
		$stop_date_attr = $this->getAttribute('stop_date');
		if (($stop_timestamp = $stop_date_attr->getTimestamp()) == -1)
			return "$res, until further notice";
		else
			return "$res, until " . readable_datetime($stop_timestamp);

	}//end getFriendlyFrequency()


	/**
	* Called by the design to print the body of this asset
	*
	* @return void
	* @access public
	*/
	function printBody()
	{
		include_once SQ_FUDGE_PATH.'/general/datetime.inc';
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


}//end class

?>