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
* $Id: calendar_common.inc,v 1.39 2006/01/26 22:34:16 lwright Exp $
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
* @version $Revision: 1.39 $
* @package MySource_Matrix_Packages
* @subpackage calendar
*/

require_once SQ_FUDGE_PATH.'/general/datetime.inc';


/**
* Builds the common part of the single events query
*
* @param array	&$trees	(int => string) an array of tree ids indexed by root node
*
* @return string	the common part of the SQL query
* @access public
*/
function build_single_event_query_base(&$trees, $group_by_root_node=false, $event_type_code='calendar_event_single', $include_exceptions=true)
{
	$db =& $GLOBALS['SQ_SYSTEM']->db;

	$type_codes = quoted_descendants($event_type_code);
	if ($include_exceptions) {
		$type_codes = array_merge($type_codes, quoted_descendants('calendar_event_cancellation'));
		$type_codes = array_merge($type_codes, quoted_descendants('calendar_event_modification'));
	}

	if ($group_by_root_node) {
		$sql = 'SELECT ('.build_case_statement('t.treeid',$trees).'), a.assetid, a.type_code, a.name, t.treeid as root_node, cd.*'."\n";
	} else {
		$sql = 'SELECT a.assetid, a.type_code, a.name, t.treeid, ('.build_case_statement('t.treeid',$trees).') as root_node, cd.*'."\n";
	}
	if (!$GLOBALS['SQ_SYSTEM']->userSystemAdmin() && !$GLOBALS['SQ_SYSTEM']->userRoot()) {
		$sql .= ', p.granted as access';
	}
	$sql .= "\n".'FROM '.build_join_statements()."\n";
	$sql .= 'WHERE a.type_code IN ( '.implode(',', $type_codes).') '."\n";
	$sql .= "\t".'AND (a.status >= '.$db->quoteSmart(SQ_STATUS_LIVE).")\n\t";;

	if ($GLOBALS['SQ_SYSTEM']->currentUserId()) {
		if (!$GLOBALS['SQ_SYSTEM']->userSystemAdmin() && !$GLOBALS['SQ_SYSTEM']->userRoot()) {
			$userids = array_keys($GLOBALS['SQ_SYSTEM']->am->getParents($GLOBALS['SQ_SYSTEM']->user->id, 'user_group', false));
			array_push($userids, 0, (int) $GLOBALS['SQ_SYSTEM']->user->id);
			for (reset($userids); NULL !== ($i = key($userids)); next($userids)) {
				$userids[$i] = $db->quoteSmart($userids[$i]);
			}
			$userid_cond = ' AND p.userid  IN ('.implode(',', $userids).')';
		}
	} else {
		// public users
		$userid_cond = ' AND p.userid = 0';
	}
	if (isset($userid_cond)) {
		$sql .= $userid_cond.' AND p.granted = 1 AND (p.permission >= '.$db->quoteSmart(SQ_PERMISSION_WRITE).' OR (status >= '.$db->quoteSmart(SQ_STATUS_LIVE).' AND p.permission >= '.$db->quoteSmart(SQ_PERMISSION_READ).'))';
	}

	return $sql;

}//end build_single_event_query_base()


function build_join_statements()
{
	$base = ($GLOBALS['SQ_SYSTEM']->userSystemAdmin() || $GLOBALS['SQ_SYSTEM']->userRoot()) ? SQ_TABLE_RUNNING_PREFIX.'ast a' : '('.SQ_TABLE_RUNNING_PREFIX.'ast a JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_perm p ON a.assetid = p.assetid)';
	return '('.$base.' JOIN ('.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk_tree t ON l.linkid = t.linkid) ON l.minorid = a.assetid) JOIN '.SQ_TABLE_RUNNING_PREFIX.'cal_date_val cd ON a.assetid = cd.assetid';

}//end build_join_statements()


