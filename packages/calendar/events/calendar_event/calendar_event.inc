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
* $Id: calendar_event.inc,v 1.7 2004/07/22 06:40:23 tbarrett Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';
require_once SQ_FUDGE_PATH.'/datetime_field/datetime_field.inc';
require_once SQ_FUDGE_PATH . '/general/datetime.inc';

/**
* Calendar_Event
*
* Purpose
*     
*
*
* @author  Luke Wright <lwright@squiz.net>
* @author  Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.7 $ - 1.0
* @package MySource_Matrix_Packages
* @subpackage calendar
*/
class Calendar_Event extends Asset
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Calendar_Event($assetid=0)
	{
		$this->Asset($assetid);

	}//end Calendar_Event()

 
	/**
	* Returns name of the asset
	*
	* @param bool	$short_name	whether or not we are after the shortname or the full name
	*
	* @see Asset::_getName()
	* @return string
	* @access protected
	*/
	function _getName($short_name=false)
	{
		return $this->attr('name');
	
	}//end _getName()


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

		// get rid of the raw attribute values which will just make a mess
		unset($keywords['asset_attribute_start_date']);
		unset($keywords['asset_attribute_end_date']);

		$keywords['event_start_datetime'] = Array('value'=>$this->getFriendlyStart());
		$keywords['event_end_datetime'] = Array('value'=>$this->getFriendlyEnd());
		$keywords['event_datetime_summary'] = Array('value'=>$this->getFriendlyDatesAndTimes());
		$keywords['event_duration'] = Array('value'=>easy_time_total($this->getDuration(), true));

		if ($descriptions) {
			$keywords['event_start_datetime']['description'] = 'event start date and time';
			$keywords['event_end_datetime']['description'] = 'event end date and time';
			$keywords['event_datetime_summary']['description'] = 'friendly display of event dates and times';
			$keywords['event_duration']['description'] = 'event duration';
		}
		return $keywords;

	}//end getAssetKeywords()


	/**
	* Save attributes set with setAttrValue to the database
	*
	* @param boolean	$dont_run_updated	if true this->_updated() won't be run
	*										(THIS IS ONLY EVER CALLED FROM Asset::create())
	*
	* @return boolean
	* @access public
	* @see Asset::setAttrValue()
	*/
	function saveAttributes($dont_run_updated=false)
	{
		$start_date = $this->attr('start_date');
		$end_date   = $this->attr('end_date');

		// work out if we have any 'null date' or 'null time' sections
		$start_has_time = (strpos($start_date, '--:--:--', 11) === false);
		$end_has_time   = (strpos($end_date, '--:--:--', 11) === false);
		$start_has_date = (strpos($start_date, '----------') === false);
		$end_has_date   = (strpos($end_date, '----------') === false);

		// All events must have a starting date at least!
		if (!$start_has_date) {
			trigger_error('Calendar events must at a minimum include the date component of a Start Date', E_USER_WARNING);
			return false;
		}

		// cannot have End Date being like '---------- 11:00:00'
		if (!$end_has_date && $end_has_time) {
			trigger_error('If the \'End Date\' includes a time component it must also include a date', E_USER_WARNING);
			return false;
		}

		// Cannot have an end time if you have no start time
		if ($end_has_time && (!$start_has_time)) {
			trigger_error('You have specified an end time but not a start time.  Please specify a start time.', E_USER_WARNING);
			return false;
		}

		// Cannot have an end date and start time if you have no end time
		if ($end_has_date && $start_has_time && (!$end_has_time)) {
			trigger_error('You have specified an end date and a start time but no end time.  Please specify an end time.', E_USER_WARNING);
			return false;
		}


		// If an End Date is specified (and we need date component so we can use that to check),
		// the End Date must be no earlier than the start Date
		if ($end_has_date) {
			if (iso8601_ts($end_date) < iso8601_ts($start_date)) {
				trigger_error('The \'End Date\' of this '.ucwords(str_replace('_',' ',$this->type())).' cannot be prior to the \'Start Date\'', E_USER_WARNING);
				return false;
			}
		}

		// oh no, we're not using this to display anything... just to use its units extraction :-)
		$datetime = new Datetime_Field('', $value='', $params=Array());

		// check individual sections to make sure we can't commit part dates like '16th, 2005 48'
		$start_components = $datetime->strToUnitsArray($start_date);
		$end_components = $datetime->strToUnitsArray($end_date);

		// don't need to check Start Date's date component as it is required!
		// must check other parts of both Start and End Date so a meaningful date is stored

		// For dates, this means all three parts of the date have a value
		if ($end_has_date && (($end_components['d'] == -1) || ($end_components['m'] == -1) || ($end_components['y'] == -1))) {
			trigger_error('If the date component of \'End Date\' is specified it must specify all of day, month and year', E_USER_WARNING);
			return false;
		}

		// For times, at least hour and minute must be specified. You can get away with not specifying
		// seconds, it's optional.
		if ($start_has_time && (($start_components['h'] == -1) || ($start_components['i'] == -1))) {
			trigger_error('If the time component of \'Start Date\' is specified it must specify at least hour and minute', E_USER_WARNING);
			return false;
		}

		if ($end_has_time && (($end_components['h'] == -1) || ($end_components['i'] == -1))) {
			trigger_error('If the time component of \'End Date\' is specified it must specify at least hour and minute', E_USER_WARNING);
			return false;
		}
			
		// all sweet, so it seems
		return parent::saveAttributes($dont_run_updated);

	}//end saveAttributes()
	

	/**
	* Returns the duration between the start and end dates, if they are set
	*
	* If end date/time is null => return 0 if start time specified (single point in time)
	*                          => return '1 day' (86400 seconds) if start time not specified
	*                             (single-day event)
	* If start date and end date only specified	=> return calendar days between two dates
	*                                              (ie. calendar days * 86400)
	* If both fields specified in full => return the difference between the two times in seconds
	*
	* @return int
	* @access public
	*/
	function getDuration()
	{
		$seconds_per_day = 24 * 60 * 60;

		$start_date = $this->attr('start_date');
		$end_date   = $this->attr('end_date');

		// work out if we have any 'null date' or 'null time' sections
		$start_has_time = (strpos($start_date, '--:--:--', 11) === false);
		$end_has_time   = (strpos($end_date, '--:--:--', 11) === false);
		$start_has_date = (strpos($start_date, '----------') === false);
		$end_has_date   = (strpos($end_date, '----------') === false);

		if (!$start_has_date) return null;

		if (!$end_has_date) {
			if ($start_has_time) {
				// single point in time, and so it has no duration
				return 0;
			} else {
				// this is a single day event, return '1 day'
				return $seconds_per_day;
			}
		}

		// NOTE: no need to replace null time fields with 'midnight' (00:00:00) because the
		// ISO-to-timestamp conversion function already recognises that it's not a full datetime
		// if that's the case

		if (!$start_has_time && !$end_has_time)	{
			// in a multiple day event without times it's assumed to be the start of the start date
			// (ie. 00:00) to the end of the end date (ie. 00:00 the day after end date).
			return iso8601_ts($end_date) - iso8601_ts($start_date) + $seconds_per_day;

		} else {
			// all fields specified, from one time point to another
			return iso8601_ts($end_date) - iso8601_ts($start_date);

		}

	}//end getDuration()


	/**
	* Get the start date and time in friendly format
	*
	* @return string the start date and time if appropriate, in friendly format
	* @access public
	*/
	function getFriendlyStart() 
	{
		if (strpos($this->attr('start_date'), '--:--:--') === FALSE) {
			return readable_datetime(iso8601_ts($this->attr('start_date')));
		} else { 
			return date('d M Y', iso8601_ts($this->attr('start_date')));
		}

	}//end getFriendlyStart()


	/**
	* Get the end date and time in friendly format
	*
	* @return string the end date and time if appropriate, in friendly format
	* @access public
	*/
	function getFriendlyEnd() 
	{
		if (strpos($this->attr('end_date'), '--:--:--') === FALSE) {
			return readable_datetime(iso8601_ts($this->attr('end_date')));
		} elseif  (iso8601_ts($this->attr('end_date')) == -1) {
			return '';
		} else {
			return date('d M Y', iso8601_ts($this->attr('end_date')));
		}

	}//end getFriendlyEnd()


	/**
	* Get the start and end details in friendly format
	* 
	* If date components of Start Date and End Date attributes are the same, the date is not repeated,
	* and the times are shown in abbreviated format (see example returns below).
	*
	* Example returns:
	* if (start date != end date)		"23 Jul 2004 4:00pm - 25 Jul 2004 10:00am"
	* if (start date == end date)		"23 Jul 2004 4:00pm-5:00pm"
	* if (end date is empty)			"23 Jul 2004 4:00pm"
	*
	* @return string
	* @access public
	*/
	function getFriendlyDatesAndTimes()
	{
		if (substr($this->attr('start_date'), 0, 10) == substr($this->attr('end_date'), 0, 10)) {
			return $this->getFriendlyStart().'-'.date('g:ia', iso8601_ts($this->attr('end_date')));
		} elseif (iso8601_ts($this->attr('end_date')) == -1) {
			return $this->getFriendlyStart();
		} else {
			return $this->getFriendlyStart().' - '.$this->getFriendlyEnd();
		}

	}//end getFriendlyDatesAndTimes()


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
				<td><b>Start Date</b></td>
				<td><?php echo $this->getFriendlyStart(); ?></td>
			</tr>
			<tr>
				<td><b>End Date</b></td>
				<td><?php echo $this->getFriendlyEnd(); ?></td>
			</tr>
			<tr>
				<td><b>Duration</b></td>
				<td><?php echo easy_time_total($this->getDuration(), true); ?></td>
			</tr>
			<tr>
				<td><b>Description</b></td>
				<td><?php echo $this->attr('description'); ?></td>
			</tr>
		</table>
		<?php

	}//end printBody()


}//end class

?>