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
* $Id: calendar_event_recurring.inc,v 1.3 2004/07/28 23:47:03 tbarrett Exp $
* $Name: not supported by cvs2svn $
*/

require_once  dirname(__FILE__).'/../../calendar_event/calendar_event.inc';

/**
* Calendar_Event_Recurring
*
* An event which recurs at a specified frequency after its first instance
*
* @author  Luke Wright <lwright@squiz.net>
* @euthor  Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.3 $ - 1.0
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
		if (is_null($after)) {
			$after = time();
		}

		// interpret frequency etc
		
	}//end getFirstOccurrenceAfter()


	/**
	* Tells us whether this event has an occurrence on the specified date
	*
	* @param string		$date	the date we're interested in, in YYYY-MM-DD format
	*
	* @return boolean	true if this event occurs on the specified date
	* @access public
	*/
	function hasOccurrenceOnDate($date) {
		$start_ts = strtotime($this->getStartDate());
		$query_ts = strtotime($date);		
		$seconds_per_day = 60 * 60 * 24;
		$start_date = $this->getStartDate();
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

			case 'MFD':
				// occurs if date and start_date are the same day of the week
				// AND they are in the same week of the month
				return (date('D', $start_ts) == date('D', $query_ts)) 
					&& ((int)((date('j', $start_ts) - 1)/ 7) == (int)((date('j', $query_ts) - 1) / 7));
			break;

			case 'MFDx':
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

			case 'MRD':
				// occurs if date and start_date are the same day of the week 
				// AND they are in the same week from the end of the month
				return (date('D', $start_ts) == date('D', $end_ts)) 
					&& (((int)(days_in_month(substr($start_date, 5, 2), substr($start_date, 0, 4)) - substr($start_date, 8, 2)) / 7) == ((int)(days_in_month(substr($date, 5, 2), substr($date, 0, 4)) - substr($date, 8, 2)) / 7));
			break;

			case 'MRDx':
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
	* @return string	this event's frequency in an easy-to-understand format
	* @access public
	*/
	function getFriendlyFrequency() {
		$freq_type = substr($this->attr('frequency'), 0, 3);
		$interval = substr($this->attr('frequency'), 3);
		$monthly_type = '';
		switch ($freq_type) {
			case 'DED':
				if ($interval) 
					$res = "every $interval days";
				else
					$res = 'every day';
			break;

			case 'DWD':
				if ($interval)
					$res = "every $interval weekdays";
				else
					$res = 'every weekday';
			break;

			case 'WEW':
				if ($interval)
					$res = "every $interval weeks";
				else
					$res = 'every week';
			break;

			case 'MFN':
				$day = (int)substr($this->attr('start_date'), 8, 2);
				$monthly_type = 'on the ' . add_ordinal_suffix($day) . ' day of the month';
			break;

			case 'MFW':
				$day = (int)substr($this->attr('start_date'), 8, 2);
				$day_of_week = date('l', iso8601_ts($this->attr('start_date')));
				$monthly_type = 'on the ' . add_ordinal_suffix((int)((($day-1) / 7) + 1)) . ' ' . $day_of_week . ' of the month';
			break;

			case 'MRN':
				$day = (int)substr($this->attr('start_date'), 8, 2);
				$month = (int)substr($this->attr('start_date'), 5, 2);
				$year = (int)substr($this->attr('start_date'), 0, 4);
				$month_length = date('j', strtotime('yesterday', iso8601_ts($year . '-' . ($month+1) . '-01')));
				$monthly_type = 'on the ' . add_reverse_ordinal_suffix($month_length - $day + 1) . ' last day of the month';
			break;

			case 'MRW':
				$day = (int)substr($this->attr('start_date'), 8, 2);
				$day_of_week = date('l', iso8601_ts($this->attr('start_date')));
				$month = (int)substr($this->attr('start_date'), 5, 2);
				$year = (int)substr($this->attr('start_date'), 0, 4);
				$month_length = date('j', strtotime('yesterday', iso8601_ts($year . '-' . ($month+1) . '-01')));
				$monthly_type = 'on the ' . add_reverse_ordinal_suffix((int)((($month_length - $day + 1) / 7) + 1)) . ' last ' . $day_of_week . ' of the month';
			break;

			default:
				$res = 'ERROR:  freq is ' . $this->attr('frequency');
		}
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
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @return Array()
	* @access private
	*/
	function _getAllowedLinks()
	{
		$links = parent::_getAllowedLinks();
		$links[SQ_LINK_TYPE_2]['calendar_event_modification'] = Array('card' => 'M', 'exclusive' => false);
		return $links;

	}//end _getAllowedLinks()


}//end class

?>