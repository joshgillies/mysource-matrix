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
* $Id: calendar_event_recurring.inc,v 1.1.1.1 2004/07/21 04:31:53 lwright Exp $
* $Name: not supported by cvs2svn $
*/


require_once  dirname(__FILE__).'/../calendar_event/calendar_event.inc';

/**
* Calendar_Event_Recurring
*
* Purpose
*     
*
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.1.1.1 $ - 1.0
* @package MySource_Matrix_Packages
* @subpackage calendar
*/
class Calendar_Event_Recurring extends Calendar_Event
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Calendar_Event_Recurring($assetid=0)
	{
		$this->Calendar_Event($assetid);

	}//end Calendar_Event_Recurring()


	/**
	* Determines the next occurrence of an event after a certain time
	*
	* @param int	$after	the time for which we want the first occurrence after
	*						if NULL, reverts to current server time
	*
	* @return int	a UNIX timestamp representing the next occurrence
	* @access public
	*/
	function getFirstOccurrenceAfter($after=null)
	{
		if (is_null($after)) {
			$after = time();
		}

		// try and work out what the frequency actually means

		$cron_line = $this->attr('frequency');
		$cron_array = Array();
		list($cron_array['i'], $cron_array['h'], $cron_array['d'], $cron_array['m'], $cron_array['w'], $cron_array['y']) =
			preg_split("/\s/", $cron_line, 6);

		// expand the cron values to a list of valid values fitting star criteria
		$cron_min = Array('y' => 1970, 'm' => 1,  'd' => 1);
		$cron_max = Array('y' => 2038, 'm' => 12, 'd' => 31);

		foreach(array_keys($cron_array) as $key) {
			$cron_array[$key] = $this->_parseCronLineComponent($cron_array[$key], $cron_min[$key], $cron_max[$key]);
		}



	}//end getFirstOccurrenceAfter()
	

	/**
	* Parses an individual cron line component and returns a list of valid scalar values which match
	*
	* @param int	$min	Lowest value that is valid for this component. Default=0
	* @param int	$max	Highest value that is valid for this component. Default=59

	* @return array(int)	a list of values which match the criteria given
	* @access protected
	*/
	function _parseCronLineComponent($cron_value, $min=0, $max=59)
	{
		// replace stars with min-to-max lists
		//$cron_value = str_replace('*', $min.'-'.$max, $cron_value);

		// now replace ranges (eg. 1-5) with comma-separated lists (1,2,3,4,5)
		preg_match_all('|(\d+)\-(\d+)|', $cron_value, $matches, PREG_SET_ORDER);

		foreach($matches as $match) {
			$cron_value = str_replace($match[1].'-'.$match[2],
				implode(',', range($match[1], $match[2])), $cron_value);
		}

		// split the cron value up on a slash, if there is any
		// even if there isn't, there'll still be one member of the array (a comma list)
		// which we want to explode into an array of its own
		$split_cron_value = explode('/', $split_cron_value);
		$split_cron_value[0] = explode(',', $split_cron_value[0]);

		// if there is no slash, we can skip this, there is no problem
		// but... if there is a slash, weed out those that match the 'every X minutes' rule
		if (count($split_cron_value) > 1) {
			// 
			foreach($split_cron_value[0] as $key => $value) {
				if ($value % $split_cron_value[1] != 0) {
					unset($split_cron_value[0][$key]);
				}
			}
		}

		return $split_cron_value[0];
		

	}//end _parseCronLineComponent()


}//end class

?>