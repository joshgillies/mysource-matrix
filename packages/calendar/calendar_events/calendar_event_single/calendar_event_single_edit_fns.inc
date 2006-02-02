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
* $Id: calendar_event_single_edit_fns.inc,v 1.11 2006/02/02 03:38:32 sdanis Exp $
*
*/

require_once  dirname(__FILE__).'/../../calendar_event/calendar_event_edit_fns.inc';

/**
* Page_Edit_Fns
*
* Functions used by the edit interface of the calendar_event asset
*
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.11 $
* @package MySource_Matrix_Packages
* @subpackage calendar
*/
class Calendar_Event_Single_Edit_Fns extends Calendar_Event_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Calendar_Event_Single_Edit_Fns()
	{
		$this->Calendar_Event_Edit_Fns();
		$this->static_screens['details']['force_unlock'] = FALSE;

	}//end constructor


	/**
	* Paint the form for entering the start/end date/time for a single event
	*
	* Paint the form for event start/end date/time input, using
	* javascript to prevent the user from choosing
	* something invalid (obviously server-side validation
	* is done too)
	*
	* @param object	&$asset	the event to print the form for
	* @param object	&$o		the outputter class (not used here)
	* @param string	$prefix	the prefix to use for form field names
	*
	* @return boolean
	* @access public
	*/
	function paintSingleStartEndChooser(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess('attributes')) {
			return $this->paintStartEndChooser($asset, $prefix, Array());
		} else {
			echo $asset->getFriendlyDatesAndTimes();
			return FALSE;
		}

	}//end paintSingleStartEndChooser()


	/**
	* Process the start/end date/time entry form fields for a single event
	*
	* @param object	&$asset	the event to process the form for
	* @param object	&$o		the outputter class (not used here)
	* @param string	$prefix	the prefix to use for form field names
	*
	* @return boolean
	* @access public
	*/
	function processSingleStartEndChooser(&$asset, &$o, $prefix)
	{
		return $this->processStartEndChooser($asset, $o, $prefix);

	}//end processSingleStartEndChooser()


}//end class

?>