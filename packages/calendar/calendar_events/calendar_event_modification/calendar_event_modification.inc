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
* $Id: calendar_event_modification.inc,v 1.3 2004/07/28 23:31:35 tbarrett Exp $
* $Name: not supported by cvs2svn $
*/

require_once  dirname(__FILE__).'/../../event/calendar_event.inc';
require_once  dirname(__FILE__).'/../calendar_event_recurring/calendar_event_recurring.inc';

/**
* Calendar_Event_Modification
*
* Objects of this class represent instances where the details (name or description)
* of a recurring event have changed for a particular instance of that event.
*
* @author  Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.3 $ - 1.0
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
	* <PRE>
	* Array ('asset'      => [ref major asset to create link under],
	*        'link_type'  => SQ_LINK_?,
	*        'value'      => [link value],
	*        'sort_order' => [link sort order],
	*        'dependant'  => [0|1],
	*        'exclusive'  => [0|1]
	*        )
	* </PRE>
	*
	* @return mixed int or false
	* @access public
	*/
	function create(&$link) 
	{
		$parent = &$link['asset'];
		if ($parent->type() != 'calendar_event_recurring') {
			trigger_error('Recurring Event Modifications can only be created underneath recurring events', E_USER_ERROR);
			return false;
		}
		//$_tmp;
		if (!$parent->hasOccurrenceOnDate($this->getStartDate())) {
			trigger_error('The event specified does not occur on the date specified', E_USER_ERROR);
			return false;
		}
		return parent::create($link);
	}


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
		if (!parent::saveAttributes($dont_run_updated)) return false;
		if ($this->id && !$dont_run_updated) {
	
			// check that the date given matches the date of an instance of the modified event
			$recurring_event_ids = $GLOBALS['SQ_SYSTEM']->am->getParents($this->id, 'calendar_event_recurring', false);
			if (empty($recurring_event_ids)) {
				trigger_error('Recurring Event Modifications can only be created underneath recurring events', E_USER_ERROR);
				return false;
			}
				
			$recurring_event = &$GLOBALS['SQ_SYSTEM']->am->getAsset($recurring_event_ids[0]);
			$sd = $this->getStartDate();
			$st = $recurring_event->getStartTime();
			if ($recurring_event->hasOccurrenceOnDate($this->getStartDate())) {
				if ($st) {
					$this->attr('start_date', $sd . ' ' . $st);
				}
				if ($duration = $recurring_event->getDuration()) {
					$this->setAttrValue('end_date', date('Y-m-d', strtotime($recurring_event->attr('start_date')) + $duration) . substr($recurring_event->attr('end_date'), 10));
				}
			} else {
				trigger_error('There is no occurrence of the specified recurring event on the specified date', E_USER_ERROR);
				return false;
			}
		}
		return true;

	}//end saveAttributes()


}//end class

?>