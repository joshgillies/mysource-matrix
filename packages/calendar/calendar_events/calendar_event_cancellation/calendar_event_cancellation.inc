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
* $Id: calendar_event_cancellation.inc,v 1.34 2012/08/30 00:57:28 ewang Exp $
*
*/

require_once  dirname(__FILE__).'/../../calendar_event/calendar_event.inc';
require_once  dirname(__FILE__).'/../calendar_event_recurring/calendar_event_recurring.inc';

/**
* Calendar_Event_Cancellation
*
* Objects of this class represent occasions when an instance of a recurring event
* is cancelled.  If the event has been moved to another time, a new single event
* which replaces the cancelled instance may be linked to it.
*
* @author  Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.34 $
* @package MySource_Matrix_Packages
* @subpackage calendar
*/
class Calendar_Event_Cancellation extends Calendar_Event
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Calendar_Event_Cancellation($assetid=0)
	{
		$this->Calendar_Event($assetid);

	}//end constructor


	/**
	* Perform any pre-validation required during the creation of this asset
	*
	* Cancellation events ensure their link data is correct and they are synchronized
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	function _preCreateCheck(&$link)
	{
		$parent =& $link['asset'];
		if (!is_a($parent, 'calendar_event_recurring')) {
			trigger_localised_error('CAL0054', E_USER_WARNING);
			return FALSE;
		}

		if (!$this->synchronizeWithParent($parent)) {
			return FALSE;
		}

		if (!parent::_preCreateCheck($link)) return FALSE;

		if (!($link['value'] = $this->getStartDate())) {
			trigger_localised_error('CAL0053', E_USER_WARNING);
			return FALSE;
		}

		return TRUE;

	}//end _preCreateCheck()


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

		if ($short_name) {
			return sprintf(translate('Cancelled %s'), date('d M Y', iso8601_ts($this->attr('start_date'))));

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
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @return array
	* @access private
	*/
	function _getAllowedLinks()
	{
		$links = parent::_getAllowedLinks();
		$links[SQ_LINK_TYPE_2]['calendar_event_single'] = Array('card' => 1, 'exclusive' => FALSE);
		return $links;

	}//end _getAllowedLinks()


	/**
	* Check the parent has an instance on the specified date and set our end time, end date, end time and name based on the parent
	*
	* @param object	&$parent	The recurring event that we belong to
	*
	* @return boolean
	* @access public
	* @see Calendar_Event_Cancellation::create(), Calendar_Event_Cancellation::saveAttributes()
	*/
	function synchronizeWithParent(&$parent)
	{
		// check that it has an occurrence on the date specified
		if (!$parent->hasOccurrenceOnDate($this->getStartDate())) {
			trigger_localised_error('CAL0055', E_USER_WARNING, $this->getStartDate());
			return FALSE;
		}

		// update our start time, end date and end time to correspond with the parent
		$sd = $this->getStartDate();
		$st = $parent->getStartTime();
		if ($st) {
			$this->setAttrValue('start_date', $sd.' '.$st.':00');
		}
		if ($duration = $parent->getDuration()) {
			$this->setAttrValue('end_date', date('Y-m-d', iso8601_ts($this->attr('start_date')) + $duration).substr($parent->attr('end_date'), 10));
		}
		// set our name based on the parent
		$this->setAttrValue('name', sprintf(translate('Cancelled %s'), date('d M Y', iso8601_ts($this->attr('start_date')))).': '.$parent->attr('name'));


		return TRUE;

	}//end synchronizeWithParent()


	/**
	* Add a replacement single event linked under this cancellation event
	*
	* @param string	$start_date		The start date attr value of the replacement event
	* @param string	$end_date		The end date attr value of the replacement event
	* @param string	$description	Description for the replacement event
	*
	* @return object
	* @access public
	*/
	function addReplacement($start_date, $end_date, $description)
	{
		if (!$this->writeAccess('')) {
			trigger_localised_error('CAL0056', E_USER_WARNING);
			return FALSE;
		}
		if ($existing_link = $this->getReplacementLink()) {
			$replacement_event = $GLOBALS['SQ_SYSTEM']->am->getAsset($existing_link['minorid']);
		} else {
			// create a replacement event
			$parent_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, NULL, '', TRUE, NULL, 'minor');
			$parent_info = reset($GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($parent_link['majorid'])));
			$new_type_code = str_replace('recurring', 'single', $parent_info['type_code']);
			$GLOBALS['SQ_SYSTEM']->am->includeAsset($new_type_code);
			$replacement_event = new $new_type_code();
		}
		$replacement_event->setAttrValue('start_date', $start_date);
		$replacement_event->setAttrValue('end_date', $end_date);
		$replacement_event->setAttrValue('description', $description);
		if (!$replacement_event->id) {
			$link = Array('asset' => &$this, 'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);
			$replacement_event->create($link);
		} else {
			if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($replacement_event->id, 'attributes')) {
				trigger_localised_error('CAL0057', E_USER_WARNING, $replacement_event->name);
				return 0;
			}
			$replacement_event->saveAttributes();
			$GLOBALS['SQ_SYSTEM']->am->releaseLock($replacement_event->id, 'attributes');
		}
		if ($replacement_event->id) {
			$replacement_event->processStatusChange($this->status);
			return $replacement_event;
		} else {
			return FALSE;
		}

	}//end addReplacement()


	/**
	* Gets the link details of the exception (cancellation or modification) event linked under
	* this event for the date given
	*
	* This function can be called statically (with both parameters) or on an object (with just
	* the first parameter)
	*
	* @param int	$id	The ID of the recurring event to test, or NULL to use the current object
	*
	* @return mixed array|boolean
	* @access public
	* @static (optionally)
	*/
	function getReplacementLink($id=NULL)
	{
		if (is_null($id)) $id = $this->id;
		$child_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($id, SQ_SC_LINK_ALL, 'calendar_event_single', FALSE, 'major');
		return current($child_links);

	}//end getReplacementLink()


	/**
	* Process the specified action on this event, and return whatever html the action wants to show
	*
	* @param string	$action_name	The action to perform, eg 'cancel'
	*
	* @return string
	* @access public
	*/
	function processEventAction($action_name)
	{
		$res = '';
		switch ($action_name) {
			case 'addreplacement':
				$parent_assetids = array_keys($GLOBALS['SQ_SYSTEM']->am->getParents($this->id, 'calendar_event_recurring'));
				$parent_object = $GLOBALS['SQ_SYSTEM']->am->getAsset($parent_assetids[0]);

				$res = '<form method="POST" action="?SQ_CALENDAR_VIEW=event&amp;SQ_CALENDAR_EVENT_ID='.$this->id.'&amp;SQ_CALENDAR_EVENT_ACTION=addreplacement2&amp;SQ_CALENDAR_DATE='.$_REQUEST['SQ_CALENDAR_DATE'].'"><table cellpadding="4" border="0">';
				$res .= '<tr><td style="vertical-align: top">'.translate('Start and End').'</td><td>';

				ob_start();
					$edit_fns = $this->getEditFns();
					$edit_fns->paintStartEndChooser($this, 'event');
					$res .= ob_get_contents();
				ob_end_clean();
				$res .= '</td></tr>';

				$res .= '<tr><td style="vertical-align: top">'.translate('Description').': </td><td><textarea name="event_description">';

				$res .= $parent_object->attr('description');
				$res .= '</textarea></td></tr>';
				$res .= '<tr><td colspan="2"><input type="submit" value="'.translate('Add Replacement').'"/>&nbsp;&nbsp;<input type="button" value="'.translate('Cancel').'" onclick="document.location=\''.$_SERVER['PHP_SELF'].'?SQ_CALENDAR_EVENT_ID='.$parent_object->id.'&SQ_CALENDAR_VIEW=event&SQ_CALENDAR_DATE='.$_REQUEST['SQ_CALENDAR_DATE'].'\'" /></td></tr></table></form>';


			break;

			case 'addreplacement2':
				if (!isset($_REQUEST['event_start_time_enabled'])) {
					$_REQUEST['event_start_hours'] = '--';
					$_REQUEST['event_start_minutes'] = '--';
				} else {
					if ($_REQUEST['event_start_is_pm'] && ($_REQUEST['event_start_hours'] != 12)) {
						$_REQUEST['event_start_hours'] += 12;
					}
				}
				if (!isset($_REQUEST['event_end_time_enabled'])) {
					$_REQUEST['event_end_hours'] = '--';
					$_REQUEST['event_end_minutes'] = '--';
				} else {
					if ($_REQUEST['event_end_is_pm'] && ($_REQUEST['event_end_hours'] != 12)) {
						$_REQUEST['event_end_hours'] += 12;
					}
				}
				if (!isset($_REQUEST['event_end_date_enabled'])) {
					$_REQUEST['event_end_year'] = '----';
					$_REQUEST['event_end_month'] = '--';
					$_REQUEST['event_end_day'] = '--';
				}

				$replacement_event = $this->addReplacement(sprintf('%04s-%02s-%02s %02s:%02s:--', $_REQUEST['event_start_year'], $_REQUEST['event_start_month'], $_REQUEST['event_start_day'],  $_REQUEST['event_start_hours'], $_REQUEST['event_start_minutes']),  sprintf('%04s-%02s-%02s %02s:%02s:--', $_REQUEST['event_end_year'], $_REQUEST['event_end_month'], $_REQUEST['event_end_day'],  $_REQUEST['event_end_hours'], $_REQUEST['event_end_minutes']),  $_REQUEST['event_description']);
				if ($replacement_event) {
					$res = '<p>'.translate('Replacement event added').'</p>';

				}
			break;

			default:
				$res = parent::processEventAction($action_name);
			break;
		}//end switch
		return $res;

	}//end processEventAction()


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
	function saveAttributes($dont_run_updated=FALSE)
	{
		if ($this->id) {

			if (!$dont_run_updated) {

				// get the parent recurring event asset
				$recurring_event_ids = array_keys($GLOBALS['SQ_SYSTEM']->am->getParents($this->id, 'calendar_event_recurring', FALSE));
				if (empty($recurring_event_ids)) {
					trigger_localised_error('CAL0054', E_USER_ERROR);
					return FALSE;
				}
				$recurring_event = $GLOBALS['SQ_SYSTEM']->am->getAsset($recurring_event_ids[0]);

				if (!$this->synchronizeWithParent($recurring_event)) {
					return FALSE;
				}
			}
		}
		return parent::saveAttributes($dont_run_updated);


	}//end saveAttributes()


	/**
	* Called by the design to print the body of this asset
	*
	* @return void
	* @access public
	*/
	function printBody()
	{
		$root_url = array_values($GLOBALS['SQ_SYSTEM']->am->getRootURL());
		if (is_array($root_url)) $root_url = $root_url[0];
		require_once SQ_FUDGE_PATH.'/general/datetime.inc';
		?>
		<table border="0" cellspacing="2" cellpadding="2">
			<tr>
				<td><b><?php echo translate('Name') ?></b></td>

				<td><?php echo $this->attr('name'); ?></td>
			</tr>
			<tr>
				<td><b><?php echo translate('Cancels') ?></b></td>

				<td><?php
				// get the parent recurring event asset
				if ($recurring_events = $GLOBALS['SQ_SYSTEM']->am->getParents($this->id, 'calendar_event_recurring', FALSE)) {
					$recurring_events = array_keys($recurring_events);
					$recurring_event = $GLOBALS['SQ_SYSTEM']->am->getAsset($recurring_events[0]);
					echo '<a href="'.$root_url.'?a='.$recurring_events[0].'">'.$recurring_event->attr('name').'</a>';
				}
				?>
				</td>
			</tr>
			<tr>
				<td><b><?php echo translate('Start Date of cancelled instance'); ?></b></td>

				<td><?php echo $this->getFriendlyStart(); ?></td>
			</tr>
			<tr>
				<td><b><?php echo translate('End Date of cancelled instance'); ?></b></td>

				<td><?php echo $this->getFriendlyEnd(); ?></td>
			</tr>
			<tr>
				<td><b><?php echo translate('Reason for cancellation'); ?></b></td>

				<td><?php echo $this->attr('description'); ?></td>
			</tr>
		</table>
		<?php

	}//end printBody()


}//end class

?>
