<?php

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
*/
function easy_time_total($secs=0, $full_format=false)
{

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

	return $str;

}//end easy_time_total()


?>