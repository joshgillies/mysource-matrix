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
* $Id: calendar_event_cancellation.inc,v 1.10 2004/09/13 05:20:33 tbarrett Exp $
* $Name: not supported by cvs2svn $
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
* @version $Revision: 1.10 $ - 1.0
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

	}//end Calendar_Event_Cancellation()


	/**
	* Create this asset
	*
	* The return value will be:<br/>
	* <ul>
	*   <li>FALSE if the asset was not created</li>
	*   <li>the ID of the newly created link if the asset and intital link were created</li>
	*   <li>TRUE if the asset was created but $link was empty</li>
	* </ul>
	*
	* @param Array	&$link	information used to create the initial link<br/>
	*
	* @return mixed int or false
	* @access public
	*/
	function create(&$link) 
	{
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$parent = &$link['asset'];
		if (!($link['value'] = $this->getStartDate())) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			trigger_error('You need to specify a start date before you can call create() for a modification event', E_USER_ERROR);
			return false;
		}
		if ($parent->type() != 'calendar_event_recurring') {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			trigger_error('Recurring Event Cancellations can only be created underneath recurring events', E_USER_ERROR);
			return false;
		}
		if (!$this->synchronizeWithParent($parent)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}
		if ($res = parent::create($link)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			return $res;
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return $res;
		}
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
		return $short_name ? 'Cancelled '.date('d M Y', iso8601_ts($this->attr('start_date'))) : $this->attr('name');
	
	}//end _getName()


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @return Array()
	* @access private
	*/
	function _getAllowedLinks()
	{
		$links = parent::_getAllowedLinks();
		$links[SQ_LINK_TYPE_2]['calendar_event_single'] = Array('card' => 1, 'exclusive' => false);
		return $links;

	}//end _getAllowedLinks()


	/**
	* Check the parent has an instance on the specified date and set our end time, end date, end time and name based on the parent
	*
	* @param Object calendar_event_recurring	&$parent	The recurring event that we belong to
	* 
	* @return boolean	Whether or not the synchronisation was successful
	* @access public
	* @see Calendar_Event_Cancellation::create(), Calendar_Event_Cancellation::saveAttributes()
	*/	
	function synchronizeWithParent(&$parent) 
	{
		// check that it has an occurrence on the date specified
		if (!$parent->hasOccurrenceOnDate($this->getStartDate())) {
			trigger_error('There is no occurrence of the specified recurring event on the specified date, being '.$this->getStartDate(), E_USER_ERROR);
			return false;
		}

		// update our start time, end date and end time to correspond with the parent
		$sd = $this->getStartDate();
		$st = $parent->getStartTime();
		if ($st) {
			$this->setAttrValue('start_date', $sd.' '.$st);
		}
		if ($duration = $parent->getDuration()) {
			$this->setAttrValue('end_date', date('Y-m-d', iso8601_ts($this->attr('start_date')) + $duration).substr($parent->attr('end_date'), 10));
		}

		// set our name based on the parent
		$this->setAttrValue('name', 'Cancelled '.date('d M Y', iso8601_ts($this->attr('start_date'))).': '.  $parent->attr('name'));

		return true;

	}//end synchronizeWithParent()


	/**
	* Add a replacement single event linked under this cancellation event
	*
	* @param string		$date	the date of the occurrence to cancel (YYYY-MM-DD)
	* @param string		$reason	text to put in the description field of the cancellation event
	*/
	function addReplacement($start_date, $end_date, $description)
	{
		if (!$this->writeAccess('')) { 
			trigger_error('You don\'t have sufficient access to create a cancellation for this event', SQ_USER_WARNING);
			return false;
		} 
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('calendar_event_single'); 
		if ($existing_link = $this->getReplacementLink()) {
			$replacement_event = &$GLOBALS['SQ_SYSTEM']->am->getAsset($existing_link['minorid']);
		} else {
			$replacement_event = new Calendar_Event_Single();
		}
		$replacement_event->setAttrValue('start_date', $start_date);
		$replacement_event->setAttrValue('end_date', $end_date);
		$replacement_event->setAttrValue('description', $description);
		if (!$replacement_event->id) {
			$link = Array('asset' => &$this, 'link_type' => SQ_LINK_TYPE_2, 'dependant' => 1, 'exclusive' => 1);
			$replacement_event->create($link);
		} else {
			if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($replacement_event->id, 'attributes')) {
				trigger_error('Could not get lock to save attrs of asset "'.$replacement_event->name.'"', E_USER_WARNING);
				return 0;
			}
			$replacement_event->saveAttributes();
			$GLOBALS['SQ_SYSTEM']->am->releaseLock($replacement_event->id, 'attributes');
		}
		return $replacement_event;

	}// end addReplacement()


	/**
	* Gets the link details of the exception (cancellation or modification) event linked under
	* this event for the date given
	*
	* This function can be called statically (with both parameters) or on an object (with just
	* the first parameter)
	*
	* @param int	$id		The ID of the recurring event to test, or NULL to use the current object
	*
	* @static (optionally)
	* @return Array|boolean		Array (assetid => type_code) of the event, or false if none exist
	*/
	function getReplacementLink($id=null)
	{
		if (is_null($id)) $id = $this->id;
		$child_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($id, SQ_SC_LINK_ALL, 'calendar_event_single', false, 'major');
		return current($child_links);

	}//end getReplacementLink()


	/**
	* Process the specified action on this event, and return whatever html the action wants to show
	*
	* @param string	$action_name	The action to perform, eg 'cancel'
	*
	* @return string	HTML to show as the result of this action
	* @access public
	*/	
	function processEventAction($action_name) 
	{
		$res = '';
		switch ($action_name) {
			case 'addreplacement':
				$parent_assetids = $GLOBALS['SQ_SYSTEM']->am->getParents($this->id, 'calendar_event_recurring');
				$parent_object = &$GLOBALS['SQ_SYSTEM']->am->getAsset($parent_assetids[0]);

				$res = '<form method="POST" action="?SQ_CALENDAR_VIEW=event&amp;SQ_CALENDAR_EVENT_ID='.$this->id.'&amp;SQ_CALENDAR_EVENT_ACTION=addreplacement2&amp;SQ_CALENDAR_DATE='.$_REQUEST['SQ_CALENDAR_DATE'].'"><table cellpadding="4" border="0">';
				$res .= '<tr><td style="vertical-align: top">Start and End: </td><td>';
				ob_start();
					$edit_fns = $this->getEditFns();
					$edit_fns->paintStartEndChooser($this, 'event');
					$res .= ob_get_contents();
				ob_end_clean();
				$res .= '</td></tr>';

				$res .= '<tr><td style="vertical-align: top">Description: </td><td><textarea name="event_description">';
				$res .= $parent_object->attr('description');
				$res .= '</textarea></td></tr>';
				$res .= '<tr><td colspan="2"><input type="submit" value="Add Replacement" />&nbsp;&nbsp;<input type="button" value="Cancel" onclick="document.location=\''.$_SERVER['PHP_SELF'].'?SQ_CALENDAR_EVENT_ID='.$parent_object->id.'&SQ_CALENDAR_VIEW=event&SQ_CALENDAR_DATE='.$_REQUEST['SQ_CALENDAR_DATE'].'\'" /></td></tr></table></form>';
			break;

			case 'addreplacement2':
				if (!isset($_REQUEST['event_start_time_enabled'])) {
					$_REQUEST['event_start_hours'] = '--';
					$_REQUEST['event_start_minutes'] = '--';
				} else {
					if ($_REQUEST['event_start_is_pm'] && ($_REQUEST['event_start_hours'] != 12)) $_REQUEST['event_start_hours'] += 12;
				}
				if (!isset($_REQUEST['event_end_time_enabled'])) {
					$_REQUEST['event_end_hours'] = '--';
					$_REQUEST['event_end_minutes'] = '--';
				} else {
					if ($_REQUEST['event_end_is_pm'] && ($_REQUEST['event_end_hours'] != 12)) $_REQUEST['event_end_hours'] += 12;				
				}
				if (!isset($_REQUEST['event_end_date_enabled'])) {
					$_REQUEST['event_end_year'] = '----';
					$_REQUEST['event_end_month'] = '--';
					$_REQUEST['event_end_day'] = '--';
				}

				$replacement_event = $this->addReplacement(sprintf('%04s-%02s-%02s %02s:%02s:--', $_REQUEST['event_start_year'], $_REQUEST['event_start_month'], $_REQUEST['event_start_day'],  $_REQUEST['event_start_hours'], $_REQUEST['event_start_minutes']),  sprintf('%04s-%02s-%02s %02s:%02s:--', $_REQUEST['event_end_year'], $_REQUEST['event_end_month'], $_REQUEST['event_end_day'],  $_REQUEST['event_end_hours'], $_REQUEST['event_end_minutes']),  $_REQUEST['event_description']);
				$res = '<p>Replacement event added</p>';
			break;

			default:
				$res = parent::processEventAction($action_name);
			break;
		}
		return $res;

	}//end processEventAction();


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
		if ($this->id) {

			if (!$dont_run_updated) {
	
				// get the parent recurring event asset
				$recurring_event_ids = $GLOBALS['SQ_SYSTEM']->am->getParents($this->id, 'calendar_event_recurring', false);
				if (empty($recurring_event_ids)) {
					trigger_error('Recurring Event Cancellations can only be created underneath recurring events', E_USER_ERROR);
					return false;
				}
				$recurring_event = &$GLOBALS['SQ_SYSTEM']->am->getAsset($recurring_event_ids[0]);

				if (!$this->synchronizeWithParent($recurring_event)) return false;
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
				<td><b>Event Name</b></td>
				<td><?php echo $this->attr('name'); ?></td>
			</tr>
			<tr>
				<td><b>Cancels</b></td>
				<td><?php 
				// get the parent recurring event asset
				if ($recurring_events = $GLOBALS['SQ_SYSTEM']->am->getParents($this->id, 'calendar_event_recurring', false)) {
					$recurring_event = &$GLOBALS['SQ_SYSTEM']->am->getAsset($recurring_events[0]);
					echo '<a href="'.$root_url.'?a='.$recurring_events[0].'">'.$recurring_event->attr('name').'</a>';
				}?></td>
			</tr>
			<tr>
				<td><b>Start Date of cancelled instance</b></td>
				<td><?php echo $this->getFriendlyStart(); ?></td>
			</tr>
			<tr>
				<td><b>End Date of cancelled instance</b></td>
				<td><?php echo $this->getFriendlyEnd(); ?></td>
			</tr>
			<tr>
				<td><b>Reason for Cancellation</b></td>
				<td><?php echo $this->attr('description'); ?></td>
			</tr>
		</table>
		<?php

	}//end printBody()


}//end class

?>