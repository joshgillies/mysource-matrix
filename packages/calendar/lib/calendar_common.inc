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
* $Id: calendar_common.inc,v 1.2 2004/10/05 02:35:18 tbarrett Exp $
* $Name: not supported by cvs2svn $
* $Revision: 1.2 $
*
* This file contains global functions that are useful throughout the calendar module
* (but not beyond)
*/

require_once SQ_FUDGE_PATH.'/general/datetime.inc';
require_once dirname(__FILE__).'/../calendar_events/calendar_event_recurring/calendar_event_recurring.inc';

/**
* Get the events occurring in the specified time frame
*
* @param Array	$root_nodes		Root nodes to draw the events from
* @param string	$start_date		First date in timeframe, in ISO8601 format
* @param int	$window_length	Number of days ahead to look
*
* @return Array	(date => Array(id:index))
*/
function get_events_in_timeframe($root_nodes, $start_date, $window_length)
{
}//end get_events_in_timeframe




/**
* Builds the common part of the single events query
*
* @param array	&$trees	(int => string) an array of tree ids indexed by root node
*
* @return string	the common part of the SQL query
*/
function build_single_event_query_base(&$trees)
{
	$db =& $GLOBALS['SQ_SYSTEM']->db;

	return
	'SELECT a.assetid, a.type_code, a.name, ('.build_case_statement('t.treeid',$trees).') as root_node, cd.* FROM ('.SQ_TABLE_RUNNING_PREFIX.'asset a join ('.SQ_TABLE_RUNNING_PREFIX.'asset_link l join '.SQ_TABLE_RUNNING_PREFIX.'asset_link_tree t on l.linkid = t.linkid) on l.minorid = a.assetid) join '.SQ_TABLE_RUNNING_PREFIX.'calendar_date_value cd on a.assetid = cd.assetid WHERE a.type_code IN ( '.$db->quote('calendar_event_single').', '.$db->quote('calendar_event_modification').', '.$db->quote('calendar_event_cancellation').')';

}//end build_single_event_query_base()


