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
* $Id: calendar_event_cancellation_edit_fns.inc,v 1.17 2012/08/30 00:57:28 ewang Exp $
*
*/

require_once  dirname(__FILE__).'/../../calendar_event/calendar_event_edit_fns.inc';

/**
* Page_Edit_Fns
*
* Functions used by the edit interface of the calendar_event asset
*
*
* @author  Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.17 $
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

	}//end constructor


	/**
	* Paint the form for entering the date of the recurring event instance to be cancelled
	*
	* @param object	&$asset	the event to print the form for
	* @param object	&$o		the outputter class (not used here)
	* @param string	$prefix	the prefix to use for form field names
	*
	* @return void
	* @access public
	*/
	function paintDateTimeChooser(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess('attributes')) {
			?>
			<script type="text/javascript" src="<?php echo sq_web_path('data').'/asset_types/calendar_event/js/date_chooser.js'?>"></script>
			<?php
			$ts = iso8601_ts($asset->attr('start_date'));
			if ($ts == -1) $ts = 0;
			$this->printDateChooser($prefix.'_start', $ts);
			hidden_field($prefix.'_start_end_chooser_printed', 1);
			?>
			<br /><?php translate('This must be a date on which the recurring event occurs'); ?>

			<?php
		} else {
			echo $asset->getFriendlyDatesAndTimes();
		}

	}//end paintDateTimeChooser()


	/**
	* Process the start date/time value entered
	*
	* @param object	&$asset	the event to process the form for
	* @param object	&$o		the outputter class (not used here)
	* @param string	$prefix	the prefix to use for form field names
	*
	* @return boolean
	* @access public
	*/
	function processDateTimeChooser(&$asset, &$o, $prefix)
	{
		return $this->processStartEndChooser($asset, $o, $prefix);

	}//end processDateTimeChooser()


	/**
	* Paint the interface for choosing the recurring event whose instance is being modified
	*
	* @param object	&$asset	the event to process the form for
	* @param object	&$o		the outputter class (not used here)
	* @param string	$prefix	the prefix to use for form field names
	*
	* @return void
	* @access public
	*/
	function paintRecurringEventChooser(&$asset, &$o, $prefix)
	{
		$modified_event_asset = NULL;
		if ($asset->id) {
			$recurring_event_ids = array_keys($GLOBALS['SQ_SYSTEM']->am->getParents($asset->id, 'calendar_event_recurring', FALSE));
			if (!empty($recurring_event_ids)) {
				$modified_event_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($recurring_event_ids[0]);
			}
		}
		if (!is_null($modified_event_asset)) {
			echo get_asset_tag_line($modified_event_asset->id);
		} else {
			echo translate('No event has been selected for cancellation');

		}

	}//end paintRecurringEventChooser()


	/**
	* Process the supplied recurring event value
	*
	* @param object	&$asset	the event to process the form for
	* @param object	&$o		the outputter class (not used here)
	* @param string	$prefix	the prefix to use for form field names
	*
	* @return boolean
	* @access public
	*/
	function processRecurringEventChooser(&$asset, &$o, $prefix)
	{
		// not doing anything here; the field is processed in saveAttributes() because
		// we need an ID first
		return TRUE;

	}//end processRecurringEventChooser()


}//end class

?>