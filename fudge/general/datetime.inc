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
* $Id: datetime.inc,v 1.48 2012/08/30 00:56:51 ewang Exp $
*
*/


/**
* DateTime Functions
*
* Purpose
*     General function for manipulating dates and times
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.48 $
* @package Fudge
* @subpackage General
*/


/**
* Takes a timestamp and returns a date/time string with detail varying by how recent the date is
*
* @param int	$then	the timestamp for some time in the future or past
*
* @return string
* @access public
*/
function easy_datetime($then='')
{
	// type verification
	if (!is_int($then)) return 0;

	$now = time();
	$now_date = date('Y-m-d');
	$then_date = date('Y-m-d', $then);

	if ($then_date == $now_date) {
		// today
		return date('g:ia', $then).' today';
	}
	if ($then < $now) {
		// time in the past
		if ($then_date == date('Y-m-d', $now - 86400)) {
			// yesterday
			return date('g:ia', $then).' yesterday';
		} else if ($now - $then < 604800) {
			// less than a week ago
			return date('g:ia', $then).' on '.date('l', $then);
		}
	} else {
		// time in the future
		if ($then_date == date('Y-m-d', $now + 86400)) {
			// tomorrow
			return date('g:ia', $then).' tomorrow';
		} else if ($then - $now < 604800) {
			// less than a week away
			return date('g:ia', $then).' this '.date('l', $then);
		}
	}
	// if we get to here it must be more than a week in either direction
	return date('g:ia \o\n jS F Y', $then);

}//end easy_datetime()


