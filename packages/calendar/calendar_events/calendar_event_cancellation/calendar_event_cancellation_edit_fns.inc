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
* | intellectual property rights in the cancellation, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: calendar_event_cancellation_edit_fns.inc,v 1.5 2004/09/14 19:07:16 brobertson Exp $
* $prefix:  $
*/


require_once  dirname(__FILE__).'/../../calendar_event/calendar_event_edit_fns.inc';

/**
* Page_Edit_Fns
*
* Functions used by the edit interface of the calendar_event asset
*
*
* @author  Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.5 $ - 1.0
* @package MySource_Matrix_Packages
* @subpackage calendar
*/
class Calendar_Event_Cancellation_Edit_Fns extends Calendar_Event_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Calendar_Event_Cancellation_Edit_Fns()
	{
		$this->Calendar_Event_Edit_Fns();

	}//end Calendar_Event_Cancellation_Edit_Fns()


	/**
	* Paint the form for entering the date of the recurring event instance to be cancelled
	*
	* @param object Calendar_Event_Cancellation	&$asset			the event to print the form for
	* @param object Backend_Outputter			&$o				the outputter class (not used here)
	* @param string								$prefix			the prefix to use for form field names
	*
	* @access public
	*/
	function paintDateTimeChooser(&$asset, &$o, $prefix) 
	{
		if ($asset->writeAccess('attributes')) {
			?>
			<script language="JavaScript" type="text/javascript" src="<?php echo sq_web_path('data').'/asset_types/calendar_event/js/date_chooser.js'?>"></script>
			<?php
			$ts = iso8601_ts($asset->attr('start_date'));
			if ($ts == -1) $ts = 0;
			$this->printDateChooser($prefix.'_start', $ts);
			?>
			<br />This must be a date on which the recurring event occurs
			<?php
		} else {
			echo $asset->getFriendlyDatesAndTimes();
		}

	}//end paintDateChooser()


	/**
	* Process the start date/time value entered
	*
	* @param object Calendar_Event_Recurring	&$asset			the event to process the form for
	* @param object Backend_Outputter			&$o				the outputter class (not used here)
	* @param string								$prefix			the prefix to use for form field names
	*
	* @return boolean	Whether the update was successful
	* @access public
	*/
	function processDateTimeChooser(&$asset, &$o, $prefix) 
	{
		return $this->processStartEndChooser($asset, $o, $prefix);

	}//end processSingleStartEndChooser


	/**
	* Paint the interface for choosing the recurring event whose instance is being modified
	*
	* @param object Calendar_Event_Recurring	&$asset			the event to process the form for
	* @param object Backend_Outputter			&$o				the outputter class (not used here)
	* @param string								$prefix			the prefix to use for form field names
	*
	* @access public
	*/
	function paintRecurringEventChooser(&$asset, &$o, $prefix)
	{
		$modified_event_asset = null;
		if ($asset->id) {
			$recurring_event_ids = $GLOBALS['SQ_SYSTEM']->am->getParents($asset->id, 'calendar_event_recurring', false);
			if (!empty($recurring_event_ids)) {
				$modified_event_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($recurring_event_ids[0]);
			}
		}
		if (!is_null($modified_event_asset)) echo $modified_event_asset->name.' (Id: #'.$modified_event_asset->id.')';
		else echo 'No event has been selected for cancellation';

	}//end paintRecurringEventChooser()


	/**
	* Process the supplied recurring event value
	*
	* @param object Calendar_Event_Recurring	&$asset			the event to process the form for
	* @param object Backend_Outputter			&$o				the outputter class (not used here)
	* @param string								$prefix			the prefix to use for form field names
	*
	* @return boolean	Whether the update was successful
	* @access public
	*/	
	function processRecurringEventChooser(&$asset, &$o, $prefix) 
	{
		// not doing anything here; the field is processed in saveAttributes() because
		// we need an ID first
		return true;

	}//end processRecurringEventChooser()


}//end class

?>