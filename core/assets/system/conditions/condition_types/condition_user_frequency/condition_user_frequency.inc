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
* $Id: condition_user_frequency.inc,v 1.3 2005/10/13 02:07:38 tbarrett Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/conditions/condition/condition.inc';

/**
* User Frequency Condition
*
* Purpose
*    This condition returns true or false depending on the comparison between
*    its parameters and the number of times the condition has been evaluated by
*    the current user.  (It sets a cookie to keep track of this).
*
* @author  David Heppell <dheppell@squiz.net>
* @version $Revision: 1.3 $
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
	* Evaluate this condition
	*
	* Takes "units" and "value" data with units being one of "hits", "session"
	* "days", "hours" or "minutes" and a value for whichever unit is selected
	* Hits and Sessions take value as a number of times this condition
	* is resolved before returning true (Session limits to 1 per day)
	* For the rest value represents a unit of time
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
						'value'		=> 0,
						'last_date'	=> time(),
					);
		}

		// time difference between now and last update in days
		$difference = (time() - $data['last_date']) / 86400;

		// EVERY X HITS
		// Condition evaluates to true every x hits, for example when
		// x is 20, it will return true on hit 20, 40, 60... etc
		if ($condition_data['units'] == 'hits') {
			$hits = (int) $condition_data['value'];

			// if the current counter divides evenly into condition return true
			if (($data['value'] % $hits) == 0) {
				$result = true;
			} else {
				$result = false;
			}

			$data['value']++;
		}

		// EVERY X HITS LIMIT 1 PER DAY
		// same as above limit 1 hit per day
		if ($condition_data['units'] == 'session') {
			$hits = (int) $condition_data['value'];

			// if the current counter divides evenly into condition return true
			if (($data['value'] % $hits) == 0) {
				$result = true;
			} else {
				$result = false;
			}

			// we only increment the count if a day has passed
			if ((int) $idfference) $data['value']++;
		}

		// EVERY X TIME PERIOD UNITS
		// Condition evaluates true every x period of time
		// for example when x is 3 days, condition will be true
		// if more than 3 days have passed since last check
		if ($condition_data['units'] == 'days' || $condition_data['units'] == 'hours' || $condition_data['units'] == 'minutes') {
			if ($condition_data['units'] == 'hours') {
				$difference *= 60;
			}

			if ($condition_data['units'] == 'minutes') {
				$difference *= 3600;
			}

			// if more units have passed than the required difference
			if (((int) $difference) > $condition_data['value']) {
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