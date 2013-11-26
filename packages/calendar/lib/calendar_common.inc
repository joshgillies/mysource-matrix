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
* $Id: calendar_common.inc,v 1.84 2013/09/17 23:25:02 akarelia Exp $
*
*/

/*
* Calendar Package Library Functions
*
* This file contains global functions that are useful throughout the calendar module
* (but not beyond)
*
* @author Tom Barrett <tbarrett@squiz.net>
* @author Luke Wright <lwright@squiz.net>
* @version $Revision: 1.84 $
* @package MySource_Matrix_Packages
* @subpackage calendar
*/

require_once SQ_FUDGE_PATH.'/general/datetime.inc';
require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';

class Calendar_Common
{


	/**
	* Builds the common part of the single events query
	*
	* @param array		$root_ids			Root node IDs
	* @param string		$event_type_code	Type of the event
	* @param boolean	$include_exceptions	Include exceptions
	* @param array		$bind_vars			Vars to bind when preparing the query later
	*
	* @return string
	* @access public
	*/
	function getSingleEventQueryBase($root_ids, $event_type_code='calendar_event_single', $include_exceptions=TRUE, &$bind_vars=Array())
	{
		$db = MatrixDAL::getDb();

		$type_codes = Calendar_Common::_getTypeDescendantsQuoted($event_type_code);
		if ($include_exceptions) {
			$type_codes = array_merge($type_codes, Calendar_Common::_getTypeDescendantsQuoted('calendar_event_cancellation'));
			$type_codes = array_merge($type_codes, Calendar_Common::_getTypeDescendantsQuoted('calendar_event_modification'));
		}

		$sql = 'SELECT a.assetid, a.type_code, a.name, t.treeid, cd.*'."\n";
		if (!$GLOBALS['SQ_SYSTEM']->userSystemAdmin() && !$GLOBALS['SQ_SYSTEM']->userRoot()) {
			$sql .= ', p.granted as granted';
		}
		$sql .= "\n".'FROM '.Calendar_Common::_getTableList()."\n";
		$sql .= 'WHERE a.type_code IN ( '.implode(',', $type_codes).') '."\n";
		$sql .= "\t".'AND (a.status >= '.MatrixDAL::quote(SQ_STATUS_LIVE).")\n\t";

		$sql .= ' AND '.Calendar_Common::_getTreeClause($root_ids);

		$bind_vars = Array();
		if (!$GLOBALS['SQ_SYSTEM']->userSystemAdmin() && !$GLOBALS['SQ_SYSTEM']->userRoot()) {
			if (isset($_SESSION['sq_effective_user_groups'][$GLOBALS['SQ_SYSTEM']->user->id])) {
				$userids = $_SESSION['sq_effective_user_groups'][$GLOBALS['SQ_SYSTEM']->user->id];
			} else {
				$userids = array_keys($GLOBALS['SQ_SYSTEM']->am->getParents($GLOBALS['SQ_SYSTEM']->user->id, 'user_group', FALSE));
			}

			$userids[] = $GLOBALS['SQ_SYSTEM']->user->id;
			$userids[] = $GLOBALS['SQ_SYSTEM']->am->getSystemAssetid('public_user');
			$user_count = 0;
			foreach ($userids as $idx => $userid) {
				$bind_vars[$userid] = 'user'.$user_count++;
			}

			$userid_cond = ' AND p.userid  IN (:'.implode(',:', $bind_vars).')';

			$sql .= $userid_cond.' AND p.granted = \'1\' AND (p.permission >= '.MatrixDAL::quote(SQ_PERMISSION_WRITE).' OR (status >= '.MatrixDAL::quote(SQ_STATUS_LIVE).' AND p.permission >= '.MatrixDAL::quote(SQ_PERMISSION_READ).'))';
		}

		return $sql;

	}//end getSingleEventQueryBase()