/**
* Builds the common part of the recurring events query
*
* The ISO 8601 date is taken as passed to the calling function - this means the:
* - YYYY-MM-DD of the day being shown in day view
* - YYYY-MM-DD of the <strong>first</strong> day in week view
* - the YYYY-MM (only, no day) of the month involved in month view
* - the YYYY (only) of the year involved in the year view
*
* @param array	&$trees	(int => string) an array of tree ids indexed by root node
* @param string	$view	the type of view we are requesting (day|week|month|year|period)
* @param string	$iso	ISO 8601 date as passed to the calling function
* @param int	$length	Optional: the number of days ahead from the start date to look when in period view
*
* @return string
*/
function build_recurring_event_query_base(&$trees, $view, $iso, $length=null) 
{
	list($year,$month,$day) = sscanf(substr($iso,0,10), '%4d-%2d-%2d');
	if (!$month) $month = 12;
	if (!$day) $day = days_in_month($month, $year);

	$now = get_date_components(sprintf('%04d-%02d-%02d', $year, $month, $day));
	$wdom_list = weekday_dates_in_month($now['wday'], $month, $year);

	$db =& $GLOBALS['SQ_SYSTEM']->db;

	$sql =
	'SELECT a.assetid, a.type_code, a.name, ('.build_case_statement('t.treeid',$trees).") as root_node, cd.* \nFROM (".SQ_TABLE_RUNNING_PREFIX.'asset a join ('.SQ_TABLE_RUNNING_PREFIX.'asset_link l join '.SQ_TABLE_RUNNING_PREFIX.'asset_link_tree t on l.linkid = t.linkid) on l.minorid = a.assetid) join '.SQ_TABLE_RUNNING_PREFIX."calendar_date_value cd on a.assetid = cd.assetid \nWHERE (a.type_code = ".$db->quote('calendar_event_recurring').") \nAND ";

	$sql_freqs = Array();

	switch($view) {
		case 'day' :
			$last_day = $now['ds'];
			$now['month_length'] = days_in_month($month, $year);
			$now['month_start_wday'] = date('w', strtotime(sprintf('%04d-%02d-%02d', $year, $month, 1)));
			$now['month_end_wday'] = date('w', strtotime(sprintf('%04d-%02d-%02d', $year, $month, $now['month_length'])));
			$prev_month_length = date('t', strtotime('-1 month', $now['ts']));
			$prev_month_start_wday = date('w', strtotime(sprintf('%04d-%02d-%02d', $year, $month, 1).' -1 month'));
			$prev_month_end_wday = date('w', strtotime(sprintf('%04d-%02d-%02d', $year, $month, 1).' -1 day'));
			if ($db->phptype == 'mysql') {
				$make_timestamp_func = create_function('$y, $m, $d', 'return "UNIX_TIMESTAMP(CONCAT($y,\'-\',$m,\'-\',$d))";');
			} else {
				$make_timestamp_func = create_function('$y, $m, $d', 'return "EXTRACT(epoch from DATE (\'$y\' || \'-\' || \'$m\' || \'-\' || \'$d\'))";');
			}

			if (($now['wday'] != 0) && ($now['wday'] != 6)) {
				$sql_freqs[] = 'frequency = '.$db->quote('DWD');
			}

			$sql_freqs[] = '(frequency = '.$db->quote('DED').' AND mod(('.$db->quote($now['ds']).' - start_date_ds), period) BETWEEN 0 and coalesce(end_date_ds - start_date_ds,0))';

			$sql_freqs[] = '(frequency = '.$db->quote('WEW').' AND mod(('.$db->quote($now['ds']).' - start_date_ds), (period*7)) BETWEEN 0 and coalesce(end_date_ds - start_date_ds,0))';

			$sql_freqs[] = '
			(SUBSTRING(frequency FROM 1 FOR 1) = '.$db->quote('M').') AND (
				(
					/* there is an instance that starts in our month */
				   ((('.$db->quote($now['ms']).' - start_date_ms) % period) = 0)
				   AND (
						/* our view date is between the start and end of the instance */
						((frequency = '.$db->quote('MFN').') AND (('.$db->quote($now['mday']).' - start_date_mday) BETWEEN 0 AND (coalesce((end_date_ds - start_date_ds), 0))))
						OR ((frequency = '.$db->quote('MRN').') AND ((start_date_rmday - '.$db->quote($now['rmday']).') BETWEEN 0 AND (coalesce((end_date_ds - start_date_ds), 0))))
						OR ((frequency = '.$db->quote('MFW').') AND (('.$db->quote($now['mday']).' - (((start_date_wdom - 1) * 7) + ((start_date_wday + 7 - '.$now['month_start_wday'].') % 7) + 1)) BETWEEN 0 AND (coalesce((end_date_ds - start_date_ds), 0))))
						OR ((frequency = '.$db->quote('MRW').') AND ((((start_date_rwdom - 1) * 7) + (('.($now['month_end_wday'] + 7).' - start_date_wday) % 7) + 1 - '.$now['rmday'].') BETWEEN 0 AND (coalesce((end_date_ds - start_date_ds), 0))))
				  )
				) OR (
						/* An instance that started in a previous month doesnt end til a subsequent month */
						((('.$now['ms'].' - start_date_ms) % period) < (('.$now['ms'].' - end_date_ms) % period))
				) OR (
					/* An instance ends this month */
					((start_date_ms != end_date_ms) AND ((('.$now['ms'].' - end_date_ms) % period) = 0))
				   AND (
					   /* its end is PROBABLY after our view date - we can only be ~90% sure here */
						((frequency = '.$db->quote('MFN').') 
						   AND ((end_date_ds - start_date_ds + 1) >= ROUND(('.$db->quote($now['ts']).' - '.$make_timestamp_func('FLOOR((('.$now['year'].' * 12) + '.($now['mon']-1).' - (end_date_ms - start_date_ms)) / 12)', '(('.$now['mon'].' + 10 - (('.($now['ms']-1).' - start_date_ms) % period)) % 12) + 1', 'start_date_mday').') / 86400)))
						OR ((frequency = '.$db->quote('MRN').')
							AND ((end_date_ds - start_date_ds + 1) >= ROUND(('.$db->quote($now['ts']).' - '.$make_timestamp_func('FLOOR((('.$now['year'].' * 12) + '.($now['mon']-1).' - (end_date_ms - start_date_ms)) / 12)', '(('.$now['mon'].' + 10 - (('.($now['ms']-1).' - start_date_ms) % period)) % 12) + 1', '((start_date_mday + 3) - (FLOOR((start_date_mday + 3) / 31) * ((start_date_mday + 3) % 31)))').') / 86400)))
						OR ((frequency = '.$db->quote('MFW').')
							AND ((end_date_ds - start_date_ds + 1) >= ROUND(('.$db->quote($now['ts']).' - '.$make_timestamp_func('FLOOR((('.$now['year'].' * 12) + '.($now['mon']-1).' - (end_date_ms - start_date_ms)) / 12)', '(('.$now['mon'].' + 10 - (('.($now['ms']-1).' - start_date_ms) % period)) % 12) + 1', '(7 * start_date_wdom) - (FLOOR((7 * start_date_wdom) / 31) * ((7 * start_date_wdom) % 31))').') / 86400)))
						OR ((frequency = '.$db->quote('MRW').')
							AND ((end_date_ds - start_date_ds + 1) >= ROUND(('.$db->quote($now['ts']).' - '.$make_timestamp_func('FLOOR((('.$now['year'].' * 12) + '.($now['mon']-1).' - (end_date_ms - start_date_ms)) / 12)', '(('.$now['mon'].' + 10 - (('.($now['ms']-1).' - start_date_ms) % period)) % 12) + 1', '(31 - ((start_date_rwdom - 1) * 7))').') / 86400)))
					)
				) OR (
						/* An instance that started last month has run into our month 
						   even though its original instance was contained in only one month
						   and it ends on/after our view date */
						((('.$now['ms'].' - start_date_ms) % period) = 1)
						   AND (
							((frequency = '.$db->quote('MFN').') 
								AND ((start_date_mday + (end_date_ds - start_date_ds)) - '.$prev_month_length.' >= '.$now['mday'].'))
							OR ((frequency = '.$db->quote('MFD').')
								AND (((((7 + '.$prev_month_start_wday.' - start_date_wday) % 7) + 1) + ((start_date_wdom - 1) * 7) + (end_date_ds - start_date_ds)) - '.$prev_month_length.' >= '.$now['mday'].'))
							OR ((frequency = '.$db->quote('MRD').')
									AND ((('.$prev_month_length.' - ((7 + '.$prev_month_end_wday.' - start_date_wday) % 7)) + (end_date_ds - start_date_ds)) - '.$prev_month_length.' >= '.$now['mday'].'))
							)
				 )
			)';
			
		break;

		case 'week' :
			// if we go into next week, we must put the next week's stuff into it
			// if we go into next month, we must put the 1st week into it
			$days_in_month = days_in_month($month,$year);

			$req_next_week = ($day % 7 != 1);
			$req_rev_next_week = (($days_in_month - $day + 1) % 7 != 1);
			$req_next_month = ($day + 6 > $days_in_month);

			$last_day = $now['ds'] + 6;

			$sql_freqs[] = 'frequency = '.$db->quote('DWD');

			// note: the slightly redundant "*1.0" is so Postgres interprets it as a
			// double instead of an int, blowing away our attempts to use floor()
			$sql_freqs[] = '(frequency = '.$db->quote('DED').' AND ((floor(('.$db->quote($now['ds']-1).' - start_date_ds) / (period*1.0)) < floor(('.$db->quote($now['ds']+6).' - start_date_ds) / (period*1.0))) or mod(('.$db->quote($now['ds']).' - start_date_ds), period) BETWEEN 0 and coalesce(end_date_ds - start_date_ds,0)))';

			$sql_freqs[] = '(frequency = '.$db->quote('WEW').' AND ((floor(('.$db->quote($now['ds']-1).' - start_date_ds)/(period*7.0)) < floor(('.$db->quote($now['ds']+6).' - start_date_ds)/(period*7.0))) or mod(('.$db->quote($now['ds']).' - start_date_ds), period) BETWEEN 0 and coalesce(end_date_ds - start_date_ds,0)))';

			$sql_freqs_m = Array();

			// every X months on xth of the month
			$sql_freqs_m[] = '(frequency = '.$db->quote('MFN').' AND start_date_mday BETWEEN '.$db->quote(intval($day)).' AND '. $db->quote(min($day+6, $days_in_month)) .')';

			// every X months on xth weekday of the month
			$sql_freqs_m[] = '(frequency = '.$db->quote('MFW').' AND start_date_wdom IN ( '.$db->quote($now['wdom']).($req_next_week ? ','.$db->quote($now['wdom']+1) : '').'))';

			// every X months on xth last day of the month
			$sql_freqs_m[] = '(frequency = '.$db->quote('MRN').' AND start_date_rmday BETWEEN '.$db->quote(max($days_in_month - $day - 5, 1)).' AND '.$db->quote(intval($days_in_month - $day + 1)).')';

			// every X months on xth last weekday of the month
			$sql_freqs_m[] = '(frequency = '.$db->quote('MRW').' AND start_date_rwdom IN ( '.$db->quote($now['rwdom']).($req_rev_next_week ? ','.$db->quote($now['rwdom']-1) : '').'))';

			$sql_freqs[] = '(('.implode("\n\t".' OR ', $sql_freqs_m).') AND (MOD('.$db->quote($now['ms']).' - start_date_ms,period) = 0))';

			// rolling over to a new month? Oh crap :o
			if($req_next_month) {
				if ($month == 12) {
					$new_month = 1;
					$new_year = $year + 1;
				} else {
					$new_month = $month + 1;
					$new_year = $year;
				}

				$req_rev_next_week = (($day + 28) > days_in_month($new_month, $new_year));
				$wdom_list = weekday_dates_in_month($now['wday'], $new_month, $new_year);
				$days_in_new_month = days_in_month($new_month,$new_year);

				// wipe the table clean
				$sql_freqs_m = Array();

				$sql_freqs_m[] = '(frequency = '.$db->quote('MFN').' AND start_date_mday BETWEEN '.$db->quote(1).' AND '. $db->quote($day+6-$days_in_month).')';

				// every X months on xth weekday of the month - it's always going to
				// be the first week of a new month
				$sql_freqs_m[] = '(frequency = '.$db->quote('MFW').' AND start_date_wdom = 1)';

				// every X months on xth last day of the month
				$sql_freqs_m[] = '(frequency = '.$db->quote('MRN').' AND start_date_rmday BETWEEN '.$db->quote($days_in_month * 2 - $day - 5).' AND '.$db->quote(intval($days_in_month)).')';

				// every X months on xth last weekday of the month
				$sql_freqs_m[] = '(frequency = '.$db->quote('MRW').' AND start_date_rwdom IN ( '.$db->quote(count($wdom_list)).($req_rev_next_week ? ','.$db->quote(count($wdom_list)-1) : '').'))';

				$sql_freqs[] = '(('.implode("\n\t".' OR ', $sql_freqs_m).') AND (MOD('.$db->quote($now['ms']+1).' - start_date_ms,period) = 0))';
			}

			$sql_freqs[] = '(frequency IN ('.$db->quote('MFN').', '.$db->quote('MRN').') AND (end_date_ds - start_date_ds >= '.$day.'))';

			$sql_freqs[] = '(frequency IN ('.$db->quote('MFW').', '.$db->quote('MRW').') AND (end_date_ds - start_date_ds >= 1 + ((start_date_rwdom-1) * 7)))';

		break;

		case 'month' :
			$last_day = $now['ds'] + days_in_month($month,$year) - 1;

			$sql_freqs[] = '(frequency = '.$db->quote('DWD').')';

			$sql_freqs[] = '(frequency = '.$db->quote('DED').' AND (floor(('.$db->quote($now['ds']-1).' - start_date_ds)/period) < floor(('.$db->quote($last_day).' - start_date_ds)/period)))';

			$sql_freqs[] = '(frequency = '.$db->quote('WEW').' and period = 1)';

			$sql_freqs[] = '(frequency = '.$db->quote('WEW').' AND (floor(('.$db->quote($now['ds']-1).' - start_date_ds)/(period*7)) < floor('.$db->quote($last_day).' - start_date_ds)/(period*7)))';

			// all monthly views can be lumped together on this one
			$sql_freqs[] = '(frequency LIKE '.$db->quote('M%').' AND (MOD(('.$db->quote($now['ms']).' - start_date_ms), period) = 0))';

			$iso = $year.'-'.sprintf('%02d', $month).'-01';

		break;

		case 'year' :
			$last_day = $now['ds']+364+is_leap_year($year);

			$sql_freqs[] = 'frequency = '.$db->quote('DWD');

			$sql_freqs[] = '(frequency = '.$db->quote('DED').' AND (floor(('.$db->quote($now['ds']-1). ' - start_date_ds)/period) < floor(('.$db->quote($last_day).' - start_date_ds)/period)))';

			$sql_freqs[] = '(frequency = '.$db->quote('WEW').' and period = 1)';

			$sql_freqs[] = '(frequency = '.$db->quote('WEW').' AND (floor(('.$db->quote($now['ds']-1).' - start_date_ds)/(period*7)) < floor(('.$db->quote($last_day).' - start_date_ds)/(period*7))))';

			// all monthly views can be lumped together on this one
			// but we need to do something like the 'every day' type thing to work out
			// whether an occurrence will happen this year
			$sql_freqs[] = '(frequency like '.$db->quote('M%').' AND (floor(('.($year * 12).' - start_date_ms) / period) < floor(('.(($year + 1) * 12).' - start_date_ms) / period)))';
		break;

		case 'period':
			$last_day = date('Y-m-d', strtotime("$iso 12:00:00 +$length days"));


		break;

	}//end switch on $view

	$sql .= '('.implode("\n\n OR \n\n ", $sql_freqs).')';

	// stop date
	$sql .= ' AND ('.$last_day.' >= start_date_ds)';
	$sql .= ' AND (stop_date = '.$db->quote('----------').' OR stop_date >= '.$db->quote($iso).')';

	return $sql;

}//end build_recurring_event_query_base()


