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
* $Id: page_calendar.inc,v 1.2 2004/07/30 06:36:17 lwright Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_PACKAGES_PATH.'/cms/page_templates/page_asset_listing/page_asset_listing.inc';
require_once SQ_FUDGE_PATH.'/general/text.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Page_Account_Manager
*
* Purpose
*
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.2 $ - 1.0
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
				//printDayView();
			break;

			case 'week':
				//printWeekView();
			break;

			case 'month':
				//printMonthView();
			break;

			case 'list':
				//printListView();
			break;

			case 'resource':
				//printResourceView();
			break;

			default:
				trigger_error('Unrecognised calendar view type "'.$cal_view.'".', E_USER_NOTICE);
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

		// base sql query
		$sql = 'select aav.assetid, aav.custom_value from sq_asset_attribute_value aav join sq_asset_attribute atr on aav.attributeid = atr.attributeid where type_code = '.$db->quote('calendar_event').' and name = '.$db->quote('start_date').';';
		
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


}//end class

?>
