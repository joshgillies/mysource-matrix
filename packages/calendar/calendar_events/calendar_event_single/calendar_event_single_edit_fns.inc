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
* $Id: calendar_event_single_edit_fns.inc,v 1.2 2004/07/28 03:30:28 lwright Exp $
* $prefix:  $
*/


require_once  dirname(__FILE__).'/../../calendar_event/calendar_event_edit_fns.inc';

/**
* Page_Edit_Fns
*
* Functions used by the edit interface of the calendar_event asset
*
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.2 $ - 1.0
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
		$this->static_screens['details']['force_unlock'] = false;

	}//end Calendar_Event_Single_Edit_Fns()


	/**
	* Paint the form for entering the start/end date/time for a single event
	*
	* Paint the form for event start/end date/time input, using
	* javascript to prevent the user from choosing
	* something invalid (obviously server-side validation
	* is done too)
	*
	* @param object Calendar_Event		&$asset			the event to print the form for
	* @param object Backend_Outputter	&$o				the outputter class (not used here)
	* @param string						$prefix			the prefix to use for form field names
	*
	* @access public
	*/
	function paintSingleStartEndChooser(&$asset, &$o, $prefix) 
	{
		if ($asset->writeAccess('attributes'))
			$this->paintStartEndChooser($asset, $prefix, '');
		else
			echo $asset->getFriendlyDatesAndTimes();

	}//end paintSingleStartEndChooser()

	/**
	* Process the start/end date/time entry form fields for a single event
	*
	* @param object Calendar_Event		&$asset			the event to process the form for
	* @param object Backend_Outputter	&$o				the outputter class (not used here)
	* @param string						$prefix			the prefix to use for form field names
	*
	* @return boolean	Whether the update was successful
	* @access public
	*/
	function processSingleStartEndChooser(&$asset, &$o, $prefix) 
	{
		$this->processStartEndChooser($asset, $o, $prefix);

	}//end processSingleStartEndChooser


}//end class

?>