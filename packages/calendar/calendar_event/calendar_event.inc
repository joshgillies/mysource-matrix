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
* $Id: calendar_event.inc,v 1.26 2004/09/09 00:53:51 tbarrett Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';
require_once SQ_FUDGE_PATH.'/datetime_field/datetime_field.inc';
require_once SQ_FUDGE_PATH.'/general/datetime.inc';

/**
* Calendar_Event
*
* Purpose
*     
*
*
* @author  Luke Wright <lwright@squiz.net>
* @author  Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.26 $ - 1.0
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
		
		$res = parent::create($link);
		if ($this->cacheCalendarData(false)) {
			return $res;
		} else {
			return false;
		}

	}//end create()
	
	
	/**
	* Delete this asset from the trash
	*
	* Note that the asset <i>MUST</i> be in the trash to delete it <i>FROM</i> the trash.
	*
	* @param boolean	$release_lock	should we release the lock after deleting
	*
	* @return boolean
	* @access public
	*/
	function delete($release_lock=true)
	{
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		
		// asset id becomes a where condition
		$where = ' assetid = '.$db->quote($this->id);
		
		if (!$GLOBALS['SQ_SYSTEM']->rollbackDelete('calendar_date_value', $where)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}
		
		if (!parent::delete($release_lock)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}
		
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;
		
	}//end delete()
	
	
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
		unset($keywords['asset_name']);
		unset($keywords['asset_short_name']);
		unset($keywords['asset_thumbnail']);
		$keywords['event_name'] = $keywords['asset_attribute_name'];
		unset($keywords['asset_attribute_name']);


		$keywords['event_start_datetime'] = Array('value'=>$this->getFriendlyStart());
		$keywords['event_end_datetime'] = Array('value'=>$this->getFriendlyEnd());
		$keywords['event_datetime_summary'] = Array('value'=>$this->getFriendlyDatesAndTimes());

		// give useful info if no duration (single point in time) - easy_time_total returns empty string
		$total_time = easy_time_total($this->getDuration(), true);
		if (empty($total_time)) $total_time = 'N/A';

		$keywords['event_duration'] = Array('value'=>$total_time);
		$keywords['edit_link'] = Array('value'=>($this->writeAccess('') ? '<a href="./_edit?limbo_assetid='.$this->id.'">Edit</a>' : ''));

		if ($descriptions) {
			$keywords['event_start_datetime']['description'] = 'Event start date/time';
			$keywords['event_end_datetime']['description'] = 'Event end date/time';
			$keywords['event_datetime_summary']['description'] = 'Friendly description of event dates and times';
			$keywords['event_duration']['description'] = 'Duration of the Event';
			$keywords['edit_link']['description'] = 'Link to edit this eveent in Limbo';
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
			trigger_error('If the date component of \'End Date\' is specified it must specify all of day, month and year '.$end_date, E_USER_WARNING);
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
		
		// cache all the calendar data into another table so the queries for the calendar
		// page go super-mega-hyperturbo (but only if already created)
		if (!empty($this->id) && !$dont_run_updated) {
			if (!$this->cacheCalendarData()) return false;
		}
		
		// all sweet, so it seems
		return parent::saveAttributes($dont_run_updated);

	}//end saveAttributes()
	

	/**
	* Takes a clone shell and clones the sources required components to the clone shell
	*
	* @param &object Asset	$clone		the clone shell
	* @param array			$components	the wanted components to clone eg.
	*                       Array(
	*                           'attributes',
	*                           'metadata_shemas',
	*                           'metadata',
	*                           'workflow',
	*                           'permissions',
	*                           'data'
	*                       );
	*                       or alternately
	*                       Array('all');
	*
	* @return boolean
	* @access public
	*/
	function cloneComponents(&$clone, $components)
	{
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		
		if (!parent::cloneComponents($clone, $components)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;			
		}
		
		if (in_array('all', $components) || in_array('attributes', $components)) {
			if (!$clone->cacheCalendarData(false)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}
		}
		
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');		
		return true;
		
	}//end cloneComponents()


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
		if ($this->getStartTime() !== false) {
			return readable_datetime(iso8601_ts($this->attr('start_date')));
		} else { 
			return date('jS M Y', iso8601_ts($this->attr('start_date')));
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
		if ($this->getEndTime() !== false) {
			return readable_datetime(iso8601_ts($this->attr('end_date')));
		} elseif  (iso8601_ts($this->attr('end_date')) == -1) {
			return '';
		} else {
			return date('jS M Y', iso8601_ts($this->attr('end_date')));
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
	* if (start date == end date)		"23 Jul 2004 (all day)"
	* if (end date is empty)			"23 Jul 2004"
	* </PRE>
	*
	* @return string
	* @access public
	*/
	function getFriendlyDatesAndTimes()
	{
		if (substr($this->attr('start_date'), 0, 10) == substr($this->attr('end_date'), 0, 10)) {
			// start and end dates are the same
			if (!$this->getStartTime()) {
				// no start time, so must be an all day event
				return $this->getFriendlyStart() . ' (all day)';
			} elseif (($this->getStartTime() == $this->getEndTime())) {
				// show start date and time only
				return $this->getFriendlyStart();
			} else {
				// show date and start/end times
				return $this->getFriendlyStart().'-'.date('g:ia', iso8601_ts($this->attr('end_date')));
			}
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


	/**
	* Process the specified action on this event, and return whatever html the action wants to show
	*
	* This should be overridden by child classes
	*
	* @param string	$action_name	The action to perform, eg 'cancel'
	*
	* @return string	HTML to show as the result of this action
	* @access public
	*/	
	function processEventAction($action_name) 
	{
		switch ($action_name) {
			case 'move':
				if (!isset($_REQUEST['SQ_CALENDAR_DATE'])) {
					trigger_error($action_name.' is an unknown action for this type of event', E_USER_WARNING);	
					return '';
				}
				$duration = $this->getDuration();
				if (!isset($_REQUEST['SQ_CALENDAR_TIME']) || empty($_REQUEST['SQ_CALENDAR_TIME'])) $_REQUEST['SQ_CALENDAR_TIME'] = '--:--';
				$this->setAttrValue('start_date', $_REQUEST['SQ_CALENDAR_DATE'].' '.$_REQUEST['SQ_CALENDAR_TIME'].':--');
				$current_end_date = $this->getEndDate();
				if ($current_end_date) {
					$new_end_date_ts = iso8601_ts($this->attr('start_date')) + $duration;
					$new_end_date = date('Y-m-d', $new_end_date_ts);
					$new_end_time = $this->getEndTime() ? date('H:i:00', $new_end_date_ts) : '--:--:--';
					$this->setAttrValue('end_date', $new_end_date.' '.$new_end_time);
				}
				if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($this->id, 'attributes')) {
					trigger_error('Could not acquire lock on asset '.$this->name.' to move it', E_USER_WARNING);
					return '';
				}
				$this->saveAttributes();
				$GLOBALS['SQ_SYSTEM']->am->releaseLock($this->id, 'attributes');
			break;

			default:
				trigger_error($action_name.' is an unknown action for this type of event', E_USER_WARNING);	
				return '';
			break;
		}

	}//end processEventAction();		


	/*
	* Cache the calendar data's stuff
	*
	* @param boolean	$updating	are we updating (TRUE) or inserting a new record
	* 								(FALSE - set only in create() call)
	*
	* @return boolean	indicating success
	* @access public
	*/
	function cacheCalendarData($updating=true)
	{
		$db = $GLOBALS['SQ_SYSTEM']->db;
	
		$date_values = Array();
		foreach(Array('start_date', 'end_date') as $date_comp) {
			$iso = $this->attr($date_comp);
						
			if (iso8601_time_component($iso) === false) {
				if (($date_comp == 'end_date') && (iso8601_date_component($iso) !== false)) {
					$iso = substr($iso,0,10).' 24:00:00';
				}
			} else {
				// we know the seconds aren't going to be there, so zero them so we can
				// use the getDate() niceness
				$iso = substr($iso,0,16).':00';
			}
			
			// we have the best, now get the rest
			$date_values += $this->getDateComponents($iso, $date_comp.'_');
		}
		
		// frequency field => blank for standard events, overridden in recurring
		$date_values['frequency'] = '';
		
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
	
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
				trigger_error('Unable to cache calendar data for '.ucwords(str_replace('_', ' ', $this->type())).' asset "'.$this->attr('name').'" (# '.$this->id.')', E_USER_WARNING);				
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}
		}
		
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;
			
	}//end cacheCalendarData()
	
	
	/**
	* Split a datetime attribute into revelant components
	*
	* @param string $iso			the date to split up in ISO format
	* @param string	$prefix			the prefix to add to array keys generated by this fn
	*
	* @static
	* @return array					the components
	* @access private
	*/
	function getDateComponents($iso, $prefix='')
	{			
		// we represent end-of-day as '24:00:00' as per ISO 8601, but strtotime()
		// doesn't recognise it. Fudge a little by flagging it, getting *start* of
		// day, then adding a day to the timestamp and setting hours to 24
		if ($iso_24h = (iso8601_time_component($iso) == '24:00:00')) {
			$iso = substr($iso,0,10).' 00:00:00';	// make it legit, fix it later
		}
		
		// we can build most of our info from the getdate() function
		if (($ts = strtotime($iso)) != -1) {
			$this_date = getdate($ts);
			$this_date['yday']++;	// to make it 1-based
			
			// unset word based dates
			unset($this_date['weekday']);
			unset($this_date['month']);
			
			// move timestamp field to a string-based key instead of [0]
			$this_date['ts'] = $this_date[0];
			unset($this_date[0]);
			
			// add 'day-count stamp'
			$this_date['ds'] = round(iso8601_ts(substr($iso,0,10)) / 86400);
			
			// 24:00:00 ISO time
			if ($iso_24h) {
				$this_date['ts'] += 86400;
				$this_date['hours'] = 24;
			}
			
		} else {
			// we have to do this manually..... great
			$this_date = Array();
			
			list($this_date['year'], $this_date['mon'], $this_date['mday'], $this_date['hours'], $this_date['minutes'], $this_date['seconds']) = sscanf($iso, '%4s-%2s-%2s %2s:%2s:%2s');						
		
			// any dashed out sections should be nulled out instead
			foreach(array_keys($this_date) as $iso_part) {
				if ($this_date[$iso_part] == str_repeat('-', ($iso_part == 'year' ? 4 : 2))) $this_date[$iso_part] = null;
			}
		
			$date_only_ts = strtotime(substr($iso,0,10));
							
			// forward day-of-year and weekday can only work with valid date
			if (is_null($this_date['mday']) || is_null($this_date['mon']) || is_null($this_date['year']) || ($date_only_ts == 1)) {
				$this_date['wday'] = null;
				$this_date['yday'] = null;
				$this_date['ts'] = null;
				$this_date['ds'] = null;
			} else {
				if (is_null($this_date['hours']) && (!is_null($this_date['minutes']) || !is_null($this_date['seconds']))) {
					// hours null in the middle of the time - no valid time
					$this_date['ts'] = null;
				} elseif (is_null($this_date['minutes']) && !is_null($this_date['seconds']))  {
					// minutes null in the middle of the time - no valid time
					$this_date['ts'] = null;
				} else {
					// make a best guess on timestamp, setting all blanks to 0
					$this_date['ts'] = strtotime(substr($iso,0,10).' '.str_replace('-','0',substr($iso,11)));
				}
				
				// we can do these in any case
				$this_date['ds'] = round(iso8601_ts(substr($iso,0,10)) / 86400);
				$this_date['wday'] = date('w', $date_only_ts);
				$this_date['yday'] = date('z', $date_only_ts) + 1;
			}
			
		}
		
		// add 'month-stamp' column which can be used to easily check
		// 'x monthly' stuff which can't be done easily with T/S alone
		if (is_null($this_date['mon']) || is_null($this_date['year'])) {
			$this_date['ms'] = null;
		} else {
			$this_date['ms'] = $this_date['year'] * 12 + $this_date['mon'];
		}
	
		if (is_null($this_date['mday']) || is_null($this_date['mon']) || is_null($this_date['year'])) {
			$this_date['ryday'] = null;
			$this_date['rmday'] = null;
			$this_date['wdom']  = null;
			$this_date['rwdom'] = null;
		} else {
			// reverse day-of-year and reverse day-of-month [1-based]
			$this_date['ryday'] = 365 + (is_leap_year($this_date['year']) ? 1 : 0) - $this_date['yday'] + 1;
			$this_date['rmday'] = days_in_month($this_date['mon'], $this_date['year']) - $this_date['mday'] + 1;
			
			// weekday of month, reverse thereof [both 1-based]
			$wdom = weekday_dates_in_month($this_date['wday'], $this_date['mon'], $this_date['year']);
			
			$this_date['wdom']  = array_search($this_date['mday'], $wdom) + 1;
			$this_date['rwdom']  = count($wdom) - $this_date['wdom'] + 1;
		}
		
		// now dump them into the array we were passed in the first place
		$res = Array();
		foreach(array_keys($this_date) as $iso_part) {
			$res[$prefix.$iso_part] = is_null($this_date[$iso_part]) ? null : intval($this_date[$iso_part]);
		}
		return $res;	
		
	}//end getDateComponents()
	
	
}//end class

?>