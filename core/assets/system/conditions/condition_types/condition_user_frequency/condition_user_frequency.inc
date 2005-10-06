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
* $Id: condition_user_frequency.inc,v 1.1 2005/10/06 01:01:40 dheppell Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/conditions/condition/condition.inc';

/**
* Condition_User_IP
*
* Purpose
*     This condition compares the IP address of the current visitor against a
*     specified comparison address
*
*
* @author  David Heppell <dheppell@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Condition_User_Frequency extends Condition
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Condition_User_Frequency($assetid=0)
	{
		$this->Condition($assetid);

	}//end constructor


	/**
	* Get list of match options
	*
	* @return array
	* @access public
	*/
	function getOptions()
	{
		$options = Array(
					'1'	=> translate('matches'),
					'0'	=> translate('does_not_match'),
				   );

		return $options;

	}//end getOptions()


	/**
	* Evaluate this condition
	*
	* when more than 1 condition is specified, all conditions must be met to return true
	* Condition data can contain several values being:
	*<pre>
	* ['area_hits']                 for the increment of how many hits until this returns true
	* ['session_hits'] and
	* ['session_units']             for the incrememnt with a limit of 1 per period
	* ['time_difference'] and
	* ['time_difference_units']     for the period and units until this returns true
	*</pre>
	*
	* @param object	&$asset			Reference to any asset we need to work with
	* @param array	$condition_data	The data we need to evaluate this condition
	*
	* @return boolean
	* @access public
	*/
	function evaluate(&$asset, $condition_data)
	{
		$result = false;

		// Retrieve the cookie or initialise it
		if (!($data = @unserialize($_COOKIE['user_frequency']))) {
			$data = Array(
						'area_hits'		=> 0,
						'session_hits'	=> 0,
						'last_date'		=> time(),
					);
		}

		// time difference between now and last update
		$difference = time() - $data['last_date'];

		// EVERY X HITS
		// Condition evaluates to true every x hits, for example when
		// x is 20, it will return true on hit 20, 40, 60... etc
		if (isset($condition_data['area_hits'])) {
			$hits = (int) $condition_data['area_hits'];

			//if the current counter divides evenly into condition return true
			if (($data['area_hits'] % $hits) == 0) {
				$result = true;
			} else {
				$result = false;
			}

			$data['area_hits']++;
		}

		// EVERY X HITS LIMIT 1 PER PERIOD
		// Condition evaluates to true every x hits, hits are only recorded
		// however once per period i.e. per day, per hour etc
		if (isset($condition_data['session_hits'])) {
			//if the current counter divides evenly into condition return true
			if (($data['session_hits'] % $condition_data['session_hits']) == 0) {
				$result = true;
			} else {
				$result = false;
			}

			// convert the difference into the appropiate unit
			if ($condition_data['session_units'] == 'days') {
				$difference = (int) $difference / 60 / 60 / 24;
			} else if ($condition_data['session_units'] == 'hours') {
				$difference = (int) $difference / 60 / 60;
			} else if ($condition_data['session_units'] == 'minutes') {
				$difference = (int) $difference / 60;
			}

			// we only increment the count if were on a different unit
			if ($idfference) $data['session_hits']++;
		}

		// EVERY X TIME PERIOD UNITS
		// Condition evaluates true every x period of time
		// for example when x is 3 days, condition will be true
		// if more than 3 days have passed since last check
		if (isset($condition_data['time_difference'])) {
			$units = $condition_data['time_difference_units'];

			// convert the difference into the appropiate unit
			if ($units == 'days') {
				$difference = (int) $difference / 60 / 60 / 24;
			} else if ($units == 'hours') {
				$difference = (int) $difference / 60 / 60;
			} else if ($units == 'minutes') {
				$difference = (int) $difference / 60;
			}

			// if more units have passed than the required difference
			if ($difference > $condition_data['time_difference']) {
				$result = true;
			} else {
				$result = false;
			}
		}

		// update the time, and set the cookie
		$data['last_date'] = time();
		setcookie('user_frequency', serialize($data));

		return $result;

	}//end evaluate()


}//end class

?>