/**
* Returns the number of days, hours, minutes and seconds that relate to the number of seconds passed
*
* @param string		$secs			the number of seconds that have passed
* @param boolean	$full_format	print full hours, minutes, seconds text
* @param boolean	$return_array	return the values as an array of data instead of a string
* @param array		$units			an array of time units to display or return (d, h, m, s)
*
* @return mixed string|array
* @access public
*/
function easy_time_total($secs=0, $full_format=FALSE, $return_array=FALSE, $units=Array('d', 'h', 'm', 's'))
{
	// type verification
	if (!is_numeric($secs) || !is_bool($full_format) || !is_bool($return_array) || !is_array($units)) {
		return '';
	}

	if ($secs < 0) {
		$suffix = ' ago';
		$secs = abs($secs);
	} else {
		$suffix = '';
	}

	$secs = $secs;

	$minute = 60;
	$hour   = 60 * $minute;
	$day    = 24 * $hour;

	$num_days    = 0;
	$num_hours   = 0;
	$num_minutes = 0;
	$num_seconds = 0;

	// if there is more than one day left
	if ($secs >= $day) {
		$num_days = intval(floor($secs/$day));
		// remove the number of seconds we have taken care of
		$secs -= ($day * $num_days);
	}

	// if there is more than one hour left
	if ($secs >= $hour) {
		$num_hours = intval(floor($secs/$hour));
		// remove the number of seconds we have taken care of
		$secs -= ($hour * $num_hours);
	}

	// if there is more than one minute left
	if ($secs >= $minute) {
		$num_minutes = intval(floor($secs/$minute));
		// remove the number of seconds we have taken care of
		$secs -= ($minute * $num_minutes);
	}

	$num_seconds = $secs;

	$str = '';

	// remove the values we dont want to show
	if (!in_array('d', $units)) $num_days = 0;
	if (!in_array('h', $units)) $num_hours = 0;
	if (!in_array('m', $units)) $num_minutes = 0;
	if (!in_array('s', $units)) $num_seconds = 0;

	// if they want the full string version
	if ($full_format) {

		if ($num_days) {
			$str .= $num_days.' day'.(($num_days > 1) ? 's' : '');
		}

		if ($num_hours) {
			// if there are no days don't need a joiner
			if (!$num_days) {
				$joiner = '';
			} else if (!$num_minutes && !$num_seconds) {
				// else if there aren't any minutes or seconds so join with " and "
				$joiner = ' and ';
			} else {
				// else there must be more following join with a comma
				$joiner = ', ';
			}

			$str .= $joiner.$num_hours.' hour'.(($num_hours > 1) ? 's' : '');
		}

		if ($num_minutes) {
			// if there are no days or hours don't need a joiner
			if (!$num_days && !$num_hours) {
				$joiner = '';
			} else if (!$num_seconds) {
				// else if there aren't any seconds so join with " and "
				$joiner = ' and ';
			} else {
				// else there must be more following so join with a comma
				$joiner = ', ';
			}

			$str .= $joiner.$num_minutes.' minute'.(($num_minutes > 1) ? 's' : '');
		}

		if ($num_seconds) {
			// if there are no days, hours or minutes don't need a joiner
			if (!$num_days && !$num_hours && !$num_minutes) {
				$joiner = '';
			} else {
				// else must be something preceding us so join with " and "
				$joiner = ' and ';
			}

			$str .= $joiner.$num_seconds.' second'.(($num_seconds > 1) ? 's' : '');
		}

	} else if (!$return_array) {

		// they just want the normal "xx days xx:xx:xx xxxxx"

		$time_type = '';

		if ($num_days) {
			$str .= $num_days.' day'.(($num_days > 1) ? 's' : '');
		}

		if ($num_hours) {
			// if there are no days don't need a joiner
			if (!$num_days) {
				$joiner = '';
			} else if ($num_minutes || $num_seconds) {
				// else we must be following the days so if there minutes or seconds we have a space
				$joiner = ' ';
			} else {
				// else we must be following the days so but as there is no minutes or seconds we have " and "
				$joiner = ' and ';
			}

			if (!$num_minutes && !$num_seconds) {
				$time_type = 'hour'.(($num_hours > 1) ? 's' : '');
			} else {
				$time_type .= 'h';
			}

			$str .= $joiner.$num_hours;
		}

		if ($num_minutes || ($num_hours && $num_seconds)) {

			// if there are no days or hours don't need a joiner
			if (!$num_days && !$num_hours) {
				$joiner = '';
			} else if ($num_hours) {
				// else if we are following hours, then have a colon
				$joiner = ':';
			} else if ($num_seconds) {
				// else we must be following the days so if there seconds we have a space
				$joiner = ' ';
			} else {
				// else we must be following the days so but as there is no seconds we have " and "
				$joiner = ' and ';
			}

			if (!$num_hours && !$num_seconds) {
				$time_type = 'minute'.(($num_minutes > 1) ? 's' : '');
				$str .= $joiner.$num_minutes;
			} else {
				$time_type .= $joiner.'m';
				// if there are hours then zerofill to 2 places
				if ($num_hours) {
					$str .= $joiner.sprintf('%02d', $num_minutes);
				} else {
					$str .= $joiner.$num_minutes;
				}
			}
		}

		if ($num_seconds) {
			// if there are no days or hours don't need a joiner
			if (!$num_days && !$num_hours && !$num_minutes) {
				$joiner = '';
			} else if ($num_hours || $num_minutes) {
				// else if there hours the have a colon
				$joiner = ':';
			} else {
				// else we must be following days, so just have a space
				$joiner = ' and ';
			}

			if (!$num_hours && !$num_minutes) {
				$time_type = 'second'.(($num_seconds > 1) ? 's' : '');
				$str .= $joiner.$num_seconds;
			} else {
				$time_type .= $joiner.'s';
				$str .= $joiner.sprintf('%02d', $num_seconds);
			}
		}

		if ($time_type) {
			$str .= ' '.((strstr($time_type, ':')) ? '('.trim($time_type).')' : trim($time_type));
		}

	} else {

		// they want the numbers in array format
		$return_value = Array('d' => 0, 'h' => 0, 'm' => 0, 's' => 0);

		if ($num_days) $return_value['d'] = $num_days;
		if ($num_hours) $return_value['h'] = $num_hours;
		if ($num_minutes) $return_value['m'] = $num_minutes;
		if ($num_seconds) $return_value['s'] = $num_seconds;

		return $return_value;
	}

	return $str.$suffix;

}//end easy_time_total()


