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
* $Id: calendar_event_modification.inc,v 1.8 2004/08/02 06:29:03 lwright Exp $
* $Name: not supported by cvs2svn $
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
* @version $Revision: 1.8 $ - 1.0
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

	}//end Calendar_Event_Modification()


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
		$parent = &$link['asset'];
		if ($parent->type() != 'calendar_event_recurring') {
			trigger_error('Recurring Event Cancellations can only be created underneath recurring events', E_USER_ERROR);
			return false;
		}
		if (!$this->synchronizeWithParent($parent)) return false;
		return parent::create($link);
	}


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
		return $short_name ? 'Modified ' . date('d M Y', iso8601_ts($this->attr('start_date'))) : $this->attr('name');
	
	}//end _getName()


	/**
	* Check the parent has an instance on the specified date and set our end time, end date, end time and name based on the parent
	*
	* @param Object calendar_event_recurring	&$parent	The recurring event that we belong to
	* 
	* @return boolean	Whether or not the synchronisation was successful
	* @see Calendar_Event_Modification::create(), Calendar_Event_Modification::saveAttributes()
	*/	
	function synchronizeWithParent(&$parent) 
	{
		// check that it has an occurrence on the date specified
		if (!$parent->hasOccurrenceOnDate($this->getStartDate())) {
			trigger_error('There is no occurrence of the specified recurring event on the specified date', E_USER_ERROR);
			return false;
		}

		// update our start time, end date and end time to correspond with the parent
		$sd = $this->getStartDate();
		$st = $parent->getStartTime();
		if ($st) {
			$this->setAttrValue('start_date', $sd . ' ' . $st);
		}
		if ($duration = $parent->getDuration()) {
			$this->setAttrValue('end_date', date('Y-m-d', iso8601_ts($this->attr('start_date')) + $duration) . substr($parent->attr('end_date'), 10));
		}

		return true;

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
	function saveAttributes($dont_run_updated=false)
	{
		if ($this->id) {

			if (!$dont_run_updated) {
	
				// get the parent recurring event asset
				$recurring_event_ids = $GLOBALS['SQ_SYSTEM']->am->getParents($this->id, 'calendar_event_recurring', false);
				if (empty($recurring_event_ids)) {
					trigger_error('Recurring Event Modifications can only be created underneath recurring events', E_USER_ERROR);
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
				<td><b>Modification of </b></td>
				<td><?php 
				// get the parent recurring event asset
				if ($recurring_events = $GLOBALS['SQ_SYSTEM']->am->getParents($this->id, 'calendar_event_recurring', false)) {
					$recurring_event = &$GLOBALS['SQ_SYSTEM']->am->getAsset($recurring_events[0]);
					echo '<a href="' . $root_url . '?a=' . $recurring_events[0] . '">' . $recurring_event->attr('name') . '</a>';
				}?></td>
			</tr>
			<tr>
				<td><b>Start Date of modified instance</b></td>
				<td><?php echo $this->getFriendlyStart(); ?></td>
			</tr>
			<tr>
				<td><b>End Date of modified instance</b></td>
				<td><?php echo $this->getFriendlyEnd(); ?></td>
			</tr>
			<tr>
				<td><b>Modified Name</b></td>
				<td><?php echo $this->attr('name'); ?></td>
			</tr>
			<tr>
				<td><b>Modified Description</b></td>
				<td><?php echo $this->attr('description'); ?></td>
			</tr>
		</table>
		<?php

	}//end printBody()

}//end class

?>