/**
* Returns an SQL fragment to restrict results to have treeids in the given range
* @param array	&$treeids		array of treeids
*
* @return string	the SQL clause
*/
function build_tree_clause($treeids)
{
	$db =& $GLOBALS['SQ_SYSTEM']->db;
	$tree_options = Array();
	foreach($treeids as $treeid) {
		$tree_options[] .= '(SUBSTRING(t.treeid from 1 for '.strlen($treeid).') LIKE '.$db->quote($treeid.'%').')';
	}
	return '('.implode(' OR ', $tree_options).')';

}//end build_tree_clause()


/**
* Returns the treeids that stem from the root nodes passed to it
*
* @param array	&$root_nodes	array of asset ids which represent root nodes
*
* @return array(int => string) a list of tree branch ids indexed by root assetid
*/
function &get_treeids(&$root_nodes) 
{
	$db =& $GLOBALS['SQ_SYSTEM']->db;
	$quoted_root_nodes = array_keys($root_nodes);
	$sql = "SELECT a.assetid, t.treeid \n FROM sq_asset a JOIN (sq_asset_link l JOIN sq_asset_link_tree t ON l.linkid = t.linkid) ON l.minorid = a.assetid \n WHERE a.assetid IN (";

	// quote the root nodes manually before we implode the thing
	foreach($quoted_root_nodes as $k => $root_node) {
		$quoted_root_nodes[$k] = $db->quote($quoted_root_nodes[$k]);
	}
	$sql .= implode(', ', $quoted_root_nodes).')';

	$trees = $db->getAssoc($sql);
	assert_valid_db_result($trees);
	return $trees;

}//end get_treeids()