/**
* Format a timestamp in a short, relative datetime format.
*
* Uses easy_datetime() and easy_time_total() along with its own handling for
* years/months/weeks to result in a short time format, that shows:
*
* Number of days either way if the date (including time) is within 14 days;
* number of weeks if within 1 month/months if within 1 year/years if more than:
* - eg. "1 year ago", "4 months ago", "2 weeks ago", "9 days in the future"
*
* If $with_day === TRUE, display day of week and time if the date (excluding
* time) is within 6 calendar days either way:
* - eg. "3:30pm yesterday", "3:30pm Monday", "3:30pm this Monday"
*   (the keyword "this" indicates "in the future")
*
* If $with_day === FALSE, duration intervals go down to seconds as appropriate.
*
* @param int		$timestamp	UNIX timestamp of the date you wish to format
* @param boolean	$with_day	If TRUE, show time and day of week, if within
*								a week (or "today", "yesterday", "tomorrow")
*
* @return string
*/
function easy_short_relative_datetime($timestamp, $with_day=TRUE)
{
	if (is_numeric($timestamp) === FALSE) return FALSE;
	$timestamp = (int)$timestamp;

	$current_time = time();
	$time_diff = $current_time - $timestamp;
	$duration_bits = easy_time_total(abs($time_diff), FALSE, TRUE);
	$duration_text = '';
	$add_ago = TRUE;

	if (empty($duration_bits) === FALSE) {
		if (strtotime('-1 year', $current_time) > $timestamp) {
			$years = (int)date('Y', $current_time) - (int)date('Y', $timestamp);
			if (date('m-d H:i:s', $current_time) < date('m-d H:i:s', $timestamp)) {
				$years--;
			}
			$duration_text = $years.' year';
			if ($years !== 1) $duration_text .= 's';

		} else if (strtotime('-1 year', $timestamp) > $current_time) {
			$years = (int)date('Y', $timestamp) - (int)date('Y', $current_time);
			if (date('m-d H:i:s', $timestamp) < date('m-d H:i:s', $current_time)) {
				$years--;
			}
			$duration_text = $years.' year';
			if ($years !== 1) $duration_text .= 's';

		} else if (strtotime('-1 month', $current_time) > $timestamp) {
			$months = 12 * ((int)date('Y', $current_time) - (int)date('Y', $timestamp));
			$months += (int)date('m', $current_time) - (int)date('m', $timestamp);
			if (date('d H:i:s', $current_time) < date('d H:i:s', $timestamp)) {
				$months--;
			}
			$duration_text = $months.' month';
			if ($months !== 1) $duration_text .= 's';

		} else if (strtotime('-1 month', $timestamp) > $current_time) {
			$months = 12 * ((int)date('Y', $timestamp) - (int)date('Y', $current_time));
			$months += (int)date('m', $timestamp) - (int)date('m', $current_time);
			if (date('d H:i:s', $timestamp) < date('d H:i:s', $current_time)) {
				$months--;
			}
			$duration_text = $months.' month';
			if ($months !== 1) $duration_text .= 's';

		} else if (abs($time_diff) >= 14*86400) {
			$duration_text = floor(abs($time_diff) / (7*86400)).' weeks';
		} else if (($with_day === TRUE) && (abs(strtotime('00:00:00 today', $current_time) - strtotime('00:00:00 today', $timestamp)) < 7*86400)) {
			return easy_datetime($timestamp);
		} else {
			reset($duration_bits);
			$duration_text = '';

			// Get the first 'duration bit' that we can find that has a value
			while (key($duration_bits) !== NULL) {
				if (current($duration_bits) > 0) {
					switch (key($duration_bits)) {
						case 'd':
							$duration_text = $duration_bits['d'].' day';
							if ($duration_bits['d'] !== 1) $duration_text .= 's';
						break;

						case 'h':
							$duration_text = $duration_bits['h'].' hour';
							if ($duration_bits['h'] !== 1) $duration_text .= 's';
						break;

						case 'm':
							$duration_text = $duration_bits['m'].' minute';
							if ($duration_bits['m'] !== 1) $duration_text .= 's';
						break;

						case 's':
							$duration_text = $duration_bits['s'].' second';
							if ($duration_bits['s'] !== 1) $duration_text .= 's';
						break;

					}//emd switch
				}//end if

				if ($duration_text !== '') {
					break;
				} else {
					next($duration_bits);
				}

			}//end while

			if (empty($duration_text) === TRUE) $duration_text = 'now';
		}

		if ($add_ago === TRUE) {
			if ($timestamp > $current_time) {
				$duration_text .= ' in the future';
			} else if ($timestamp < $current_time) {
				$duration_text .= ' ago';
			}
		}

	}

	return $duration_text;
}


