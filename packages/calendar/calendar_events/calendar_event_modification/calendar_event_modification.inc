<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +--------------------------------------------------------------------+
*
* $Id: calendar_event_modification.inc,v 1.21 2006/12/07 05:51:50 bcaldwell Exp $
*
*/

require_once  dirname(__FILE__).'/../../calendar_event/calendar_event.inc';
require_once  dirname(__FILE__).'/../calendar_event_recurring/calendar_event_recurring.inc';

/**
* Calendar_Event_Modification
*
* Objects of this class represent instances where the details (name or description)
* of a recurring event have changed for a particular instance of that event.
*
* @author  Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.21 $
* @package MySource_Matrix_Packages
* @subpackage calendar
*/
class Calendar_Event_Modification extends Calendar_Event
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Calendar_Event_Modification($assetid=0)
	{
		$this->Calendar_Event($assetid);

	}//end constructor


	/**
	* Perform any pre-validation required during the creation of this asset
	*
	* Modification events ensure their link data is correct and they are synchronized
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	function _preCreateCheck(&$link)
	{
		if (!parent::_preCreateCheck($link)) return FALSE;

		if (!($link['value'] = $this->getStartDate())) {
			trigger_localised_error('CAL0049', E_USER_ERROR);
			return FALSE;
		}

		$parent =& $link['asset'];
		if ($parent->type() != 'calendar_event_recurring') {
			trigger_localised_error('CAL0050', E_USER_ERROR);
			return FALSE;
		}

		if (!$this->synchronizeWithParent($parent)) {
			return FALSE;
		}

		return TRUE;

	}//end _preCreateCheck()


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	*
	* @return string
	* @access protected
	* @see Asset::_getName()
	*/
	function _getName($short_name=FALSE)
	{
		return $short_name ? 'Modified '.date('d M Y', iso8601_ts($this->attr('start_date'))) : $this->attr('name');

	}//end _getName()


	/**
	* Check the parent has an instance on the specified date and set our end time, end date, end time and name based on the parent
	*
	* @param object	&$parent	The recurring event that we belong to
	*
	* @return boolean
	* @access public
	* @see Calendar_Event_Modification::create(), Calendar_Event_Modification::saveAttributes()
	*/
	function synchronizeWithParent(&$parent)
	{
		// check that it has an occurrence on the date specified
		if (!$parent->hasOccurrenceOnDate($this->getStartDate())) {
			trigger_localised_error('CAL0051', E_USER_WARNING, $this->getStartDate());
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
		if (!$this->attr('name')) {
			$this->setAttrValue('name', $parent->attr('name').' - Modified '.date('d M Y', iso8601_ts($this->attr('start_date'))));
		}

		return TRUE;

	}//end synchronizeWithParent()


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
					trigger_localised_error('CAL0052', E_USER_ERROR);
					return FALSE;
				}
				$recurring_event =& $GLOBALS['SQ_SYSTEM']->am->getAsset($recurring_event_ids[0]);

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
				<td><b><?php echo translate('cal_event_modification_of'); ?></b></td>
				<td><?php
				// get the parent recurring event asset
				if ($recurring_events = $GLOBALS['SQ_SYSTEM']->am->getParents($this->id, 'calendar_event_recurring', FALSE)) {
					$recurring_events = array_keys($recurring_events);
					$recurring_event =& $GLOBALS['SQ_SYSTEM']->am->getAsset($recurring_events[0]);
					echo '<a href="'.$root_url.'?a='.$recurring_events[0].'">'.$recurring_event->attr('name').'</a>';
				}
				?></td>
			</tr>
			<tr>
				<td><b><?php echo translate('cal_event_modified_start_date'); ?></b></td>
				<td><?php echo $this->getFriendlyStart(); ?></td>
			</tr>
			<tr>
				<td><b><?php echo translate('cal_event_modified_end_date'); ?></b></td>
				<td><?php echo $this->getFriendlyEnd(); ?></td>
			</tr>
			<tr>
				<td><b><?php echo translate('cal_event_modified_name'); ?></b></td>
				<td><?php echo $this->attr('name'); ?></td>
			</tr>
			<tr>
				<td><b><?php echo translate('cal_event_modified_description'); ?></b></td>
				<td><?php echo $this->attr('description'); ?></td>
			</tr>
		</table>
		<?php

	}//end printBody()


}//end class

?>