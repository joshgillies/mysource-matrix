<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: calendar_event.inc,v 1.57 2006/01/19 23:04:11 skim Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';
require_once SQ_FUDGE_PATH.'/datetime_field/datetime_field.inc';
require_once SQ_FUDGE_PATH.'/general/datetime.inc';
require_once dirname(__FILE__).'/../lib/calendar_common.inc';

/**
* Calendar_Event
*
* Purpose
*
*
*
* @author  Luke Wright <lwright@squiz.net>
* @author  Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.57 $
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

	}//end constructor


	/**
	* Perform any pre-validation required during the creation of this asset
	*
	* Calendar events ensure a name has been set before they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @access private
	* @return boolean
	*/
	function _preCreateCheck(&$link)
	{
		if (!parent::_preCreateCheck($link)) return false;

		$name = $this->attr('name');
		if ($name == '') {
			trigger_localised_error('CAL0008', E_USER_WARNING, $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name'));
			return false;
		}

		return true;

	}//end _preCreateCheck()


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* Folders set an initial web path when they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @access private
	* @return boolean
	*/
	function _createAdditional(&$link)
	{
		if (!parent::_createAdditional($link)) return false;
		$result = $this->cacheCalendarData(false);
		// set an initial web path
		$initial_path = strtolower($this->attr('name'));
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		$valid_paths = make_valid_web_paths(Array($initial_path));
		$good_paths = $GLOBALS['SQ_SYSTEM']->am->webPathsInUse($link['asset'], $valid_paths, $this->id, true);
		$result_paths = $this->saveWebPaths($good_paths);
		return ($result && $result_paths);

	}//end _createAdditional()


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
		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$sql = 'DELETE FROM
					sq_cal_date_val
				WHERE
					 assetid = '.$db->quoteSmart($this->id);

		$result = $db->query($sql);
		assert_valid_db_result($result);

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
		$keywords['event_name'] = $keywords['asset_attribute_name'];
		unset($keywords['asset_attribute_name']);

		$keywords['event_start_date'] = Array('value'=>date('jS M Y', iso8601_ts($this->attr('start_date'))));
		$keywords['event_end_date'] = Array('value'=>date('jS M Y', iso8601_ts($this->attr('end_date'))));

		$keywords['event_start_datetime'] = Array('value'=>$this->getFriendlyStart());
		$keywords['event_end_datetime'] = Array('value'=>$this->getFriendlyEnd());

		$keywords['event_start_time_12h'] = Array('value'=>date('h:i a', iso8601_ts($this->attr('start_date'))));
		$keywords['event_end_time_12h'] = Array('value'=>date('h:i a', iso8601_ts($this->attr('end_date'))));

		$keywords['event_start_time_24h'] = Array('value'=>date('H:i', iso8601_ts($this->attr('start_date'))));
		$keywords['event_end_time_24h'] = Array('value'=>date('H:i', iso8601_ts($this->attr('end_date'))));

		$keywords['event_start_time_ical'] = Array('value'=>date('Ymd\THis', iso8601_ts($this->attr('start_date'))));
		$keywords['event_end_time_ical'] = Array('value'=>date('Ymd\THis', iso8601_ts($this->attr('end_date'))));

		$keywords['event_datetime_summary'] = Array('value'=>$this->getFriendlyDatesAndTimes());

		// give useful info if no duration (single point in time) - easy_time_total returns empty string
		$total_time = easy_time_total($this->getDuration(), true);
		if (empty($total_time)) $total_time = 'N/A';

		$keywords['event_duration'] = Array('value'=>$total_time);
		$keywords['edit_link'] = Array('value'=>($this->writeAccess('') ? '<a href="'.replace_query_string_vars(Array('SQ_CALENDAR_VIEW'=>'edit', 'SQ_CALENDAR_EVENT_ID'=>$this->id)).'">'.translate('cal_event_edit_link_text').'</a>' : ''));
		$keywords['delete_link'] = Array('value'=>$this->_getDeleteLink());

		if ($descriptions) {
			$keywords['event_start_datetime']['description'] = 'Event start date/time';
			$keywords['event_end_datetime']['description'] = 'Event end date/time';

			$keywords['event_start_time_12h']['description'] = 'Start time (12 hour)';
			$keywords['event_end_time_12h']['description'] = 'End time (12 hour)';

			$keywords['event_start_time_24h']['description'] = 'Start time (24 hour)';
			$keywords['event_end_time_24h']['description'] = 'End time (24 hour)';

			$keywords['event_start_time_ical']['description'] = 'Start time (iCalendar)';
			$keywords['event_end_time_ical']['description'] = 'End time (iCalendar)';

			$keywords['event_datetime_summary']['description'] = 'Friendly description of event dates and times';
			$keywords['event_duration']['description'] = 'Duration of the Event';
			$keywords['edit_link']['description'] = 'Link to edit this event in Limbo (if user has permission)';
			$keywords['delete_link']['description'] = 'Link to delete this event altogether (if user has permission)';
		}
		return $keywords;

	}//end getAssetKeywords()


	/**
	* Get the link that users can click to delete this event
	*
	* @return string	The HTML link
	* @access private
	*/
	function _getDeleteLink()
	{
		if (true === ($msg = $GLOBALS['SQ_SYSTEM']->am->couldTrashAsset($this->id))) {
			$res = '<a href="?SQ_CALENDAR_EVENT_ID='.$this->id.'&amp;SQ_CALENDAR_EVENT_ACTION=delete';
			if (isset($_REQUEST['SQ_CALENDAR_DATE'])) {
				$res .= '&amp;SQ_CALENDAR_DATE='.$_REQUEST['SQ_CALENDAR_DATE'];
			}
			$res .= '" onclick="return confirm(js_translate(\'confirm_delete_event\'));">'.translate('delete').'</a>';
			return $res;
		} else {
			return '';
		}

	}//end _getDeleteLink()


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
			trigger_localised_error('CAL0009', E_USER_WARNING);
			return false;
		}

		// cannot have End Date being like '---------- 11:00:00'
		if (!$end_has_date && $end_has_time) {
			trigger_localised_error('CAL0010', E_USER_WARNING);
			return false;
		}

		// Cannot have an end time if you have no start time
		if ($end_has_time && (!$start_has_time)) {
			trigger_localised_error('CAL0011', E_USER_WARNING);
			return false;
		}

		// Cannot have an end date and start time if you have no end time
		if ($end_has_date && $start_has_time && (!$end_has_time)) {
			trigger_localised_error('CAL0012', E_USER_WARNING);
			return false;
		}

		// If an End Date is specified (and we need date component so we can use that to check),
		// the End Date must be no earlier than the start Date
		if ($end_has_date) {
			if (iso8601_ts($end_date) < iso8601_ts($start_date)) {
				trigger_localised_error('CAL0013', E_USER_WARNING, $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name'));
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
			trigger_localised_error('CAL0014', E_USER_WARNING);
			return false;
		}

		// For times, at least hour and minute must be specified. You can get away with not specifying
		// seconds, it's optional.
		if ($start_has_time && (($start_components['h'] == -1) || ($start_components['i'] == -1))) {
			trigger_localised_error('CAL0015', E_USER_WARNING);
			return false;
		}

		if ($end_has_time && (($end_components['h'] == -1) || ($end_components['i'] == -1))) {
			trigger_localised_error('CAL0016', E_USER_WARNING);
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
		$res = substr($this->attr('start_date'), 11, 5);
		return ($res == '--:--') ? false : $res;

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
		$res = substr($this->attr('end_date'), 11, 5);
		return ($res == '--:--') ? false : $res;

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
		} else if (iso8601_ts($this->attr('end_date')) == -1) {
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
				return $this->getFriendlyStart().' (all day)';
			} else if (($this->getStartTime() == $this->getEndTime())) {
				// show start date and time only
				return $this->getFriendlyStart();
			} else {
				// show date and start/end times
				return $this->getFriendlyStart().'-'.date('g:ia', iso8601_ts($this->attr('end_date')));
			}
		} else if (iso8601_ts($this->attr('end_date')) == -1) {
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
				if (isset($_REQUEST['SQ_CALENDAR_OLD_LOC']) && !empty($_REQUEST['SQ_CALENDAR_OLD_LOC'])) {
					$old_link = &$GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($_REQUEST['SQ_CALENDAR_OLD_LOC'], $this->id);
					if (empty($old_link)) {
						trigger_localised_error('CAL0017', E_USER_WARNING, $_REQUEST['SQ_CALENDAR_OLD_LOC'], $this->id);
						return '';
					}
					if (!$GLOBALS['SQ_SYSTEM']->am->moveLink($old_link['linkid'], $_REQUEST['SQ_CALENDAR_NEW_LOC'], $old_link['link_type'], -1)) {
						trigger_localised_error('CAL0018', E_USER_WARNING, $_REQUEST['SQ_CALENDAR_OLD_LOC'], $_REQUEST['SQ_CALENDAR_NEW_LOC']);
						return '';
					}
				}
				if (!isset($_REQUEST['SQ_CALENDAR_DATE'])) {
					trigger_localised_error('CAL0019', E_USER_WARNING);
					return '';
				}
				$duration = $this->getDuration();
				if (!isset($_REQUEST['SQ_CALENDAR_TIME']) || empty($_REQUEST['SQ_CALENDAR_TIME'])) { $_REQUEST['SQ_CALENDAR_TIME'] = '--:--';}
				$this->setAttrValue('start_date', $_REQUEST['SQ_CALENDAR_DATE'].' '.$_REQUEST['SQ_CALENDAR_TIME'].':--');
				$current_end_date = $this->getEndDate();
				if ($current_end_date) {
					$new_end_date_ts = iso8601_ts($this->attr('start_date')) + $duration;
					$new_end_date = date('Y-m-d', $new_end_date_ts);
					$new_end_time = $this->getEndTime() ? date('H:i:00', $new_end_date_ts) : '--:--:--';
					$this->setAttrValue('end_date', $new_end_date.' '.$new_end_time);
				}
				if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($this->id, 'attributes')) {
					trigger_localised_error('CAL0020', E_USER_WARNING, $this->name, $this->id);
					return '';
				}
				$this->saveAttributes();
				$GLOBALS['SQ_SYSTEM']->am->releaseLock($this->id, 'attributes');
			break;

			case 'delete':
				$GLOBALS['SQ_SYSTEM']->am->trashAsset($this->id);
			break;

			default:
				trigger_localised_error('CAL0021', E_USER_WARNING, $action_name);
				return '';
			break;
		}

	}//end processEventAction()


	/**
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
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$date_values = Array();
		foreach (Array('start_date', 'end_date') as $date_comp) {
			$iso = $this->attr($date_comp);

			if (iso8601_time_component($iso) === false) {
				if (($date_comp == 'end_date') && (iso8601_date_component($iso) !== false)) {
					$iso = substr($iso, 0, 10).' 24:00:00';
				}
			} else {
				// we know the seconds aren't going to be there, so zero them so we can
				// use the getDate() niceness
				$iso = substr($iso, 0, 16).':00';
			}

			// we have the best, now get the rest
			$date_values += get_date_components($iso, $date_comp.'_');
		}

		// frequency field => blank for standard events, overridden in recurring
		$date_values['frequency'] = '';

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if (!$updating) {
			// set the asset ids
			$date_values['assetid'] = $this->id;

			for (reset($date_values); null !== ($key = key($date_values)); next($date_values)) {
				$date_values[$key] = $db->quoteSmart($date_values[$key]);
			}
			$val_string = implode(',', $date_values);

			$sql = 'INSERT INTO
						sq_cal_date_val
						(
							'.implode(',', array_keys($date_values)).'
						)
						VALUES
						(
							'.$val_string.'
						)';

			$result = $db->query($sql);
			assert_valid_db_result($result);

		} else {
			// asset id becomes a where condition
			$sql = 'UPDATE
						sq_cal_date_val
					SET ';

			$set_array = Array();
			foreach ($date_values as $key => $value) {
				$set_array[] = $key.' = '.((is_null($value)) ? 'null' : $db->quoteSmart($value));
			}
			$sql .= implode(',', $set_array);
			$sql .=' WHERE
						assetid = '.$db->quoteSmart($this->id);

			$result = $db->query($sql);
			assert_valid_db_result($result);
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		return true;

	}//end cacheCalendarData()


	/**
	* Get bodycopy keyword replacement
	*
	* Provides a mechanism for providing keyword replacement functions by
	* looking up the function named 'getKeywordNameKeywordReplacement'
	* so an asset wishing to add support for 'best_ever' custom keyword
	* should implement a function getBestEverKeywordReplacement()
	* which returns a replacement string
	*
	* @param string	$keyword	Keyword to find a replacement for
	*
	* @return string
	* @access public
	*/
	function getKeywordReplacement($keyword)
	{
		switch ($keyword) {
			case 'event_name':
				return $this->name;
			case 'event_start_date':
				return date('jS M Y', iso8601_ts($this->attr('start_date')));
			case 'event_end_date':
				// In the case of 'end_date' of the first instance is not set
				if (strcmp($this->attr('end_date'), '---------- --:--:--') == 0 ) {
					return '';
				}
				return date('jS M Y', iso8601_ts($this->attr('end_date')));
			case 'event_start_datetime':
				return $this->getFriendlyStart();
			case 'event_end_datetime':
				return $this->getFriendlyEnd();
			case 'event_start_time_12h':
				return date('h:i a', iso8601_ts($this->attr('start_date')));
			case 'event_end_time_12h':
				return date('h:i a', iso8601_ts($this->attr('end_date')));
			case 'event_start_time_24h':
				return date('H:i', iso8601_ts($this->attr('start_date')));
			case 'event_end_time_24h':
				return date('H:i', iso8601_ts($this->attr('end_date')));
			case 'event_start_time_ical':
				return date('Ymd\THis', iso8601_ts($this->attr('start_date')));
			case 'event_end_time_ical':
				return date('Ymd\THis', iso8601_ts($this->attr('end_date')));
			case 'event_datetime_summary':
				return $this->getFriendlyDatesAndTimes();
			case 'event_duration':
				// give useful info if no duration (single point in time) - easy_time_total returns empty string
				$total_time = easy_time_total($this->getDuration(), true);
				if (empty($total_time)) $total_time = 'N/A';
				return $total_time;
			case 'edit_link':
				return $this->writeAccess('') ? '<a href="'.replace_query_string_vars(Array('SQ_CALENDAR_VIEW'=>'edit', 'SQ_CALENDAR_EVENT_ID'=>$this->id)).'">'.translate('cal_event_edit_link_text').'</a>' : '';
			case 'delete_link':
				return $this->_getDeleteLink();
			default:
				return parent::getKeywordReplacement($keyword);
		}

	}//end getKeywordReplacement()

}//end class

?>
