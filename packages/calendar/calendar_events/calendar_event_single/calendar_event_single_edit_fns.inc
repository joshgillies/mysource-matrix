<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: calendar_event_single_edit_fns.inc,v 1.13 2008/11/24 23:17:14 mbrydon Exp $
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
* @version $Revision: 1.13 $
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