/**
* Takes a timestamp and returns a human-friendly date/time string
*
* @param int	$then	the timestamp to convert
*
* @return string
* @access public
*/
function readable_datetime($then=NULL)
{
	return (is_null($then)) ? date('jS M Y g:ia') : date('jS M Y g:ia', $then);

}//end readable_datetime()


/**
* Returns whether the passed year is a leap year or not
*
* @param int	$year	the year to check if it's a leap year or not
*
* @return boolean
* @access public
*/
function is_leap_year($year=NULL)
{
	if (is_null($year)) $year = (int) date('Y');
	if ($year % 4 != 0) return FALSE;

	// only 1 out of every four hundred year milestones is a leap year
	if ($year % 100 == 0) return (($year / 100) % 4 == 0);

	return TRUE;

}//end is_leap_year()


/**
* Returns the number of days in the specified month
*
* @param int	$month	the month to return the number of days in (NULL means current month)
* @param int	$year	the year that the month is in (NULL means current year)
*
* @return boolean
* @access public
*/
function days_in_month($month, $year)
{
	if (is_null($month)) $month = (int) date('n');
	if (is_null($year))  $year  = (int) date('Y');

	switch ($month) {
		case  2 : // Feb
			return (is_leap_year($year)) ? 29 : 28;

		case  1 : // Jan
		case  3 : // March
		case  5 : // May
		case  7 : // July
		case  8 : // Aug
		case 10 : // Oct
		case 12 : // Dec
			return 31;

		case  4 : // April
		case  6 : // June
		case  9 : // Sept
		case 11 : // Nov
			return 30;
		default :
			trigger_error('UNKNOWN MONTH : '.$month, E_USER_ERROR);
	}//end switch

}//end days_in_month()


/**
* Increments the given date by the specified number of days
*
* Months and years are rolled over as necessary
*
* @param int	&$day			day component of the date to increment (1-31)
* @param int	&$month			month component of the date to increment (1-12)
* @param int	&$year			year component of the date to increment (YYYY)
* @param int	$days_to_add	number of days to add to the date
*
* @return void
* @access public
*/
function increment_date(&$day, &$month, &$year, $days_to_add=1)
{
	if ($days_to_add < 0) {
		decrement_date($day, $month, $year, -$days_to_add);
		return;
	}

	$day += $days_to_add;
	while ($day > days_in_month($month, $year)) {
		$day = $day - days_in_month($month, $year);
		increment_month($month, $year);
	}

}//end increment_date()