/**
* Generates an SQL CASE statement for mapping one set of values to another
*
* @param string	$col_name	the column to map the values of
* @param array	&$trees		array of (return_values => column_values)
*
* @return string
*/
function build_case_statement($col_name, &$trees) 
{
	$db =& $GLOBALS['SQ_SYSTEM']->db;
	$sql = 'CASE';
	foreach($trees as $root_node => $treeid) {
		$sql .= ' WHEN '.$col_name.' LIKE '.$db->quote($treeid.'%').' THEN '.$db->quote($root_node);
	}
	$sql .= ' ELSE 0';
	$sql .= ' END';
	return $sql;

}//end build_case_statement()


/**
* Expands list of multiple day and/or recurring events into a series of single-day events
*
* Returned list of events is indexed by a shadow-asset-like identifier of
* 'assetid:instance' where instance is a 0-based index of how many times an instance
* the event has occurred previously.
*
* @param string	$first_day		The first day we want to display (YYYY-MM-DD)
* @param string	$last_day		The last day we want to display (YYYY-MM-DD)
* @param string	$day_start_time	Earliest time of day we will display (HH:mm)
* @param string	$day_end_time	Latest time of day we will display (HH:mm)
*
* @return array
*/
function &expand_event_list(&$events, $first_day, $last_day, $day_start_time='00:00', $day_end_time='23:59') 
{
	$res = Array();
	foreach(array_keys($events) as $key) {
		$event =& $events[$key];
		if ($event['type_code'] == 'calendar_event_recurring') {
			expand_recurring_event($res, $event, $key, $first_day, $last_day, $day_start_time, $day_end_time);
		} else {
			expand_single_event($res, $event, $key, $first_day, $last_day, $day_start_time, $day_end_time);
		}
	}
	return $res;

}//end expand_event_list()


