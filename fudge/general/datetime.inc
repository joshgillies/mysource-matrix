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
* $Id: datetime.inc,v 1.14 2004/07/23 04:29:07 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


/**
* DateTime Functions
*
* Purpose
*     General function for manipulating dates and times
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Fudge
* @subpackage General
*/

/**
* Takes a timestamp and returns a date/time string with detail varying by how recent the date is
*
* @param string	$then	the timestamp for some time in the future
*
* @return string
* @access public
*/
function easy_datetime($then='')
{
	$now = time();
	if ($now - $then < 86400) { // A Day
		return ((date('H')>=date('H',$then))?'Today ':'Yesterday ') . date('h:ia',$then);
	} elseif ($now - $then < 604800) { // A week
		return date('D h:ia',$then);
	} else {
		return date('d M Y H:i',$then);
	}
}//end easy_datetime()


/**
* Returns the number of days, hours, minutes and seconds that relate to the number of seconds passed
*
* @param string		$secs			the number of seconds that have passed
* @param boolean	$full_format	print full hours, minutes, seconds text
*
* @return string
* @access public
*/
function easy_time_total($secs=0, $full_format=false)
{
	if ($secs < 0) { 
		$suffix = ' ago';
		$secs = abs($secs);
	} else {
		$suffix = '';
	}

	$secs = (int) $secs;

	$minute = 60;
	$hour   = 60 * $minute;
	$day    = 24 * $hour;

	$num_days    = 0;
	$num_hours   = 0;
	$num_minutes = 0;
	$num_seconds = 0;

	// if there is more than one day left
	if ($secs >= $day) {
		$num_days = floor($secs/$day);
		// remove the number of seconds we have taken care of
		$secs -= ($day * $num_days);
	}

	// if there is more than one hour left
	if ($secs >= $hour) {
		$num_hours = floor($secs/$hour);
		// remove the number of seconds we have taken care of
		$secs -= ($hour * $num_hours);
	}

	// if there is more than one minute left
	if ($secs >= $minute) {
		$num_minutes = floor($secs/$minute);
		// remove the number of seconds we have taken care of
		$secs -= ($minute * $num_minutes);
	}

	$num_seconds = $secs;

	$str = '';

	// if they want the full string version
	if ($full_format) {

		if ($num_days) {
			$str .= $num_days.' day'.(($num_days > 1) ? 's' : '');
		}

		if($num_hours) {
			// if there are no days don't need a joiner
			if (!$num_days) $joiner = '';
			// else if there aren't any minutes or seconds so join with " and "
			elseif (!$num_minutes && !$num_seconds) $joiner = ' and ';
			// else there must be more following join with a comma
			else $joiner = ', ';

			$str .= $joiner.$num_hours.' hour'.(($num_hours > 1) ? 's' : '');
		}

		if($num_minutes) {
			// if there are no days or hours don't need a joiner
			if (!$num_days && !$num_hours) $joiner = '';
			// else if there aren't any seconds so join with " and "
			elseif (!$num_seconds) $joiner = ' and ';
			// else there must be more following so join with a comma
			else $joiner = ', ';

			$str .= $joiner.$num_minutes.' minute'.(($num_minutes > 1) ? 's' : '');
		}

		if($num_seconds) {
			// if there are no days, hours or minutes don't need a joiner
			if (!$num_days && !$num_hours && !$num_minutes) $joiner = '';
			// else must be something preceding us so join with " and "
			else $joiner = ' and ';

			$str .= $joiner.$num_seconds." second".(($num_seconds > 1) ? 's' : '');
		}

	// else they just want the normal "xx days xx:xx:xx xxxxx"
	} else {

		$time_type = '';

		if ($num_days) {
			$str .= $num_days.' day'.(($num_days > 1) ? 's' : '');
		}

		if($num_hours) {
			// if there are no days don't need a joiner
			if (!$num_days) $joiner = '';
			// else we must be following the days so if there minutes or seconds we have a space
			elseif ($num_minutes || $num_seconds) $joiner = ' ';
			// else we must be following the days so but as there is no minutes or seconds we have " and "
			else $joiner = ' and ';

			if (!$num_minutes && !$num_seconds) {
				$time_type = 'hour'.(($num_hours > 1) ? 's' : '');
			} else {
				$time_type .= 'h';
			}

			$str .= $joiner.$num_hours;
		}

		if($num_minutes || ($num_hours && $num_seconds)) {

			// if there are no days or hours don't need a joiner
			if (!$num_days && !$num_hours) $joiner = '';
			// else if we are following hours, then have a colon
			elseif ($num_hours) $joiner = ':';
			// else we must be following the days so if there seconds we have a space
			elseif ($num_seconds) $joiner = ' ';
			// else we must be following the days so but as there is no seconds we have " and "
			else $joiner = ' and ';

			if (!$num_hours && !$num_seconds) {
				$time_type = 'minute'.(($num_minutes > 1) ? 's' : '');
				$str .= $joiner.$num_minutes;
			} else {
				$time_type .= $joiner.'m';
				// if there are hours then zerofill to 2 places
				if ($num_hours) {
					$str .= $joiner.sprintf("%02d", $num_minutes);
				} else {
					$str .= $joiner.$num_minutes;
				}
			}
		}

		if($num_seconds) {
			// if there are no days or hours don't need a joiner
			if (!$num_days && !$num_hours && !$num_minutes) $joiner = '';
			// else if there hours the have a colon
			elseif ($num_hours || $num_minutes) $joiner = ':';
			// else we must be following days, so just have a space
			else $joiner = ' and ';

			if (!$num_hours && !$num_minutes) {
				$time_type = 'second'.(($num_seconds > 1) ? 's' : '');
				$str .= $joiner.$num_seconds;
			} else {
				$time_type .= $joiner.'s';
				$str .= $joiner.sprintf("%02d", $num_seconds);
			}
		}

		if ($time_type) {
			$str .= ' '.((strstr($time_type, ':')) ? '('.trim($time_type).')' : trim($time_type));
		}

	}

	return $str.$suffix;

}//end easy_time_total()


/**
* Takes a timestamp and returns a human-friendly date/time string
*
* @param int	$then	the timestamp to convert
*
* @return string
* @access public
*/
function readable_datetime($then=null)
{
	$format = 'jS M Y g:ia';
	return (is_null($then)) ? date($format) : date($format, $then);

}//end readable_datetime()


/**
* Returns whether the passed year is a leap year or not
*
* @param int	$year	the year to check if it's a leap year or not
*
* @return boolean
* @access public
*/
function is_leap_year($year=null)
{
	if (is_null($year)) $year = (int) date('Y');
	if ($year % 4 != 0) return false;

	// only 1 out of every four hundred year milestones is a leap year
	if ($year % 100 == 0) {
		return (($year / 100) % 4 == 0);
	}

	return true;

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

	switch($month) {
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
* converts a integer into its roman numeral counterpart
*
* @param integer $num the number to convert
*
* @access public
* @return string
*/
function int_to_roman($num = 0) 
{
	$conv = array(10 => array('X', 'C', 'M'),
				  5  => array('V', 'L', 'D'),
				  1  => array('I', 'X', 'C'));
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
			$roman .= $conv[1][$i] . $conv[$digit+1][$i];
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



?>