/**
* Decrements the given date by the specified number of days
*
* Months and years are rolled over as necessary
*
* @param int	&$day			day component of the date to decrement (1-31)
* @param int	&$month			month component of the date to decrement (1-12)
* @param int	&$year			year component of the date to decrement (YYYY)
* @param int	$days_to_sub	number of days to subtract from the date
*
* @return void
* @access public
*/
function decrement_date(&$day, &$month, &$year, $days_to_sub=1)
{
	if ($days_to_sub < 0) {
		increment_date($day, $month, $year, -$days_to_sub);
		return;
	}

	$day = $day - $days_to_sub;
	while ($day < 1) {
		decrement_month($month, $year);
		$day = days_in_month($month, $year) + $day;
	}

}//end decrement_date()


/**
* Increments the given month by the specified number
*
* Years are rolled over as necessary
*
* @param int	&$month			month component of the date to increment (1-12)
* @param int	&$year			year component of the date to increment (YYYY)
* @param int	$months_to_add	number of months to add to the date
*
* @return void
* @access public
*/
function increment_month(&$month, &$year, $months_to_add=1)
{
	if ($months_to_add < 0) {
		decrement_month($month, $year, -$months_to_add);
		return;
	}

	$month += $months_to_add;
	while ($month > 12) {
		$month = $month - 12;
		$year++;
	}

}//end increment_month()


/**
* Decrements the given month by the specified number
*
* Years are rolled over as necessary
*
* @param int	&$month			month component of the date to increment (1-12)
* @param int	&$year			year component of the date to increment (YYYY)
* @param int	$months_to_sub	number of months to subtract from the date
*
* @return void
* @access public
*/
function decrement_month(&$month, &$year, $months_to_sub=1)
{
	if ($months_to_sub < 0) {
		increment_month($month, $year, -$months_to_sub);
		return;
	}

	$month = $month - $months_to_sub;
	while ($month < 1) {
		$month = 12 + $month;
		$year--;
	}

}//end decrement_month()


/**
* Increments an ISO date by the specified number of days and returns the changed ISO
*
* @param string	$iso			ISO date component (YYYY-MM-DD)
* @param int	$days_to_add	number of days to add to the date
*
* @return string
* @access public
*/
function add_days_to_iso($iso, $days_to_add=1)
{
	list($year,$month,$day) = sscanf($iso, '%04d-%02d-%02d');
	increment_date($day, $month, $year, $days_to_add);
	return sprintf('%04d-%02d-%02d', $year, $month, $day);

}//end add_days_to_iso()


/**
* Increments an ISO date by the specified number of months and returns the changed ISO
*
* @param string	$iso			ISO year-month component (YYYY-MM)
* @param int	$months_to_add	number of months to add to the date
*
* @return string
* @access public
*/
function add_months_to_iso($iso, $months_to_add=1)
{
	list($year,$month) = explode('-', $iso);
	increment_month($month, $year, $months_to_add);
	return sprintf('%04d-%02d', $year, $month);

}//end add_months_to_iso()


/**
* Returns the difference between two ISO dates in whole days
*
* The dates passed must be of the standard ISO format (ie. YYYY-MM-DD, YYYY-MM-DD HH:MM:SS
* etc), and only the date components (ie. YYYY-MM-DD) will be used in the calculation.
* Order of arguments is important; if $higher < $lower than the result of this function
* will be negative.
* It returns the number of days between the two dates
*
* @param string	$higher	the higher/later ISO date
* @param string	$lower	the lower/earlier ISO date
*
* @return int
* @access public
*/
function days_between_isos($higher, $lower)
{
	$higher_ts = iso8601_ts(substr($higher,0,10));
	$lower_ts  = iso8601_ts(substr($lower,0,10));

	// using round() to remove any one-hour
	return round(($higher_ts - $lower_ts) / 86400);

}//end days_between_isos()


