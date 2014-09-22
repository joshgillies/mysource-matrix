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
* $Id: calendar_event.inc,v 1.117 2013/10/14 04:30:57 cupreti Exp $
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
* @version $Revision: 1.117 $
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
	* @return boolean
	* @access private
	*/
	function _preCreateCheck(&$link)
	{
		if (!parent::_preCreateCheck($link)) return FALSE;

		$name = $this->attr('name');
		if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_INTEGRITY) && ($name == '')) {
			trigger_localised_error('CAL0008', sprintf(translate('Unable to create a %s with a blank name'), $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name')), E_USER_WARNING)
			return FALSE;
		}

		return TRUE;

	}//end _preCreateCheck()


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* Folders set an initial web path when they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	function _createAdditional(&$link)
	{
		if (!parent::_createAdditional($link)) return FALSE;
		$result = $this->cacheCalendarData(FALSE);
		// set an initial web path
		$initial_path = strtolower($this->attr('name'));
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		$valid_paths = make_valid_web_paths(Array($initial_path));
		$good_paths = $GLOBALS['SQ_SYSTEM']->am->webPathsInUse($link['asset'], $valid_paths, $this->id, TRUE);
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
	function delete($release_lock=TRUE)
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$db = MatrixDAL::getDb();
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		try {
			$bind_vars	= Array (
							'assetid'	=> $this->id,
						  );
			$result = MatrixDAL::executeQuery('calendar_event', 'deleteCalendarEvent', $bind_vars);
		} catch (Exception $e) {
			throw new Exception('Unable to delete the eventid #:'.$this->id.' from the trash due to the following database error:'.$e->getMessage());
		}//end try catch

		if (!parent::delete($release_lock)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return TRUE;

	}//end delete()


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	* @param int		$contextid	what context to return the name from
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	protected function _getName($short_name=FALSE, $contextid=NULL)
	{
		// No context specified, using the current context
		if ($contextid === NULL) {
			$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();
		}//end if

		// Obtain the attribute value for Name from the specified Context
		$values = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName('name', $this->type(), Array($this->id), $contextid);
		if (empty($values) === TRUE) {
			return parent::_getName($short_name, $contextid);
		} else {
			return $values[$this->id];
		}

	}//end _getName()


	/**
	* Return the keywords for this asset type.
	*
	* This function is called in asset-listing situations like the asset listing, search page etc
	* The return value is in the form:
	* <PRE>
	* Array(
	*   'name' => 'description' => 'the asset name',
	* );
	* </PRE>
	*
	* @return array
	* @access public
	*/
	function getAvailableKeywords()
	{
		$keywords = parent::getAvailableKeywords();

		$keywords['asset_attribute_start_date'] = translate('Event Start (ISO8601 Format)');

		$keywords['asset_attribute_end_date']   = translate('Event End (ISO8601 Format)'); // = 'End date (iso8601)';

		$keywords['event_start_date']           = translate('Event Start (readable date)'); // = 'Event Start Date';

		$keywords['event_end_date']             = translate('Event End (readable date)'); // = 'Event End Date';

		$keywords['event_start_datetime']       = translate('Event Start (Readable date/time)'); // = 'Event start date/time';

		$keywords['event_end_datetime']         = translate('Event End (Readable date/time'); // = 'Event end date/time';

		$keywords['event_start_time_12h']       = translate('Event Start (12 hour time)'); // = 'Start time (12 hour)';

		$keywords['event_end_time_12h']         = translate('Event End (12 hour time)'); // = 'End time (12 hour)';

		$keywords['event_start_time_24h']       = translate('Event Start (24 hour time)'); // = 'Start time (24 hour)';

		$keywords['event_end_time_24h']         = translate('Event End (24 hour time)'); // = 'End time (24 hour)';

		$keywords['event_short_start_time']     = translate('Short start time (12 hour time, without minutes if on the hour)'); // = 'Short start time (12 hour)';

		$keywords['event_short_end_time']       = translate('Short end time (12 hour time, without minutes if on the hour)'); // = 'Short end time (12 hour)';

		$keywords['event_start_time_ical']      = translate('Event Start (iCalendar)'); // = 'Start time (iCalendar)';

		$keywords['event_end_time_ical']        = translate('Event End (iCalendar)'); // = 'End time (iCalendar)';

		$keywords['event_datetime_summary']     = translate('Event start/end summary'); // = 'Friendly description of event dates and times';

		$keywords['event_duration']             = translate('Event Duration'); // = 'Duration of the Event';

		$keywords['edit_link']                  = translate('Link to edit this event in Limbo (if user has permission)'); // = 'Link to edit this event in Limbo (if user has permission)';

		$keywords['delete_link']                = translate('Link to delete this event altogether (if user has permission)'); // = 'Link to delete this event altogether (if user has permission)';

		$keywords['event_start_datetime_']      = translate('Event Start expressed with PHP date format which follow the last underscore character of the keyword.'); // = dynamic keyword to print start date and time

		$keywords['event_end_datetime_']        = translate('Event End expressed with PHP date format which follow the last underscore character of the keyword.'); // = dynamic keyword to print end date and time

		$keywords['event_calendar_date_']       = translate('Event Calendar Date with PHP date format after the last underscore character.'); // = dynamic keyword to print calendar date


		return $keywords;

	}//end getAvailableKeywords()


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	* In the form
	*
	*   Array('[link_type]' => Array('[type_code]' => Array('card' => [cardinality], 'exclusive' => [exclusive])));
	* Where:
	*   link_type   = SQ_LINK_TYPE_1|SQ_LINK_TYPE_2|SQ_LINK_TYPE_3|SQ_LINK_NOTICE
	*   cardinality = [max number of links]|'M'
	*   exclusive   = TRUE|FALSE
	*
	* @return array
	* @access private
	*/
	function _getAllowedLinks()
	{
		return Array(
				SQ_LINK_TYPE_1	=> Array(
									'file'	=> Array('card' => 'M', 'exclusive' => FALSE),
									'page'	=> Array('card' => 'M', 'exclusive' => FALSE),
								   ),
				SQ_LINK_TYPE_2	=> Array(
									'file'	=> Array('card' => 'M', 'exclusive' => FALSE),
									'page'	=> Array('card' => 'M', 'exclusive' => FALSE),
								   ),
				SQ_LINK_TYPE_3	=> Array(),
				SQ_LINK_NOTICE	=> Array(
									'image'				=> Array('card' =>   1, 'exclusive' => FALSE),
									'design'			=> Array('card' => 'M', 'exclusive' => FALSE),
									'paint_layout_page'	=> Array('card' => 'M', 'exclusive' => FALSE),
								   ),
			   );

	}//end _getAllowedLinks()


	/**
	* Get the HTML link that users can click to delete this event
	*
	* @return string
	* @access private
	*/
	function _getDeleteLink()
	{
		if (TRUE === ($msg = $GLOBALS['SQ_SYSTEM']->am->couldTrashAsset($this->id))) {
			$res = '<a href="?SQ_CALENDAR_EVENT_ID='.$this->id.'&amp;SQ_CALENDAR_EVENT_ACTION=delete';
			if (isset($_REQUEST['SQ_CALENDAR_DATE'])) {
				$res .= '&amp;SQ_CALENDAR_DATE='.$_REQUEST['SQ_CALENDAR_DATE'];
			}
			$res .= '" onclick="return confirm(js_translate(\'confirm_delete_event\'));">'.translate('Delete').'</a>';

			return $res;
		} else {
			return '';
		}

	}//end _getDeleteLink()


	/**
	* Save attributes set with setAttrValue to the database
	*
	* @param boolean	$dont_run_updated	if TRUE this->_updated() won't be run
	*										(THIS IS ONLY EVER CALLED FROM Asset::create())
	*
	* @return boolean
	* @access public
	* @see Asset::setAttrValue()
	*/
	function saveAttributes($dont_run_updated=FALSE)
	{
		$start_date = $this->attr('start_date');
		$end_date   = $this->attr('end_date');

		// work out if we have any 'null date' or 'null time' sections
		$start_has_time = $this->getStartTime();
		$end_has_time   = $this->getEndTime();
		$start_has_date = $this->getStartDate();
		$end_has_date   = $this->getEndDate();

		if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_INTEGRITY)) {

			// All events must have a starting date at least!
			if (!$start_has_date) {
				trigger_localised_error('CAL0009', translate('Calendar Events must at least have a start date'), E_USER_WARNING)
				return FALSE;
			}

			// cannot have End Date being like '---------- 11:00:00'
			if (!$end_has_date && $end_has_time) {
				trigger_localised_error('CAL0010', translate('If the \'End Date\' includes a time component it must also include a date'), E_USER_WARNING)
				return FALSE;
			}

			// Cannot have an end time if you have no start time
			if ($end_has_time && (!$start_has_time)) {
				trigger_localised_error('CAL0011', translate('If the \'End Time\' is specified you must also specify a \'Start Time\''), E_USER_WARNING)
				return FALSE;
			}

			// Cannot have an end date and start time if you have no end time
			if ($end_has_date && $start_has_time && (!$end_has_time)) {
				trigger_localised_error('CAL0012', translate('If the \'End Date\' and \'Start Time\' are specified you must also specify an \'End Time\''), E_USER_WARNING)
				return FALSE;
			}

			// If an End Date is specified (and we need date component so we can use that to check),
			// the End Date must be no earlier than the start Date
			if ($end_has_date) {
				if (iso8601_ts($end_date) < iso8601_ts($start_date)) {
					trigger_localised_error('CAL0013', sprintf(translate('The \'End Date\' of this %s cannot be prior to the \'Start Date\''), $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name')), E_USER_WARNING)
					return FALSE;
				}
			}

		}//end if

		// oh no, we're not using this to display anything... just to use its units extraction :-)
		$params = Array();
		$value = '';
		$datetime = new Datetime_Field('', $value, $params);

		// check individual sections to make sure we can't commit part dates like '16th, 2005 48'
		$start_components = $datetime->strToUnitsArray($start_date);
		$end_components = $datetime->strToUnitsArray($end_date);

		// don't need to check Start Date's date component as it is required!
		// must check other parts of both Start and End Date so a meaningful date is stored

		// For dates, this means all three parts of the date have a value
		if ($end_has_date && (($end_components['d'] == -1) || ($end_components['m'] == -1) || ($end_components['y'] == -1))) {
			trigger_localised_error('CAL0014', translate('If the date component of \'End Date\' is specified it must specify all of day, month and year'), E_USER_WARNING)
			return FALSE;
		}

		// For times, at least hour and minute must be specified. You can get away with not specifying
		// seconds, it's optional.
		if ($start_has_time && (($start_components['h'] == -1) || ($start_components['i'] == -1))) {
			trigger_localised_error('CAL0015', translate('If the time component of \'Start Date\' is specified it must specify at least hour and minute'), E_USER_WARNING)
			return FALSE;
		}

		if ($end_has_time && (($end_components['h'] == -1) || ($end_components['i'] == -1))) {
			trigger_localised_error('CAL0016', translate('If the time component of \'End Date\' is specified it must specify at least hour and minute'), E_USER_WARNING)
			return FALSE;
		}

		// cache all the calendar data into another table so the queries for the calendar
		// page go super-mega-hyperturbo (but only if already created)
		if (!empty($this->id) && !$dont_run_updated) {
			if (!$this->cacheCalendarData()) return FALSE;
		}
		$treeid = Array();

		// bug fix #5364 Recurring Calendar Event: Time Changes don't get sent to child Event Modifications
		// we need to make ssure to cascade the changes to and children of the recurring event,if thats the
		// asset type we are updating.
		// also make sure "THIS" asset does have the valid treeid before we try to get its children
		if ($this->id) $treeid = $GLOBALS['SQ_SYSTEM']->am->getAssetTreeids($this->id);
 		if (($this->id) && $this->type() == 'calendar_event_recurring' && $GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_INTEGRITY) && !empty($treeid)) {
			$events_to_update = $GLOBALS['SQ_SYSTEM']->am->getChildren($this->id, 'calendar_event_modification');
			// because we do not have locks on the child modofication assets
			// we need to set the run level so that we can update these assets
			$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
			foreach ($events_to_update as $event_id => $info) {
				$modification_event = $GLOBALS['SQ_SYSTEM']->am->getAsset($event_id);
				if (!$modification_event->saveAttributes()) {
					$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
					return FALSE;
				}
			}
			$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
		}

		// all sweet, so it seems
		return parent::saveAttributes($dont_run_updated);

	}//end saveAttributes()


	/**
	* Takes a clone shell and clones the sources required components to the clone shell
	*
	* @param object		&$clone		the clone shell
	* @param array		$components	the wanted components to clone eg.
	* <pre>
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
	* </pre>
	* @param boolean	$override	whether or not to override the existing permission, metadata schemas, workflow schemas with the new ones.
	*
	* @return boolean
	* @access public
	*/
	function cloneComponents(&$clone, $components, $override=FALSE)
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if (!parent::cloneComponents($clone, $components, $override)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		$GLOBALS['SQ_CLONE_COMPONENTS'] = TRUE;

		if (in_array('all', $components) || in_array('attributes', $components)) {
			if (!$clone->cacheCalendarData(FALSE)) {
				$GLOBALS['SQ_CLONE_COMPONENTS'] = FALSE;
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}
		}

		$GLOBALS['SQ_CLONE_COMPONENTS'] = FALSE;

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return TRUE;

	}//end cloneComponents()


	/**
	* Get the date component of the Start Date attribute.
	*
	* It returns the date component of the start date (ISO8601 fmt,
	* yyyy-mm-dd) or boolean FALSE if the date component is empty
	*
	* @return mixed string|boolean
	* @access public
	*/
	function getStartDate()
	{
		$res = substr($this->attr('start_date'), 0, 10);
		return ($res == '----------') ? FALSE : $res;

	}//end getStartDate()


	/**
	* Get the date component of the End Date attribute
	*
	* It returns the date component of the end date (ISO8601 fmt,
	* yyyy-mm-dd) or boolean FALSE if the date component is empty
	*
	* @return mixed string|boolean
	* @access public
	*/
	function getEndDate()
	{
		$res = substr($this->attr('end_date'), 0, 10);
		return ($res == '----------') ? FALSE : $res;

	}//end getEndDate()


	/**
	* Get the time component of the Start Date attribute
	*
	* It returns the time component of the start date (hh:mm:ss)
	* or boolean FALSE if the time component is empty (--:--:--)
	*
	* @return mixed string|boolean
	* @access public
	*/
	function getStartTime()
	{
		$res = substr($this->attr('start_date'), 11, 5);
		return ($res == '--:--') ? FALSE : $res;

	}//end getStartTime()


	/**
	* Get the time component of the End Date attribute
	*
	* It returns the time component of the end date (hh:mm:ss)
	* or boolean FALSE if the time component is empty (--:--:--)
	*
	* @return mixed string|boolean
	* @access public
	*/
	function getEndTime()
	{
		$res = substr($this->attr('end_date'), 11, 5);
		return ($res == '--:--') ? FALSE : $res;

	}//end getEndTime()


	/**
	* Returns the duration between the start and end dates, if they are set
	*
	* If end date/time is NULL => return 0 (single point in time)
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

		if (!$start_has_date) return NULL;

		if (!$end_has_date) {
			// single point in time, and so it has no duration
			return 0;
		}

		// NOTE: no need to replace null time fields with 'midnight' (00:00:00) because the
		// ISO-to-timestamp conversion function already recognises that it's not a full datetime
		// if that's the case

		if (!$start_has_time && !$end_has_time) {
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
	* It returns the start date and time if appropriate, in friendly format
	*
	* @return string
	* @access public
	*/
	function getFriendlyStart()
	{
		if ($this->getStartTime() !== FALSE) {
			return readable_datetime(iso8601_ts($this->attr('start_date')));
		} else {
			return date('jS M Y', iso8601_ts($this->attr('start_date')));
		}

	}//end getFriendlyStart()


	/**
	* Get the end date and time in friendly format
	*
	* It returns the end date and time if appropriate, in friendly format
	*
	* @return string
	* @access public
	*/
	function getFriendlyEnd()
	{
		if ($this->getEndTime() !== FALSE) {
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
	* Example returns (assuming date and time components specified):
	* <pre>
	* if (start date != end date)		"23 Jul 2004 4:00pm - 25 Jul 2004 10:00am"
	* if (start date == end date)		"23 Jul 2004 4:00pm-5:00pm"
	* if (end date is empty)			"23 Jul 2004 4:00pm"
	* </pre>
	* <pre>
	* If only date components are specified:
	* if (start date != end date)		"23 Jul 2004 - 25 Jul 2004"
	* if (start date == end date)		"23 Jul 2004 (all day)"
	* if (end date is empty)			"23 Jul 2004"
	* </pre>
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
						$total_time = easy_time_total($this->getDuration(), TRUE);
						echo (empty($total_time) ? 'N/A' : $total_time);
					?></td>
			</tr>
			<tr>
				<td><b>Description</b></td>
				<td><?php echo $this->getKeywordReplacement('asset_attribute_description'); ?></td>
			</tr>
		</table>
		<?php

	}//end printBody()


	/**
	* Process the specified action on this event, and return whatever html the action wants to show
	*
	* This should be overridden by child classes. It returns
	* HTML to show as the result of this action
	*
	* @param string	$action_name	The action to perform, eg 'cancel'
	*
	* @return string
	* @access public
	*/
	function processEventAction($action_name)
	{
		switch ($action_name) {
			case 'move':
				if (isset($_REQUEST['SQ_CALENDAR_OLD_LOC']) && !empty($_REQUEST['SQ_CALENDAR_OLD_LOC'])) {
					$old_link = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($_REQUEST['SQ_CALENDAR_OLD_LOC'], $this->id);
					if (empty($old_link)) {
						trigger_localised_error('CAL0017', sprintf(translate('Cannot move event, cannot find old link from asset #%1$s to asset #%2$s'), $_REQUEST['SQ_CALENDAR_OLD_LOC'], $this->id), E_USER_WARNING)
						return '';
					}
					if (!$GLOBALS['SQ_SYSTEM']->am->moveLink($old_link['linkid'], $_REQUEST['SQ_CALENDAR_NEW_LOC'], $old_link['link_type'], -1)) {
						trigger_localised_error('CAL0018', sprintf(translate('Cannot move event, unable to move link from under asset #%1$s to under asset #%2$s'), $_REQUEST['SQ_CALENDAR_OLD_LOC'], $_REQUEST['SQ_CALENDAR_NEW_LOC']), E_USER_WARNING)
						return '';
					}
				}
				if (!isset($_REQUEST['SQ_CALENDAR_NEW_DATE'])) {
					trigger_localised_error('CAL0019', translate('Cannot move event, new date not specified'), E_USER_WARNING)
					return '';
				}
				$duration = $this->getDuration();
				if (!isset($_REQUEST['SQ_CALENDAR_NEW_TIME']) || empty($_REQUEST['SQ_CALENDAR_NEW_TIME'])) {
					$_REQUEST['SQ_CALENDAR_NEW_TIME'] = '--:--';
				}
				$this->setAttrValue('start_date', $_REQUEST['SQ_CALENDAR_NEW_DATE'].' '.$_REQUEST['SQ_CALENDAR_NEW_TIME'].':--');
				$current_end_date = $this->getEndDate();
				if ($current_end_date) {
					$new_end_date_ts = iso8601_ts($this->attr('start_date')) + $duration;
					$new_end_date = date('Y-m-d', $new_end_date_ts);
					$new_end_time = $this->getEndTime() ? date('H:i:00', $new_end_date_ts) : '--:--:--';
					$this->setAttrValue('end_date', $new_end_date.' '.$new_end_time);
				}
				if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($this->id, 'attributes')) {
					trigger_localised_error('CAL0020', sprintf(translate('Could not acquire lock on asset "%1$s" (#%2$s) to move it'), $this->name, $this->id), E_USER_WARNING)
					return '';
				}
				$this->saveAttributes();
				$GLOBALS['SQ_SYSTEM']->am->releaseLock($this->id, 'attributes');
			break;

			case 'delete':
				$GLOBALS['SQ_SYSTEM']->am->trashAsset($this->id);
				$this->updateLookups();
			break;

			default:
				trigger_localised_error('CAL0021', sprintf(translate('%s is an unknown action for this type of event'), $action_name), E_USER_WARNING)
				return '';
			break;
		}//end switch

	}//end processEventAction()


	/**
	* Cache the calendar data's stuff
	*
	* The return value indicates the success
	*
	* @param boolean	$updating	are we updating (TRUE) or inserting a new record
	*								(FALSE - set only in create() call)
	*
	* @return boolean
	* @access public
	*/
	function cacheCalendarData($updating=TRUE)
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$db = MatrixDal::getDb();

		$date_values = Array();

		foreach (Array('start_date', 'end_date') as $date_comp) {
			$iso = $this->attr($date_comp);

			if (iso8601_time_component($iso) === FALSE) {
				if (($date_comp == 'end_date') && (iso8601_date_component($iso) !== FALSE)) {
					$iso = substr($iso, 0, 10).' 24:00:00';
				}
			} else {
				// we know the seconds aren't going to be there, so zero them so we can
				// use the getDate() niceness
				$iso = substr($iso, 0, 16).':00';
			}

			// we have the best, now get the rest
			$date_values += Calendar_Common::getDateComponents($iso, $date_comp.'_');
		}

		// frequency field => blank for standard events, overridden in recurring
		$date_values['frequency'] = '';

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if (!$updating) {
			// set the asset ids
			$date_values['assetid'] = $this->id;

			for (reset($date_values); NULL !== ($key = key($date_values)); next($date_values)) {
				if (is_null($date_values[$key]))
					$date_values[$key] = 'NULL';
				else
				$date_values[$key] = MatrixDAL::quote($date_values[$key]);
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

			try {
				$query = MatrixDAL::preparePdoQuery($sql);
				MatrixDAL::execPdoQuery($query);
			} catch (Exception $e) {
				throw new Exception($e->getMessage());
			}
		} else {
			// asset id becomes a where condition
			$sql = 'UPDATE
						sq_cal_date_val
					SET ';

			$set_array = Array();
			foreach ($date_values as $key => $value) {
				$set_array[] = $key.' = '.((is_null($value)) ? 'null' : MatrixDAL::quote($value));
			}
			$sql .= implode(',', $set_array);
			$sql .=' WHERE
						assetid = :assetid';

			try {
				$query = MatrixDAL::preparePdoQuery($sql);
				MatrixDAL::bindValueToPdo($query, 'assetid', $this->id);
				MatrixDAL::execPdoQuery($query);
			} catch (Exception $e) {
				throw new Exception($e->getMessage());
			}
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return TRUE;

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
		$end_date_not_set = ($this->getEndDate() === FALSE);
		$start_time_not_set = ($this->getStartTime() === FALSE);
		$end_time_not_set = ($this->getEndTime() === FALSE);

		$full_keyword = $keyword;
		$keyword = parse_keyword($keyword, $modifiers);

		switch ($keyword) {
			case 'event_name':
				$replacement = $this->name;
				break;

			case 'event_start_date':
				$replacement = date('jS M Y', iso8601_ts($this->attr('start_date')));
				break;

			case 'event_start_time_12h':
				$replacement = $start_time_not_set ? '' : date('h:i a', iso8601_ts($this->attr('start_date')));
				break;

			case 'event_start_time_24h':
				$replacement = $start_time_not_set ? '' : date('H:i', iso8601_ts($this->attr('start_date')));
				break;

			case 'event_start_time_ical':
				// If theres no time bit, then only use the date portion to get the correct iCal timestamp
				if (preg_match("/^(.*?) (--):(--):(--)$/", $this->attr('start_date'))) {
					$replacement = date('Ymd', iso8601_ts($this->attr('start_date')));
				} else {
					$replacement = date('Ymd\THis', iso8601_ts($this->attr('start_date')));
				}
				break;

			case 'event_end_date':
				// In the case of 'end_date' of the first instance is not set
				$replacement = $end_date_not_set ? '' : date('jS M Y', iso8601_ts($this->attr('end_date')));
				break;

			case 'event_end_time_12h':
				$replacement = $end_time_not_set ? '' : date('h:i a', iso8601_ts($this->attr('end_date')));
				break;

			case 'event_end_time_24h':
				$replacement = $end_time_not_set ? '' : date('H:i', iso8601_ts($this->attr('end_date')));
				break;

			case 'event_short_start_time':
				if ($start_time_not_set) {
					$replacement = '';
				} else {
					$mins = date('i', iso8601_ts($this->attr('start_date')));
					if ($mins == '00') {
						$replacement = date('ga', iso8601_ts($this->attr('start_date')));
					} else {
						$replacement = date('g:ia', iso8601_ts($this->attr('start_date')));
					}
				}
				break;

			case 'event_short_end_time':
				if ($start_time_not_set) {
					$replacement = '';
				} else {
					$mins = date('i', iso8601_ts($this->attr('end_date')));
					if ($mins == '00') {
						$replacement = date('ga', iso8601_ts($this->attr('end_date')));
					} else {
						$replacement = date('g:ia', iso8601_ts($this->attr('end_date')));
					}
				}
				break;

			case 'event_end_time_ical':
				if ($end_date_not_set) {
					$replacement = '';
				} else {
					// If theres no end time for given end date then accourding to iCal
					// standards this means end date is next day without time bit but
					// this is confusing and misleading for the users so as a fix to
					// issue reported on #5359 iCal date keyword prints incorrected if
					// end date has no time, we will let the end date be according to
					// hCalendar standards. More information on bug report's comments
					$no_endtime_pattern = "/^(....)-(..)-([0-9]{2}) (--:--:--)$/";
					$replacement = "$1-$2-$3";
					if (preg_match($no_endtime_pattern, $this->attr('end_date'), $matches)) {
						$end_date = preg_replace($no_endtime_pattern, $replacement, $this->attr('end_date'));
					} else {
						$end_date = $this->attr('end_date');
					}

					// Don't worry about possibility of invalid date resulting from incrementing
					// the day bit above; php's date() will handle it correctly
					$replacement = preg_match("/..:..:..$/",$end_date) ? date('Ymd\THis', iso8601_ts($end_date)) : date('Ymd', iso8601_ts($end_date));
				}
				break;

			case 'event_start_datetime':
				$replacement = $this->getFriendlyStart();
				break;

			case 'event_end_datetime':
				$replacement = $this->getFriendlyEnd();
				break;

			case 'event_datetime_summary':
				$replacement = $this->getFriendlyDatesAndTimes();
				break;

			case 'event_duration':
				// give useful info if no duration (single point in time) - easy_time_total returns empty string
				$total_time = easy_time_total($this->getDuration(), TRUE);
				if (empty($total_time)) $total_time = 'N/A';
				$replacement = $total_time;
				break;

			case 'edit_link':
				$replacement = $this->writeAccess('') ? '<a href="'.htmlentities(replace_query_string_vars(Array('SQ_CALENDAR_VIEW'=>'edit', 'SQ_CALENDAR_EVENT_ID'=>$this->id))).'">'.translate('Edit').'</a>' : '';

				break;

			case 'delete_link':
				$replacement = $this->_getDeleteLink();
				break;

			case 'instance_start_datetime':

				$start_ts = isset($this->vars['start_date_ts']) ? $this->attr('start_date_ts') : '';
				if (!empty($start_ts)) {
					$replacement = date('Y-m-d H:i', $start_ts);
				}
				break;

			case 'instance_end_datetime':
				$end_ts = isset($this->vars['end_date_ts']) ? $this->attr('end_date_ts') : '';
				if (!empty($end_ts)) {
					$replacement = empty($end_ts) ? '' : date('Y-m-d H:i', $end_ts);
				}
				break;

			default:
				// if the keyword at this point starts with event_start_datetime_ or event_end_datetime_
				// it means this is a dynamic keyword to display a date
				preg_match('/^event_start_datetime_(.+)/', $keyword, $matches);
				if (!empty($matches)) {
					$replacement = date($matches[1], iso8601_ts($this->attr('start_date')));
				}
				preg_match('/^event_end_datetime_(.+)/', $keyword, $matches);
				if (!empty($matches)) {
					$replacement = $end_date_not_set ? '' : date($matches[1], iso8601_ts($this->attr('end_date')));
				}

				preg_match('/^event_calendar_date_(.+)/', $keyword, $matches);
				if (!empty($matches) && !empty($_REQUEST['SQ_CALENDAR_DATE'])) {
					$replacement = date($matches[1], strtotime($_REQUEST['SQ_CALENDAR_DATE']));
				}

				// Event instance keywords
				if (isset($this->vars['start_date_ts'])) {
					$start_ts = $this->attr('start_date_ts');
					preg_match('/^instance_start_(.+)/', $keyword, $matches);
					if (!empty($matches)) {
						if ($matches[1] == 'datetime_ical') {
							$replacement = empty($start_ts) ? '' : date('Ymd\THis', $start_ts);
						} else if (strpos($matches[1], 'datetime_') !== FALSE) {
							$replacement = empty($start_ts) ? '' : date(str_replace('datetime_', '', $matches[1]), $start_ts);
						} else if ($matches[1] == 'date') {
							$replacement = empty($start_ts) ? '' : date('jS M Y', iso8601_ts($start_ts));
						} else if ($matches[1] == 'time') {
							$iso_formated = iso8601_ts($start_ts);
							$mins = date('i', $iso_formated);
							if ($mins == '00') {
								$replacement = date('ga', $iso_formated);
							} else {
								$replacement = date('g:ia', $iso_formated);
							}
						} else if ($matches[1] == 'time_12h') {
							$replacement = empty($start_ts) ? '' : date('h:i a', iso8601_ts($start_ts));
						} else if ($matches[1] == 'time_24h') {
							$replacement = empty($start_ts) ? '' : date('H:i', iso8601_ts($start_ts));
						}
					}//end if
				}

				if (isset($this->vars['end_date_ts'])) {
					$end_ts = $this->attr('end_date_ts');
					preg_match('/^instance_end_(.+)/', $keyword, $matches);
					if (!empty($matches)) {
						if ($matches[1] == 'datetime_ical') {
							$replacement = empty($end_ts) ? '' : date('Ymd\THis', $end_ts);
						} else if (strpos($matches[1], 'datetime_') !== FALSE){
							$replacement = empty($end_ts) ? '' : date(str_replace('datetime_', '', $matches[1]), $end_ts);
						} else if ($matches[1] == 'date') {
							$replacement = empty($end_ts) ? '' : date('jS M Y', iso8601_ts($end_ts));
						} else if ($matches[1] == 'time') {
							$iso_formated = iso8601_ts($end_ts);
							$mins = date('i', $iso_formated);
							if ($mins == '00') {
								$replacement = date('ga', $iso_formated);
							} else {
								$replacement = date('g:ia', $iso_formated);
							}
						} else if ($matches[1] == 'time_12h') {
							$replacement = empty($end_ts) ? '' : date('h:i a', iso8601_ts($end_ts));
						} else if ($matches[1] == 'time_24h') {
							$replacement = empty($end_ts) ? '' : date('H:i', iso8601_ts($end_ts));
						}
					}//end if
				}

				if (empty($replacement)) {
					// For an event instance (shadow asset), get the replacement from the event asset (bridge)
					$assetid_parts = explode(':', $this->id);
					if (!empty($assetid_parts[0]) && !empty($assetid_parts[1]) && $keyword != 'asset_assetid') {
						$event_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid_parts[0]);
						$replacement = $event_asset->getKeywordReplacement($keyword);
						$GLOBALS['SQ_SYSTEM']->am->forgetAsset($event_asset);
					}
				}

				if (empty($replacement)) $replacement = parent::getKeywordReplacement($keyword);

				break;

		}//end switch

		// If replacement found, apply the modifiers
		// Otherwise put the it back, so it can get replaced at the higher level
		if ($replacement != '%'.$keyword.'%') {
			apply_keyword_modifiers($replacement, $modifiers, Array('assetid' => $this->id));
		} else {
			$replacement = '%'.$full_keyword.'%';
		}

		return $replacement;

	}//end getKeywordReplacement()


	/**
	* Returns a reference to the event asset instance represented by the passed assetid
	*
	* @param int	$assetid	the entire asset id of the asset to be loaded
	*
	* @return mixed object|NULL
	* @access public
	* @see Asset_Manager::getAsset()
	*/
	public function getAsset($whole_assetid, $type_code='', $mute_errors=FALSE)
	{
		$asset = NULL;
		$assetid_parts = explode(':', $whole_assetid);

		$assetid = $assetid_parts[0];
		if (empty($assetid_parts[1])) {
			return $asset;
		} else {
			$shdw_part_data = explode('|', $assetid_parts[1]);
			$instance_data = Array();
			foreach($shdw_part_data as $data) {
				list($key, $val) = explode("=", $data);
				$instance_data[$key] = $val;
			}
			if (!$GLOBALS['SQ_SYSTEM']->am->getAssetInfo($assetid)) {
				return $asset;
			}
			$event_asset = clone $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);

			$event_asset->id = $whole_assetid;
			foreach($instance_data as $attr => $value) {
				$event_asset->vars[$attr]['attrid'] = 0;
				$event_asset->vars[$attr]['type'] = 'text';
				$event_asset->vars[$attr]['value'] = $value;
				$event_asset->vars[$attr]['is_contextable'] = 0;
			}
		}

		return $event_asset;

	}//end getAsset()


	/**
	* Returns an array of information about assets of the passed codes
	*
	* If one value in $field then the return value is Array(assetid => '[field]'), otherwise Array(assetid => Array())
	*
	* @param array		$assetids			an array of assets to limit
	* @param mixed		$type_code			an asset type code, or array of typecodes, to limit results to
	* @param boolean	$strict_type_code	ensure returned assets are of the passed type codes
	* @param string		$field				single field from the asset table to return (along with the assetid)
	*										Default is all fields
	*
	* @return array
	* @access public
	*/
	public function getAssetInfo(Array $assetids, $type_code=Array(), $strict_type_code=TRUE, $field='')
	{
		$info = Array();
		foreach ($assetids as $whole_assetid) {
			$assetid_parts = explode(':', $whole_assetid);
			$assetid = $assetid_parts[0];
			if (empty($assetid_parts[1])) {
				$asset_type = $this->type();
			} else {
				$event_data_raw = explode('|', $assetid_parts[1]);
				$event_data = Array();
				foreach($event_data_raw as $event) {
					list($key, $val) = explode("=", $event);
					$event_data[$key] = $val;
				}
				$asset_type = $event_data['type_code'];
			}

			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			// if a sort field is specified then return only that field value else return all of the fields
			if (!empty($field)) {
				if (isset($event_data[$field])) {
					$info[$whole_assetid] = $event_data[$field];
				} else if (isset($asset->{$field})) {
					$info[$whole_assetid] = $asset->{$field};
				} else {
					trigger_error('Invalid field for the asset type Calendar Event asset', E_USER_ERROR);
				}
			} else {
				$info[$whole_assetid] = Array(
									'type_code'				=> $asset_type,
									'version'				=> $asset->version,
									'name'					=> $asset->name,
									'short_name'			=> $asset->short_name,
									'status'				=> $asset->status,
									'languages'				=> '',
									'charset'				=> '',
									'force_secure'			=> '0',
									'created'				=> $asset->getKeywordReplacement('asset_created'),
									'created_userid'		=> $asset->created_userid,
									'updated'				=> $asset->getKeywordReplacement('asset_updated'),
									'updated_userid'		=> $asset->updated_userid,
									'published'				=> $asset->getKeywordReplacement('asset_published'),
									'published_userid'		=> $asset->published_userid,
									'status_changed'		=> $asset->getKeywordReplacement('asset_status_changed'),
									'status_changed_userid'	=> $asset->status_changed_userid,
								  );
			}//end if field not empty
		}//end foreach of assetids

		return $info;

	}//end getAssetInfo()


	/**
	* Get the values of the attribute with the given name for the given assetids
	*
	* If the assetid is an event instance (shadow assetid),
	* get the attribute of the corresponding event asset instead
	*
	* @param string	$attr_name	The name of the attribute to get the values of
	* @param string	$asset_type	The type code of the asset type that defines the attribute
	* @param array	$assetids	IDs of the assets whose values we want
	* @param int	$contextid	The ID of the context we wish to get the value of the attr for.
	*							(If omitted, use the currently active context.)
	*
	* @return array
	* @access public
	*/
	public function getAttributeValuesByName($attr_name, $asset_type, $assetids, $contextid=NULL)
	{
		if (empty($assetids)) return Array();

		$real_assetids = Array();
		foreach($assetids as $full_assetid) {
			if (strpos($full_assetid, ':') !== FALSE) {
				list($real_assetids[]) = explode(':', $full_assetid);
			} else {
				$real_assetids[] = $full_assetid;
			}
		}//end foreach

		$real_res = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName($attr_name, $asset_type, $real_assetids, $contextid);

		$res = Array();
		foreach($assetids as $full_assetid) {
			list($real_assetid) = explode(':', $full_assetid);
			$res[$full_assetid] = isset($real_res[$real_assetid]) ? $real_res[$real_assetid] : '';
		}//end foreach

		return $res;

	}//end getAttributeValuesByName()




	/**
	* Get asset info for use by asset map
	*
	* @param string	$assetid	Full Asset id to get information for
	*
	* @return array
	* @access public
	*/
	public function getAssetMapAssetInfo($assetid)
	{
		return Array();

	}//end getAssetMapAssetInfo()


	/**
	* Return the number of links in the system that involve the specified asset
	*
	* Can be restricted by either link type afnd/or asset type code
	*
	* @param int		$assetid			id of the the $side_of_link asset
	* @param string		$side_of_link		Which side of the link this (the current) asset
	*										is on ('major' or 'minor')
	* @param int		$link_types			integer that can be the product of bitwise operations
	*										on the SQ_LINK_* constants
	* @param mixed		$type_code			the type of asset that is linked (eg 'Page', 'File', etc) (can be a string or an array of strings)
	*										if an array returns link if matches any
	*										of the array values
	* @param boolean	$strict_type_code	whether we are finding assets that are just a $type_code
	*										or $type_code and any of it's sub-classes
	* @param int		$ignore_linkid		ignore the link represented by this link id when returning the count
	*
	* @return int
	* @access public
	*/
	public function countLinks($assetid, $side_of_link='major', $link_types=0, $type_code='', $strict_type_code=TRUE, $ignore_linkid=0)
	{
		return 0;

	}//end countLinks()


	/**
	* Return all links that this asset has to other assets
	*
	* @param int		$assetid			id of the the $side_of_link asset
	* @param int		$link_types			integer that can be the product of bitwise operations
	*										on the SQ_LINK_* constants
	* @param mixed		$type_code			the type of asset that is linked
	*										(eg 'Page', 'File', etc)
	*										if an array returns link if matches any
	*										of the array values
	* @param boolean	$strict_type_code	whether we are finding assets that are just a $type_code
	*										or $type_code and any of it's sub-classes
	* @param string		$side_of_link		Which side of the link this (the current) asset
	*										is on ('major' or 'minor')
	* @param string		$sort_by			a field in the asset table you would like the results
	*										sorted by (eg. name, short_name etc)
	* @param boolean	$dependant			Whether we want dependant links, non-dependent or (NULL) don't care
	* @param boolean	$exclusive			Whether we want exclusive links, non-exclusive or (NULL) don't care
	*
	* @return array
	* @access public
	* @see Asset_Manager::getLinks()
	*/
	function getLinks($assetid, $link_types, $type_code='', $strict_type_code=TRUE, $side_of_link='major', $sort_by=NULL, $dependant=NULL, $exclusive=NULL)
	{
	    $id_parts = explode(':', $assetid);
	    if (isset($id_parts[0])) $assetid = $id_parts[0];
	    return $GLOBALS['SQ_SYSTEM']->am->getLinks($assetid, $link_types, $type_code, $strict_type_code, $side_of_link, $sort_by, $dependant, $exclusive);
	}


	/**
	* Get all asset ids that are above the passed assetid in the various trees in which it exists
	*
	* Can only find parents of type <i>user_group</i>
	*
	* @param int		$assetid			the id of the asset to get its parents for
	* @param mixed		$type_code			the type of asset that is linked (eg 'User', 'User_Group', etc)
	*										if an array returns link if matches any of the array values
	* @param boolean	$strict_type_code	whether we are finding assets that are just a $type_code
	*										or $type_code and any of it's sub-classes
	*
	* @return array
	* @access public
	* @see Asset_Manager::getParents()
	*/
	function getParents($assetid, $type_code='', $strict_type_code=TRUE)
	{
	    $id_parts = explode(':', $assetid);
	    if (isset($id_parts[0])) $assetid = $id_parts[0];
	    return $GLOBALS['SQ_SYSTEM']->am->getParents($assetid, $type_code, $strict_type_code);
	}


		/**
	* Get all asset ids that are below the passed assetid in the various trees in which it exists
	*
	* @param int		$assetid			the id of the asset to get its children for
	* @param mixed		$type_code			the type of asset that is linked
	*										(eg 'Page', 'File', etc)
	*										if an array returns link if matches any
	*										of the array values
	* @param boolean	$strict_type_code	whether we are finding assets that are just a $type_code
	*										or $type_code and any of it's sub-classes
	* @param boolean	$dependant			if TRUE, results will be filtered to assets that are dependants.
	*										If FALSE, results will all not be dependants.
	*										If NULL, results will not be filtered
	* @param string		$sort_by			a field in the asset table you would like the results
	*										sorted by (eg. name, short_name etc)
	*
	* @return array
	* @access public
	* @see Asset_Manager::getChildren()
	*/
	function getChildren($assetid, $type_code='', $strict_type_code=TRUE, $dependant=NULL, $sort_by=NULL)
	{
	    $id_parts = explode(':', $assetid);
	    if (isset($id_parts[0])) $assetid = $id_parts[0];
	    return $GLOBALS['SQ_SYSTEM']->am->getChildren($assetid, $type_code, $strict_type_code, $dependant, $sort_by);
	}


	/**
	* Returns an array of assetid's + info in the order that they propogate out from THIS BRIDGE'S url
	*
	* @param string	$assetid	the id of the last asset in the lineage
	* @param string	$protocol	the protocol to match -> NULL means it is ignored
	* @param string	$url		the url to check for -> NULL defaults it to current url
	*
	* @return array
	* @access public
	*/
	function getLineageFromURL($assetid, $protocol, $url)
	{
	    return Array();
	}

	/**
	* Remove a link by id
	*
	* @param int		$linkid	the link id of the link to remove
	* @param boolean	$moving	TRUE if this delete is part of a move operation
	*
	* @return boolean
	* @access public
	*/
	function deleteAssetLink($linkid, $moving=FALSE)
	{
		return FALSE;

	}//end deleteAssetLink()


	/**
	* Get details of a link with the specified characteristics
	*
	* @param int            $assetid            id of the the $side_of_link asset
	* @param int            $link_type          integer that should be a single integer of the SQ_LINK_* constants
	* @param string|array   $type_code          the type of asset that is linked
	*                                           (eg 'Page', 'File', etc)
	*                                           if an array returns link if matches any
	*                                           of the array values
	* @param boolean        $strict_type_code   whether we are finding an asset that
	*                                           is just a $type_code or potentially an inherited type
	*                                           or $type_code and any of it's sub-classes
	* @param string         $value              the value that is associated with this link
	* @param string         $side_of_link       Which side of the link this (the current) asset
	*                                           is on ('major' or 'minor')
	* @param boolean        $exclusive          The exclusive status for the link must be
	*                                           this (if not null)
	*
	* @return array
	* @access public
	*/
    function getLink($assetid, $link_type=NULL, $type_code='', $strict_type_code=TRUE, $value=NULL, $side_of_link='major', $exclusive=NULL)
    {
	    $id_parts = explode(':', $assetid);
	    if (isset($id_parts[0])) {
			$assetid = $id_parts[0];
		}

	    return $GLOBALS['SQ_SYSTEM']->am->getLink($assetid, $link_type, $type_code, $strict_type_code, $value, $side_of_link, $exclusive);

	}//end getLink()


}//end class

?>
