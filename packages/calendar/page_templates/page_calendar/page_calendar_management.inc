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
* $Id: page_calendar_management.inc,v 1.9 2004/08/20 06:56:49 tbarrett Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Calendar_Management
*
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.9 $ - 1.0
* @package MySource_Matrix_Packages
* @subpackage calendar
*/
class Page_Calendar_Management extends Asset_Management
{


	/**
	* Constructor
	*
	* @param object Package_Manager	&$pm	Reference to the package manager object for this package
	*
	*/
	function Page_Calendar_Management(&$pm)
	{
		$this->Asset_Management($pm);

		// root nodes will be held in an array indexed by asset id and containing
		// data such as CSS styles and other stuff (structure not yet determined)
		$this->vars['root_nodes']			= Array(
														'added'     => '0.1',
														'type'      => 'serialise',
														'default'   => Array(),
														);

		
		// the view types enabled for this calendar page
		$this->vars['enabled_views']			= Array(
														'added'     => '0.1',
														'type'      => 'serialise',
														'default'   => Array(	'year'=>'',
																				'month'=>'',
																				'week'=>'',
																				'month'=>'',
																				'day'=>'',
																				'event'=>'')
													);
		$this->vars['initial_view'] = Array(
													'added'			=> '0.1',
													'type'			=> 'selection',
													'default'		=> 'year',
													'description'	=> 'The first view the user sees',
													'parameters'	=> Array(
														'options' => Array(
															'year'	=> 'Year', 
															'month'	=> 'Month',
															'week'  => 'Week',
															'day'   => 'Day')
													 )
											);

		// These are stored as strings, 'hh:mm' because I want '24:00' to represent the
		// latest possible end time (ie. end of the day) but that will be rejected by a
		// datetime field as out of the range
		$this->vars['day_starts_at']	= Array(
														'added'     => '0.1',
														'type'      => 'text',
														'default'   => '00:00',
														);
		
		$this->vars['day_ends_at']	= Array(
														'added'     => '0.1',
														'type'      => 'text',
														'default'   => '24:00',
														);
														
		
		// spacing of the day view calendar, in minutes
		$this->vars['day_partition_time']	= Array(
													'added'			=> '0.1',
													'type'			=> 'selection',
													'default'		=> '60',
													'description'	=> 'The spacing of intervals in the calendar day view',
													'parameters'	=> Array(
														'options' => Array(
															10	=> '10 mins', 
															15	=> '15 mins', 
															20	=> '20 mins', 
															30	=> '30 mins', 
															60	=> '1 hour')
													 )
											);

		$this->vars['day_partition_height'] = Array(
													'added'			=> '0.1',
													'type'			=> 'float',
													'default'		=> '4.0',
													'description'   => 'The height of each day interval\'s row in the table, in \'ex\'es',
													'parameters'	=> Array('decimal_places' => 2)
												   );	

		$this->vars['table_column_width'] = Array(
													'added'			=> '0.1',
													'type'			=> 'float',
													'default'		=> '8.0',
													'description'   => 'The width of each column in the day and week display tables, in \'ex\'es',
													'parameters'	=> Array('decimal_places' => 2)
												   );	
										

		$this->vars['columnise_day_view_by_root_node'] = Array(
																'added'		=> '0.1',
																'type'		=> 'boolean',
																'default'	=> 'true',
																'description' => 'whether to split the day view into columns according to the root node the event came from'
														);

		$this->vars['week_starts_on']		= Array(
													'added'			=> '0.1',
													'type'			=> 'selection',
													'default'		=> '1',
													'description'	=> 'The first view the user sees',
													'parameters'	=> Array(
														'options' => Array(
															0 => 'Sunday',
															1 => 'Monday',
															2 => 'Tuesday',
															3 => 'Wednesday',
															4 => 'Thursday',
															5 => 'Friday',
															6 => 'Saturday')
													 )
											);
														
		// currently applies to week view, to display a selective day view
		// defaults to every day
		$this->vars['week_view_show_days']	= Array(
													'added'     => '0.1',
													'type'      => 'serialise',
													'default'   => Array(1, 1, 1, 1, 1, 1, 1)
												);

		// The view the user is taken to when they click on a day in year view
		$this->vars['year_drill_down_view']	= Array(
													'added'     => '0.1',
													'type'      => 'text',
													'default'   => 'day'
												);

		// Date format for calendar view titles
		$this->vars['view_title_formats'] = Array(
													'added'     => '0.1',
													'type'      => 'serialise',
													'default'   => Array(
																		'year'	=> 'Y',
																		'month'	=> 'F Y',
																		'week'	=> 'F Y',
																		'day'	=> 'jS F Y',
																		'event'	=> 't'
																	)
												  );

		$this->vars['month_column_width']	= Array(
													'added'			=> '0.1',
													'type'			=> 'float',
													'default'		=> '10.0',
													'description'   => 'The width of the columns in the month calendar table, in \'ex\'es',
													'parameters'	=> Array('decimal_places' => 2)
											);
												   
		$this->vars['month_row_height']		= Array(
													'added'			=> '0.1',
													'type'			=> 'float',
													'default'		=> '7.0',
													'description'   => 'The height of the rows in the month calendar table, in \'ex\'es',
													'parameters'	=> Array('decimal_places' => 2)
											);
		
		$this->vars['up_link_text'] = Array(
												'added'     => '0.1',
												'type'      => 'text',
												'default'   => '%v% view'
											);
											
		$this->vars['next_link_text'] = Array(
												'added'     => '0.1',
												'type'      => 'text',
												'default'   => 'Next %v% >>'
											);
											
		$this->vars['prev_link_text'] = Array(
												'added'     => '0.1',
												'type'      => 'text',
												'default'   => '<< Previous %v%'
											);
											
		
	}//end Page_Calendar_Management()


	/**
	* Gets a list of all metadata files that we want web accessable
	*
	* @access private
	* @return array(string)
	*/
	function _getFileList()
	{
		return Array(
					 'css/calendar_default.css',
					 );

	}//end _getFileList()
}//end class

?>