/**
* converts a integer into its roman numeral counterpart
*
* @param int	$num	the number to convert
*
* @return string
* @access public
*/
function int_to_roman($num=0)
{
	$conv = Array(
				10	=> Array('X', 'C', 'M'),
				5	=> Array('V', 'L', 'D'),
				1	=> Array('I', 'X', 'C'),
			);
	$roman = '';

	$num = (int) $num;

	$digit  = (int) ($num / 1000);
	$num   -= $digit * 1000;
	$roman = str_repeat('M',$digit);

	for ($i = 2; $i >= 0; $i--) {
		$power = pow(10, $i);
		$digit = (int) ($num / $power);
		$num -= $digit * $power;

		if (($digit == 9) || ($digit == 4)) {
			$roman .= $conv[1][$i].$conv[$digit+1][$i];
		} else {
			if ($digit >= 5) {
				$roman .= $conv[5][$i];
				$digit -= 5;
			}

			while ($digit > 0) {
				$roman .= $conv[1][$i];
				$digit--;
			}
		}
	}
	return($roman);

}//end int_to_roman()


/**
* Takes a month in a certain year and returns an array of dates in that month that are a certain days of the week
* NOTE: dates returned are zero-padded
*
* @param int	$weekday	number for the weekday (Sunday = 0, Monday = 1 ... Saturday = 6)
* @param int	$month		the month to use
* @param int	$year		the year that the month is in
*
* @return array
* @access public
*/
function weekday_dates_in_month($weekday, $month, $year)
{
	$day = mktime(0, 0, 0, (int) $month, 1, (int) $year);
	$first_weekday = (int) date('w', $day);

	if ((int) $weekday < $first_weekday) {
		$day_add = 7 - $first_weekday + (int) $weekday;
	} else {
		$day_add = (int) $weekday - $first_weekday;
	}
	$day = strtotime('+'.$day_add.' days', $day);

	$days = Array();
	do {
		$days[] = date('d', $day);
		$day = strtotime('+1 week', $day);
	} while ((int) date('n', $day) == $month);

	return $days;

}//end weekday_dates_in_month()


/**
* Returns the date component of an ISO8601 format date
*
* It returns the date part (still in ISO format, YYYY-MM-DD)\
* OR false if date passed is not valid ISO date
* OR has a null date part
*
* @param string	$iso8601	The date string to strip the date part from
*
* @return mixed string|boolean
* @access public
*/
function iso8601_date_component($iso8601)
{
	if (!is_iso8601($iso8601)) return FALSE;
	if (substr($iso8601,0,10) == '----------') {
		return FALSE;
	}
	return substr($iso8601,0,10);

}//end iso8601_date_component()


/**
* Returns the time component of an ISO8601 format date
*
* It returns the time from the ISO data(HH:MM:SS)
* OR false if invalid date or does not have time part
*
* @param string	$iso8601	The date to strip the time from
*
* @return mixed string|boolean
* @access public
*/
function iso8601_time_component($iso8601)
{
	if (!is_iso8601($iso8601)) return FALSE;
	if (strlen($iso8601) < 13) return FALSE;
	if (substr($iso8601,11) == '--:--:--') return FALSE;
	return substr($iso8601,11,8);

}//end iso8601_time_component()


/**
* Returns true if passed string is a valid ISO8601 time with at least a full date
* Currently valid
* Date only (2005-10-27) - five digit dates (for 10000+ AD) are not permitted
* Date and time without time zone (2005-10-27T09:16:00)
* Date and time with time zone (2005-10-27T09:16:00+10:00)
* Date and time with "Z" (Zulu) time zone for UTC (2005-10-26T23:16:00Z)
* The "T" can also be replaced by a space, for backward compatibility with
* those using SQL
*
* Dates and times must be "full" - abbreviated versions (eg. omission of seconds)
* are not permitted
*
* @param string	$iso8601	The date that we are going to check
*
* @return boolean
* @access public
*/
function is_iso8601($iso8601)
{
	return (boolean)preg_match("/^\-?[0-9\-]{4}-[0-9\-]{2}-[0-9\-]{2}([ T]([0-9\-]{2}\:?){3}(Z|([+-]([0-9\-]{2}\:?){2}))?)?$/",$iso8601);

}//end is_iso8601()


?>
