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
* $Id: page_calendar.inc,v 1.1.1.1 2004/07/21 04:31:53 lwright Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_PACKAGES_PATH.'/cms/page_templates/page_asset_listing/page_asset_listing_edit_fns.inc';
require_once SQ_FUDGE_PATH.'/general/text.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Page_Account_Manager
*
* Purpose
*
*
* @author  Greg Sherwoood <greg@squiz.net>
* @version $Version$ - 1.0
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

	}//end printDayView()

	
	/**
	* print a weekly view starting from the specified day
	*
	* @return void
	* @access public
	*/
	function printWeekView($year, $month, $day)
	{

	}//end printWeekView()


	/**
	* print a monthly view with for the specified month
	*
	* @return void
	* @access public
	*/
	function printMonthView($year, $month)
	{
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


	/**
	* print a 'resource day view' list with one column for each resource
	*
	* 'Resources' are defined to be assets which you link underneath the calendar event for certain
	* purposes. This will display events which have that 'resource' asset(s) underneath them.
	* One 'resource' asset to a column.
	*
	* @return void
	* @access public
	*/
	function printResourceView()
	{

	}//end printResourceView()

}//end class

?>