	/**
	* Builds the common part of the recurring events query
	*
	* The ISO 8601 date is taken as passed to the calling function - this means the:
	* - YYYY-MM-DD of the day being shown in day view
	* - YYYY-MM-DD of the <strong>first</strong> day in week view
	* - the YYYY-MM (only, no day) of the month involved in month view
	* - the YYYY (only) of the year involved in the year view
	*
	* @param array	$root_ids			Array of root nodes
	* @param string	$view				the type of view we are requesting (day|week|month|year|period)
	* @param string	$iso				ISO 8601 date as passed to the calling function
	* @param int	$length				Length of period to look in (period view only)
	* @param string	$event_type			Desired event type
	* @param array	$bind_vars			Vars to bind when preparing the query later
	*
	* @return string
	* @access public
	*/
	function getRecurringEventQueryBase($root_ids, $view, $iso, $length=NULL, $event_type='calendar_event_recurring', &$bind_vars=Array())
	{
		list($year,$month,$day) = sscanf(substr($iso,0,10), '%4d-%2d-%2d');
		if (!$month) $month = 12;
		if (!$day) $day = 1;
		if (($view == 'period') && ($length == 0)) {
			$view = 'day';
		}
		$length = (int) $length;
		
		$now = Calendar_Common::getDateComponents(sprintf('%04d-%02d-%02d', $year, $month, $day));
		$wdom_list = weekday_dates_in_month($now['wday'], $month, $year);

		$db = MatrixDAL::getDb();

		$type_codes = Calendar_Common::_getTypeDescendantsQuoted($event_type);

		$sql = 'SELECT a.assetid, a.type_code, a.name, t.treeid, cd.*'."\n";
		if (!$GLOBALS['SQ_SYSTEM']->userSystemAdmin() && !$GLOBALS['SQ_SYSTEM']->userRoot()) {
			$sql .= ', p.granted as granted';
		}
		$sql .= "\nFROM ".Calendar_Common::_getTableList()." \nWHERE (a.type_code IN (".implode(',', $type_codes).")) \n\tAND (a.status >= ".MatrixDAL::quote(SQ_STATUS_LIVE).")\n";
		$bind_vars = Array();
		if (!$GLOBALS['SQ_SYSTEM']->userSystemAdmin() && !$GLOBALS['SQ_SYSTEM']->userRoot()) {
			if (isset($_SESSION['sq_effective_user_groups'][$GLOBALS['SQ_SYSTEM']->user->id])) {
				$userids = $_SESSION['sq_effective_user_groups'][$GLOBALS['SQ_SYSTEM']->user->id];
			} else {
				$userids = array_keys($GLOBALS['SQ_SYSTEM']->am->getParents($GLOBALS['SQ_SYSTEM']->user->id, 'user_group', FALSE));
			}
			$userids[] = $GLOBALS['SQ_SYSTEM']->user->id;
			$userids[] = $GLOBALS['SQ_SYSTEM']->am->getSystemAssetid('public_user');
			$user_count = 0;
			foreach ($userids as $idx => $userid) {
				$bind_vars[$userid] = 'user'.$user_count++;
			}
			$userid_cond = ' AND p.userid  IN (:'.implode(',:', $bind_vars).')';
			$sql .= $userid_cond.' AND p.granted = \'1\' AND (p.permission >= '.MatrixDAL::quote(SQ_PERMISSION_WRITE).' OR (status >= '.MatrixDAL::quote(SQ_STATUS_LIVE).' AND p.permission >= '.MatrixDAL::quote(SQ_PERMISSION_READ).'))';
		}
		$sql .= ' AND ';

		$sql_freqs = Array();

		switch ($view) {
			case 'day' :
				$last_day = $now['ds'];
				$now['month_length'] = days_in_month($month, $year);
				$now['month_start_wday'] = date('w', strtotime(sprintf('%04d-%02d-%02d', $year, $month, 1)));
				$now['month_end_wday'] = date('w', strtotime(sprintf('%04d-%02d-%02d', $year, $month, $now['month_length'])));
				$prev_month_length = date('t', strtotime('-1 month', $now['ts']));
				$prev_month_start_wday = date('w', strtotime(sprintf('%04d-%02d-%02d', $year, $month, 1).' -1 month'));
				$prev_month_end_wday = date('w', strtotime(sprintf('%04d-%02d-%02d', $year, $month, 1).' -1 day'));

				if (($now['wday'] != 0) && ($now['wday'] != 6)) {
					$sql_freqs[] = 'frequency = '.MatrixDAL::quote('DWD');
				} else {
					$sql_freqs[] = 'frequency = '.MatrixDAL::quote('DWE');
				}

				$sql_freqs[] = '(frequency = '.MatrixDAL::quote('DED').' AND mod(('.MatrixDAL::quote($now['ds']).' - start_date_ds), period) BETWEEN 0 AND COALESCE(end_date_ds - start_date_ds,0))';

				$sql_freqs[] = '(frequency = '.MatrixDAL::quote('WEW').' AND mod(('.MatrixDAL::quote($now['ds']).' - start_date_ds), (period*7)) BETWEEN 0 AND COALESCE(end_date_ds - start_date_ds,0))';

				$sql_freqs[] = '
				(SUBSTR(frequency, 1, 1) = '.MatrixDAL::quote('M').') AND (
					(
						/* there is an instance that starts in our month */
					   (MOD('.MatrixDAL::quote($now['ms']).' - start_date_ms, period) = 0)
					   AND (
							/* our view date is between the start and end of the instance */
							((frequency = '.MatrixDAL::quote('MFN').') AND (('.MatrixDAL::quote($now['mday']).' - start_date_mday) BETWEEN 0 AND (COALESCE((end_date_ds - start_date_ds), 0))))
							OR ((frequency = '.MatrixDAL::quote('MRN').') AND ((start_date_rmday - '.MatrixDAL::quote($now['rmday']).') BETWEEN 0 AND (COALESCE((end_date_ds - start_date_ds), 0))))
							OR ((frequency = '.MatrixDAL::quote('MFW').') AND (('.MatrixDAL::quote($now['mday']).' - (((start_date_wdom - 1) * 7) + MOD(start_date_wday + 7 - '.$now['month_start_wday'].', 7) + 1)) BETWEEN 0 AND (COALESCE((end_date_ds - start_date_ds), 0))))
							OR ((frequency = '.MatrixDAL::quote('MRW').') AND ((((start_date_rwdom - 1) * 7) + MOD('.($now['month_end_wday'] + 7).' - start_date_wday, 7) + 1 - '.$now['rmday'].') BETWEEN 0 AND (COALESCE((end_date_ds - start_date_ds), 0))))
					  )
					) OR (
							/* An instance that started in a previous month doesnt end til a subsequent month */
							(MOD('.$now['ms'].' - start_date_ms, period) < MOD('.$now['ms'].' - end_date_ms, period))
					) OR (
						/* An instance ends this month */
						((start_date_ms != end_date_ms) AND (MOD('.$now['ms'].' - end_date_ms, period) = 0))
					   AND (
						   /* its end is PROBABLY after our view date - we can only be ~90% sure here */
							((frequency = '.MatrixDAL::quote('MFN').')
							   AND ((end_date_ds - start_date_ds + 1) >= ROUND(('.MatrixDAL::quote($now['ts']).' - '.Calendar_Common::_ts2Sql('FLOOR((('.$now['year'].' * 12) + '.($now['mon']-1).' - (end_date_ms - start_date_ms)) / 12)', 'MOD('.$now['mon'].' + 10 - MOD('.($now['ms']-1).' - start_date_ms, period), 12) + 1', 'start_date_mday').') / 86400)))
							OR ((frequency = '.MatrixDAL::quote('MRN').')
								AND ((end_date_ds - start_date_ds + 1) >= ROUND(('.MatrixDAL::quote($now['ts']).' - '.Calendar_Common::_ts2Sql('FLOOR((('.$now['year'].' * 12) + '.($now['mon']-1).' - (end_date_ms - start_date_ms)) / 12)', 'MOD('.$now['mon'].' + 10 - MOD('.($now['ms']-1).' - start_date_ms, period), 12) + 1', '((start_date_mday + 3) - (FLOOR((start_date_mday + 3) / 31) * MOD(start_date_mday + 3, 31)))').') / 86400)))
							OR ((frequency = '.MatrixDAL::quote('MFW').')
								AND ((end_date_ds - start_date_ds + 1) >= ROUND(('.MatrixDAL::quote($now['ts']).' - '.Calendar_Common::_ts2Sql('FLOOR((('.$now['year'].' * 12) + '.($now['mon']-1).' - (end_date_ms - start_date_ms)) / 12)', 'MOD('.$now['mon'].' + 10 - MOD('.($now['ms']-1).' - start_date_ms, period), 12) + 1', '(7 * start_date_wdom) - (FLOOR((7 * start_date_wdom) / 31) * MOD(7 * start_date_wdom, 31))').') / 86400)))
							OR ((frequency = '.MatrixDAL::quote('MRW').')
								AND ((end_date_ds - start_date_ds + 1) >= ROUND(('.MatrixDAL::quote($now['ts']).' - '.Calendar_Common::_ts2Sql('FLOOR((('.$now['year'].' * 12) + '.($now['mon']-1).' - (end_date_ms - start_date_ms)) / 12)', 'MOD('.$now['mon'].' + 10 - MOD('.($now['ms']-1).' - start_date_ms, period), 12) + 1', '(31 - ((start_date_rwdom - 1) * 7))').') / 86400)))
						)
					) OR (
							/* An instance that started last month has run into our month
							   even though its original instance was contained in only one month
							   and it ends on/after our view date */
							(MOD('.$now['ms'].' - start_date_ms, period) = 1)
							   AND (
								((frequency = '.MatrixDAL::quote('MFN').')
									AND ((start_date_mday + (end_date_ds - start_date_ds)) - '.$prev_month_length.' >= '.$now['mday'].'))
								OR ((frequency = '.MatrixDAL::quote('MFD').')
									AND (((MOD(7 + '.$prev_month_start_wday.' - start_date_wday, 7) + 1) + ((start_date_wdom - 1) * 7) + (end_date_ds - start_date_ds)) - '.$prev_month_length.' >= '.$now['mday'].'))
								OR ((frequency = '.MatrixDAL::quote('MRD').')
										AND ((('.$prev_month_length.' - MOD(7 + '.$prev_month_end_wday.' - start_date_wday, 7)) + (end_date_ds - start_date_ds)) - '.$prev_month_length.' >= '.$now['mday'].'))
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

				$sql_freqs[] = 'frequency = '.MatrixDAL::quote('DWD');
				$sql_freqs[] = 'frequency = '.MatrixDAL::quote('DWE');

				// note: the slightly redundant "*1.0" is so Postgres interprets it as a
				// double instead of an int, blowing away our attempts to use floor()
				$sql_freqs[] = '(frequency = '.MatrixDAL::quote('DED').' AND ((FLOOR(('.MatrixDAL::quote($now['ds']-1).' - start_date_ds) / (period*1.0)) < FLOOR(('.MatrixDAL::quote($now['ds']+6).' - start_date_ds) / (period*1.0))) or mod(('.MatrixDAL::quote($now['ds']).' - start_date_ds), period) BETWEEN 0 AND COALESCE(end_date_ds - start_date_ds,0)))';

				$sql_freqs[] = '(frequency = '.MatrixDAL::quote('WEW').' AND ((FLOOR(('.MatrixDAL::quote($now['ds']-1).' - start_date_ds)/(period*7.0)) < FLOOR(('.MatrixDAL::quote($now['ds']+6).' - start_date_ds)/(period*7.0))) OR MOD(('.MatrixDAL::quote($now['ds']).' - start_date_ds), period) BETWEEN 0 AND COALESCE(end_date_ds - start_date_ds,0)))';

				$sql_freqs_m = Array();

				// every X months on xth of the month
				$sql_freqs_m[] = '(frequency = '.MatrixDAL::quote('MFN').' AND start_date_mday BETWEEN '.MatrixDAL::quote(intval($day)).' AND '.MatrixDAL::quote(min($day+6, $days_in_month)).')';

				// every X months on xth weekday of the month
				$sql_freqs_m[] = '(frequency = '.MatrixDAL::quote('MFW').' AND start_date_wdom IN ( '.MatrixDAL::quote($now['wdom']).($req_next_week ? ','.MatrixDAL::quote($now['wdom']+1) : '').'))';

				// every X months on xth last day of the month
				$sql_freqs_m[] = '(frequency = '.MatrixDAL::quote('MRN').' AND start_date_rmday BETWEEN '.MatrixDAL::quote(max($days_in_month - $day - 5, 1)).' AND '.MatrixDAL::quote(intval($days_in_month - $day + 1)).')';

				// every X months on xth last weekday of the month
				$sql_freqs_m[] = '(frequency = '.MatrixDAL::quote('MRW').' AND start_date_rwdom IN ( '.MatrixDAL::quote($now['rwdom']).($req_rev_next_week ? ','.MatrixDAL::quote($now['rwdom']-1) : '').'))';

				$sql_freqs[] = '(('.implode("\n\t".' OR ', $sql_freqs_m).') AND (MOD('.MatrixDAL::quote($now['ms']).' - start_date_ms, period) = 0))';

				// rolling over to a new month? Oh crap :o
				if ($req_next_month) {
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

					$sql_freqs_m[] = '(frequency = '.MatrixDAL::quote('MFN').' AND start_date_mday BETWEEN '.MatrixDAL::quote(1).' AND '.MatrixDAL::quote($day+6-$days_in_month).')';

					// every X months on xth weekday of the month - it's always going to
					// be the first week of a new month
					$sql_freqs_m[] = '(frequency = '.MatrixDAL::quote('MFW').' AND start_date_wdom = 1)';

					// every X months on xth last day of the month
					$sql_freqs_m[] = '(frequency = '.MatrixDAL::quote('MRN').' AND start_date_rmday BETWEEN '.MatrixDAL::quote($days_in_month * 2 - $day - 5).' AND '.MatrixDAL::quote(intval($days_in_month)).')';

					// every X months on xth last weekday of the month
					$sql_freqs_m[] = '(frequency = '.MatrixDAL::quote('MRW').' AND start_date_rwdom IN ( '.MatrixDAL::quote(count($wdom_list)).($req_rev_next_week ? ','.MatrixDAL::quote(count($wdom_list)-1) : '').'))';

					$sql_freqs[] = '(('.implode("\n\t".' OR ', $sql_freqs_m).') AND (MOD('.MatrixDAL::quote($now['ms']+1).' - start_date_ms, period) = 0))';
				}

				$sql_freqs[] = '(frequency IN ('.MatrixDAL::quote('MFN').', '.MatrixDAL::quote('MRN').') AND (end_date_ds - start_date_ds >= '.$day.'))';

				$sql_freqs[] = '(frequency IN ('.MatrixDAL::quote('MFW').', '.MatrixDAL::quote('MRW').') AND (end_date_ds - start_date_ds >= 1 + ((start_date_rwdom-1) * 7)))';

			break;

			case 'month' :
				$last_day = $now['ds'] + days_in_month($month,$year) - 1;

				$sql_freqs[] = '(frequency = '.MatrixDAL::quote('DWD').')';
				$sql_freqs[] = '(frequency = '.MatrixDAL::quote('DWE').')';

				$sql_freqs[] = '(frequency = '.MatrixDAL::quote('DED').' AND (FLOOR(('.MatrixDAL::quote($now['ds']-1).' - start_date_ds)/period) < FLOOR(('.MatrixDAL::quote($last_day).' - start_date_ds)/period)))';

				$sql_freqs[] = '(frequency = '.MatrixDAL::quote('WEW').' AND period = 1)';

				$sql_freqs[] = '(frequency = '.MatrixDAL::quote('WEW').' AND (floor(('.MatrixDAL::quote($now['ds']-1).' - start_date_ds)/(period*7)) < FLOOR('.MatrixDAL::quote($last_day).' - start_date_ds)/(period*7)))';

				// all monthly views can be lumped together on this one
				$sql_freqs[] = '(frequency LIKE '.MatrixDAL::quote('M%').' AND (MOD(('.MatrixDAL::quote($now['ms']).' - start_date_ms), period) = 0))';

				$iso = $year.'-'.sprintf('%02d', $month).'-01';

			break;

			case 'year' :
				$last_day = $now['ds']+364+is_leap_year($year);

				$sql_freqs[] = 'frequency = '.MatrixDAL::quote('DWD');
				$sql_freqs[] = 'frequency = '.MatrixDAL::quote('DWE');

				$sql_freqs[] = '(frequency = '.MatrixDAL::quote('DED').' AND (FLOOR(('.MatrixDAL::quote($now['ds']-1).' - start_date_ds)/period) < FLOOR(('.MatrixDAL::quote($last_day).' - start_date_ds)/period)))';

				$sql_freqs[] = '(frequency = '.MatrixDAL::quote('WEW').' AND period = 1)';

				$sql_freqs[] = '(frequency = '.MatrixDAL::quote('WEW').' AND (FLOOR(('.MatrixDAL::quote($now['ds']-1).' - start_date_ds)/(period*7)) < FLOOR(('.MatrixDAL::quote($last_day).' - start_date_ds)/(period*7))))';

				// all monthly views can be lumped together on this one
				// but we need to do something like the 'every day' type thing to work out
				// whether an occurrence will happen this year
				$sql_freqs[] = '(frequency LIKE '.MatrixDAL::quote('M%').' AND (FLOOR(('.($year * 12).' - start_date_ms) / period) < FLOOR(('.(($year + 1) * 12).' - start_date_ms) / period)))';
			break;

			case 'period' :
				$last_date = date('Y-m-d', strtotime("$iso 00:00:00 +$length days") - strtotime('1970-01-01 00:00:00 Z'));
				$last_date_components = Calendar_Common::getDateComponents($last_date);
				$last_day = $last_date_components['ds'];

				// GENERIC MATCES
				$sql_freqs[] = 	'(
									(start_date_ds BETWEEN '.$now['ds'].' AND '.$last_date_components['ds'].')
									OR
									(end_date_ds BETWEEN '.$now['ds'].' AND '.$last_date_components['ds'].')
								)';

				// DAILY-RECURRING
				$sql_freqs[] = '(
									(frequency = '.MatrixDAL::quote('DED').')
									AND
									(
										(period <= '.$length.')
										OR
										(FLOOR(('.$now['ds'].' - start_date_ds) / period) < FLOOR(('.$last_date_components['ds'].' - start_date_ds) / period))
										OR
										(floor(('.$now['ds'].' - end_date_ds) / period) < FLOOR(('.$last_date_components['ds'].' - end_date_ds) / period))
									)
								)';
				if (($length > 2) || !in_array($now['wday'], Array(0, 6)) || !in_array($last_date_components['wday'], Array(0, 6))) {
					// the period includes a weekday
					$sql_freqs[] = '(frequency = '.MatrixDAL::quote('DWD').')';
				}
				if (($length > 5) || in_array($now['wday'], Array(0, 6)) || in_array($last_date_components['wday'], Array(0, 6)) || ($last_date_components['wday'] < $now['wday'])) {
					// the period includes a weekend day
					$sql_freqs[] = '(frequency = '.MatrixDAL::quote('DWE').')';
				}

				// WEEKLY-RECURRING
				$sql_freqs[] = '(
									(frequency = '.MatrixDAL::quote('WEW').')
									AND
									(
										((period * 7) <= '.$length.')
										OR
										(FLOOR(('.($now['ds']-1).' - start_date_ds) / (period * 7)) < FLOOR(('.$last_date_components['ds'].' - start_date_ds) / (period * 7)))
										OR
										(FLOOR(('.($now['ds']-1).' - end_date_ds) / (period * 7)) < FLOOR(('.$last_date_components['ds'].' - end_date_ds) / (period * 7)))
									)
								)';

				// MONTHLY-RECURRING
				$monthly_conditions = Array();
				if ($last_date_components['ms'] == $now['ms']) {
					// we are within one month

					// get MFN and MRN events
					$monthly_conditions[] = '((frequency = '.MatrixDAL::quote('MFN').') AND (((MOD('.$now['ms'].' - start_date_ms, period) = 0) AND (start_date_mday BETWEEN '.$now['mday'].' AND '.$last_date_components['mday'].')) OR ((MOD('.$now['ms'].' - end_date_ms, period) = 0) AND (end_date_mday BETWEEN '.$now['mday'].' AND '.$last_date_components['mday'].'))))';
					$monthly_conditions[] = '((frequency = '.MatrixDAL::quote('MRN').') AND ((start_date_rwdom BETWEEN '.$last_date_components['rwdom'].' AND '.$now['rwdom'].') OR (end_date_rwdom BETWEEN '.$last_date_components['rwdom'].' AND '.$now['rwdom'].')))';

					// get MFW events
					$mfw_conditions = Array();
					if ($last_date_components['wdom'] == $now['wdom']) {
						// we are within one month-week

						// double check these
						$mfw_conditions[] = '((MOD('.$now['ms'].' - start_date_ms, period) = 0)
							AND (start_date_wdom = '.$now['wdom'].')
							AND (start_date_mday - (7 * (start_date_wdom - 1)) BETWEEN '.($now['mday'] - (7 * ($now['wdom'] - 1))).' AND '.($last_date_components['mday'] - (7 * ($last_date_components['wdom'] - 1))).'))';
						$mfw_conditions[] = '((MOD('.$now['ms'].' - end_date_ms, period) = 0)
							AND (end_date_wdom = '.$now['wdom'].')
							AND (end_date_mday - (7 * (end_date_wdom - 1)) BETWEEN '.($now['mday'] - (7 * ($now['wdom'] - 1))).' AND '.($last_date_components['mday'] - (7 * ($last_date_components['wdom'] - 1))).'))';

					} else {
						// we start and end in different month-weeks

						// want to know if an event starts or ends late enough in the start week
						$mfw_conditions[] = '(((start_date_wdom = '.$now['wdom'].') AND (start_date_mday - ((start_date_wdom - 1) * 7) >= '.($now['mday'] - (($now['wdom'] - 1) * 7)).')) OR (((end_date_wdom = '.$now['wdom'].') AND (end_date_mday - ((end_date_wdom - 1) * 7) >= '.($now['mday'] - (($now['wdom'] - 1) * 7)).'))))';

						// or early enough in the end week
						$mfw_conditions[] = '(((start_date_wdom = '.$last_date_components['wdom'].') AND (start_date_mday - ((start_date_wdom - 1) * 7) <= '.($last_date_components['mday'] - (($last_date_components['wdom'] - 1) * 7)).')) OR (((end_date_wdom = '.$last_date_components['wdom'].') AND (end_date_mday - ((end_date_wdom - 1) * 7) <= '.($last_date_components['mday'] - (($last_date_components['wdom'] - 1) * 7)).'))))';

						// or covers the whole thing
						$mfw_conditions[] = '((start_date_wdom <= '.$now['wdom'].') AND (end_date_wdom >= '.$last_date_components['wdom'].') AND (start_date_mday - ((start_date_wdom - 1) * 7) < '.($now['mday'] - (($now['wdom'] - 1) * 7)).') AND (end_date_mday - ((end_date_wdom - 1) * 7) > '.($last_date_components['mday'] - (($last_date_components['wdom'] - 1) * 7)).'))';

						if ($last_date_components['wdom'] - $now['wdom'] != 1) {
							// we have intervening weeks
							// we want to know if any event started or ended in the intervening weeks
							$mfw_conditions[] = '((frequency = '.MatrixDAL::quote('MFW').') AND ((start_date_wdom BETWEEN '.($now['wdom'] + 1).' AND '.($last_date_components['wdom'] - 1).') OR (end_date_wdom BETWEEN '.($now['wdom'] + 1).' AND '.($last_date_components['wdom'] - 1).')))';
						}
					}
					$monthly_conditions[] = '((frequency = '.MatrixDAL::quote('MFW').') AND ('.implode(' OR ', $mfw_conditions)."\n\n     ))";

					// get MRW events
					$mrw_conditions = Array();
					if ($last_date_components['rwdom'] == $now['rwdom']) {
						// we are within one reverse-month-week

						$mrw_conditions[] = '((MOD('.$now['ms'].' - start_date_ms, period) = 0)
							AND (start_date_rwdom = '.$now['rwdom'].')
							AND (start_date_rmday - (7 * (start_date_rwdom - 1)) BETWEEN '.($now['rmday'] - (7 * ($now['rwdom'] - 1))).' AND '.($last_date_components['rmday'] - (7 * ($last_date_components['rwdom'] - 1))).'))';
						$mrw_conditions[] = '((MOD('.$now['ms'].' - end_date_ms, period) = 0)
							AND (end_date_rwdom = '.$now['rwdom'].')
							AND (end_date_rmday - (7 * (end_date_rwdom - 1)) BETWEEN '.($now['rmday'] - (7 * ($now['rwdom'] - 1))).' AND '.($last_date_components['rmday'] - (7 * ($last_date_components['rwdom'] - 1))).'))';
					} else {
						// we start and end in different reverse-month-weeks

						// want to know if an event starts or ends late enough in the start week
						$mrw_conditions[] = '(((start_date_rwdom = '.$now['rwdom'].') AND (start_date_rmday - ((start_date_rwdom - 1) * 7) >= '.($now['rmday'] - (($now['rwdom'] - 1) * 7)).')) OR (((end_date_rwdom = '.$now['rwdom'].') AND (end_date_rmday - ((end_date_rwdom - 1) * 7) <= '.($now['rmday'] - (($now['rwdom'] - 1) * 7)).'))))';

						// or early enough in the end week
						$mrw_conditions[] = '(((start_date_rwdom = '.$last_date_components['rwdom'].') AND (start_date_rmday - ((start_date_rwdom - 1) * 7) <= '.($last_date_components['rmday'] - (($last_date_components['rwdom'] - 1) * 7)).')) OR (((end_date_rwdom = '.$last_date_components['rwdom'].') AND (end_date_rmday - ((end_date_rwdom - 1) * 7) >= '.($last_date_components['rmday'] - (($last_date_components['rwdom'] - 1) * 7)).'))))';

						// or covers the whole thing
						$mrw_conditions[] = '((start_date_rwdom >= '.$now['rwdom'].') AND (end_date_rwdom <= '.$last_date_components['rwdom'].') AND (start_date_rmday - ((start_date_rwdom - 1) * 7) > '.($now['rmday'] - (($now['rwdom'] - 1) * 7)).') AND (end_date_rmday - ((end_date_rwdom - 1) * 7) < '.($last_date_components['rmday'] - (($last_date_components['rwdom'] - 1) * 7)).'))';

						if ($last_date_components['rwdom'] - $now['rwdom'] != 1) {
							// we have intervening weeks
							// we want to know if any event started or ended in the intervening weeks
							$mrw_conditions[] = '((start_date_rwdom BETWEEN '.($now['rwdom'] - 1).' AND '.($last_date_components['rwdom'] + 1).') OR (end_date_rwdom BETWEEN '.($now['rwdom'] - 1).' AND '.($last_date_components['rwdom'] + 1).'))';
						}
					}
					$monthly_conditions[] = '((frequency = '.MatrixDAL::quote('MRW').') AND ('.implode(' OR ', $mrw_conditions).'))';

					// get month events that cover the whole of our month
					$sql_freqs[] = '(
									/* begin month options */
										(frequency LIKE '.MatrixDAL::quote('M%').')
										AND (
												(
													(end_date_ms IS NOT NULL)
													AND (end_date_ms - start_date_ms >= 3)
													AND MOD('.$now['ms'].' - start_date_ms, period)
															BETWEEN 1 AND (end_date_ms - start_date_ms - 1)
												) OR (
													(
														MOD('.$now['ms'].' - start_date_ms, period) = 0
															OR MOD('.$now['ms'].' - end_date_ms, period) = 0
													) AND (
															'.implode("\n\t\t\t\t\t\t\t\t\t\t\t\t\t\t OR ", $monthly_conditions).'
													)
												)
										)
									/* end month options */
									)';
				} else {
					// start and end are in different months

					// we want to see if the start or end is late enough in the first month
					$sql_freqs[] = '((frequency = '.MatrixDAL::quote('MFN').') AND (((MOD('.$now['ms'].' - start_date_ms, period) = 0) AND (start_date_mday >= '.$now['mday'].')) OR ((MOD('.$now['ms'].' - end_date_ms, period) = 0) AND (end_date_mday >= '.$now['mday'].'))))';

					$sql_freqs[] = '((frequency = '.MatrixDAL::quote('MRN').') AND (((MOD('.$now['ms'].' - start_date_ms, period) = 0) AND (start_date_rmday <= '.$now['rmday'].')) OR ((MOD('.$now['ms'].' - end_date_ms, period) = 0) AND (end_date_rmday <= '.$now['rmday'].'))))';

					$sql_freqs[] = '((frequency = '.MatrixDAL::quote('MFW').') AND (MOD('.$now['ms'].' - start_date_ms, period) = 0) AND ((start_date_wdom > '.$now['wdom'].') OR (((start_date_wdom = '.$now['wdom'].') AND (start_date_mday - ((start_date_wdom - 1) * 7) >= '.($now['mday'] - (($now['wdom'] - 1) * 7)).')))))';

					$sql_freqs[] = '((frequency = '.MatrixDAL::quote('MFW').') AND (MOD('.$now['ms'].' - end_date_ms, period) = 0) AND ((end_date_wdom > '.$now['wdom'].') OR (((end_date_wdom = '.$now['wdom'].') AND (end_date_mday - ((end_date_wdom - 1) * 7) >= '.($now['mday'] - (($now['wdom'] - 1) * 7)).')))))';

					$sql_freqs[] = '((frequency = '.MatrixDAL::quote('MRW').') AND (MOD('.$now['ms'].' - start_date_ms, period) = 0) AND ((start_date_rwdom < '.$now['rwdom'].') OR (((start_date_rwdom = '.$now['rwdom'].') AND (start_date_rmday - ((start_date_rwdom - 1) * 7) <= '.($now['rmday'] - (($now['rwdom'] - 1) * 7)).')))))';

					$sql_freqs[] = '((frequency = '.MatrixDAL::quote('MRW').') AND (MOD('.$now['ms'].' - end_date_ms, period) = 0) AND ((end_date_rwdom > '.$now['rwdom'].') OR (((end_date_rwdom = '.$now['rwdom'].') AND (end_date_rmday - ((end_date_rwdom - 1) * 7) <= '.($now['rmday'] - (($now['rwdom'] - 1) * 7)).')))))';

					// or early enough in the last month
					$sql_freqs[] = '((frequency = '.MatrixDAL::quote('MFN').') AND (((MOD('.$last_date_components['ms'].' - start_date_ms, period) = 0) AND (start_date_mday <= '.$last_date_components['mday'].')) OR ((MOD('.$last_date_components['ms'].' - end_date_ms, period) = 0) AND (end_date_mday <= '.$last_date_components['mday'].'))))';

					$sql_freqs[] = '((frequency = '.MatrixDAL::quote('MRN').') AND (((MOD('.$last_date_components['ms'].' - start_date_ms, period) = 0) AND (start_date_rmday >= '.$last_date_components['rmday'].')) OR ((MOD('.$last_date_components['ms'].' - end_date_ms, period) = 0) AND (end_date_rmday >= '.$last_date_components['rmday'].'))))';

					$sql_freqs[] = '((frequency = '.MatrixDAL::quote('MFW').') AND (MOD('.$now['ms'].' - start_date_ms, period) = 0) AND ((start_date_wdom < '.$now['wdom'].') OR (((start_date_wdom = '.$now['wdom'].') AND (start_date_mday - ((start_date_wdom - 1) * 7) <= '.($now['mday'] - (($now['wdom'] - 1) * 7)).')))))';

					$sql_freqs[] = '((frequency = '.MatrixDAL::quote('MFW').') AND (MOD('.$now['ms'].' - end_date_ms, period) = 0) AND ((end_date_wdom < '.$now['wdom'].') OR (((end_date_wdom = '.$now['wdom'].') AND (end_date_mday - ((end_date_wdom - 1) * 7) <= '.($now['mday'] - (($now['wdom'] - 1) * 7)).')))))';

					$sql_freqs[] = '((frequency = '.MatrixDAL::quote('MRW').') AND (MOD('.$now['ms'].' - start_date_ms, period) = 0) AND ((start_date_rwdom > '.$now['rwdom'].') OR (((start_date_rwdom = '.$now['rwdom'].') AND (start_date_rmday - ((start_date_rwdom - 1) * 7) >= '.($now['rmday'] - (($now['rwdom'] - 1) * 7)).')))))';

					$sql_freqs[] = '((frequency = '.MatrixDAL::quote('MRW').') AND (MOD('.$now['ms'].' - end_date_ms, period) = 0) AND ((end_date_rwdom > '.$now['rwdom'].') OR (((end_date_rwdom = '.$now['rwdom'].') AND (end_date_rmday - ((end_date_rwdom - 1) * 7) <= '.($now['rmday'] - (($now['rwdom'] - 1) * 7)).')))))';

					if ($last_date_components['ms'] - $now['ms'] > 1) {
						// we have intervening months, so we want to see if the event starts or ends in one of them
						$sql_freqs[] = '((frequency LIKE '.MatrixDAL::quote('M%').') AND ((MOD('.$last_date_components['ms'].' - start_date_ms, period) BETWEEN 1 AND '.($last_date_components['ms'] - $now['ms'] - 1).') OR (MOD('.$last_date_components['ms'].' - end_date_ms, period) BETWEEN 1 AND '.($last_date_components['ms'] - $now['ms'] - 1).')))';
					}
				}//end else

			break;

		}//end switch on $view
		$sql .= "(/* begin sql freqs */\n\n".implode("\n\n\nOR\n\n\n ", $sql_freqs)."\n/* end sql freqs */\n)\n\n";

		// stop date
		$sql .= ' AND ('.$last_day.' >= start_date_ds)';
		$sql .= ' AND (stop_date = '.MatrixDAL::quote('----------').' OR stop_date >= '.MatrixDAL::quote($iso).')';

		$sql .= ' AND '.Calendar_Common::_getTreeClause($root_ids);

		return $sql;

	}//end getRecurringEventQueryBase()


	/**
	* Condense a result with a new row for each treeid to a result with a new row for each assetid
	*
	* Makes the 'treeid' entry into an array of treeids
	*
	* @param array	$raw	The raw result with a row for each treeid
	*
	* @return array
	* @access public
	*/
	function condenseResultTreeids($raw)
	{
		$res = Array();
		foreach ($raw as $r) {
			if (!isset($res[$r['assetid']])) {
				$r['treeid'] = Array($r['treeid']);
				$res[$r['assetid']] = $r;
			} else {
				$res[$r['assetid']]['treeid'][] = $r['treeid'];
			}
		}
		return $res;

	}//end condenseResultTreeids()


	/**
	* Expands list of multiple day and/or recurring events into a series of single-day events
	*
	* Returned list of events is indexed by a shadow-asset-like identifier of
	* 'assetid:instance' where instance is a 0-based index of how many times an instance
	* the event has occurred previously.
	*
	* @param array	&$events		Array of Events
	* @param string	$first_day		The first day we want to display (YYYY-MM-DD)
	* @param string	$last_day		The last day we want to display (YYYY-MM-DD)
	* @param string	$day_start_time	Earliest time of day we will display (HH:mm)
	* @param string	$day_end_time	Latest time of day we will display (HH:mm)
	*
	* @return array
	* @access public
	*/
	function &expandEventList(&$events, $first_day, $last_day, $day_start_time='00:00', $day_end_time='23:59')
	{
		$res = Array();
		foreach (array_keys($events) as $key) {
			$event =& $events[$key];
			$recurring_descandants = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants('calendar_event_recurring');
			if (($event['type_code'] == 'calendar_event_recurring') || in_array($event['type_code'], $recurring_descandants)) {
				Calendar_Common::_expandRecurringEvent($res, $event, $key, $first_day, $last_day, $day_start_time, $day_end_time);
			} else {
				Calendar_Common::_expandSingleEvent($res, $event, $key, $first_day, $last_day, $day_start_time, $day_end_time);
			}
		}
		return $res;

	}//end expandEventList()


	/**
	* Process exception events (cancellations and modifications) from the set of events passed
	*
	* When a cancellation or modification event is found in the set, its parent recurring event
	* is removed from the set, and for cancellation events they themselves are removed too
	*
	* @param array	&$res	a ($id => array(...)) array of events
	*
	* @return void
	* @access public
	*/
	function processExceptions(&$res)
	{
		$db = MatrixDAL::getDb();
		$exception_ids = Array();
		foreach ($res as $id => $details) {
			if (in_array($details['type_code'], Array('calendar_event_cancellation', 'calendar_event_modification'))) {
				$id_bits = explode(':', $id);
				$real_id = $id_bits[0];
				$exception_ids[] = MatrixDAL::quote($real_id);
			}
		}
		if (empty($exception_ids)) return FALSE;

		$exception_links = Array();
		try {
			$query = MatrixDAL::preparePdoQuery('SELECT majorid, minorid FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk WHERE minorid IN ('.implode(', ',	$exception_ids).')');
			$exception_links = MatrixDAL::executePdoGrouped($query);
		} catch (Exception $e) {
			throw new Exception($e->getMessage());
		}

		// if there are no event modification or event cancalletions found
		// it is safe to return back from here as we dont need to remove
		// any of the events
		if (empty($exception_links)) return FALSE;

		// so guess we have found some event modification or cancellation
		// lets deal with them Matrix style B-)
		$events_n_exceptions = Array();

		foreach ($exception_links as $event => $exceptions) {
			foreach ($exceptions as $index => $exception) {
				$exception_event = $GLOBALS['SQ_SYSTEM']->am->getAsset($exception[0]);
				$events_n_exceptions[$event][] = $exception_event;
			}
		}

		foreach ($events_n_exceptions as $event_id => $exceptions) {
			foreach ($exceptions as $i => $exception) {
				foreach ($res as $key => $details) {
					$id = explode(':', $key);
					$id = $id[0];
					if ($event_id == $id) {
						// get the start date of the calendar_event_modification
						// or calendar_event_cancellation (exception)
						// and compare it with the instance we have
						// because we need to just cancel/modify that occurance
						// and not all of the occurance of that particular event
						$exception_date = $exception->attr('start_date');
						if (strpos($exception_date, gmdate('Y-m-d', $details['start_date_ts'])) !== FALSE) {
							unset($res[$key]);
						}
					}
					// also if we are ourselves the cancellation event
					// unset ouselves
					if ($details['type_code'] == 'calendar_event_cancellation') {
						unset($res[$key]);
					}
				}
			}
		}

	}//end processExceptions()


	/**
	* Return true if event $a starts before $b
	* If the events start time is same, compare by the events name
	*
	* @param array	$a	Details of first comparee event
	* @param array	$b	Details of second comparee event
	*
	* @return boolean
	* @access public
	*/
	function compareStartDates($a, $b)
	{
		if ($a['start_date_ts'] == $b['start_date_ts']) {
			return $a['name'].$a['assetid'] > $b['name'].$b['assetid'];
		}

		return ($a['start_date_ts'] < $b['start_date_ts']) ? -1 : 1;

	}//end compareStartDates()


	/**
	* compare lengths of two events
	*
	* @param array	$a	Details of first comparee event
	* @param array	$b	Details of second comparee event
	*
	* @return int
	* @access public
	*/
	function compareEventLengths($a, $b)
	{
		return ($b['end_date_ts'] - $b['start_date_ts']) - ($a['end_date_ts'] - $a['start_date_ts']);

	}//end compareEventLengths()


	/**
	* Split a datetime attribute into revelant components
	*
	* @param string	$iso	the date to split up in ISO format
	* @param string	$prefix	the prefix to add to array keys generated by this fn
	*
	* @return array
	* @access public
	*/
	function getDateComponents($iso, $prefix='')
	{
		// we represent end-of-day as '24:00:00' as per ISO 8601, but strtotime()
		// doesn't recognise it. Fudge a little by flagging it, getting *start* of
		// day, then adding a day to the timestamp and setting hours to 24
		if ($iso_24h = (iso8601_time_component($iso) == '24:00:00')) {
			$iso = substr($iso,0,10).' 00:00:00';	// make it legit, fix it later
		}

		// we can build most of our info from the getdate() function
		if (($ts = strtotime($iso)) != FALSE) {
			$this_date = getdate($ts);
			$this_date['yday']++;	// to make it 1-based

			// unset word based dates
			unset($this_date['weekday']);
			unset($this_date['month']);

			// move timestamp field to a string-based key instead of [0]
			$this_date['ts'] = strtotime($iso.' Z') - strtotime('1970-01-01 00:00:00 Z');
			unset($this_date[0]);

			// add 'day-count stamp'
			$this_date['ds'] = Calendar_Common::_getDayStamp($iso);

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
			foreach (array_keys($this_date) as $iso_part) {
				if ($this_date[$iso_part] == str_repeat('-', ($iso_part == 'year' ? 4 : 2))) {
					$this_date[$iso_part] = NULL;
				}
			}

			$date_only_ts = strtotime(substr($iso,0,10));

			// forward day-of-year and weekday can only work with valid date
			if (is_null($this_date['mday']) || is_null($this_date['mon']) || is_null($this_date['year']) || ($date_only_ts == 1)) {
				$this_date['wday'] = NULL;
				$this_date['yday'] = NULL;
				$this_date['ts'] = NULL;
				$this_date['ds'] = NULL;
			} else {
				if (is_null($this_date['hours']) && (!is_null($this_date['minutes']) || !is_null($this_date['seconds']))) {
					// hours null in the middle of the time - no valid time
					$this_date['ts'] = NULL;
				} else if (is_null($this_date['minutes']) && !is_null($this_date['seconds'])) {
					// minutes null in the middle of the time - no valid time
					$this_date['ts'] = NULL;
				} else {
					// make a best guess on timestamp, setting all blanks to 0
					$this_date['ts'] = strtotime(substr($iso,0,10).' '.str_replace('-','0',substr($iso,11)).' Z') - strtotime('1970-01-01 00:00:00 Z');
				}

				// we can do these in any case
				$this_date['ds'] = Calendar_Common::_getDayStamp($iso);
				$this_date['wday'] = date('w', $date_only_ts);
				$this_date['yday'] = date('z', $date_only_ts) + 1;
			}

		}//end else

		// add 'month-stamp' column which can be used to easily check
		// 'x monthly' stuff which can't be done easily with T/S alone
		if (is_null($this_date['mon']) || is_null($this_date['year'])) {
			$this_date['ms'] = NULL;
		} else {
			$this_date['ms'] = Calendar_Common::_getMonthStamp($iso);
		}

		if (is_null($this_date['mday']) || is_null($this_date['mon']) || is_null($this_date['year'])) {
			$this_date['ryday'] = NULL;
			$this_date['rmday'] = NULL;
			$this_date['wdom']  = NULL;
			$this_date['rwdom'] = NULL;
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
		foreach (array_keys($this_date) as $iso_part) {
			$res[$prefix.$iso_part] = is_null($this_date[$iso_part]) ? NULL : intval($this_date[$iso_part]);
		}
		return $res;

	}//end getDateComponents()


	/**
	* Build SQL join statements
	*
	* @return string
	* @access private
	*/
	function _getTableList()
	{
		$base = ($GLOBALS['SQ_SYSTEM']->userSystemAdmin() || $GLOBALS['SQ_SYSTEM']->userRoot()) ? SQ_TABLE_RUNNING_PREFIX.'ast a' : '('.SQ_TABLE_RUNNING_PREFIX.'ast a JOIN '.SQ_TABLE_RUNNING_PREFIX.'vw_ast_perm p ON a.assetid = p.assetid)';
		return '('.$base.' JOIN ('.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk_tree t ON l.linkid = t.linkid) ON l.minorid = a.assetid) JOIN '.SQ_TABLE_RUNNING_PREFIX.'cal_date_val cd ON a.assetid = cd.assetid';

	}//end _getTableList()


	/**
	* Build SQL to express the unix timestamp represented by the supplied date component expressions
	*
	* @param string	$y	Expression for the year component of the date
	* @param string	$m	Expression for the month component of the date
	* @param string	$d	Expression for the day component of the date
	* @param string	$h	Expression for the hour component of the time
	* @param string	$i	Expression for the minute component of the time
	* @param string	$s	Expression for the second component of the time
	*
	* @return string
	* @access public
	*/
	function _ts2Sql($y, $m, $d, $h='00', $i='00', $s='00')
	{
		switch (MatrixDAL::getDbType()) {
			case 'pgsql':
				// can only subtract dates because subtracting timestamps results
				// in an INTERVAL data type, which can't be converted. Only DATE
				// subtraction returns an integer.
				// Add the hours, minutes and seconds manually afterwards
				$sql = "(86400 * (DATE (($y) || '-' || ($m) || '-' || ($d)) - DATE '1970-01-01') + ($h) * 3600 + ($i) * 60 + ($s))";
			break;

			case 'oci':
				// Subtracting datetimes in Oracle gives a fractional number of
				// days - I haven't had any problems yet but if rounding errors
				// occur then might have to go with similar to above
				$sql = '(86400 * ('.db_extras_todate(MatrixDAL::getDbType(), "(($y) || '-' || ($m) || '-' || ($d) || ' ' || ($h) || ':' || ($i) || ':' || ($s))", FALSE).' - '.db_extras_todate(MatrixDAL::getDbType(), '1970-01-01 00:00:00').'))';
			break;
		}

		return $sql;

	}//end _ts2Sql()


	/**
	* Returns an SQL fragment to restrict results to have treeids in the given range
	*
	* @param array	$root_ids	Root nodes for the listing
	*
	* @return string
	* @access public
	*/
	function _getTreeClause($root_ids)
	{
		$treeids = $GLOBALS['SQ_SYSTEM']->am->getAssetTreeids($root_ids);
		$db = MatrixDAL::getDb();
		$tree_options = Array();
		foreach ($treeids as $assetid => $treeids) {
			foreach ($treeids as $treeid) {
				$tree_options[] .= '(t.treeid LIKE '.MatrixDAL::quote($treeid[0].'%').')';
			}
		}
		return '('.implode(' OR ', $tree_options).')';

	}//end _getTreeClause()


	/**
	* Expands a multiple-day single event into a series of single-day events
	*
	* @param array	&$result_list	The result array to append the expanded events to
	* @param array	&$event			The cached calendar components of the event to expand
	*								(format as per structure of cal_date_val table)
	* @param int	$assetid		The asset ID of the event being expanded
	* @param string	$first_day		The first day we want to display (YYYY-MM-DD)
	* @param string	$last_day		The last day we want to display (YYYY-MM-DD)
	* @param string	$day_start_time	Earliest time of day we will display (HH:mm)
	* @param string	$day_end_time	Latest time of day we will display (HH:mm)
	*
	* @return void
	* @access public
	*/
	function _expandSingleEvent(&$result_list, &$event, $assetid, $first_day, $last_day, $day_start_time, $day_end_time)
	{
		list($day_start_hours, $day_start_minutes) = sscanf($day_start_time, '%02d:%02d');
		list($day_end_hours, $day_end_minutes) = sscanf($day_end_time, '%02d:%02d');
		$day_start_dts = ($day_start_hours * 60) + $day_start_minutes;
		$day_end_dts = ($day_end_hours * 60) + $day_end_minutes;

		$event_start_date = sprintf('%04d-%02d-%02d %02d:%02d:00', $event['start_date_year'], $event['start_date_mon'], $event['start_date_mday'], $event['start_date_hours'], $event['start_date_minutes']);
		$event_start_dts = ($event['start_date_hours'] * 60) + $event['start_date_minutes'];

		$i = 0;

		if (!is_null($event['end_date_ds'])) {
			// end date is specified
			$event_end_date = sprintf('%04d-%02d-%02d %02d:%02d:00', $event['end_date_year'], $event['end_date_mon'], $event['end_date_mday'], $event['end_date_hours'], $event['end_date_minutes']);
			$event_end_dts = (($event['end_date_hours'] * 60) + $event['end_date_minutes']);

			if ($event['start_date_ds'] != $event['end_date_ds']) {
				// covers multiple days
				$event['expanded'] = 1;

				if (($first_day <= substr($event_start_date, 0, 10)) && ($event_start_dts < $day_end_dts)) {
					// insert first day
					$result_list[$assetid.':'.$i] = Array();
					for (reset($event); NULL !== ($k = key($event)); next($event)) {
						$result_list[$assetid.':'.$i][$k] = $event[$k];
					}

					$result_list[$assetid.':'.$i] = array_merge($result_list[$assetid.':'.$i],
														Calendar_Common::getDateComponents($event_start_date, 'start_date_'),
														Calendar_Common::getDateComponents(substr($event_start_date, 0, 10).' 24:00:00', 'end_date_'));

					$i++;
				}

				// insert intermediate days
				$inter_date = date('Y-m-d', strtotime(max(add_days_to_iso($event_start_date, 1), $first_day)));
				$last_inter_date = min($last_day, add_days_to_iso($event_end_date, -1));
				while ($inter_date <= $last_inter_date) {
					$result_list[$assetid.':'.$i] = Array();
					for (reset($event); NULL !== ($k = key($event)); next($event)) {
						$result_list[$assetid.':'.$i][$k] = $event[$k];
					}

					$result_list[$assetid.':'.$i] = array_merge($result_list[$assetid.':'.$i],
														Calendar_Common::getDateComponents($inter_date.' 00:00:00', 'start_date_'),
														Calendar_Common::getDateComponents($inter_date.' 24:00:00', 'end_date_'));

					$inter_date = add_days_to_iso($inter_date, 1);
					$i++;
				}

				if ((substr($event_end_date, 0, 10) <= $last_day) && ($event_end_dts > $day_start_dts)) {
					// insert the last day
					$result_list[$assetid.':'.$i] = Array();
					for (reset($event); NULL !== ($k = key($event)); next($event)) {
						$result_list[$assetid.':'.$i][$k] = $event[$k];
					}

					$result_list[$assetid.':'.$i] = array_merge($result_list[$assetid.':'.$i],
														Calendar_Common::getDateComponents(substr($event_end_date, 0, 10).' 00:00:00', 'start_date_'),
														Calendar_Common::getDateComponents($event_end_date, 'end_date_'));

				}
				$i++;

			} else {
				// covers a single day only
				if (($event_end_dts < $day_start_dts) || ($event_start_dts > $day_end_dts)) {
					// ended too early or started too late to be shown
					return;
				}
				$start_time = gmmktime(0, 0, 0, $event['start_date_mon'], $event['start_date_mday'], $event['start_date_year']);
				$result_list[$assetid.':'.$i] = Array();
				for (reset($event); NULL !== ($k = key($event)); next($event)) {
					$result_list[$assetid.':'.$i][$k] = $event[$k];
				}

				$result_list[$assetid.':'.$i] = array_merge($result_list[$assetid.':'.$i],
													Calendar_Common::getDateComponents($event_start_date, 'start_date_'),
													Calendar_Common::getDateComponents($event_end_date, 'end_date_'));

				$i++;
			}
		} else {
			// end date is not specified - must be just a date or a point in time
			if (!is_null($event['start_date_hours'])) {
				// the time is specified
				if (($event_start_dts < $day_start_dts) || ($event_start_dts > $day_end_dts)) {
					// the time is too early or late in the day to be shown
					return;
				}
			}
			$start_time = gmmktime(0, 0, 0,  $event['start_date_mon'], $event['start_date_mday'], $event['start_date_year']);
			$result_list[$assetid.':'.$i] = Array();
			for (reset($event); NULL !== ($k = key($event)); next($event)) {
				$result_list[$assetid.':'.$i][$k] = $event[$k];
			}

			if (!is_null($event['start_date_hours'])) {
				$result_list[$assetid.':'.$i] = array_merge($result_list[$assetid.':'.$i],
													Calendar_Common::getDateComponents($event_start_date, 'start_date_'),
													Calendar_Common::getDateComponents($event_start_date, 'end_date_'));
			} else {
				// when this timestamp was cached, it was calculated with the UTC standard, so respect this when converting it back
				$start_date = gmdate('Y-m-d', $start_time);
				$result_list[$assetid.':'.$i] = array_merge($result_list[$assetid.':'.$i],
													Calendar_Common::getDateComponents($start_date.' 00:00:00', 'start_date_'),
													Calendar_Common::getDateComponents($start_date.' 24:00:00', 'end_date_'));
			}

			$i++;

		}

	}//end _expandSingleEvent()


	/**
	* Expands a list of recurring events into a series of single-day events
	*
	* @param array	&$result_list	The result array to append the expanded events to
	* @param array	&$event			The cached calendar components of the event to expand
	*								(format as per structure of cal_date_val table)
	* @param int	$assetid		The asset ID of the event being expanded
	* @param string	$first_day		The first day we want to display (YYYY-MM-DD)
	* @param string	$last_day		The last day we want to display (YYYY-MM-DD)
	* @param string	$day_start_time	Earliest time of day we will display (HH:mm)
	* @param string	$day_end_time	Latest time of day we will display (HH:mm)
	*
	* @return void
	* @access public
	*/
	function _expandRecurringEvent(&$result_list, &$event, $assetid, $first_day, $last_day, $day_start_time, $day_end_time)
	{
		require_once dirname(__FILE__).'/../calendar_events/calendar_event_recurring/calendar_event_recurring.inc';
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('calendar_event_recurring');

		$event['expanded'] = 1;

		$day_start_bits = explode(':', $day_start_time);
		$day_start_dts = ($day_start_bits[0] * 60) + $day_start_bits[1];
		$day_end_bits = explode(':', $day_end_time);
		$day_end_dts = ($day_end_bits[0] * 60) + $day_end_bits[1];

		$no_evt_end_date_ds			= is_null($event['end_date_ds']) || empty($event['end_date_ds']);
		$no_evt_start_date_hours	= is_null($event['start_date_hours']) || empty($event['start_date_hours']);
		$no_evt_end_date_hours		= is_null($event['end_date_hours']) || empty($event['end_date_hours']);
		$no_evt_start_date_hours	= is_null($event['start_date_hours']) || empty($event['start_date_hours']);

		$event_start_date = sprintf('%04d-%02d-%02d %02d:%02d:00', $event['start_date_year'], $event['start_date_mon'], $event['start_date_mday'], $event['start_date_hours'], $event['start_date_minutes']);
		$event_end_date = $no_evt_end_date_ds ? NULL : sprintf('%04d-%02d-%02d %02d:%02d:00', $event['end_date_year'], $event['end_date_mon'], $event['end_date_mday'], $event['end_date_hours'], $event['end_date_minutes']);

		$event_start_dts = $no_evt_start_date_hours ? NULL : (($event['start_date_hours'] * 60) + $event['start_date_minutes']);
		$event_end_dts = $no_evt_end_date_hours ? NULL : (($event['end_date_hours'] * 60) + $event['end_date_minutes']);

		// dispose of one case immediately
		if ($no_evt_end_date_ds && !$no_evt_start_date_hours) {
			// this is a single point in time
			if (($event_start_dts < $day_start_dts) || ($event_start_dts > $day_end_dts)) {
				// it occurs too early or late in the day
				return;
			}
		}

		// figure out the effective stop date
		if (($event['stop_date'] != '----------') && ($event['stop_date'] < $last_day)) {
			// stop date is before last day
			$event_last_day = $event['stop_date'];
		} else {
			// stop date is after last day
			$event_last_day = $last_day;
		}

		// find the earliest potentially-relevant occurrence
		switch ($event['frequency']{0}) {
			case 'D':
				$first_occur = add_days_to_iso($first_day, -$event['period']);
			break;

			case 'W':
				$first_occur = add_days_to_iso($first_day, -$event['period']*7);
			break;

			case 'M':
				$first_occur = add_months_to_iso($first_day, -$event['period']).'-'.date('d', strtotime($first_day));
			break;

		}
		// if multiple day event, subtract a further <event length> days so it catches those
		// events occurring before last recurrence but long enough to extend into this period
		if (!is_null($event['end_date_ds'])) {
			$first_occur = add_days_to_iso($first_day, -($event['end_date_ds']-$event['start_date_ds']));
		}
		$next_occur = Calendar_Event_Recurring::getFirstOccurrenceAfter($first_occur, $event_start_date, $event['frequency'], $event['period'], $event_last_day);

		// process instances until we get to the end of the period
		$i = 0;

		while (($next_occur !== FALSE) && (substr($next_occur, 0, 10) <= $last_day)) {

			$next_occur_ds = Calendar_Common::_getDayStamp($next_occur);
			// days between occurrences
			$difference = $next_occur_ds - $event['start_date_ds'];

			if (!$no_evt_end_date_ds) {
				// end date is specified
				if ($event['start_date_ds'] != $event['end_date_ds']) {

					// multiple day
					$start_time = gmmktime(0, 0, 0, $event['start_date_mon'], $event['start_date_mday'], $event['start_date_year'])+$difference*86400;
					$start_date = gmdate('Y-m-d', $start_time);

					$end_time = gmmktime(0, 0, 0, $event['end_date_mon'], $event['end_date_mday'], $event['end_date_year'])+$difference*86400;
					$end_date = gmdate('Y-m-d', $end_time);

					if (($start_date >= $first_day) && ($start_date <= $last_day) && (is_null($event_start_dts) || ($event_start_dts <= $day_end_dts))) {
						// add the first day
						$result_list[$assetid.':'.$i] = Array();
						for (reset($event); NULL !== ($k = key($event)); next($event)) {
							$result_list[$assetid.':'.$i][$k] = $event[$k];
						}

						$result_list[$assetid.':'.$i] = array_merge($result_list[$assetid.':'.$i],
														Calendar_Common::getDateComponents($event_start_date.' +'.$difference.' days', 'start_date_'),
														Calendar_Common::getDateComponents($start_date.' 24:00:00', 'end_date_'));

						$i++;

					}

					// add the intervening days
					for ($inter_time = $start_time + 86400; $inter_time < $end_time; $inter_time += 86400) {
						$inter_date = date('Y-m-d', $inter_time);
						if (($inter_date >= $first_day) && ($inter_date <= $last_day)) {

							$result_list[$assetid.':'.$i] = Array();
							for (reset($event); NULL !== ($k = key($event)); next($event)) {
								$result_list[$assetid.':'.$i][$k] = $event[$k];
							}

							$result_list[$assetid.':'.$i] = array_merge($result_list[$assetid.':'.$i],
																Calendar_Common::getDateComponents($inter_date.' 00:00:00', 'start_date_'),
																Calendar_Common::getDateComponents($inter_date.' 24:00:00', 'end_date_'));

							$result_list[$assetid.':'.$i]['partition_start']   = $day_start_time;

							$i++;
						}
					}

					if (($end_date >= $first_day) && ($end_date <= $last_day) && (is_null($event_end_dts) || ($event_end_dts > $day_start_dts))) {
						// add the last day
						$result_list[$assetid.':'.$i] = Array();
						for (reset($event); NULL !== ($k = key($event)); next($event)) {
							$result_list[$assetid.':'.$i][$k] = $event[$k];
						}

						$result_list[$assetid.':'.$i] = array_merge($result_list[$assetid.':'.$i],
																Calendar_Common::getDateComponents($end_date.' 00:00:00', 'start_date_'),
																Calendar_Common::getDateComponents($event_end_date.' +'.$difference.' days', 'end_date_'));

						$result_list[$assetid.':'.$i]['partition_start'] = $day_start_time;

						$i++;
					}

				} else {

					// fully-specified but falls within a single day
					$start_time = gmmktime(0, 0, 0, $event['start_date_mon'], $event['start_date_mday'], $event['start_date_year'])+$difference*86400;
					$start_date = date('Y-m-d', $start_time);
					if (($start_date >= $first_day) && ($start_date <= $last_day) &&
							(is_null($event_start_dts) ||
							 (($event_start_dts >= $day_start_dts) && ($event_start_dts < $day_end_dts)) ||
							 (($event_end_dts <= $day_end_dts) && ($event_end_dts > $day_start_dts)) ||
							 (($event_start_dts < $day_start_dts) && ($event_end_dts > $day_end_dts))
							)
						) {
						$result_list[$assetid.':'.$i] = Array();
						for (reset($event); NULL !== ($k = key($event)); next($event)) {
							$result_list[$assetid.':'.$i][$k] = $event[$k];
						}

						$event_date_components = array_merge(Calendar_Common::getDateComponents($event_start_date.' +'.$difference.' days', 'start_date_'),
															Calendar_Common::getDateComponents($event_end_date.' +'.$difference.' days', 'end_date_')
												);

						// If there is not a time specified, this event will be shown in the "all-day" row.
						// Null values need to be preserved for the time for this to occur
						$date_vars = Array(
										'start_date_hours', 'start_date_minutes', 'start_date_seconds',
										'end_date_hours', 'end_date_minutes', 'end_date_seconds',
									 );

						foreach ($date_vars as $date_var) {
							if (is_null($event[$date_var]) || empty($event[$date_var])) {
								$event_date_components[$date_var] = NULL;
							}
						}

						$result_list[$assetid.':'.$i] = array_merge($result_list[$assetid.':'.$i], $event_date_components);

						// Free memory
						$event_date_components = NULL;

						$i++;
					}
				}//end else

			} else {

				// end date not specified, so it's either just a date or a point in time
				$start_time = gmmktime(0, 0, 0,  $event['start_date_mon'], $event['start_date_mday'], $event['start_date_year'])+$difference*86400;
				$start_date = gmdate('Y-m-d', $start_time);

				if (($start_date >= $first_day) && ($start_date <= $last_day)) {
					$result_list[$assetid.':'.$i] = Array();
					for (reset($event); NULL !== ($k = key($event)); next($event)) {
						$result_list[$assetid.':'.$i][$k] = $event[$k];
					}

					if (!is_null($event['start_date_hours'])) {
						$result_list[$assetid.':'.$i] = array_merge($result_list[$assetid.':'.$i],
																Calendar_Common::getDateComponents($event_start_date.' +'.$difference.' days', 'start_date_'),
																Calendar_Common::getDateComponents($event_start_date.' +'.$difference.' days', 'end_date_'));
					} else {
						$result_list[$assetid.':'.$i] = array_merge($result_list[$assetid.':'.$i],
																Calendar_Common::getDateComponents($start_date.' 00:00:00', 'start_date_'),
																Calendar_Common::getDateComponents($start_date.' 24:00:00', 'end_date_'));
					}

					$i++;
				}

			}
			// get the next occurrence
			$next_occur = Calendar_Event_Recurring::getFirstOccurrenceAfter(add_days_to_iso($next_occur, 1), $event_start_date, $event['frequency'], $event['period'], $event_last_day);

		}//end while

	}//end _expandRecurringEvent()


	/**
	* Get the daystamp value for the supplied date
	*
	* @param int|string	$value	The date to convert, either a timestamp or ISO-format date
	*
	* @return int
	* @access public
	*/
	function _getDayStamp($value)
	{
		if (is_numeric($value)) {
			// timestamp
			return floor($value / 86400);
		} else {
			// iso format
			$value = strtotime(substr($value, 0, 10).' 00:00:00 Z') - strtotime('1970-01-01 00:00:00 Z');
			return floor($value / 86400);
		}

	}//end _getDayStamp()


	/**
	* Get the monthstamp value for the supplied date
	*
	* @param int|string	$value	The date to convert, either a timestamp or ISO-format date
	*
	* @return int
	* @access public
	*/
	function _getMonthStamp($value)
	{
		if (is_numeric($value)) {
			// convert timestamp to ISO
			$value = ts_iso8601($value);
		}
		list($year, $month) = sscanf($value, '%04d-%02d');
		return ($year * 12) + $month;

	}//end _getMonthStamp()


	/**
	* Returns a QUOTED array of a type's descendants
	*
	* @param string	$type_code	Type code
	*
	* @return array
	* @access public
	* @see Asset_Manager::getTypeDescendants()
	*/
	function _getTypeDescendantsQuoted($type_code)
	{
		// only needed because of quote()
		$db =MatrixDAL::getDb();

		$type_codes = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants($type_code);
		$type_codes[] = $type_code;

		foreach ($type_codes as $tc_key => $tc_value) {
			$type_codes[$tc_key] = MatrixDAL::quote($tc_value);
		}

		return $type_codes;

	}//end _getTypeDescendantsQuoted()


	/**
	* Removes the recurring events instances all but first one if event's
	* frequency is less than the predefined event frequency threshold
	*
	* @param array	$events			Array of Events
	* @param string	$first_day		Events instances
	*
	* @return array
	* @access public
	*/
	function limitRecurringEventInstances($events, $instances)
	{
		$frequency_threshold = $GLOBALS['SQ_SYSTEM']->getUserPrefs('calendar_event_recurring', 'SQ_CALENDAR_EVENTS_FREQUENCY');

		if ($frequency_threshold <= 0) {
			return $instances;
		}
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('calendar_event_recurring');

		$limit_events = Array();
		foreach($events as $event) {
			$recurring_descandants = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants('calendar_event_recurring');
			if (($event['type_code'] == 'calendar_event_recurring') || in_array($event['type_code'], $recurring_descandants)) {
				$event_frequency = Calendar_Event_Recurring::getEventFrequency($event);
				if ($event_frequency < $frequency_threshold) {
					$limit_events[$event['assetid']] = 1;
				}
			}//end if
		}//end foreach

		// If the events frequnecy is less than the defined frequency threshold,
		// remove all the instances but the first one
		foreach($instances as $key => $instance) {

			if (isset($limit_events[$instance['assetid']]) && $key != $instance['assetid'].':0') {
				unset($instances[$key]);
			}
		}//end foreach

		return $instances;

	}//end limitRecurringEventInstances()


	/**
	* Expands list of single day and/or recurring events into a series of whole event instances
	*
	* @param array	&$events		Array of Events
	* @param string	$first_day		The first day we want to display (YYYY-MM-DD)
	* @param string	$last_day		The last day we want to display (YYYY-MM-DD)
	* @param string	$day_start_time	Earliest time of day we will display (HH:mm)
	* @param string	$day_end_time	Latest time of day we will display (HH:mm)
	*
	* @return array
	* @access public
	*/
	function getWholeEventInstances($events, $first_day, $last_day, $day_start_time='00:00', $day_end_time='23:59')
	{
		$frequency_limit = $GLOBALS['SQ_SYSTEM']->getUserPrefs('calendar_event_recurring', 'SQ_CALENDAR_EVENTS_FREQUENCY');

		$multi_date_event = Array();
		foreach ($events as $key => $event) {
			$assetid = $event['assetid'];
		}

		$res = Array();
		foreach ($events as $key => $event) {
			$assetid = $event['assetid'];
			$recurring_descandants = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants('calendar_event_recurring');
			if (($event['type_code'] == 'calendar_event_recurring') || in_array($event['type_code'], $recurring_descandants)) {
				Calendar_Common::_getRecurringEventWholeInstances($res, $event, $assetid, $first_day, $last_day, $day_start_time, $day_end_time, $frequency_limit);
			} else {
				Calendar_Common::_getSingleEventWholeInstance($res, $event, $assetid, $first_day, $last_day, $day_start_time, $day_end_time);
			}
		}

		return $res;

	}//end expandEventList()


	/**
	* Returns the whole single event instance
	*
	* @param array	&$result_list	The result array to append the expanded events to
	* @param array	$event			The cached calendar components of the event to expand
	*								(format as per structure of cal_date_val table)
	* @param int	$assetid		The asset ID of the event being expanded
	* @param string	$first_day		The first day we want to display (YYYY-MM-DD)
	* @param string	$last_day		The last day we want to display (YYYY-MM-DD)
	* @param string	$day_start_time	Earliest time of day we will display (HH:mm)
	* @param string	$day_end_time	Latest time of day we will display (HH:mm)
	*
	* @return void
	* @access public
	*/
	function _getSingleEventWholeInstance(&$result_list, &$event, $assetid, $limit_start_day, $limit_end_day, $limit_start_time, $limit_end_time)
	{
		if (empty($limit_start_day ) || empty($limit_end_day)) {
			return;
		}

		// Event start time data
		$start_day = $event['start_date_year'].'-'.$event['start_date_mon'].'-'.$event['start_date_mday'];
		$start_time = (is_null($event['start_date_hours']) ? '00' : $event['start_date_hours']).':'.(is_null($event['start_date_minutes']) ? '00' : $event['start_date_minutes']).':00';
		$start_ts = strtotime($start_day.' '.$start_time);
		$start_time_offset = ((is_null($event['start_date_hours']) ? 0 : $event['start_date_hours'])*3600)+((is_null($event['start_date_minutes']) ? 0 : $event['start_date_minutes'])*60);

		// Event end time data
		if (!is_null($event['end_date_year'])) {
			$end_day = $event['end_date_year'].'-'.$event['end_date_mon'].'-'.$event['end_date_mday'];
			$end_time = (is_null($event['end_date_hours']) ? '23' : $event['end_date_hours']).':'.(is_null($event['end_date_minutes']) ? '59' : $event['end_date_minutes']).':00';
			if (strpos($end_time,'24:') !== FALSE) $end_time = '23:59:00';
			$end_ts = strtotime($end_day.' '.$end_time);
		} else {
			$end_day = $start_day;
			$end_time = '23:59:00';
			$end_ts = $start_ts;
		}

		$limit_start_ts = strtotime($limit_start_day.' '.$limit_start_time);
		$limit_end_ts = strtotime($limit_end_day.' '.$limit_end_time);

		// Check that the event is within the given limit
		if (($start_ts >= $limit_start_ts && $start_ts <= $limit_end_ts ) || ($start_ts <= $limit_start_ts && $end_ts >= $limit_start_ts )) {

			$type_code = $event['type_code'];
			$result = Array();

			$result[$event['assetid'].':start_date_ts='.$start_ts.'|end_date_ts='.$end_ts.'|type_code='.$type_code] = $start_ts;

			$result_list = array_merge($result_list, $result);
		}//end if

	}//end _getSingleEventWholeInstance()


	/**
	* Expands a list of recurring events into a series of whole events
	*
	* @param array	&$result_list	The result array to append the expanded events to
	* @param array	$event			The cached calendar components of the event to expand
	*								(format as per structure of cal_date_val table)
	* @param int	$assetid		The asset ID of the event being expanded
	* @param string	$first_day		The first day we want to display (YYYY-MM-DD)
	* @param string	$last_day		The last day we want to display (YYYY-MM-DD)
	* @param string	$day_start_time	Earliest time of day we will display (HH:mm)
	* @param string	$day_end_time	Latest time of day we will display (HH:mm)
	* @param string $frequency_threshold
	*
	* @return void
	* @access public
	*/
	function _getRecurringEventWholeInstances(&$result_list, $event, $assetid, $first_day, $last_day, $day_start_time, $day_end_time, $frequency_threshold)
	{
		// Event start time data (first instance)
		$start_day = $event['start_date_year'].'-'.str_pad($event['start_date_mon'],2,'0',STR_PAD_LEFT).'-'.str_pad($event['start_date_mday'],2,'0',STR_PAD_LEFT);
		$start_time = (is_null($event['start_date_hours']) ? '00' : $event['start_date_hours']).':'.(is_null($event['start_date_minutes']) ? '00' : $event['start_date_minutes']).':00';

		// Event end time data (first instance)
		if (!is_null($event['end_date_year'])) {
			$end_day = $event['end_date_year'].'-'.str_pad($event['end_date_mon'],2,'0',STR_PAD_LEFT).'-'.str_pad($event['end_date_mday'],2,'0',STR_PAD_LEFT);
			$end_time = (is_null($event['end_date_hours']) ? '23' : $event['end_date_hours']).':'.(is_null($event['end_date_minutes']) ? '59' : $event['end_date_minutes']).':00';
			if (strpos($end_time,'24:') !== FALSE) $end_time = '23:59:00';
		} else {
			$end_day = $start_day;
			$end_time = '23:59:00';
		}

		$event_period_ts = strtotime($end_day.' '.$end_time) - strtotime($start_day.' '.$start_time);
		if ($event_period_ts < 0) return;

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('calendar_event_recurring');

		$after = $first_day;
		$type_code = $event['type_code'];
		$event_frequency = Calendar_Event_Recurring::getEventFrequency($event);

		// Event runs untill
		$stop_day = $event['stop_date'] == '----------'  ? $last_day : $event['stop_date'];

		do {
			$next_occurance = Calendar_Event_Recurring::getFirstOccurrenceAfter($after, $start_day, $event['frequency'], $event['period'], $stop_day);
			if ($next_occurance > $last_day) break;

			if ($next_occurance) {
				$i_start_ts = strtotime($next_occurance.' '.$start_time);
				$i_end_ts = $i_start_ts+$event_period_ts;
				$result_list[$assetid.':start_date_ts='.$i_start_ts.'|end_date_ts='.$i_end_ts.'|type_code='.$type_code] = $i_start_ts;

				// Based on recurring event frequency set, put only one instance for given recurring event
				if ($event_frequency < $frequency_threshold) {
					break;
				}

				$after = add_days_to_iso($next_occurance);
			}
		} while($next_occurance);

	}//end _getRecurringEventWholeInstances()

}//end class
?>