/**
* Expands a multiple-day single event into a series of single-day events
*
* While this function can easily be called from anywhere, it
* is RECOMMENDED that this only be called from expand_event_list().
*
* @param array	&$result_list	The result array to append the expanded events to
* @param array	&$event			The cached calendar components of the event to expand
*								(format as per structure of calendar_date_value table)
* @param int	$assetid		The asset ID of the event being expanded
* @param string	$first_day		The first day we want to display (YYYY-MM-DD)
* @param string	$last_day		The last day we want to display (YYYY-MM-DD)
* @param string	$day_start_time	Earliest time of day we will display (HH:mm)
* @param string	$day_end_time	Latest time of day we will display (HH:mm)
*
* @return void
*/
function expand_single_event(&$result_list, &$event, $assetid, $first_day, $last_day, $day_start_time, $day_end_time) 
{
	$event_start_date = sprintf('%04d-%02d-%02d', $event['start_date_year'], $event['start_date_mon'], $event['start_date_mday']);
	$event_end_date = sprintf('%04d-%02d-%02d', $event['end_date_year'], $event['end_date_mon'], $event['end_date_mday']);

	$first_day_ts = iso8601_ts($first_day.' 00:00:00');
	$last_day_ts = iso8601_ts($last_day.' 00:00:00');

	list($day_start_hours, $day_start_minutes) = sscanf($day_start_time, '%02d:%02d');
	list($day_end_hours, $day_end_minutes) = sscanf($day_end_time, '%02d:%02d');
	$i = 0;

	if (!is_null($event['end_date_ds'])) {
		// end date is specified
		if ($event['start_date_ds'] != $event['end_date_ds']) {
			// multiple day
			$start_time = mktime(12, 0, 0, $event['start_date_mon'], $event['start_date_mday'], $event['start_date_year']);
			$start_date = date('Y-m-d', $start_time);

			$end_time = mktime(12, 0, 0, $event['end_date_mon'], $event['end_date_mday'], $event['end_date_year']);
			$end_date = date('Y-m-d', $end_time);

			if ($first_day <= $event_start_date) {
				// insert first day
				$result_list[$assetid.':'.$i] = Array();
				for(reset($event); null !== ($k = key($event)); next($event)) {
					$result_list[$assetid.':'.$i][$k] = $event[$k];
				}
				$result_list[$assetid.':'.$i]['start_date_year'] = date('Y', $start_time);
				$result_list[$assetid.':'.$i]['start_date_mon']  = date('n', $start_time);
				$result_list[$assetid.':'.$i]['start_date_mday'] = date('j', $start_time);
				$result_list[$assetid.':'.$i]['end_date_year']   = date('Y', $start_time);
				$result_list[$assetid.':'.$i]['end_date_mon']    = date('n', $start_time);
				$result_list[$assetid.':'.$i]['end_date_mday']   = date('j', $start_time);
				$result_list[$assetid.':'.$i] = array_merge($event, Array(
					'start_date_year' => date('Y', $start_time), 'start_date_mon' => date('n', $start_time),
					'start_date_mday' => date('j', $start_time),
					'end_date_year' => date('Y', $start_time), 'end_date_mon' => date('n', $start_time),
					'end_date_mday' => date('j', $start_time)));
					if (!is_null($result_list[$assetid.':'.$i]['start_date_hours'])) {
						$result_list[$assetid.':'.$i]['end_date_hours'] = $day_end_hours;
						$result_list[$assetid.':'.$i]['end_date_minutes'] = $day_end_minutes;
					}
				$i++;
			}

			// insert intermediate days
			$inter_date = date('Y-m-d', strtotime(max(add_days_to_iso($event_start_date, 1), $first_day),' +1 day'));
			$last_inter_date = min($last_day, add_days_to_iso($event_end_date, -1));
			while ($inter_date <= $last_inter_date) {
				$inter_time = iso8601_ts($inter_date.' 12:00:00');
				$result_list[$assetid.':'.$i] = Array();
				for(reset($event); null !== ($k = key($event)); next($event)) {
					$result_list[$assetid.':'.$i][$k] = $event[$k];
				}
				$result_list[$assetid.':'.$i]['start_date_year'] = date('Y', $inter_time);
				$result_list[$assetid.':'.$i]['start_date_mon']  = date('n', $inter_time);
				$result_list[$assetid.':'.$i]['start_date_mday'] = date('j', $inter_time);
				$result_list[$assetid.':'.$i]['end_date_year']   = date('Y', $inter_time);
				$result_list[$assetid.':'.$i]['end_date_mon']    = date('n', $inter_time);
				$result_list[$assetid.':'.$i]['end_date_mday']   = date('j', $inter_time);

				if (!is_null($result_list[$assetid.':'.$i]['start_date_hours'])) {
					$result_list[$assetid.':'.$i]['start_date_hours'] = $day_start_hours;
					$result_list[$assetid.':'.$i]['start_date_minutes'] = $day_start_minutes;
					$result_list[$assetid.':'.$i]['end_date_hours'] = $day_end_hours;
					$result_list[$assetid.':'.$i]['end_date_minutes'] = $day_end_minutes;
				}
				$inter_date = add_days_to_iso($inter_date, 1);
				$i++;
			}

			if ($event_end_date <= $last_day) {
				// insert the last day
				$result_list[$assetid.':'.$i] = Array();
				for(reset($event); null !== ($k = key($event)); next($event)) {
					$result_list[$assetid.':'.$i][$k] = $event[$k];
				}

				$result_list[$assetid.':'.$i]['start_date_year'] = date('Y', $end_time);
				$result_list[$assetid.':'.$i]['start_date_mon']  = date('n', $end_time);
				$result_list[$assetid.':'.$i]['start_date_mday'] = date('j', $end_time);
				$result_list[$assetid.':'.$i]['end_date_year']   = date('Y', $end_time);
				$result_list[$assetid.':'.$i]['end_date_mon']    = date('n', $end_time);
				$result_list[$assetid.':'.$i]['end_date_mday']   = date('j', $end_time);					
				if (!is_null($result_list[$assetid.':'.$i]['start_date_hours'])) {
					$result_list[$assetid.':'.$i]['start_date_hours'] = $day_start_hours;
					$result_list[$assetid.':'.$i]['start_date_minutes'] = $day_start_minutes;
				}
			}
			$i++;

		} else {
			$start_time = mktime(12, 0, 0, $event['start_date_mon'], $event['start_date_mday'], $event['start_date_year']);
			$start_date = date('Y-m-d', $start_time);
			// single day
			$result_list[$assetid.':'.$i] = Array();
			for(reset($event); null !== ($k = key($event)); next($event)) {
				$result_list[$assetid.':'.$i][$k] = $event[$k];
			}
			
			$result_list[$assetid.':'.$i]['start_date_year'] = date('Y', $start_time);
			$result_list[$assetid.':'.$i]['start_date_mon']  = date('n', $start_time);
			$result_list[$assetid.':'.$i]['start_date_mday'] = date('j', $start_time);
			$result_list[$assetid.':'.$i]['end_date_year']   = date('Y', $start_time);
			$result_list[$assetid.':'.$i]['end_date_mon']    = date('n', $start_time);
			$result_list[$assetid.':'.$i]['end_date_mday']   = date('j', $start_time);
			
			$i++;
		}
	} else {
		$start_time = mktime(12, 0, 0,  $event['start_date_mon'], $event['start_date_mday'], $event['start_date_year']);
		$start_date = date('Y-m-d', $start_time);
		// single day in any case
		$result_list[$assetid.':'.$i] = Array();
		for (reset($event); null !== ($k = key($event)); next($event)) {
			$result_list[$assetid.':'.$i][$k] = $event[$k];
		}
		
		$result_list[$assetid.':'.$i]['start_date_year'] = date('Y', $start_time);
		$result_list[$assetid.':'.$i]['start_date_mon']  = date('n', $start_time);
		$result_list[$assetid.':'.$i]['start_date_mday'] = date('j', $start_time);
		$result_list[$assetid.':'.$i]['end_date_ds']     = round(iso8601_ts($start_time) / 86400);
		$result_list[$assetid.':'.$i]['end_date_year']   = date('Y', $start_time);
		$result_list[$assetid.':'.$i]['end_date_mon']    = date('n', $start_time);
		$result_list[$assetid.':'.$i]['end_date_mday']   = date('j', $start_time);

		$i++;

	}

}//end expand_single_event()


