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
* $Id: page_calendar.inc,v 1.7 2004/08/05 02:54:45 lwright Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_PACKAGES_PATH.'/cms/page_templates/page_asset_listing/page_asset_listing.inc';
require_once SQ_FUDGE_PATH.'/general/text.inc';
require_once SQ_FUDGE_PATH.'/general/datetime.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Page_Account_Manager
*
* Purpose
*
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.7 $ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Calendar extends Page_Asset_Listing
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Page_Calendar($assetid=0)
	{
		$this->Page_Asset_Listing($assetid);

	}//end Page_Calendar()


	/**
	* Called by the design to print the body of this asset
	*
	* @return void
	* @access public
	*/
	function printBody()
	{
		$cal_view = $_REQUEST['SQ_CALENDAR_VIEW'];

		switch($cal_view) {
			case 'day':
				$this->printDayView(2004,8,2);
			break;

			case 'week':
				$this->printWeekView(2004,7,27);
				//printWeekView();
			break;

			case 'month':
				$this->printMonthView(2004,7);
			break;

			case 'year':
				//$this->printYearView(2004);
			break;

			case 'list':
				//printListView();
			break;

			case 'resource':
				//printResourceView();
			break;

			default:
				trigger_error('Unrecognised calendar view type "'.$cal_view.'".', E_USER_NOTICE);
			break;
		}

	}//end printBody()


	/**
	* print a day view for the specified day
	*
	* @return void
	* @access public
	*/
	function printDayView($year, $month, $day)
	{
		/* pseudo-code version of this method...
		 * get all events that lie underneath the selected root nodes, using a search matching
		 * the front part of the tree id with the root node
		 *
		 * OR, if reverse lookup option turned on, it must be the minor
		 * [LIMIT:(?) only use immediate links for reverse lookup]
		 *
		 * [FOR SINGLE EVENTS - IGNORE TIME COMPONENTS:]
		 * ... that have a start date of 'today' (nb: by 'today' I mean the passed date)
		 *     and no end date
		 * ... OR, have a start date earlier than or equal to today AND
		 * ... have an end date later than or equal to today
		 *     (ie. between start date and end date, inclusive)
		 *
		 * [FOR MULTIPLE EVENTS:]
		 * ... that have a start date later than or equal to today
		 * ... if it has an STOP date, it must be before that
		 *     (ie. between start date and any STOP date, inclusive)
		 * ... beyond that, stuffed if I know how to do all of the things I want in a database
		 *     query >_<
		 *	   except ... DED [every day] is always included
		 *            ... DWD [every weekday] is included if date passed is a Mon-Fri
		 *            ... 'Every week' could be easy enough if there is a SQL function that can
		 *                handle returning of weekday indices
		 *
		 * Grab an attribute for start time of the day view (eg. 08.30)
		 * Grab an attribute for end time of the day view (eg. 17.00)
		 * Grab an attribute for the length of each viewable time slot (eg. 1 hour, 30 minutes
		 * 15 minutes)
		 * Grab display format (eg. is 'RESOURCE VIEW/ONE COLUMN PER NODE' enabled?)
		 *
		 * Work out final list of events in an array which includes all relevant information,
		 * including start and end times (if it rolls over to another day then use 00.00 and
		 * 24.00 if necessary), including style/colour information
		 *
		 * Print the bastard =D (one column per node if that is specified, otherwise
		 * just one column straight down)
		 */
		 
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		
		// base sql query FOR SINGLE EVENTS (holy crap... O_O')
		$sql = 'select a.name, a.assetid, v.custom_value as start_date, v1.custom_value as end_date from ((('.SQ_TABLE_RUNNING_PREFIX.'asset a join '.SQ_TABLE_PREFIX.'asset_attribute atr on a.type_code = atr.type_code) join '.SQ_TABLE_PREFIX.'asset_attribute atr1 on a.type_code = atr1.type_code) left join '.SQ_TABLE_RUNNING_PREFIX.'asset_attribute_value v on atr.attributeid = v.attributeid and a.assetid = v.assetid) left join '.SQ_TABLE_RUNNING_PREFIX.'asset_attribute_value v1 on atr1.attributeid = v1.attributeid and a.assetid = v1.assetid where atr.name = '.$db->quote('start_date').' and atr1.name = '.$db->quote('end_date').' and a.type_code = '.$db->quote('calendar_event_single').' and ((substring(v.custom_value from 1 for 10) = '.$db->quote($this->_buildDate($year,$month,$day)).' and v1.custom_value IS NULL) OR (substring(v.custom_value from 1 for 10) <= '.$db->quote($this->_buildDate($year,$month,$day)).' and substring(v1.custom_value from 1 for 10) >= '.$db->quote($this->_buildDate($year,$month,$day)).'))';
		
		$result = $db->getAll($sql);
		
		foreach($result as $key => $result_item) {
			$start_time = (iso8601_time_component($result_item['start_date']) === false ? '00:00:00' : str_replace('-','0',iso8601_time_component($result_item['start_date'])));
			$end_time = (iso8601_time_component($result_item['end_date']) === false ? (iso8601_time_component($result_item['start_date']) === false ? '24:00:00' : $start_time) : str_replace('-','0',iso8601_time_component($result_item['end_date'])));
			$result[$key] = array_merge($result_item, Array(
												'start_date' => $start_time,
												'end_date' => $end_time,));
		}
		
		list($start_time_h, $start_time_m) = explode(':', $this->attr('day_view_start_time'));
		list($end_time_h, $end_time_m) = explode(':', $this->attr('day_view_end_time'));
		$appt_spacing = intval($this->attr('appointment_spacing'));
		
		foreach($result as $result_item) {
					
			echo '<br/><b>'.$result_item['name'].'</b> running ';
			if (($result_item['start_date'] == '00:00:00') && ($result_item['end_date'] == '24:00:00')) {
				echo 'all day';
			} else {
				echo $result_item['start_date'].' to '.$result_item['end_date'];
			}
					
		}?>
			<table>
			<?php for($i = $start_time_h * 60 + $start_time_m; $i < $end_time_h * 60 + $end_time_m; $i += $appt_spacing) { ?>
				<tr>
					<td align="right"><?php echo intval($i/60).':'.(($i % 60) < 10 ? '0' : '').intval($i % 60) ?></td>
					<td>&nbsp;</td>
				</tr>
			<?php } ?>
			</table>
		<?php
		
	}//end printDayView()

	
	/**
	* print a weekly view starting from the specified day
	*
	* @return void
	* @access public
	*/
	function printWeekView($year, $month, $day)
	{
		/* pseudo-code version of this method...
		
		 * first bit as for day, yada yada yada
		 *
		 * [FOR SINGLE EVENTS - IGNORE TIME COMPONENTS:]
		 * ... that have a start date of  and no end date
		 * ... OR, have a start date earlier than or equal to starting day AND
		 * ... have an end date later than or equal to ending day
		 *     (ie. between start date and end date, inclusive)
		 *
		 * [FOR MULTIPLE EVENTS:]
		 * ... that have a start date later than or equal to today
		 * ... if it has an STOP date, it must be before that
		 *     (ie. between start date and any STOP date, inclusive)
		 * ... beyond that...
		 *	   ... DED [every day] is always included
		 *     ... DEDx [every x days] is included if x <= 7
		 *     ... DWD [every weekday] is always included as it will likely occur 5 times :-)
		 *     ... WEW [every week] is always included
		 *
		 * Grab an attribute for start time of the day view (eg. 08.30)
		 * Grab an attribute for end time of the day view (eg. 17.00)
		 * Grab an attribute for the length of each viewable time slot (eg. 1 hour, 30 minutes
		 * 15 minutes)
		 * Grab an attribute for first day of week (eg. Monday or Sunday, or another day)
		 * Grab display formats (not likely to have a resource/node view here)
		 *
		 * Work out final list of events in an array which includes all relevant information,
		 * including start and end times FOR EACH DAY (this is for simplicity when printing
		 * the calendar out), including style/colour information
		 *
		 * Print the bastard =D, one column for each day
		 */
		 
	
		
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		
		// base sql query FOR SINGLE EVENTS (holy crap... O_O')
		$sql = 'select a.name, a.assetid, v.custom_value as start_date, v1.custom_value as end_date from ((('.SQ_TABLE_RUNNING_PREFIX.'asset a join '.SQ_TABLE_PREFIX.'asset_attribute atr on a.type_code = atr.type_code) join '.SQ_TABLE_PREFIX.'asset_attribute atr1 on a.type_code = atr1.type_code) left join '.SQ_TABLE_RUNNING_PREFIX.'asset_attribute_value v on atr.attributeid = v.attributeid and a.assetid = v.assetid) left join '.SQ_TABLE_RUNNING_PREFIX.'asset_attribute_value v1 on atr1.attributeid = v1.attributeid and a.assetid = v1.assetid where atr.name = '.$db->quote('start_date').' and atr1.name = '.$db->quote('end_date').' and a.type_code = '.$db->quote('calendar_event_single').' and (((substring(v.custom_value from 1 for 10) BETWEEN '.$db->quote($this->_buildDate($year,$month,$day)).' AND '.$db->quote($this->_buildDate($year,$month,$day + 6)).') and v1.custom_value IS NULL) OR (substring(v.custom_value from 1 for 10) < '.$db->quote($this->_buildDate($year,$month,$day + 7)).' and substring(v1.custom_value from 1 for 10) >= '.$db->quote($this->_buildDate($year,$month,$day)).'))';
		
		$result = $db->getAll($sql);
		$final_result = Array();
			
		foreach($result as $result_item) {
			for($loop_day = $day; $loop_day < $day + 7; $loop_day++) {
				$iso_date = $this->_buildDate($year,$month,$loop_day);
				if(empty($final_result[$iso_date])) $final_result[$iso_date] = Array();
				if ((iso8601_date_component($result_item['start_date']) == $iso_date && is_null($result_item['end_date'])) || (iso8601_date_component($result_item['start_date']) <= $iso_date && iso8601_date_component($result_item['end_date']) >= $iso_date)) {	
					$start_time = (iso8601_time_component($result_item['start_date']) === false ? '00:00:00' : str_replace('-','0',iso8601_time_component($result_item['start_date'])));
					$end_time = (iso8601_time_component($result_item['end_date']) === false ? (iso8601_time_component($result_item['start_date']) === false ? '24:00:00' : $start_time) : str_replace('-','0',iso8601_time_component($result_item['end_date'])));
					$final_result[$iso_date][] = array_merge($result_item, Array(
														'start_date' => $start_time,
														'end_date' => $end_time,));
				}
			}	
		}
		
		list($start_time_h, $start_time_m) = explode(':', $this->attr('day_view_start_time'));
		$start_time_h = 8; $start_time_m = 30;
		list($end_time_h, $end_time_m) = explode(':', $this->attr('day_view_end_time'));
		$end_time_h = 17; $end_time_m = 0;
		$appt_spacing = intval($this->attr('appointment_spacing'));
				
		$days_of_week = Array('Sunday', 'Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday', 'Saturday');
		$first_day = mktime(0,0,0,$month,$day,$year);
		$date_info = getdate($first_day);
		$first_wday = $date_info['wday'];
		?>
		<table width="100%">
		<tr>
		<th>Time</th>
		<?php for($i = $first_wday; $i < $first_wday + 7; $i++) { ?><th align="center" width="<?php echo intval(100 / 7) ?>%"><?php echo $days_of_week[$i % 7]; ?> <?php echo date('jS', mktime(0,0,0,$month,$day + $i - $first_wday,$year)) ?><br/>
		<?php pre_echo($final_result[$this->_buildDate($year,$month,$day + $i - $first_wday)]) ?></th><?php } ?>
		</tr>
		<?php for($i = $start_time_h * 60 + $start_time_m; $i < $end_time_h * 60 + $end_time_m; $i += $appt_spacing) { ?>
			<tr>
			<td align="right"><?php echo intval($i/60).':'.(($i % 60) < 10 ? '0' : '').intval($i % 60) ?></td>
			<td>&nbsp;</td>
			</tr>
		<?php } ?>
		</table><?php
		
	}//end printWeekView()


	/**
	* print a monthly view with for the specified month
	*
	* @return void
	* @access public
	*/
	function printMonthView($year, $month)
	{
		/* pseudo-code version of this method...
		 * first bit as for day, yada yada yada
		 *
		 * [FOR SINGLE EVENTS - IGNORE TIME COMPONENTS:]
		 * ... that have a start date the same month as today and no end date
		 * ... OR, have a start date earlier than or equal to 1st of today's month AND
		 * ... have an end date later than or equal to last of today's month
		 *     (ie. between start date and end date, inclusive)
		 *
		 * [FOR MULTIPLE EVENTS:]
		 * ... that have a start date later than or equal to today
		 * ... if it has an STOP date, it must be before that
		 *     (ie. between start date and any STOP date, inclusive)
		 * ... beyond that...
		 *	   ... DED [every day] is always included
		 *     ... DEDx [every x days] is included if x <= 28 (substitute # of days in month)
		 *     ... DWD [every weekday] is always included
		 *     ... WEW [every week] is always included
		 *     ... WEWx [every x weeks] is definitely included if x <= 3, maybe if x = 4
		 *     ... M** [every month] is always included
		 *
		 * Grab an attribute for first day of week (eg. Monday or Sunday, or another day)
		 * Grab display formats (not likely to have a resource/node view here)
		 *
		 * Work out final list of events in an array which includes all relevant information,
		 * including start and end times FOR EACH DAY (this is for simplicity when printing
		 * the calendar out), including style/colour information
		 *
		 * Print the bastard =D, in a monthly calendar format (duh!)
		 */
		 
		$db =& $GLOBALS['SQ_SYSTEM']->db;

		// base sql query FOR SINGLE EVENTS (holy crap... O_O')
		$sql = 'select a.name, a.assetid, v.custom_value as start_date, v1.custom_value as end_date from ((('.SQ_TABLE_RUNNING_PREFIX.'asset a join '.SQ_TABLE_PREFIX.'asset_attribute atr on a.type_code = atr.type_code) join '.SQ_TABLE_PREFIX.'asset_attribute atr1 on a.type_code = atr1.type_code) left join '.SQ_TABLE_RUNNING_PREFIX.'asset_attribute_value v on atr.attributeid = v.attributeid and a.assetid = v.assetid) left join '.SQ_TABLE_RUNNING_PREFIX.'asset_attribute_value v1 on atr1.attributeid = v1.attributeid and a.assetid = v1.assetid where atr.name = '.$db->quote('start_date').' and atr1.name = '.$db->quote('end_date').' and a.type_code = '.$db->quote('calendar_event_single').' and ((substring(v.custom_value from 1 for 7) = '.$db->quote($year.'-'.str_pad($month, 2, '0', STR_PAD_LEFT)).' and v1.custom_value IS NULL) OR (substring(v.custom_value from 1 for 7) <= '.$db->quote($this->_buildDate($year,$month)).' and substring(v1.custom_value from 1 for 7) >= '.$db->quote($this->_buildDate($year,$month)).'))';
		
		
		$result = $db->getAll($sql);
		
		// print calendar
		$days_of_week = Array('Sunday', 'Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday', 'Saturday');
		$first_wday = $this->attr('week_starts_on');
		$first_wday = 4;
		?>
		<table width="100%">
		<tr>
		<?php for($i = $first_wday; $i < $first_wday + 7; $i++) { ?><th align="center" width="<?php echo intval(100 / 7) ?>%"><?php echo $days_of_week[$i % 7]; ?></th><?php } ?>
		</tr>
		<?php
		$first_mday = getdate(mktime(0,0,0,$month,1,$year));
		
		
		// find the number of blank days to print - note that we add 7 to this so
		// the numerator is always positive and % then gives a meaningful result if
		// 1st of the month is earlier in the week than the Week Starts On attrib
		$gap = ($first_mday['wday'] - $first_wday + 7) % 7;
		
		$weeks_required = ceil((days_in_month($month, $year) + $gap) / 7);
				
		$print_day = 0;
		for($day = 1-$gap; $day <= ($weeks_required*7)-$gap; $day++) {
			if ($print_day % 7 == 0) echo '<tr>';
			
			echo '<td>';
			if (($day >= 1) && ($day <= days_in_month($month, $year))) {
				echo $day;
				$date_value = ts_iso8601(mktime(0,0,0,$month,$day,$year));
				foreach($result as $result_item) {
					if (((iso8601_date_component($date_value) == iso8601_date_component($result_item['start_date'])) && is_null($result_item['end_date'])) || ((iso8601_date_component($date_value) >= iso8601_date_component($result_item['start_date'])) && ((iso8601_date_component($date_value) <= iso8601_date_component($result_item['end_date'])) )) ) {
						echo '<br/><b>'.$result_item['name'].'</b>';
					}
				}
			} else {
				echo '&nbsp;';
			}
			echo '</td>';
			
			$print_day++;
			if ($print_day % 7 == 0) echo '</tr>';
		}?></table><?php
		
	}//end printMonthView()


	/**
	* print a standard list view
	*
	* Should this just be called from our printBody() ?
	*
	* @see Asset_Listing::printBody()
	* @return void
	* @access public
	*/
	function printListView()
	{

	}//end printListView()


	/**
	* Builds a full or partial ISO8601 format date (for use in database searching)
	*
	* This function uses mktime() to construct the date, therefore months/days outside the
	* valid range will be quietly handled as mktime() would (eg. params of 2004,8,32 would
	* return the date 2004-09-01). This is done mainly to simplify the week view script
	*
	* @param int	$year	year of date
	* @param int	$month  the month
	* @param int	$day	the day (optional)
	*
	* @return string	YYYY-MM if $day omitted, YYYY-MM-DD if $day included
	* @access private
	*/
	function _buildDate($year, $month, $day=null)
	{
		$date_value = mktime(0,0,0,$month,(is_null($day) ? 1 : $day), $year);
		
		if (is_null($day)) {
			return date('Y-m', $date_value);
		} else {
			return date('Y-m-d', $date_value);
		}
		
		return $str;
		
	}//end _buildDate()
	
	
}//end class

?>