/**
* Build SQL to express the unix timestamp represented by the supplied date component expressions
*
* @param string	$y	Expression for the year component of the date
* @param string	$m	Expression for the month component of the date
* @param string	$d	Expression for the day component of the date
*
* @access public
* @return void
*/
function make_timestamp_sql($y, $m, $d)
{
	return "EXTRACT(epoch from DATE (($y) || '-' || ($m) || '-' || ($d)))";

}//end make_timestamp_sql()


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
* @access public
*/
function build_recurring_event_query_base(&$trees, $view, $iso, $length=null, $group_by_root_node=false, $event_type='calendar_event_recurring')
{
	list($year,$month,$day) = sscanf(substr($iso,0,10), '%4d-%2d-%2d');
	if (!$month) $month = 12;
	if (!$day) $day = 1;
	if (($view == 'period') && ($length == 0)) {
		$view = 'day';
	}

	$now = get_date_components(sprintf('%04d-%02d-%02d', $year, $month, $day));
	$wdom_list = weekday_dates_in_month($now['wday'], $month, $year);

	$db = &$GLOBALS['SQ_SYSTEM']->db;

	$type_codes = quoted_descendants($event_type);

	if ($group_by_root_node) {
		$sql = 'SELECT ('.build_case_statement('t.treeid',$trees).'), a.assetid, a.type_code, a.name, t.treeid as root_node, cd.*'."\n";
	} else {
		$sql = 'SELECT a.assetid, a.type_code, a.name, t.treeid, ('.build_case_statement('t.treeid',$trees).') as root_node, cd.*'."\n";
	}
	if (!$GLOBALS['SQ_SYSTEM']->userSystemAdmin() && !$GLOBALS['SQ_SYSTEM']->userRoot()) {
		$sql .= ', p.granted as access';
	}
	$sql .= "\nFROM ". build_join_statements()." \nWHERE (a.type_code IN (".implode(',', $type_codes).")) \n\tAND (a.status >= ".$db->quoteSmart(SQ_STATUS_LIVE).")\n";

	if ($GLOBALS['SQ_SYSTEM']->currentUserId()) {
		if (!$GLOBALS['SQ_SYSTEM']->userSystemAdmin() && !$GLOBALS['SQ_SYSTEM']->userRoot()) {
			$userids = array_keys($GLOBALS['SQ_SYSTEM']->am->getParents($GLOBALS['SQ_SYSTEM']->user->id, 'user_group', false));
			array_push($userids, 0, (int) $GLOBALS['SQ_SYSTEM']->user->id);
			for (reset($userids); NULL !== ($i = key($userids)); next($userids)) {
				$userids[$i] = $db->quoteSmart($userids[$i]);
			}
			$userid_cond = ' AND p.userid  IN ('.implode(',', $userids).')';
		}
	} else {
		// public users
		$userid_cond = ' AND p.userid = 0';
	}
	// if user id condition is set - currently not set when user is root or system admin
	if (isset($userid_cond)) {
		$sql .= $userid_cond.' AND p.granted = 1 AND (p.permission >= '.$db->quoteSmart(SQ_PERMISSION_WRITE).' OR (status >= '.$db->quoteSmart(SQ_STATUS_LIVE).' AND p.permission >= '.$db->quoteSmart(SQ_PERMISSION_READ).'))';
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
				$sql_freqs[] = 'frequency = '.$db->quoteSmart('DWD');
			} else {
				$sql_freqs[] = 'frequency = '.$db->quoteSmart('DWE');
			}

			$sql_freqs[] = '(frequency = '.$db->quoteSmart('DED').' AND mod(('.$db->quoteSmart($now['ds']).' - start_date_ds), period) BETWEEN 0 AND COALESCE(end_date_ds - start_date_ds,0))';

			$sql_freqs[] = '(frequency = '.$db->quoteSmart('WEW').' AND mod(('.$db->quoteSmart($now['ds']).' - start_date_ds), (period*7)) BETWEEN 0 AND COALESCE(end_date_ds - start_date_ds,0))';

			$sql_freqs[] = '
			(SUBSTR(frequency, 1, 1) = '.$db->quoteSmart('M').') AND (
				(
					/* there is an instance that starts in our month */
				   (MOD('.$db->quoteSmart($now['ms']).' - start_date_ms, period) = 0)
				   AND (
						/* our view date is between the start and end of the instance */
						((frequency = '.$db->quoteSmart('MFN').') AND (('.$db->quoteSmart($now['mday']).' - start_date_mday) BETWEEN 0 AND (COALESCE((end_date_ds - start_date_ds), 0))))
						OR ((frequency = '.$db->quoteSmart('MRN').') AND ((start_date_rmday - '.$db->quoteSmart($now['rmday']).') BETWEEN 0 AND (COALESCE((end_date_ds - start_date_ds), 0))))
						OR ((frequency = '.$db->quoteSmart('MFW').') AND (('.$db->quoteSmart($now['mday']).' - (((start_date_wdom - 1) * 7) + MOD(start_date_wday + 7 - '.$now['month_start_wday'].', 7) + 1)) BETWEEN 0 AND (COALESCE((end_date_ds - start_date_ds), 0))))
						OR ((frequency = '.$db->quoteSmart('MRW').') AND ((((start_date_rwdom - 1) * 7) + MOD('.($now['month_end_wday'] + 7).' - start_date_wday, 7) + 1 - '.$now['rmday'].') BETWEEN 0 AND (COALESCE((end_date_ds - start_date_ds), 0))))
				  )
				) OR (
						/* An instance that started in a previous month doesnt end til a subsequent month */
						(MOD('.$now['ms'].' - start_date_ms, period) < MOD('.$now['ms'].' - end_date_ms, period))
				) OR (
					/* An instance ends this month */
					((start_date_ms != end_date_ms) AND (MOD('.$now['ms'].' - end_date_ms, period) = 0))
				   AND (
					   /* its end is PROBABLY after our view date - we can only be ~90% sure here */
						((frequency = '.$db->quoteSmart('MFN').')
						   AND ((end_date_ds - start_date_ds + 1) >= ROUND(('.$db->quoteSmart($now['ts']).' - '.make_timestamp_sql('FLOOR((('.$now['year'].' * 12) + '.($now['mon']-1).' - (end_date_ms - start_date_ms)) / 12)', 'MOD('.$now['mon'].' + 10 - MOD('.($now['ms']-1).' - start_date_ms, period), 12) + 1', 'start_date_mday').') / 86400)))
						OR ((frequency = '.$db->quoteSmart('MRN').')
							AND ((end_date_ds - start_date_ds + 1) >= ROUND(('.$db->quoteSmart($now['ts']).' - '.make_timestamp_sql('FLOOR((('.$now['year'].' * 12) + '.($now['mon']-1).' - (end_date_ms - start_date_ms)) / 12)', 'MOD('.$now['mon'].' + 10 - MOD('.($now['ms']-1).' - start_date_ms, period), 12) + 1', '((start_date_mday + 3) - (FLOOR((start_date_mday + 3) / 31) * MOD(start_date_mday + 3, 31)))').') / 86400)))
						OR ((frequency = '.$db->quoteSmart('MFW').')
							AND ((end_date_ds - start_date_ds + 1) >= ROUND(('.$db->quoteSmart($now['ts']).' - '.make_timestamp_sql('FLOOR((('.$now['year'].' * 12) + '.($now['mon']-1).' - (end_date_ms - start_date_ms)) / 12)', 'MOD('.$now['mon'].' + 10 - MOD('.($now['ms']-1).' - start_date_ms, period), 12) + 1', '(7 * start_date_wdom) - (FLOOR((7 * start_date_wdom) / 31) * MOD(7 * start_date_wdom, 31))').') / 86400)))
						OR ((frequency = '.$db->quoteSmart('MRW').')
							AND ((end_date_ds - start_date_ds + 1) >= ROUND(('.$db->quoteSmart($now['ts']).' - '.make_timestamp_sql('FLOOR((('.$now['year'].' * 12) + '.($now['mon']-1).' - (end_date_ms - start_date_ms)) / 12)', 'MOD('.$now['mon'].' + 10 - MOD('.($now['ms']-1).' - start_date_ms, period), 12) + 1', '(31 - ((start_date_rwdom - 1) * 7))').') / 86400)))
					)
				) OR (
						/* An instance that started last month has run into our month
						   even though its original instance was contained in only one month
						   and it ends on/after our view date */
						(MOD('.$now['ms'].' - start_date_ms, period) = 1)
						   AND (
							((frequency = '.$db->quoteSmart('MFN').')
								AND ((start_date_mday + (end_date_ds - start_date_ds)) - '.$prev_month_length.' >= '.$now['mday'].'))
							OR ((frequency = '.$db->quoteSmart('MFD').')
								AND (((MOD(7 + '.$prev_month_start_wday.' - start_date_wday, 7) + 1) + ((start_date_wdom - 1) * 7) + (end_date_ds - start_date_ds)) - '.$prev_month_length.' >= '.$now['mday'].'))
							OR ((frequency = '.$db->quoteSmart('MRD').')
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

			$sql_freqs[] = 'frequency = '.$db->quoteSmart('DWD');
			$sql_freqs[] = 'frequency = '.$db->quoteSmart('DWE');

			// note: the slightly redundant "*1.0" is so Postgres interprets it as a
			// double instead of an int, blowing away our attempts to use floor()
			$sql_freqs[] = '(frequency = '.$db->quoteSmart('DED').' AND ((FLOOR(('.$db->quoteSmart($now['ds']-1).' - start_date_ds) / (period*1.0)) < FLOOR(('.$db->quoteSmart($now['ds']+6).' - start_date_ds) / (period*1.0))) or mod(('.$db->quoteSmart($now['ds']).' - start_date_ds), period) BETWEEN 0 AND COALESCE(end_date_ds - start_date_ds,0)))';

			$sql_freqs[] = '(frequency = '.$db->quoteSmart('WEW').' AND ((FLOOR(('.$db->quoteSmart($now['ds']-1).' - start_date_ds)/(period*7.0)) < FLOOR(('.$db->quoteSmart($now['ds']+6).' - start_date_ds)/(period*7.0))) OR MOD(('.$db->quoteSmart($now['ds']).' - start_date_ds), period) BETWEEN 0 AND COALESCE(end_date_ds - start_date_ds,0)))';

			$sql_freqs_m = Array();

			// every X months on xth of the month
			$sql_freqs_m[] = '(frequency = '.$db->quoteSmart('MFN').' AND start_date_mday BETWEEN '.$db->quoteSmart(intval($day)).' AND '. $db->quoteSmart(min($day+6, $days_in_month)) .')';

			// every X months on xth weekday of the month
			$sql_freqs_m[] = '(frequency = '.$db->quoteSmart('MFW').' AND start_date_wdom IN ( '.$db->quoteSmart($now['wdom']).($req_next_week ? ','.$db->quoteSmart($now['wdom']+1) : '').'))';

			// every X months on xth last day of the month
			$sql_freqs_m[] = '(frequency = '.$db->quoteSmart('MRN').' AND start_date_rmday BETWEEN '.$db->quoteSmart(max($days_in_month - $day - 5, 1)).' AND '.$db->quoteSmart(intval($days_in_month - $day + 1)).')';

			// every X months on xth last weekday of the month
			$sql_freqs_m[] = '(frequency = '.$db->quoteSmart('MRW').' AND start_date_rwdom IN ( '.$db->quoteSmart($now['rwdom']).($req_rev_next_week ? ','.$db->quoteSmart($now['rwdom']-1) : '').'))';

			$sql_freqs[] = '(('.implode("\n\t".' OR ', $sql_freqs_m).') AND (MOD('.$db->quoteSmart($now['ms']).' - start_date_ms, period) = 0))';

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

				$sql_freqs_m[] = '(frequency = '.$db->quoteSmart('MFN').' AND start_date_mday BETWEEN '.$db->quoteSmart(1).' AND '. $db->quoteSmart($day+6-$days_in_month).')';

				// every X months on xth weekday of the month - it's always going to
				// be the first week of a new month
				$sql_freqs_m[] = '(frequency = '.$db->quoteSmart('MFW').' AND start_date_wdom = 1)';

				// every X months on xth last day of the month
				$sql_freqs_m[] = '(frequency = '.$db->quoteSmart('MRN').' AND start_date_rmday BETWEEN '.$db->quoteSmart($days_in_month * 2 - $day - 5).' AND '.$db->quoteSmart(intval($days_in_month)).')';

				// every X months on xth last weekday of the month
				$sql_freqs_m[] = '(frequency = '.$db->quoteSmart('MRW').' AND start_date_rwdom IN ( '.$db->quoteSmart(count($wdom_list)).($req_rev_next_week ? ','.$db->quoteSmart(count($wdom_list)-1) : '').'))';

				$sql_freqs[] = '(('.implode("\n\t".' OR ', $sql_freqs_m).') AND (MOD('.$db->quoteSmart($now['ms']+1).' - start_date_ms, period) = 0))';
			}

			$sql_freqs[] = '(frequency IN ('.$db->quoteSmart('MFN').', '.$db->quoteSmart('MRN').') AND (end_date_ds - start_date_ds >= '.$day.'))';

			$sql_freqs[] = '(frequency IN ('.$db->quoteSmart('MFW').', '.$db->quoteSmart('MRW').') AND (end_date_ds - start_date_ds >= 1 + ((start_date_rwdom-1) * 7)))';

		break;

		case 'month' :
			$last_day = $now['ds'] + days_in_month($month,$year) - 1;

			$sql_freqs[] = '(frequency = '.$db->quoteSmart('DWD').')';
			$sql_freqs[] = '(frequency = '.$db->quoteSmart('DWE').')';

			$sql_freqs[] = '(frequency = '.$db->quoteSmart('DED').' AND (FLOOR(('.$db->quoteSmart($now['ds']-1).' - start_date_ds)/period) < FLOOR(('.$db->quoteSmart($last_day).' - start_date_ds)/period)))';

			$sql_freqs[] = '(frequency = '.$db->quoteSmart('WEW').' AND period = 1)';

			$sql_freqs[] = '(frequency = '.$db->quoteSmart('WEW').' AND (floor(('.$db->quoteSmart($now['ds']-1).' - start_date_ds)/(period*7)) < FLOOR('.$db->quoteSmart($last_day).' - start_date_ds)/(period*7)))';

			// all monthly views can be lumped together on this one
			$sql_freqs[] = '(frequency LIKE '.$db->quoteSmart('M%').' AND (MOD(('.$db->quoteSmart($now['ms']).' - start_date_ms), period) = 0))';

			$iso = $year.'-'.sprintf('%02d', $month).'-01';

		break;

		case 'year' :
			$last_day = $now['ds']+364+is_leap_year($year);

			$sql_freqs[] = 'frequency = '.$db->quoteSmart('DWD');
			$sql_freqs[] = 'frequency = '.$db->quoteSmart('DWE');

			$sql_freqs[] = '(frequency = '.$db->quoteSmart('DED').' AND (FLOOR(('.$db->quoteSmart($now['ds']-1). ' - start_date_ds)/period) < FLOOR(('.$db->quoteSmart($last_day).' - start_date_ds)/period)))';

			$sql_freqs[] = '(frequency = '.$db->quoteSmart('WEW').' AND period = 1)';

			$sql_freqs[] = '(frequency = '.$db->quoteSmart('WEW').' AND (FLOOR(('.$db->quoteSmart($now['ds']-1).' - start_date_ds)/(period*7)) < FLOOR(('.$db->quoteSmart($last_day).' - start_date_ds)/(period*7))))';

			// all monthly views can be lumped together on this one
			// but we need to do something like the 'every day' type thing to work out
			// whether an occurrence will happen this year
			$sql_freqs[] = '(frequency LIKE '.$db->quoteSmart('M%').' AND (FLOOR(('.($year * 12).' - start_date_ms) / period) < FLOOR(('.(($year + 1) * 12).' - start_date_ms) / period)))';
		break;

		case 'period':
			$last_date = date('Y-m-d', strtotime("$iso 12:00:00 +$length days"));
			$last_date_components = get_date_components($last_date);
			$last_day = $last_date_components['ds'];

			$sql_freqs[] = '((frequency = '.$db->quoteSmart('DED').') AND ((period <= '.$length.') OR (FLOOR(('.$now['ds'].' - start_date_ds) / period) < FLOOR(('.$last_date_components['ds'].' - start_date_ds) / period)) OR (floor(('.$now['ds'].' - end_date_ds) / period) < FLOOR(('.$last_date_components['ds'].' - end_date_ds) / period))))';

			if (($length > 2) || !in_array($now['wday'], Array(0, 6)) || !in_array($last_date_components['wday'], Array(0, 6))) {
				// the period includes a weekday
				$sql_freqs[] = '(frequency = '.$db->quoteSmart('DWD').')';
			}
			if (($length > 5) || in_array($now['wday'], Array(0, 6)) || in_array($last_date_components['wday'], Array(0, 6)) || ($last_date_components['wday'] < $now['wday'])) {
				// the period includes a weekend day
				$sql_freqs[] = '(frequency = '.$db->quoteSmart('DWE').')';
			}

			$sql_freqs[] = '((frequency = '.$db->quoteSmart('WEW').') AND (((period * 7) <= '.$length.') OR (FLOOR(('.($now['ds']-1).' - start_date_ds) / (period * 7)) < FLOOR(('.$last_date_components['ds'].' - start_date_ds) / (period * 7))) OR (FLOOR(('.($now['ds']-1).' - end_date_ds) / (period * 7)) < FLOOR(('.$last_date_components['ds'].' - end_date_ds) / (period * 7)))))';

			$monthly_conditions = Array();

			if ($last_date_components['ms'] == $now['ms']) {
				// we are within one month

				// get MFN and MRN events
				$monthly_conditions[] = '((frequency = '.$db->quoteSmart('MFN').') AND (((MOD('.$now['ms'].' - start_date_ms, period) = 0) AND (start_date_mday BETWEEN '.$now['mday'].' AND '.$last_date_components['mday'].')) OR ((MOD('.$now['ms'].' - end_date_ms, period) = 0) AND (end_date_mday BETWEEN '.$now['mday'].' AND '.$last_date_components['mday'].'))))';
				$monthly_conditions[] = '((frequency = '.$db->quoteSmart('MRN').') AND ((start_date_rwdom BETWEEN '.$last_date_components['rwdom'].' AND '.$now['rwdom'].') OR (end_date_rwdom BETWEEN '.$last_date_components['rwdom'].' AND '.$now['rwdom'].')))';

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
						$mfw_conditions[] = '((frequency = '.$db->quoteSmart('MFW').') AND ((start_date_wdom BETWEEN '.($now['wdom'] + 1).' AND '.($last_date_components['wdom'] - 1).') OR (end_date_wdom BETWEEN ' .($now['wdom'] + 1).' AND '.($last_date_components['wdom'] - 1).')))';
					}
				}
				$monthly_conditions[] = '((frequency = '.$db->quoteSmart('MFW').") AND (".implode(" OR ", $mfw_conditions)."\n\n     ))";

				// get MRW events
				$mrw_conditions = Array();
				if ($last_date_components['rwdom'] == $now['rwdom']) {
					// we are within one reverse-month-week

					$mrw_conditions[] = '((MOD('.$now['ms'].' - start_date_ms, period) = 0)
						AND (start_date_rwdom = '.$now['rwdom'].')
						AND (start_date_rmday - (7 * (start_date_rwdom - 1)) BETWEEN '.($now['rmday'] - (7 * ($now['rwdom'] - 1))).' AND '.($last_date_components['rmday'] - (7 * ($last_date_components['rwdom'] - 1))).'))';
					$mrw_conditions[] = '(((MOD('.$now['ms'].' - end_date_ms, period) = 0)
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
						$mrw_conditions[] = '((start_date_rwdom BETWEEN '.($now['rwdom'] - 1).' AND '.($last_date_components['rwdom'] + 1).') OR (end_date_rwdom BETWEEN ' .($now['rwdom'] - 1).' AND '.($last_date_components['rwdom'] + 1).'))';
					}
				}
				$monthly_conditions[] = '((frequency = '.$db->quoteSmart('MRW').") AND (".implode(" OR ", $mrw_conditions).'))';

				// get month events that cover the whole of our month
				$sql_freqs[] = '(
								/* begin month options */
									(frequency LIKE '.$db->quoteSmart('M%').')
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
				$sql_freqs[] = '((frequency = '.$db->quoteSmart('MFN').') AND (((MOD('.$now['ms'].' - start_date_ms, period) = 0) AND (start_date_mday >= '.$now['mday'].')) OR ((MOD('.$now['ms'].' - end_date_ms, period) = 0) AND (end_date_mday >= '.$now['mday'].'))))';

				$sql_freqs[] = '((frequency = '.$db->quoteSmart('MRN').') AND (((MOD('.$now['ms'].' - start_date_ms, period) = 0) AND (start_date_rmday <= '.$now['rmday'].')) OR ((MOD('.$now['ms'].' - end_date_ms, period) = 0) AND (end_date_rmday <= '.$now['rmday'].'))))';

				$sql_freqs[] = '((frequency = '.$db->quoteSmart('MFW').') AND (MOD('.$now['ms'].' - start_date_ms, period) = 0) AND ((start_date_wdom > '.$now['wdom'].') OR (((start_date_wdom = '.$now['wdom'].') AND (start_date_mday - ((start_date_wdom - 1) * 7) >= '.($now['mday'] - (($now['wdom'] - 1) * 7)).')))))';

				$sql_freqs[] = '((frequency = '.$db->quoteSmart('MFW').') AND (MOD('.$now['ms'].' - end_date_ms, period) = 0) AND ((end_date_wdom > '.$now['wdom'].') OR (((end_date_wdom = '.$now['wdom'].') AND (end_date_mday - ((end_date_wdom - 1) * 7) >= '.($now['mday'] - (($now['wdom'] - 1) * 7)).')))))';

				$sql_freqs[] = '((frequency = '.$db->quoteSmart('MRW').') AND (MOD('.$now['ms'].' - start_date_ms, period) = 0) AND ((start_date_rwdom < '.$now['rwdom'].') OR (((start_date_rwdom = '.$now['rwdom'].') AND (start_date_rmday - ((start_date_rwdom - 1) * 7) <= '.($now['rmday'] - (($now['rwdom'] - 1) * 7)).')))))';

				$sql_freqs[] = '((frequency = '.$db->quoteSmart('MRW').') AND (MOD('.$now['ms'].' - end_date_ms, period) = 0) AND ((end_date_rwdom > '.$now['rwdom'].') OR (((end_date_rwdom = '.$now['rwdom'].') AND (end_date_rmday - ((end_date_rwdom - 1) * 7) <= '.($now['rmday'] - (($now['rwdom'] - 1) * 7)).')))))';

				// or early enough in the last month
				$sql_freqs[] = '((frequency = '.$db->quoteSmart('MFN').') AND (((MOD('.$last_date_components['ms'].' - start_date_ms, period) = 0) AND (start_date_mday <= '.$last_date_components['mday'].')) OR ((MOD('.$last_date_components['ms'].' - end_date_ms, period) = 0) AND (end_date_mday <= '.$last_date_components['mday'].'))))';

				$sql_freqs[] = '((frequency = '.$db->quoteSmart('MRN').') AND (((MOD('.$last_date_components['ms'].' - start_date_ms, period) = 0) AND (start_date_rmday >= '.$last_date_components['rmday'].')) OR ((MOD('.$last_date_components['ms'].' - end_date_ms, period) = 0) AND (end_date_rmday >= '.$last_date_components['rmday'].'))))';

				$sql_freqs[] = '((frequency = '.$db->quoteSmart('MFW').') AND (MOD('.$now['ms'].' - start_date_ms, period) = 0) AND ((start_date_wdom < '.$now['wdom'].') OR (((start_date_wdom = '.$now['wdom'].') AND (start_date_mday - ((start_date_wdom - 1) * 7) <= '.($now['mday'] - (($now['wdom'] - 1) * 7)).')))))';

				$sql_freqs[] = '((frequency = '.$db->quoteSmart('MFW').') AND (MOD('.$now['ms'].' - end_date_ms, period) = 0) AND ((end_date_wdom < '.$now['wdom'].') OR (((end_date_wdom = '.$now['wdom'].') AND (end_date_mday - ((end_date_wdom - 1) * 7) <= '.($now['mday'] - (($now['wdom'] - 1) * 7)).')))))';

				$sql_freqs[] = '((frequency = '.$db->quoteSmart('MRW').') AND (MOD('.$now['ms'].' - start_date_ms, period) = 0) AND ((start_date_rwdom > '.$now['rwdom'].') OR (((start_date_rwdom = '.$now['rwdom'].') AND (start_date_rmday - ((start_date_rwdom - 1) * 7) >= '.($now['rmday'] - (($now['rwdom'] - 1) * 7)).')))))';

				$sql_freqs[] = '((frequency = '.$db->quoteSmart('MRW').') AND (MOD('.$now['ms'].' - end_date_ms, period) = 0) AND ((end_date_rwdom > '.$now['rwdom'].') OR (((end_date_rwdom = '.$now['rwdom'].') AND (end_date_rmday - ((end_date_rwdom - 1) * 7) <= '.($now['rmday'] - (($now['rwdom'] - 1) * 7)).')))))';

				if ($last_date_components['ms'] - $now['ms'] > 1) {
					// we have intervening months, so we want to see if the event starts or ends in one of them
					$sql_freqs[] = '((frequency LIKE '.$db->quoteSmart('M').') AND ((MOD('.$last_date_components['ms'].' - start_date_ms, period) BETWEEN 1 AND '.($last_date_components['ms'] - $now['ms'] - 1).') OR (MOD('.$last_date_components['ms'].' - end_date_ms, period) BETWEEN 1 AND '.($last_date_components['ms'] - $now['ms'] - 1).')))';
				}
			}

		break;

	}//end switch on $view
	$sql .= "(/* begin sql freqs */\n\n".implode("\n\n\nOR\n\n\n ", $sql_freqs)."\n/* end sql freqs */\n)\n\n";

	// stop date
	$sql .= ' AND ('.$last_day.' >= start_date_ds)';
	$sql .= ' AND (stop_date = '.$db->quoteSmart('----------').' OR stop_date >= '.$db->quoteSmart($iso).')';

	return $sql;

}//end build_recurring_event_query_base()