/**
* Expands a list of recurring events into a series of single-day events
*
* While this function can easily be called from anywhere, it
* is RECOMMENDED that this only be called from expand_event_list().
*
* @param array	&$result_list	The result array to append the expanded events to
* @param array	&$event			The cached calendar components of the event to expand
*								(format as per structure of calendar_date_value table)
* @param int	$assetid		The asset ID of the event being expanded
* @param string	$first_day		The first day we want to display (YYYY-MM-DD)
* @param string	$last_day		The last day we want to display (YYYY-MM-DD)
* @param string	$day_start_time	Earliest time of day we will display (HH:mm)
* @param string	$day_end_time	Latest time of day we will display (HH:mm)

* @return void
*/
function expand_recurring_event(&$result_list, &$event, $assetid, $first_day, $last_day, $day_start_time, $day_end_time) 
{
	$event_start_date = sprintf('%04d-%02d-%02d', $event['start_date_year'], $event['start_date_mon'], $event['start_date_mday']);
	$event_end_date = sprintf('%04d-%02d-%02d', $event['end_date_year'], $event['end_date_mon'], $event['end_date_mday']);

	$event_last_day = $last_day;
	if (($event['stop_date'] != '----------') && ($event['stop_date'] < $last_day)) {
		$event_last_day = $event['stop_date'];
	}

	switch($event['frequency']{0}) {
		case 'D':
			$first_occur = add_days_to_iso($first_day, -$event['period']);
		break;

		case 'W':
			$first_occur = add_days_to_iso($first_day, -$event['period']*7);
		break;

		case 'M':
			$first_occur = add_months_to_iso($first_day, -$event['period']).'-'.date('d', $first_day);
		break;

	}

	// if multiple day event, subtract a further <event length> days so it catches those
	// events occurring before last recurrence but long enough to extend into this period
	if (!is_null($event['end_date_ds'])) {
		$first_occur = add_days_to_iso($first_day, -($event['end_date_ds']-$event['start_date_ds']));
	}

	$next_occur = Calendar_Event_Recurring::getFirstOccurrenceAfter($first_occur, $event_start_date, $event['frequency'], $event['period'], $event_last_day);
	$i = 0;

	while (($next_occur !== false) && ($next_occur <= $last_day)) {

		$next_occur_ds = round(iso8601_ts($next_occur) / 86400);
		// days between occurrences
		$difference = $next_occur_ds - $event['start_date_ds'];

		if (!is_null($event['end_date_ds'])) {
			// end date is specified
			if ($event['start_date_ds'] != $event['end_date_ds']) {

				// multiple day
				$start_time = mktime(12, 0, 0, $event['start_date_mon'], $event['start_date_mday'], $event['start_date_year'])+$difference*86400;
				$start_date = date('Y-m-d', $start_time);

				$end_time = mktime(12, 0, 0, $event['end_date_mon'], $event['end_date_mday'], $event['end_date_year'])+$difference*86400;
				$end_date = date('Y-m-d', $end_time);

				if (($start_date >= $first_day) && ($start_date <= $last_day)) {
					$result_list[$assetid.':'.$i] = Array();
					for(reset($event); null !== ($k = key($event)); next($event)) {
						$result_list[$assetid.':'.$i][$k] = $event[$k];
					}
					
					$result_list[$assetid.':'.$i]['start_date_year'] = date('Y', $start_time);
					$result_list[$assetid.':'.$i]['start_date_mon']  = date('n', $start_time);
					$result_list[$assetid.':'.$i]['start_date_mday'] = date('j', $start_time);
					$result_list[$assetid.':'.$i]['end_date_year']   = date('Y', $start_time);
					$result_list[$assetid.':'.$i]['end_date_mon']    = date('n', $start_time);
					$result_list[$assetid.':'.$i]['end_date_mday']   = date('j', $start_time);
				
					if (!is_null($event['end_date_hours'])) {
						list($result_list[$assetid.':'.$i]['end_date_hours'], $result_list[$assetid.':'.$i]['end_date_minutes']) = explode(':', $day_end_time);
						$result_list[$assetid.':'.$i]['end_date_ts'] = iso8601_ts($start_date.' '.$day_end_time.':00');
					}
					$i++;
				
				}

				for ($inter_time = $start_time + 86400; $inter_time < $end_time; $inter_time += 86400) {
					$inter_date = date('Y-m-d', $inter_time);
					if (($inter_date >= $first_day) && ($inter_date <= $last_day)) {
					
						$result_list[$assetid.':'.$i] = Array();
						for(reset($event); null !== ($k = key($event)); next($event)) {
							$result_list[$assetid.':'.$i][$k] = $event[$k];
						}

						$result_list[$assetid.':'.$i]['start_date_year'] = date('Y', $inter_time);
						$result_list[$assetid.':'.$i]['start_date_mon']  = date('n', $inter_time);
						$result_list[$assetid.':'.$i]['start_date_mday'] = date('j', $inter_time);
						$result_list[$assetid.':'.$i]['end_date_year']   = date('Y', $inter_time);
						$result_list[$assetid.':'.$i]['end_date_mon']    = date('n', $inter_time);
						$result_list[$assetid.':'.$i]['end_date_mday']   = date('j', $inter_time);
						$result_list[$assetid.':'.$i]['partition_start']   = $day_start_time;
				
						if (!is_null($event['start_date_hours'])) {
							list($result_list[$assetid.':'.$i]['start_date_hours'], $result_list[$assetid.':'.$i]['start_date_minutes']) = explode(':', $day_start_time);
							$result_list[$assetid.':'.$i]['start_date_ts'] = iso8601_ts($inter_date.' '.$day_start_time.':00');
						}
						if (!is_null($event['end_date_hours'])) {
							list($result_list[$assetid.':'.$i]['end_date_hours'], $result_list[$assetid.':'.$i]['end_date_minutes']) = explode(':', $day_end_time);
							$result_list[$assetid.':'.$i]['end_date_ts'] = iso8601_ts($inter_date.' '.$day_end_time.':00');
						}
						$i++;
					}
				}

				if (($end_date >= $first_day) && ($end_date <= $last_day)) {
					$result_list[$assetid.':'.$i] = Array();
					for(reset($event); null !== ($k = key($event)); next($event)) {
						$result_list[$assetid.':'.$i][$k] = $event[$k];
					}
					
					$result_list[$assetid.':'.$i]['start_date_year'] = date('Y', $end_time);
					$result_list[$assetid.':'.$i]['start_date_mon']  = date('n', $end_time);
					$result_list[$assetid.':'.$i]['start_date_mday'] = date('j', $end_time);
					$result_list[$assetid.':'.$i]['end_date_year']   = date('Y', $end_time);
					$result_list[$assetid.':'.$i]['end_date_mon']    = date('n', $end_time);
					$result_list[$assetid.':'.$i]['end_date_mday']   = date('j', $end_time);
					$result_list[$assetid.':'.$i]['partition_start'] = $day_start_time;

					if (!is_null($event['start_date_hours'])) {
						list($result_list[$assetid.':'.$i]['start_date_hours'], $result_list[$assetid.':'.$i]['start_date_minutes']) = explode(':', $day_start_time);
						$result_list[$assetid.':'.$i]['start_date_ts'] = iso8601_ts($end_date.' '.$day_start_time.':00');
					}

					$i++;
				}

			} else {
				$start_time = mktime(12, 0, 0, $event['start_date_mon'], $event['start_date_mday'], $event['start_date_year'])+$difference*86400;
				$start_date = date('Y-m-d', $start_time);
				// single day
				if (($start_date >= $first_day) && ($start_date <= $last_day)) {
					$result_list[$assetid.':'.$i] = Array();
					for(reset($event); null !== ($k = key($event)); next($event)) {
						$result_list[$assetid.':'.$i][$k] = $event[$k];
					}

					$result_list[$assetid.':'.$i]['start_date_year'] = date('Y', $start_time);
					$result_list[$assetid.':'.$i]['start_date_mon']  = date('n', $start_time);
					$result_list[$assetid.':'.$i]['start_date_mday'] = date('j', $start_time);
					$result_list[$assetid.':'.$i]['end_date_ds']     = round(iso8601_ts($start_time) / 86400);
					$result_list[$assetid.':'.$i]['end_date_year']   = date('Y', $start_time);
					$result_list[$assetid.':'.$i]['end_date_mon']    = date('n', $start_time);
					$result_list[$assetid.':'.$i]['end_date_mday']   = date('j', $start_time);

					$i++;
				}
			}
		} else {
			$start_time = mktime(12, 0, 0,  $event['start_date_mon'], $event['start_date_mday'], $event['start_date_year'])+$difference*86400;
			$start_date = date('Y-m-d', $start_time);
			// single day in any case
			
			if (($start_date >= $first_day) && ($start_date <= $last_day)) {
				$result_list[$assetid.':'.$i] = Array();
				for(reset($event); null !== ($k = key($event)); next($event)) {
					$result_list[$assetid.':'.$i][$k] = $event[$k];
				}
				
				$result_list[$assetid.':'.$i]['start_date_year'] = date('Y', $start_time);
				$result_list[$assetid.':'.$i]['start_date_mon']  = date('n', $start_time);
				$result_list[$assetid.':'.$i]['start_date_mday'] = date('j', $start_time);
				$result_list[$assetid.':'.$i]['end_date_ds']     = round(iso8601_ts($start_time) / 86400);
				$result_list[$assetid.':'.$i]['end_date_year']   = date('Y', $start_time);
				$result_list[$assetid.':'.$i]['end_date_mon']    = date('n', $start_time);
				$result_list[$assetid.':'.$i]['end_date_mday']   = date('j', $start_time);

				$i++;
			}

		}
		// get the next occurrence
		$next_occur = Calendar_Event_Recurring::getFirstOccurrenceAfter(add_days_to_iso($next_occur, 1), $event_start_date, $event['frequency'], $event['period'], $event_last_day);

	}//end while

}//end expand_recurring_event()


