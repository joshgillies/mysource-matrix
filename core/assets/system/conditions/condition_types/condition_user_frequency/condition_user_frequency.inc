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
* $Id: condition_user_frequency.inc,v 1.13 2012/08/30 01:09:10 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/conditions/condition/condition.inc';

/**
* User Frequency Condition
*
* Purpose
*    This condition returns TRUE or FALSE depending on the comparison between
*    its parameters and the number of times the condition has been evaluated by
*    the current user.  (It sets a cookie to keep track of this).
*
* @author  David Heppell <dheppell@squiz.net>
* @version $Revision: 1.13 $
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
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Evaluate this condition
	*
	* Takes "units" and "value" data with units being one of "hits", "session"
	* "days", "hours" or "minutes" and a value for whichever unit is selected
	* Hits and Sessions take value as a number of times this condition
	* is resolved before returning TRUE (Session limits to 1 per day)
	* For the rest value represents a unit of time
	*
	* @param Asset	$asset			Reference to any asset we need to work with
	* @param array	$condition_data	The data we need to evaluate this condition
	*
	* @return boolean
	* @access public
	*/
	public static function evaluate(Asset $asset, Array $condition_data, $prefix)
	{
		$result = FALSE;
		if(empty($prefix)) $prefix = '';
		
		// Retrieve the cookie or initialise it
		if (isset($_COOKIE['user_frequency_cond_'.$prefix]) && (FALSE !== strpos($_COOKIE['user_frequency_cond_'.$prefix], '__'))) {
			$data = Array();
			list($data['value'], $data['last_date']) = explode('__', $_COOKIE['user_frequency_cond_'.$prefix]);
		} else {
			$data = Array(
						'value'		=> 1,
						'last_date'	=> time(),
					);
		}

		// time difference between now and last update in days
		$difference = (time() - $data['last_date']) / 86400;
		// EVERY X HITS
		// Condition evaluates to TRUE every x hits, for example when
		// x is 20, it will return TRUE on hit 20, 40, 60... etc
		if ($condition_data['units'] == 'hits') {
			$hits = (int) $condition_data['value'];
			// if the current counter divides evenly into condition return TRUE
			if (($hits != 0) && (($data['value'] % $hits) == 0)) {
				$result = TRUE;
			} else {
				$result = FALSE;
			}
			$data['value']++;
		}

		// EVERY X HITS LIMIT 1 PER DAY
		// same as above limit 1 hit per day
		if ($condition_data['units'] == 'session') {
			$hits = (int) $condition_data['value'];

			// if the current counter divides evenly into condition return TRUE
			if (($data['value'] % $hits) == 0) {
				$result = TRUE;
			} else {
				$result = FALSE;
			}

			// we only increment the count if a day has passed
			if ((int) $difference) $data['value']++;
		}

		// EVERY X TIME PERIOD UNITS
		// Condition evaluates TRUE every x period of time
		// for example when x is 3 days, condition will be TRUE
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
				$result = TRUE;
			} else {
				$result = FALSE;
			}
		}
		// update the time, and set the cookie
		$data['last_date'] = time();
		$root_url = str_replace(Array('http://', 'https://'), '', sq_web_path('root_url'));
		$domain = $root_url;
		$path = '/';
		if (FALSE !== ($slashpos = strpos($root_url, '/'))) {
			$domain = substr($root_url, 0, $slashpos);
			$path = substr($root_url, $slashpos);
		}
		setcookie('user_frequency_cond_'.$prefix, $data['value'].'__'.$data['last_date'], strtotime('+3 months'), $path, $domain);

		return $result;

	}//end evaluate()


}//end class

?>
