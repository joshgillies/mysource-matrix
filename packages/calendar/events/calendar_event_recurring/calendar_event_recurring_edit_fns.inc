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
* $Id: calendar_event_recurring_edit_fns.inc,v 1.1.1.1 2004/07/21 04:31:53 lwright Exp $
* $Name: not supported by cvs2svn $
*/


require_once  dirname(__FILE__).'/../calendar_event/calendar_event_edit_fns.inc';

/**
* Page_Edit_Fns
*
* Purpose
*
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.1.1.1 $ - 1.0
* @package MySource_Matrix_Packages
* @subpackage calendar
*/
class Calendar_Event_Recurring_Edit_Fns extends Calendar_Event_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Calendar_Event_Recurring_Edit_Fns()
	{
		$this->Asset_Edit_Fns();

	}//end Calendar_Event_Recurring_Edit_Fns()


	/**
	* Prints the recurrence frequency field of the recurring event
	*
	* @param object Calendar_Asset_Recurring	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter			&$o		the outputter class
	* @param string								$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function printFrequencyField(&$asset, &$o, $prefix)
	{
	
	}//end printFrequencyField()


	/**
	* Processes the recurrence frequency field of the recurring event
	*
	* @param object Calendar_Asset_Recurring	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter			&$o		the outputter class
	* @param string								$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processFrequencyField(&$asset, &$o, $prefix)
	{
	
	}//end processFrequencyField()

}//end class

?>