/**
* Process exception events (cancellations and modifications) from the set of events passed
*
* When a cancellation or modification event is found in the set, its parent recurring event
* is removed from the set, and for cancellation events they themselves are removed too
*
* @param array	$res					a ($id => array(...)) array of events
*
* @return void
*/
function process_exceptions(&$res)
{
	$exception_ids = Array();
	foreach ($res as $id => $details) {
		if (in_array($details['type_code'], Array('calendar_event_cancellation', 'calendar_event_modification'))) {
			$id_bits = explode(':', $id);
			$real_id = $id_bits[0];
			$exception_ids[] = $GLOBALS['SQ_SYSTEM']->db->quote($real_id);
			if ($details['type_code'] == 'calendar_event_cancellation') unset($res[$id]);
		}
	}
	if (empty($exception_ids)) return false;
	$exception_links = $GLOBALS['SQ_SYSTEM']->db->getAssoc('SELECT majorid, minorid FROM sq_asset_link WHERE minorid IN ('.implode(', ',	$exception_ids).')');
	foreach ($res as $key => $details) {
		$id = explode(':', $key);
		$id = $id[0];
		if (isset($exception_links[$id])) {
			unset($res[$key]);
		}
	}
}//end process_exceptions()


/**
* Split a datetime attribute into revelant components
*
* @param string $iso			the date to split up in ISO format
* @param string	$prefix			the prefix to add to array keys generated by this fn
*
* @return array					the components
*/
function get_date_components($iso, $prefix='')
{			
	// we represent end-of-day as '24:00:00' as per ISO 8601, but strtotime()
	// doesn't recognise it. Fudge a little by flagging it, getting *start* of
	// day, then adding a day to the timestamp and setting hours to 24
	if ($iso_24h = (iso8601_time_component($iso) == '24:00:00')) {
		$iso = substr($iso,0,10).' 00:00:00';	// make it legit, fix it later
	}
	
	// we can build most of our info from the getdate() function
	if (($ts = strtotime($iso)) != -1) {
		$this_date = getdate($ts);
		$this_date['yday']++;	// to make it 1-based
		
		// unset word based dates
		unset($this_date['weekday']);
		unset($this_date['month']);
		
		// move timestamp field to a string-based key instead of [0]
		$this_date['ts'] = $this_date[0];
		unset($this_date[0]);
		
		// add 'day-count stamp'
		$this_date['ds'] = round(iso8601_ts(substr($iso,0,10)) / 86400);
		
		// 24:00:00 ISO time
		if ($iso_24h) {
			$this_date['ts'] += 86400;
			$this_date['hours'] = 24;
		}
		
	} else {
		// we have to do this manually..... great
		$this_date = Array();
		
		list($this_date['year'], $this_date['mon'], $this_date['mday'], $this_date['hours'], $this_date['minutes'], $this_date['seconds']) = sscanf($iso, '%4s-%2s-%2s %2s:%2s:%2s');						
	
		// any dashed out sections should be nulled out instead
		foreach(array_keys($this_date) as $iso_part) {
			if ($this_date[$iso_part] == str_repeat('-', ($iso_part == 'year' ? 4 : 2))) $this_date[$iso_part] = null;
		}
	
		$date_only_ts = strtotime(substr($iso,0,10));
						
		// forward day-of-year and weekday can only work with valid date
		if (is_null($this_date['mday']) || is_null($this_date['mon']) || is_null($this_date['year']) || ($date_only_ts == 1)) {
			$this_date['wday'] = null;
			$this_date['yday'] = null;
			$this_date['ts'] = null;
			$this_date['ds'] = null;
		} else {
			if (is_null($this_date['hours']) && (!is_null($this_date['minutes']) || !is_null($this_date['seconds']))) {
				// hours null in the middle of the time - no valid time
				$this_date['ts'] = null;
			} elseif (is_null($this_date['minutes']) && !is_null($this_date['seconds']))  {
				// minutes null in the middle of the time - no valid time
				$this_date['ts'] = null;
			} else {
				// make a best guess on timestamp, setting all blanks to 0
				$this_date['ts'] = strtotime(substr($iso,0,10).' '.str_replace('-','0',substr($iso,11)));
			}
			
			// we can do these in any case
			$this_date['ds'] = round(iso8601_ts(substr($iso,0,10)) / 86400);
			$this_date['wday'] = date('w', $date_only_ts);
			$this_date['yday'] = date('z', $date_only_ts) + 1;
		}
		
	}
	
	// add 'month-stamp' column which can be used to easily check
	// 'x monthly' stuff which can't be done easily with T/S alone
	if (is_null($this_date['mon']) || is_null($this_date['year'])) {
		$this_date['ms'] = null;
	} else {
		$this_date['ms'] = $this_date['year'] * 12 + $this_date['mon'];
	}

	if (is_null($this_date['mday']) || is_null($this_date['mon']) || is_null($this_date['year'])) {
		$this_date['ryday'] = null;
		$this_date['rmday'] = null;
		$this_date['wdom']  = null;
		$this_date['rwdom'] = null;
	} else {
		// reverse day-of-year and reverse day-of-month [1-based]
		$this_date['ryday'] = 365 + (is_leap_year($this_date['year']) ? 1 : 0) - $this_date['yday'] + 1;
		$this_date['rmday'] = days_in_month($this_date['mon'], $this_date['year']) - $this_date['mday'] + 1;
		
		// weekday of month, reverse thereof [both 1-based]
		$wdom = weekday_dates_in_month($this_date['wday'], $this_date['mon'], $this_date['year']);
		
		$this_date['wdom']  = array_search($this_date['mday'], $wdom) + 1;
		$this_date['rwdom']  = count($wdom) - $this_date['wdom'] + 1;
	}
	
	// now dump them into the array we were passed in the first place
	$res = Array();
	foreach(array_keys($this_date) as $iso_part) {
		$res[$prefix.$iso_part] = is_null($this_date[$iso_part]) ? null : intval($this_date[$iso_part]);
	}
	return $res;	
	
}//end get_date_components()