/**
* Returns an SQL fragment to restrict results to have treeids in the given range
* @param array	&$treeids		array of treeids
*
* @return string	the SQL clause
* @access public
*/
function build_tree_clause($treeids)
{
	$db =& $GLOBALS['SQ_SYSTEM']->db;
	$tree_options = Array();
	foreach ($treeids as $treeid) {
		$tree_options[] .= '(SUBSTR(t.treeid, 1, '.strlen($treeid).') LIKE '.$db->quoteSmart($treeid.'%').')';
	}
	return '('.implode(' OR ', $tree_options).')';

}//end build_tree_clause()


/**
* Returns the treeids that stem from the root nodes passed to it
*
* @param array	&$root_nodes	array of asset ids which represent root nodes
*
* @return array(int => string) a list of tree branch ids indexed by root assetid
* @access public
*/
function &get_treeids(&$root_nodes)
{
	if (empty($root_nodes)) return ($empty = Array());

	$db =& $GLOBALS['SQ_SYSTEM']->db;
	$quoted_root_nodes = array_keys($root_nodes);
	$sql = "SELECT a.assetid, t.treeid \n FROM ".SQ_TABLE_RUNNING_PREFIX."ast a JOIN (".SQ_TABLE_RUNNING_PREFIX."ast_lnk l JOIN ".SQ_TABLE_RUNNING_PREFIX."ast_lnk_tree t ON l.linkid = t.linkid) ON l.minorid = a.assetid \n WHERE a.assetid IN (";

	// quote the root nodes manually before we implode the thing
	foreach ($quoted_root_nodes as $k => $root_node) {
		$quoted_root_nodes[$k] = $db->quoteSmart($quoted_root_nodes[$k]);
	}
	$sql .= implode(', ', $quoted_root_nodes).')';
	$sql .= ' ORDER BY t.treeid ASC';

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
* @access public
*/
function build_case_statement($col_name, &$trees)
{
	$db =& $GLOBALS['SQ_SYSTEM']->db;
	$sql = 'CASE';
	foreach ($trees as $root_node => $treeid) {
		$sql .= ' WHEN '.$col_name.' LIKE '.$db->quoteSmart($treeid.'%').' THEN '.$db->quoteSmart($root_node);
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
* @access public
*/
function &expand_event_list(&$events, $first_day, $last_day, $day_start_time='00:00', $day_end_time='23:59')
{
	$res = Array();
	foreach (array_keys($events) as $key) {
		$event =& $events[$key];
		$recurring_descandants = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants('calendar_event_recurring');
		if (($event['type_code'] == 'calendar_event_recurring') || in_array($event['type_code'], $recurring_descandants)) {
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
function expand_single_event(&$result_list, &$event, $assetid, $first_day, $last_day, $day_start_time, $day_end_time)
{
	list($day_start_hours, $day_start_minutes) = sscanf($day_start_time, '%02d:%02d');
	list($day_end_hours, $day_end_minutes) = sscanf($day_end_time, '%02d:%02d');
	$day_start_dts = ($day_start_hours * 60) + $day_start_minutes;
	$day_end_dts = ($day_end_hours * 60) + $day_end_minutes;

	$event_start_date = sprintf('%04d-%02d-%02d', $event['start_date_year'], $event['start_date_mon'], $event['start_date_mday']);
	$event_start_dts = ($event['start_date_hours'] * 60) + $event['start_date_minutes'];

	$i = 0;

	if (!is_null($event['end_date_ds'])) {
		// end date is specified
		$event_end_date = sprintf('%04d-%02d-%02d', $event['end_date_year'], $event['end_date_mon'], $event['end_date_mday']);
		$event_end_dts = (($event['end_date_hours'] * 60) + $event['end_date_minutes']);

		if ($event['start_date_ds'] != $event['end_date_ds']) {
			// covers multiple days
			$event['expanded'] = 1;
			$start_time = mktime(12, 0, 0, $event['start_date_mon'], $event['start_date_mday'], $event['start_date_year']);

			$end_time = mktime(12, 0, 0, $event['end_date_mon'], $event['end_date_mday'], $event['end_date_year']);

			if (($first_day <= $event_start_date) && ($event_start_dts < $day_end_dts)) {
				// insert first day
				$result_list[$assetid.':'.$i] = Array();
				for (reset($event); null !== ($k = key($event)); next($event)) {
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
				for (reset($event); null !== ($k = key($event)); next($event)) {
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

			if (($event_end_date <= $last_day) && ($event_end_dts > $day_start_dts)) {
				// insert the last day
				$result_list[$assetid.':'.$i] = Array();
				for (reset($event); null !== ($k = key($event)); next($event)) {
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
			// covers a single day only
			if (($event_end_dts < $day_start_dts) || ($event_start_dts > $day_end_dts)) {
				// ended too early or started too late to be shown
				return;
			}
			$start_time = mktime(12, 0, 0, $event['start_date_mon'], $event['start_date_mday'], $event['start_date_year']);
			$result_list[$assetid.':'.$i] = Array();
			for (reset($event); null !== ($k = key($event)); next($event)) {
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
		// end date is not specified - must be just a date or a point in time
		if (!is_null($event['start_date_hours'])) {
			// the time is specified
			if (($event_start_dts < $day_start_dts) || ($event_start_dts > $day_end_dts)) {
				// the time is too early or late in the day to be shown
				return;
			}
		}
		$start_time = mktime(12, 0, 0,  $event['start_date_mon'], $event['start_date_mday'], $event['start_date_year']);
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
function expand_recurring_event(&$result_list, &$event, $assetid, $first_day, $last_day, $day_start_time, $day_end_time)
{
	require_once dirname(__FILE__).'/../calendar_events/calendar_event_recurring/calendar_event_recurring.inc';
	$GLOBALS['SQ_SYSTEM']->am->includeAsset('calendar_event_recurring');

	$event['expanded'] = 1;

	$day_start_bits = explode(':', $day_start_time);
	$day_start_dts = ($day_start_bits[0] * 60) + $day_start_bits[1];
	$day_end_bits = explode(':', $day_end_time);
	$day_end_dts = ($day_end_bits[0] * 60) + $day_end_bits[1];

	$event_start_date = sprintf('%04d-%02d-%02d', $event['start_date_year'], $event['start_date_mon'], $event['start_date_mday']);
	$event_end_date = is_null($event['end_date_ds']) ? null : sprintf('%04d-%02d-%02d', $event['end_date_year'], $event['end_date_mon'], $event['end_date_mday']);
	$event_start_dts = is_null($event['start_date_hours']) ? null : (($event['start_date_hours'] * 60) + $event['start_date_minutes']);
	$event_end_dts = is_null($event['end_date_hours']) ? null : (($event['end_date_hours'] * 60) + $event['end_date_minutes']);

	// dispose of one case immediately
	if (is_null($event['end_date_ds']) && !is_null($event['start_date_hours'])) {
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
			$first_occur = add_months_to_iso($first_day, -$event['period']).'-'.date('d', $first_day);
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
	while (($next_occur !== false) && ($next_occur <= $last_day)) {

		$next_occur_ds = get_ds($next_occur);
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

				if (($start_date >= $first_day) && ($start_date <= $last_day) && (is_null($event_start_dts) || ($event_start_dts <= $day_end_dts))) {
					// add the first day
					$result_list[$assetid.':'.$i] = Array();
					for (reset($event); null !== ($k = key($event)); next($event)) {
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

				// add the intervening days
				for ($inter_time = $start_time + 86400; $inter_time < $end_time; $inter_time += 86400) {
					$inter_date = date('Y-m-d', $inter_time);
					if (($inter_date >= $first_day) && ($inter_date <= $last_day)) {

						$result_list[$assetid.':'.$i] = Array();
						for (reset($event); null !== ($k = key($event)); next($event)) {
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

				if (($end_date >= $first_day) && ($end_date <= $last_day) && (is_null($event_end_dts) || ($event_end_dts > $day_start_dts))) {
					// add the last day
					$result_list[$assetid.':'.$i] = Array();
					for (reset($event); null !== ($k = key($event)); next($event)) {
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
				// fully-specified but falls within a single day
				$start_time = mktime(12, 0, 0, $event['start_date_mon'], $event['start_date_mday'], $event['start_date_year'])+$difference*86400;
				$start_date = date('Y-m-d', $start_time);
				if (($start_date >= $first_day) && ($start_date <= $last_day) &&
						(is_null($event_start_dts) ||
						 (($event_start_dts >= $day_start_dts) && ($event_start_dts < $day_end_dts)) ||
						 (($event_end_dts <= $day_end_dts) && ($event_end_dts > $day_start_dts)) ||
						 (($event_start_dts < $day_start_dts) && ($event_end_dts > $day_end_dts))
						)
					) {
					$result_list[$assetid.':'.$i] = Array();
					for (reset($event); null !== ($k = key($event)); next($event)) {
						$result_list[$assetid.':'.$i][$k] = $event[$k];
					}

					$result_list[$assetid.':'.$i]['start_date_year'] = date('Y', $start_time);
					$result_list[$assetid.':'.$i]['start_date_mon']  = date('n', $start_time);
					$result_list[$assetid.':'.$i]['start_date_mday'] = date('j', $start_time);
					$result_list[$assetid.':'.$i]['end_date_ds']     = get_ds($start_time);
					$result_list[$assetid.':'.$i]['end_date_year']   = date('Y', $start_time);
					$result_list[$assetid.':'.$i]['end_date_mon']    = date('n', $start_time);
					$result_list[$assetid.':'.$i]['end_date_mday']   = date('j', $start_time);

					if (!is_null($event['start_date_hours'])) {
						$result_list[$assetid.':'.$i]['start_date_ts'] = strtotime($start_date.' '.$event['start_date_hours'].':'.$event['start_date_minutes'].':00');
						$result_list[$assetid.':'.$i]['end_date_ts'] = strtotime($start_date.' '.$event['end_date_hours'].':'.$event['end_date_minutes'].':00');
					} else {
						$event[$assetid.':'.$i]['start_date_ts'] = strtotime($start_date);
						$event[$assetid.':'.$i]['end_date_ts'] = strtotime($start_date);
					}
					$i++;
				}
			}
		} else {
			// end date not specified, so it's either just a date or a point in time
			$start_time = mktime(12, 0, 0,  $event['start_date_mon'], $event['start_date_mday'], $event['start_date_year'])+$difference*86400;
			$start_date = date('Y-m-d', $start_time);

			if (($start_date >= $first_day) && ($start_date <= $last_day)) {
				$result_list[$assetid.':'.$i] = Array();
				for (reset($event); null !== ($k = key($event)); next($event)) {
					$result_list[$assetid.':'.$i][$k] = $event[$k];
				}

				$result_list[$assetid.':'.$i]['start_date_year'] = date('Y', $start_time);
				$result_list[$assetid.':'.$i]['start_date_mon']  = date('n', $start_time);
				$result_list[$assetid.':'.$i]['start_date_mday'] = date('j', $start_time);
				$result_list[$assetid.':'.$i]['end_date_ds']     = get_ds($start_time);
				$result_list[$assetid.':'.$i]['end_date_year']   = date('Y', $start_time);
				$result_list[$assetid.':'.$i]['end_date_mon']    = date('n', $start_time);
				$result_list[$assetid.':'.$i]['end_date_mday']   = date('j', $start_time);

				if (!is_null($event['start_date_hours'])) {
					$result_list[$assetid.':'.$i]['start_date_ts'] = strtotime($start_date.' '.$event['start_date_hours'].':'.$event['start_date_minutes'].':00');
					$result_list[$assetid.':'.$i]['end_date_ts'] = strtotime($start_date.' '.$event['end_date_hours'].':'.$event['end_date_minutes'].':00');
				} else {
					$event[$assetid.':'.$i]['start_date_ts'] = strtotime($start_date);
					$event[$assetid.':'.$i]['end_date_ts'] = strtotime($start_date);
				}

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
* @access public
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
	$exception_links = $GLOBALS['SQ_SYSTEM']->db->getAssoc('SELECT majorid, minorid FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk WHERE minorid IN ('.implode(', ',	$exception_ids).')');
	assert_valid_db_result($exception_links);
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
* @access public
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
		$this_date['ds'] = get_ds($iso);

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
			} else if (is_null($this_date['minutes']) && !is_null($this_date['seconds']))  {
				// minutes null in the middle of the time - no valid time
				$this_date['ts'] = null;
			} else {
				// make a best guess on timestamp, setting all blanks to 0
				$this_date['ts'] = strtotime(substr($iso,0,10).' '.str_replace('-','0',substr($iso,11)));
			}

			// we can do these in any case
			$this_date['ds'] = get_ds($iso);
			$this_date['wday'] = date('w', $date_only_ts);
			$this_date['yday'] = date('z', $date_only_ts) + 1;
		}

	}

	// add 'month-stamp' column which can be used to easily check
	// 'x monthly' stuff which can't be done easily with T/S alone
	if (is_null($this_date['mon']) || is_null($this_date['year'])) {
		$this_date['ms'] = null;
	} else {
		$this_date['ms'] = get_ms($iso);
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
	foreach (array_keys($this_date) as $iso_part) {
		$res[$prefix.$iso_part] = is_null($this_date[$iso_part]) ? null : intval($this_date[$iso_part]);
	}
	return $res;

}//end get_date_components()


/**
* Get the daystamp value for the supplied date
*
* @param int|string	$value	The date to convert, either a timestamp or ISO-format date
*
* @return int	The day-stamp
* @access public
*/
function get_ds($value)
{
	if (is_numeric($value)) {
		// timestamp
		return round($value / 86400);
	} else {
		// iso format
		$value = substr($value, 0, 10).' 12:00:00';
		return round(iso8601_ts($value) / 86400);
	}

}//end get_ds()


/**
* Get the monthstamp value for the supplied date
*
* @param int|string	$value	The date to convert, either a timestamp or ISO-format date
*
* @return int	The monthstamp
* @access public
*/
function get_ms($value)
{
	if (is_numeric($value)) {
		// convert timestamp to ISO
		$value = ts_iso8601($value);
	}
	list($year, $month) = sscanf($value, '%04d-%02d');
	return ($year * 12) + $month;

}//end get_ms()


/**
* Returns a QUOTED array of a type's descendants
*
* @param string $type_code
*
* @see    Asset_Manager::getTypeDescendants()
* @return Array
* @access public
*/
function quoted_descendants($type_code)
{
	// only needed because of quote()
	$db =& $GLOBALS['SQ_SYSTEM']->db;

	$type_codes = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants($type_code);
	$type_codes[] = $type_code;

	foreach ($type_codes as $tc_key => $tc_value) {
		$type_codes[$tc_key] = $db->quoteSmart($tc_value);
	}

	return $type_codes;

}//end quoted_descendants()


/**
* Return true if $a starts before $b
*
* @access public
* @return void
*/
function compare_start_dates($a, $b)
{
	return ($a['start_date_ts'] < $b['start_date_ts'])? -1 : 1;

}//end compare_start_dates()


/**
* Returns array of events sorted by occurrence
*
* @param $date
* @param $events	Events to be sorted
*
* @return array
* @access public
*/
function sortByOccurrence($date, $events) {
	$dates = Array();

	foreach ($events as $event_id => $info) {
		$event =& $GLOBALS['SQ_SYSTEM']->am->getAsset(strtok($event_id, ':'));
		if (!empty($event)) {
			if (method_exists($event, 'getInstanceDates')) {
				$ocDate = $event->getInstanceDates($date);
				$start = $ocDate['start'];
				$dates[$event_id] = strtotime($start);
			} else {
				$dates[$event_id] = strtotime($event->getStartDate().' '.$event->getStartTime());
			}
		}
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($event);
	}

	asort($dates);
	$sorted_events = Array();
	foreach ($dates as $event_id => $timestamp) {
		$sorted_events[$event_id] = $events[$event_id];
	}

	return $sorted_events;

}//end sortByOccurrence()


?>