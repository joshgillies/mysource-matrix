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
* $Id: calendar_event_recurring.inc,v 1.44 2004/09/22 06:57:02 tbarrett Exp $
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
* @version $Revision: 1.44 $ - 1.0
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
	* Determines the next occurrence of an event on or after a certain time
	*
	* This function can be called on an object (with just the first parameter) or
	* statically (with all the parameters). If only $after is specified the object's
	* attributes are used.
	* If $after falls ON the date of one of the event's instances, that instance will be
	* returned. If $after falls after the first day of an instance, the next instance will be
	* returned.
	*
	* @param string	$after		the earliest date (YYYY-MM-DD) we are interested in
	* @param string	$start		The date (YYYY-MM-DD) on which the event's first instance started 
	* @param string	$freq_type	3-letter frequency code
	* @param int	$interval	The frequency interval
	* @param string	$stop		The date on which the event stops recurring
	*
	* @static
	* @return int	the date (YYYY-MM-DD) of the next occurrence
	* @access public
	*/
	function getFirstOccurrenceAfter($after=null, $start=null, $freq_type=null, $interval=null, $stop=null)
	{
		if (is_null($after)) $after = date('Y-m-d');
		
		// if we are calling statically make sure that we have all our arguments
		if ((!(isset($this) && is_a($this, "Calendar_Event_Recurring"))) && (func_num_args() < 5)) {
			trigger_error('If Calendar_Event_Recurring::getFirstOccurrenceAfter() is called statically it must be called using all 5 arguments', E_USER_ERROR);
		}

		if (is_null($start)) {
			// called on an object
			$freq_type = substr($this->attr('frequency'), 0, 3);
			$interval = substr($this->attr('frequency'), 3);
			if (!$interval)	$interval = 1;
			$start = substr($this->attr('start_date'),0,10);
			$stop = $this->attr('stop_date');
			// null stop date attribute?
			if ($stop == '---------- --:--:--') $stop = null;
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
				$result = add_days_to_iso($start, $interval * $periods_so_far);
			break;

			case 'DWD':				
				$periods_so_far = ceil(days_between_isos($after, $start) / $interval);
				$result = add_days_to_iso($start, $interval * $periods_so_far);
				while (in_array(date('D', iso8601_ts($result)), Array('Sat', 'Sun'))) {
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
	* @param string	$start		The iso datetime at which the event's 1st instance started 
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
		
		// fill in defaults if we haven't been provided with all values
		if (is_null($start)) {
			// called on an object
			$freq_type = substr($this->attr('frequency'), 0, 3);
			$interval = substr($this->attr('frequency'), 3);
			$start = substr($this->attr('start_date'),0,10);
			$stop = $this->attr('stop_date');
		}
		if (is_null($date)) $date = date('Y-m-d'); 
		if (strpos($stop, '----------') !== false) $stop = null;

		// dispose of trivial cases
		if ($start > $date) return false;
		if ((!is_null($stop)) && ($stop < $date)) return false;

		// get components and get into it
		$date_components = $this->getDateComponents($date);
		$start_components = $this->getDateComponents($start);

		if (!$interval)	$interval = 1;
		if ($interval > 1) $freq_type .= 'x';

		switch ($freq_type) {
			case 'DED':
				// definitely occurs
				return true;
			break;

			case 'DEDx':
				// days between start_date and date must be a multiple of $interval
				return (($date_components['ds'] - $start_components['ds']) % $interval == 0);
			break;

			case 'DWD':
				// definitely occurs unless this is a weekend day
				return !in_array($date_components['wday'], Array(0, 6));
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
		}
		return false;

	}//end hasOccurrenceOnDate()


	/**
	* Returns the start and end dates/times of the instance containing a specified ISO date
	*
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
		$res = Array();
		if ($this->hasOccurrenceOnDate($date)) {
			$res['start'] = $date;
		} else {
			$period = substr($this->attr('frequency'), 3);
			switch (substr($this->attr('frequency'), 0, 1)) {
				case 'D':
					$first_occur = add_days_to_iso($date, -$period);
				break;
				
				case 'W':
					$first_occur = add_days_to_iso($date, -$period);
				break;
				
				case 'M':
					$first_occur = add_months_to_iso($date, -$period).'-'.date('d', $date);
				break;
				
			}
			$res['start'] = $this->getFirstOccurrenceAfter($first_occur);
		}
		$res['end'] = add_days_to_iso($res['start'], $this->getDuration() / -86400);
		if ($this->getStartTime()) $res['start'] .= ' '.$this->getStartTime();
		if ($this->getEndTime()) $res['end'] .= ' '.$this->getEndTime();

		return $res;

	}//end getInstanceDates()
	
	
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
			return "$res, until ".date('jS M Y', $stop_timestamp);
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
		unset($keywords['asset_attribute_frequency']);
		$write_access = $this->writeAccess('');

		if (isset($_REQUEST['SQ_CALENDAR_DATE'])) {
			if ($exception_link = $this->getExceptionLink($_REQUEST['SQ_CALENDAR_DATE'])) {
				// we need to show the exception event
				bam("SPECIAL CASE: This asset has an exception today");
				bam($exception_link);
			} else {
				// we are showing a normal instance of this event
				$instance_dates = $this->getInstanceDates($_REQUEST['SQ_CALENDAR_DATE']);
				$keywords['instance_start_datetime'] = Array('value'=>$instance_dates['start']);
				$keywords['instance_end_datetime'] = Array('value'=>$instance_dates['end']);
				$keywords['cancel_link'] = Array('value'=>($write_access ? $this->_getCancelLink() : '(cancel link)'));
				$keywords['modify_link'] = Array('value'=>($write_access ? $this->_getModifyLink() : '(modify link)'));
			}
		} else {
			// somehow we're just viewing the event by itself, so set the instance-specific
			// keywords to blank for tidiness's sake
			$keywords['instance_start_datetime'] = Array('value'=>'');
			$keywords['instance_end_datetime'] = Array('value'=>'');
			$keywords['cancel_link'] = Array('value'=>'');
			$keywords['modify_link'] = Array('value'=>'');
		}		

		if ($descriptions) {
			$keywords['event_frequency']['description'] = 'The frequency with which this event recurs';
			$keywords['cancel_link']['description'] = 'Link to cancel this instance of the event';
			$keywords['modify_link']['description'] = 'Link to modify this instance of the event';
			$keywords['instance_start_datetime']['description'] = 'Start date/time of this instance of the event';
			$keywords['instance_end_datetime']['description'] = 'End date/time of this instance of the event';
			
			// change the descriptions of Calendar Event's attributes just to make it clear
			$keywords['event_start_datetime']['description'] = 'Start date/time of original instance';
			$keywords['event_end_datetime']['description'] = 'End date/time of original instance';
		}
		return $keywords;

	}//end getAssetKeywords()


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
	* @static (optionally)
	* @return Array|boolean		Array (assetid => type_code) of the event, or false if none exist
	*/
	function getExceptionLink($date, $id=null)
	{
		if (is_null($id)) $id = $this->id;
		$child_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($id, SQ_SC_LINK_ALL, Array('calendar_event_cancellation', 'calendar_event_modification'), false, 'major', $date);
		return current($child_links);

	}//end getExceptionEvent()


	/**
	* Return the link that the user can click to add a cancellation of this event from the event view
	*
	* @return string
	* @access private
	*/	
	function _getCancelLink()
	{
		if (isset($_REQUEST['SQ_CALENDAR_DATE']))	return '<a href="?SQ_CALENDAR_VIEW=event&amp;SQ_CALENDAR_EVENT_ID='.$this->id.'&amp;SQ_CALENDAR_EVENT_ACTION=cancel1&amp;SQ_CALENDAR_DATE='.$_REQUEST['SQ_CALENDAR_DATE'].'" onclick="return confirm(\'Are you sure you want to cancel this instance?\');">Cancel this instance</a>';
		else return '(cancel link)';

	}//end _getCancelLink()
	

	/**
	* Return the link that the user can click to modify an instance of this event from the event view
	*
	* @return string
	* @access private
	*/	
	function _getModifyLink()
	{
		if (isset($_REQUEST['SQ_CALENDAR_DATE']))	return '<a href="?SQ_CALENDAR_VIEW=event&amp;SQ_CALENDAR_EVENT_ID='.$this->id.'&amp;SQ_CALENDAR_EVENT_ACTION=modify1&amp;SQ_CALENDAR_DATE='.$_REQUEST['SQ_CALENDAR_DATE'].'">Modify this instance</a>';
		else return '(modify link)';

	}//end _getModifyLink()

	/**
	* Process the specified action on this event, and return whatever html the action wants to show
	*
	* @param string	$action_name	The action to perform, eg 'cancel'
	*
	* @return string	HTML to show as the result of this action
	* @access public
	*/	
	function processEventAction($action_name) 
	{
		$res = '';
		switch ($action_name) {
			case 'cancel1':
				if (!isset($_REQUEST['SQ_CALENDAR_DATE'])) {
					trigger_error('Attempt to create a cancellation with no date', E_USER_WARNING);
					return '';
				}
				$res = '<form method="POST" action="?SQ_CALENDAR_VIEW=event&amp;SQ_CALENDAR_EVENT_ID='.$this->id.'&amp;SQ_CALENDAR_EVENT_ACTION=cancel2&amp;SQ_CALENDAR_DATE='.$_REQUEST['SQ_CALENDAR_DATE'].'"><table cellpadding="4" border="0">';
				$res .= '<tr><td style="vertical-align: top">Reason for<br />cancellation: </td><td><textarea rows="4" cols="40" name="event_cancellation_reason"></textarea></td></tr>';
				$res .= '<tr><td colspan="2"><input type="submit" value="Add Cancellation" />&nbsp;&nbsp;<input type="button" value="Go Back" onclick="document.location=\''.$_SERVER['HTTP_REFERER'].'\'" /></td></tr></table></form>';
			break;

			case 'cancel2':
				if ((!isset($_REQUEST['SQ_CALENDAR_DATE'])) || (!isset($_REQUEST['event_cancellation_reason']))) {
					trigger_error('Attempt to create a cancellation one of the parameters missing', E_USER_WARNING);
					return '';
				}

				$cancel_event = $this->addCancellation($_REQUEST['SQ_CALENDAR_DATE'], $_REQUEST['event_cancellation_reason']);

				$res = '<p>The recurring event <i>'.$this->attr('name').'</i> has been cancelled on '.date('jS M Y', strtotime($_REQUEST['SQ_CALENDAR_DATE'].' 12:00:00 GMT')).' only.</p>';
				$res .= '<p style="margin: 0px; ">Would you like to create a replacement for the cancelled event?<p>';
				$res .= '<form style="margin: 0px; float: left" method="POST" action="?SQ_CALENDAR_VIEW=event&amp;SQ_CALENDAR_EVENT_ID='.$cancel_event->id.'&amp;SQ_CALENDAR_EVENT_ACTION=addreplacement&amp;SQ_CALENDAR_DATE='.$_REQUEST['SQ_CALENDAR_DATE'].'"><input type="submit" value="Yes" /></form><form style="margin: 0px 1ex; float: left" method="POST" action="?SQ_CALENDAR_VIEW=event&amp;SQ_CALENDAR_EVENT_ID='.$this->id.'&amp;SQ_CALENDAR_DATE='.$_REQUEST['SQ_CALENDAR_DATE'].'"><input type="submit" value="No" /></form>';
			break;

			case 'modify1':
				if (!isset($_REQUEST['SQ_CALENDAR_DATE'])) {
					trigger_error('Attempt to create a modification with no date', E_USER_WARNING);
					return '';
				}
				$res = '<form method="POST" action="?SQ_CALENDAR_VIEW=event&amp;SQ_CALENDAR_EVENT_ID='.$this->id.'&amp;SQ_CALENDAR_EVENT_ACTION=modify2&amp;SQ_CALENDAR_DATE='.$_REQUEST['SQ_CALENDAR_DATE'].'"><table cellpadding="4" border="0">';
				
				$res .= '<tr><td style="vertical-align: top">Start and End: </td><td>';
				ob_start();
					$edit_fns = $this->getEditFns();
					$edit_fns->paintStartEndChooser($this, 'event');
					$res .= ob_get_contents();
				ob_end_clean();
				$res .= '</td></tr>';

				$res .= '<tr><td style="vertical-align: top">Description: </td><td><textarea name="event_description">';
				$res .= $this->attr('description');
				$res .= '</textarea></td></tr>';
				
				$res .= '<tr><td colspan="2"><input type="submit" value="Add Modification" />&nbsp;&nbsp;<input type="button" value="Cancel" onclick="document.location=\''.$_SERVER['HTTP_REFERER'].'\'" /></td></tr></table></form>';

			break;

			case 'modify2':
				$occurrence_end_date = $this->getEndDate() ? date('Y-m-d', iso8601_ts($_REQUEST['SQ_CALENDAR_DATE'].' '.($this->getStartTime() ? $this->getStartTime() : ' 12:00:00')) + $this->getDuration()) : false;
				// We can only avoid cancelling and rescheduling if the time components are all the same, ie if
				// (start date field equals the occurrence start date)
				// AND (event has start time AND start time field is equal to it) 
				//		OR (event has no start time AND start time field is disabled)
				// AND (occurrence has end date AND end date field is equal to it)
				//		OR (event has no end date AND end date field is disabled)
				// AND (event has end time AND end time field is equal to it)
				//		OR (event has no end time AND end time field is disabled)
				// ... in other words...

				$start_dates_agree = ($_REQUEST['SQ_CALENDAR_DATE'] == sprintf('%04d-%02d-%02d', $_REQUEST['event_start_year'], $_REQUEST['event_start_month'], $_REQUEST['event_start_day']));
						
				$start_times_agree = ((($start_time = $this->getStartTime()) && isset($_REQUEST['event_start_time_enabled']) &&  (sprintf('%02d:%02d', $_REQUEST['event_start_hours'], $_REQUEST['event_start_minutes']) == $start_time)) || ((!$this->getStartTime()) && (!isset($_REQUEST['event_start_time_enabled']))));
				
				$end_dates_agree = ((($occurrence_end_date) && isset($_REQUEST['event_end_date_enabled']) && (sprintf('%04d-%02d-%02d', $_REQUEST['event_end_year'], $_REQUEST['event_end_month'], $_REQUEST['event_end_day']) == $occurrence_end_date)) || ((!$occurrence_end_date && (!isset($_REQUEST['event_end_date_enabled'])))));

				$end_times_agree = ((($end_time = $this->getEndTime()) && isset($_REQUEST['event_end_time_enabled']) &&  (sprintf('%02d:%02d', $_REQUEST['event_end_hours'], $_REQUEST['event_end_minutes']) == $end_time)) || ((!$this->getEndTime()) && (!isset($_REQUEST['event_end_time_enabled']))));
				//bam("start dates agree: $start_dates_agree; start times agree: $start_times_agree; end dates agree: $end_dates_agree; end times agree: $end_times_agree");
				if ($start_dates_agree && $start_times_agree && $end_dates_agree && $end_times_agree) {
					$this->addModification($_REQUEST['SQ_CALENDAR_DATE'], $_REQUEST['event_description']);
					$res = '<p>Event instance modified</p>';
				} else {
					$cancellation_event = $this->addCancellation($_REQUEST['SQ_CALENDAR_DATE'], '(rescheduled)'); 

					if (!isset($_REQUEST['event_start_time_enabled'])) {
						$_REQUEST['event_start_hours'] = '--';
						$_REQUEST['event_start_minutes'] = '--';
					} else {
						if ($_REQUEST['event_start_is_pm'] && ($_REQUEST['event_start_hours'] != 12)) $_REQUEST['event_start_hours'] += 12;
					}
					$start_date_string = sprintf('%04s-%02s-%02s %02s:%02s:--', $_REQUEST['event_start_year'], $_REQUEST['event_start_month'], $_REQUEST['event_start_day'],  $_REQUEST['event_start_hours'], $_REQUEST['event_start_minutes']);

					
					if (!isset($_REQUEST['event_end_time_enabled'])) {
						$_REQUEST['event_end_hours'] = '--';
						$_REQUEST['event_end_minutes'] = '--';
					} else {
						if ($_REQUEST['event_end_is_pm'] && ($_REQUEST['event_end_hours'] != 12)) $_REQUEST['event_end_hours'] += 12;				
					}
					if (!isset($_REQUEST['event_end_enabled'])) {
						$end_date_string = '---------- --:--:--';
					} elseif (isset($_REQUEST['event_end_type']) && ($_REQUEST['event_end_type'] == 'date')) {
						$end_date_string = sprintf('%04s-%02s-%02s %02s:%02s:--', $_REQUEST['event_end_year'], $_REQUEST['event_end_month'], $_REQUEST['event_end_day'],  $_REQUEST['event_end_hours'], $_REQUEST['event_end_minutes']);
					} else {
						$duration_secs = 60 * $_REQUEST['event_duration'] * (($_REQUEST['event_duration_type'] == 'i') ? 1 : 60);
						$end_date_string = ts_iso8601(iso8601_ts($start_date_string + $duration_secs));
					}						

					$cancellation_event->addReplacement($start_date_string, $end_date_string ,  $_REQUEST['event_description']);
					
					$res = '<p>Event rescheduled</p>';
				}
			break;

			default:
				$res = parent::processEventAction($action_name);
			break;
		}
		return $res;

	}//end processEventAction()

	

	/**
	* Add a cancellation event linked under this recurring event
	*
	* @param string		$date	the date of the occurrence to cancel (YYYY-MM-DD)
	* @param string		$reason	text to put in the description field of the cancellation event
	*
	* @return mixed	(object Calendar_Event_Cancellation|boolean)
	*				the Cancellation asset, or false on failure
	*/
	function &addCancellation($date, $reason)
	{
		if (!$this->writeAccess('')) { 
			trigger_error('You don\'t have sufficient access to create a cancellation for this event', SQ_USER_WARNING);
			return false;
		}
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('calendar_event_cancellation');
		if ($existing_link = $this->getExceptionLink($date)) {
			if ($existing_link['minor_type_code'] == 'calendar_event_cancellation') {
				$cancellation_event = &$GLOBALS['SQ_SYSTEM']->am->getAsset($existing_link['minorid']);
			} else {
				$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($existing_link['linkid']);
				$cancellation_event = new Calendar_Event_Cancellation();
			}
		} else {
			$cancellation_event = new Calendar_Event_Cancellation();
		}
		$cancellation_event->setAttrValue('start_date', $date.' --:--:--');
		$cancellation_event->setAttrValue('description', $reason);
		if (!$cancellation_event->id) {
			$link = Array('asset' => &$this, 'link_type' => SQ_LINK_TYPE_2, 'dependant' => 1, 'exclusive' => 0, 'value' => $date);
			$cancellation_event->create($link);
		} else {
			if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($cancellation_event->id, 'attributes')) {
				trigger_error('Could not get lock to save attrs of asset "'.$cancellation_event->name.'"', E_USER_WARNING);
				return 0;
			}
			$cancellation_event->saveAttributes();
			$GLOBALS['SQ_SYSTEM']->am->releaseLock($cancellation_event->id, 'attributes');
		}
		return $cancellation_event;

	}// end addCancellation()


	/**
	* Add a modification event linked under this recurring event
	*
	* @param string		$date	the date of the occurrence to cancel (YYYY-MM-DD)
	* @param string		$reason	text to put in the description field of the cancellation event
	*
	* @return mixed	(object Calendar_Event_Modification|boolean)
	*				the Modification asset, or false on failure
	*/
	function &addModification($date, $description)
	{
		if (!$this->writeAccess('')) {
			trigger_error('You don\'t have sufficient access to create a modification for this event', SQ_USER_WARNING);
			return false;
		}
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('calendar_event_modification');

		if ($existing_link = $this->getExceptionLink($date)) {
			if ($existing_link['minor_type_code'] == 'calendar_event_modification') {
				$modification_event = &$GLOBALS['SQ_SYSTEM']->am->getAsset($existing_link['minorid']);
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
			$link = Array('asset' => &$this, 'link_type' => SQ_LINK_TYPE_2, 'dependant' => 1, 'exclusive' => 0, 'value' => $date);
			$modification_event->create($link);
		} else {
			if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($modification_event->id, 'attributes')) {
				trigger_error('Could not get lock to save attrs of asset "'.$modification_event->name.'"', E_USER_WARNING);
				return 0;
			}
			$modification_event->saveAttributes();
			$GLOBALS['SQ_SYSTEM']->am->releaseLock($modification_event->id, 'attributes');
		}
		return $modification_event;

	}// end addModification()


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