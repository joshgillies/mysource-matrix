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
* $Id: calendar_event_recurring.inc,v 1.1 2004/07/28 01:27:23 lwright Exp $
* $Name: not supported by cvs2svn $
*/

require_once  dirname(__FILE__).'/../../calendar_event/calendar_event.inc';

/**
* Calendar_Event_Recurring
*
* Purpose
*     
*
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.1 $ - 1.0
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


}//end class

?>