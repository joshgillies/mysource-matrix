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
* $Id: calendar_event.inc,v 1.4 2004/08/02 06:29:03 lwright Exp $
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
* @version $Revision: 1.4 $ - 1.0
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
	* Create this asset
	*
	* @param Array	&$link	information used to create the initial link
	*
	* @return mixed int or boolean	the link ID or the create status if no link
	* @access public
	* @see Asset::create()
	*/
	function create(&$link) 
	{
		$name = $this->attr('name');
		
		// should not be able to create an event with a blank name
		if ($name == '') {
			trigger_error('Unable to create a '.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name').' with a blank name', E_USER_WARNING);
			return false;
		}
		
		return parent::create($link);

	}//end create()
	
	
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

		// give useful info if no duration (single point in time) - easy_time_total returns empty string
		$total_time = easy_time_total($this->getDuration(), true);
		if (empty($total_time)) $total_time = 'N/A';

		$keywords['event_duration'] = Array('value'=>$total_time);

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
		$start_has_time = $this->getStartTime();
		$end_has_time   = $this->getEndTime();
		$start_has_date = $this->getStartDate();
		$end_has_date   = $this->getEndDate();

		// All events must have a starting date at least!
		if (!$start_has_date) {
			trigger_error('Calendar Events must at least have a start date', E_USER_WARNING);
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
	* Get the date component of the Start Date attribute
	*
	* @return mixed (string|boolean) the date component of the start date (ISO8601 fmt,
	* 				yyyy-mm-dd) or boolean FALSE if the date component is empty
	* @access public
	*/
	function getStartDate() 
	{
		$res = substr($this->attr('start_date'), 0, 10);
		return ($res == '----------') ? false : $res;
		
	}//end getStartDate()


	/**
	* Get the date component of the End Date attribute
	*
	* @return mixed (string|boolean) the date component of the end date (ISO8601 fmt,
	* 				yyyy-mm-dd) or boolean FALSE if the date component is empty
	* @access public
	*/
	function getEndDate() 
	{
		$res = substr($this->attr('end_date'), 0, 10);
		return ($res == '----------') ? false : $res;
		
	}//end getEndDate()

	
	/**
	* Get the time component of the Start Date attribute
	*
	* @return mixed (string|boolean) the time component of the start date (hh:mm:ss)
					or boolean FALSE if the time component is empty (--:--:--)
	* @access public
	*/
	function getStartTime() 
	{
		$res = substr($this->attr('start_date'), 11);
		return ($res == '--:--:--') ? false : $res;
		
	}//end getStartTime()


	/**
	* Get the time component of the End Date attribute
	*
	* @return mixed (string|boolean) the time component of the end date (hh:mm:ss)
					or boolean FALSE if the time component is empty (--:--:--)
	* @access public
	*/
	function getEndTime() 
	{
		$res = substr($this->attr('end_date'), 11);
		return ($res == '--:--:--') ? false : $res;
		
	}//end getEndTime()
	

	/**
	* Returns the duration between the start and end dates, if they are set
	*
	* If end date/time is null => return 0 (single point in time)
	* If start date and end date only specified	=> return calendar days covered by two dates
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
		$start_has_time = $this->getStartTime();
		$end_has_time   = $this->getEndTime();
		$start_has_date = $this->getStartDate();
		$end_has_date   = $this->getEndDate();

		if (!$start_has_date) return null;

		if (!$end_has_date) {
			// single point in time, and so it has no duration
			return 0;
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
	* Example returns (assuming date and time components specified):<PRE>
	* if (start date != end date)		"23 Jul 2004 4:00pm - 25 Jul 2004 10:00am"
	* if (start date == end date)		"23 Jul 2004 4:00pm-5:00pm"
	* if (end date is empty)			"23 Jul 2004 4:00pm"
	* </PRE>
	* If only date components are specified:<PRE>
	* if (start date != end date)		"23 Jul 2004 - 25 Jul 2004"
	* if (start date == end date)
	* 	OR (end date is empty)			"23 Jul 2004"
	* </PRE>
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
		require_once SQ_FUDGE_PATH.'/general/datetime.inc';
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
				<td><?php
						// give useful info if no duration - easy_time_total returns empty string
						$total_time = easy_time_total($this->getDuration(), true);
						echo (empty($total_time) ? 'N/A' : $total_time);
					?></td>
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