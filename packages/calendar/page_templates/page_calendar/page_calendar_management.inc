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
* $Id: page_calendar_management.inc,v 1.63 2012/08/30 00:57:28 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Calendar_Management
*
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.63 $
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

		// The view	types enabled for this calendar	page
		$this->vars['enabled_views'] = Array(
										'added'		=> '0.1',
										'type'		=> 'serialise',
										'default'	=> Array(
														'year'	=> 'calendar_format',
														'month'	=> 'calendar_format',
														'week'	=> 'calendar_format',
														'month'	=> 'calendar_format',
														'day'	=> 'calendar_format',
														'event'	=> 'single_event_view',
													   ),
									   );

		// The view	type you see when you first	come to	the	page
		$this->vars['initial_view'] = Array(
										'added'			=> '0.1',
										'type'			=> 'selection',
										'default'		=> 'year',
										'description'	=> 'The first	view the user sees',
										'parameters'	=> Array(
															'options'	=> Array(
																			'year'	=> 'Year',
																			'month'	=> 'Month',
																			'week'	=> 'Week',
																			'day'	=> 'Day',
																		   ),
															'multiple'	=> FALSE,
														   ),
									  );

		// The locations from which	we draw	event assets, assetid => Array('classname' => className), where	classname
		// is the CSS class	that should	be applied to representations of events	from that location
		$this->vars['root_nodes'] = Array(
										'added'		=> '0.1',
										'type'		=> 'serialise',
										'default'	=> Array(),
									);

		$this->vars['personalised'] = Array(
										'added'			=> '0.5',
										'type'			=> 'boolean',
										'default'		=> '0',
										'parameters'	=> Array('allow_empty' => FALSE),
									  );

		// Which of	the	root nodes should get their	own	column in day view
		$this->vars['columnise_day_view_by_root_node'] = Array(
															'added'			=> '0.2',
															'type'			=> 'serialise',
															'default'		=> Array('*'),
															'description'	=> 'list of root nodes	which should have their	own	columns	in day view, and * if you want a column	for	events that	don\'t belong in any other column',
														 );
		// The name	of the 'other' column in day view
		$this->vars['other_column_name'] = Array(
											'added'		=> '0.1',
											'type'		=> 'text',
											'default'	=> '',
											'is_contextable'	=> TRUE,
										   );

		// Earliest	and	latest times to	display	in week	and	day	view, stored as	strings
		// rather than datetimes because we	want to	have 24:00 as a	valid option
		$this->vars['day_starts_at'] = Array(
										'added'		=> '0.1',
										'type'		=> 'text',
										'default'	=> '00:00',
									   );

		$this->vars['day_ends_at'] = Array(
										'added'		=> '0.1',
										'type'		=> 'text',
										'default'	=> '24:00',
									 );

		// Time-length of vertical partitions in the week and day views
		$this->vars['day_partition_time'] = Array(
												'added'			=> '0.1',
												'type'			=> 'selection',
												'default'		=> '60m',
												'description'	=> 'The	spacing	of intervals in	the	calendar week and day views',
												'parameters'	=> Array(
																	'options'		=> Array(
																						'10m'	=> '10 mins',
																						'15m'	=> '15 mins',
																						'20m'	=> '20 mins',
																						'30m'	=> '30 mins',
																						'60m'	=> '1 hour',
																					   ),
																	'allow_empty'	=> FALSE,
																   ),
											);

		// Physical	height of the vertical partitions in the week and day views
		$this->vars['day_partition_height'] = Array(
												'added'			=> '0.1',
												'type'			=> 'int',
												'default'		=> '20',
												'description'	=> 'The	height of each day interval\'s row in the week and day	tables,	in pixels',
											  );

		// The width of	columns	in week	view (pardon the bad name, a historical	artefact)
		$this->vars['table_column_width'] = Array(
												'added'			=> '0.1',
												'type'			=> 'int',
												'default'		=> '110',
												'description'	=> 'The width of each column in the WEEK table, in pixels',
											);

		// The width of	column in day view
		$this->vars['day_column_width'] = Array(
											'added'			=> '0.1',
											'type'			=> 'int',
											'default'		=> '110',
											'description'	=> 'The	width of each column in	the	DAY	table, in pixels',
										  );

		// The day to show first in the week
		$this->vars['week_starts_on'] = Array(
											'added'			=> '0.1',
											'type'			=> 'selection',
											'default'		=> '1',
											'description'	=> 'First day of the week',
											'parameters'	=> Array(
																'options'		=> Array(
																					0	=> 'Sunday',
																					1	=> 'Monday',
																					2	=> 'Tuesday',
																					3	=> 'Wednesday',
																					4	=> 'Thursday',
																					5	=> 'Friday',
																					6	=> 'Saturday',
																				   ),
																'allow_empty'	=> FALSE,
															   ),
										);

		// Which weekdays to show in week view
		$this->vars['week_view_show_days'] = Array(
												'added'		=> '0.1',
												'type'		=> 'serialise',
												'default'	=> Array(1, 1, 1, 1, 1, 1, 1),
											 );


		// Date	format for calendar	view titles
		$this->vars['view_title_formats'] =	Array(
												'added'		=> '0.1',
												'type'		=> 'serialise',
												'default'	=> Array(
																'year'	=> 'Y',
																'month'	=> 'F Y',
																'week'	=> '\W\e\e\k \o\f l j F Y',
																'day'	=> 'jS F Y',
																'event'	=> 't',
															   ),
											);

		// Date	format for calendar	heading titles
		$this->vars['view_heading_formats'] = Array(
												'added'		=> '0.8',
												'type'		=> 'serialise',
												'default'	=> Array(
																'month_day'	=> 'l jS',
																'week_day'	=> 'l jS',
															   ),
											  );

		// width of	table columns in month view
		$this->vars['month_column_width'] =	Array(
												'added'			=> '0.1',
												'type'			=> 'int',
												'default'		=> '90',
												'description'	=> 'The width of the columns in the month calendar table, in pixels',
											);

		// default height of table rows	in month view
		$this->vars['month_row_height']	= Array(
											'added'			=> '0.1',
											'type'			=> 'int',
											'default'		=> '70',
											'description'	=> 'The height of the rows in the month calendar table, in pixels',
										  );

		// Whether to show links to	week view at the side of the month view	table
		$this->vars['month_links_to_week'] = Array(
												'added'			=> '0.1',
												'type'			=> 'boolean',
												'default'		=> 'true',
												'parameters'	=> Array('allow_empty' => FALSE),
											 );

		// Text	for	the	link that takes	you	to the broader view
		$this->vars['up_link_text'] = Array(
										'added'		=> '0.1',
										'type'		=> 'text',
										'default'	=> '%v% view',
										'is_contextable'	=> TRUE,
									  );

		// Text	for	the	link that takes	you	to the next	year/month/week/day
		$this->vars['next_link_text'] = Array(
											'added'		=> '0.1',
											'type'		=> 'text',
											'default'	=> 'Next %v% >>',
											'is_contextable'	=> TRUE,
										);

		// Text	for	the	link that takes	you	to the previous	year/month/week/day
		$this->vars['prev_link_text'] = Array(
											'added'		=> '0.1',
											'type'		=> 'text',
											'default'	=> '<< Previous %v%',
											'is_contextable'	=> TRUE,
										);

		// Text	for	the	link that takes	you	to the today's date
		$this->vars['today_link_text'] = Array(
											'added'		=> '0.1',
											'type'		=> 'text',
											'default'	=> 'Show Today',
											'is_contextable'	=> TRUE,
										 );

		// for the body copy no results use. If true we use the body copy no results.
		$this->vars['use_no_results_body_copy'] = Array(
													'added'			=> '0.7',
													'type'			=> 'boolean',
													'default'		=> '0',
													'parameters'	=> Array('allow_empty' => FALSE),
												  );



		// If you are using	listing	format for a view, this	stores what	sort
		// of list entry you want -	the	full bodycopy or just the title
		$this->vars['list_entry_types'] = Array(
											'added'		=> '0.1',
											'type'		=> 'serialise',
											'default'	=> Array(
															'year'	=> 'title',
															'month'	=> 'title',
															'week'	=> 'title',
															'day'	=> 'bodycopy',
														   ),
										  );

		// If this is set the clicking on empty	space in a table cell
		// will	redirect you to	this asset builder page	to add an event
		$this->vars['asset_builder_page'] = Array(
												'added'		=> '0.1',
												'type'		=> 'assetid',
												'default'	=> '0',
											);

		// Whether you should be allowed to	drag events	around in week and day views
		$this->vars['enable_event_dragging'] = Array(
												'added'			=> '0.1',
												'type'			=> 'boolean',
												'default'		=> 'false',
												'parameters'	=> Array('allow_empty' => FALSE),
											   );

		// Whether you want	to show	a popup	with the event description onmouseover
		$this->vars['enable_description_popups'] = Array(
													'added'			=> '0.1',
													'type'			=> 'boolean',
													'default'		=> 'false',
													'parameters'	=> Array('allow_empty' => FALSE),
												   );

		// If this is nonzero, event DIVs will be sized	so that	there is always
		// room	for	this number	of them	to sit side	by side, even if there aren't
		// ever	this number	of events occurring	concurrently
		$this->vars['column_capacity'] = Array(
											'added'		=> '0.4',
											'type'		=> 'int',
											'default'	=> 0,
										 );

		$this->vars['parameter_map'] = Array(
										'added'			=> '0.4',
										'type'			=> 'parameter_map',
										'default'		=> Array(),
										'parameters'	=> Array(
															'parameters'	=> Array(
																				'replacement_root_node'	=> 'Replacement Event Source (must be a child of the static root node)',
																			   ),
														   ),
									   );

		$this->vars['month_view_event_label_format'] = Array(
														'added'		=> '0.1',
														'type'		=> 'text',
														'default'	=> '%asset_name%',
													   );
		$this->vars['week_view_event_label_format'] = Array(
														'added'		=> '0.1',
														'type'		=> 'text',
														'default'	=> '%asset_name%',
													  );
		$this->vars['day_view_event_label_format'] = Array(
														'added'		=> '0.1',
														'type'		=> 'text',
														'default'	=> '%asset_name%',
													 );

		$this->vars['year_navi_limit'] = Array(
											'added'		=> '0.9',
											'type'		=> 'serialise',
											'default'	=> Array(
															'forward'	=> '',
															'backward'	=> '',
														   ),
										 );
		$this->vars['month_navi_limit'] = Array(
											'added'		=> '0.9',
											'type'		=> 'serialise',
											'default'	=> Array(
															'forward'	=> '',
															'backward'	=> '',
														   ),
										 );
		$this->vars['week_navi_limit'] = Array(
											'added'		=> '0.9',
											'type'		=> 'serialise',
											'default'	=> Array(
															'forward'	=> '',
															'backward'	=> '',
														   ),
										 );
		$this->vars['day_navi_limit'] = Array(
											'added'		=> '0.9',
											'type'		=> 'serialise',
											'default'	=> Array(
															'forward'	=> '',
															'backward'	=> '',
														   ),
										 );

		// Day Labels
		$this->vars['label_mon'] = Array(
										'added'		=> '0.9',
										'type'		=> 'text',
										'default'	=> 'Monday',
										'is_contextable'	=> TRUE,
								   );
		$this->vars['label_tue'] = Array(
										'added'		=> '0.9',
										'type'		=> 'text',
										'default'	=> 'Tuesday',
										'is_contextable'	=> TRUE,
								   );
		$this->vars['label_wed'] = Array(
										'added'		=> '0.9',
										'type'		=> 'text',
										'default'	=> 'Wednesday',
										'is_contextable'	=> TRUE,
								   );
		$this->vars['label_thu'] = Array(
										'added'		=> '0.9',
										'type'		=> 'text',
										'default'	=> 'Thursday',
										'is_contextable'	=> TRUE,
								   );
		$this->vars['label_fri'] = Array(
										'added'		=> '0.9',
										'type'		=> 'text',
										'default'	=> 'Friday',
										'is_contextable'	=> TRUE,
								   );
		$this->vars['label_sat'] = Array(
										'added'		=> '0.9',
										'type'		=> 'text',
										'default'	=> 'Saturday',
										'is_contextable'	=> TRUE,
								   );
		$this->vars['label_sun'] = Array(
										'added'		=> '0.9',
										'type'		=> 'text',
										'default'	=> 'Sunday',
										'is_contextable'	=> TRUE,
								   );

		$this->vars['enable_click_js'] = Array(
										'added'			=> '0.9',
										'type'			=> 'boolean',
										'default'		=> 'true',
										'parameters'	=> Array('allow_empty' => FALSE),
								   );

	}//end constructor


	/**
	* Gets a list of all files that we want web accessable
	*
	* @return array
	* @access private
	*/
	function _getFileList()
	{
		return Array(
				'css/calendar_default.css',
				'js/drag_n_drop.js',
				'js/popup.js',
				'js/click.js',
			   );

	}//end _getFileList()


	/**
	* Return an array with information about the events that this page is listening for
	*
	* @return boolean
	* @access public
	*/
	function getEventList()
	{
		return Array(
				Array(
					'event_name'					=> 'requestKeywords',
					'broadcast_type_code'			=> 'content_type',
					'broadcast_strict_type_code'	=> FALSE,
					'options'						=> Array(
														'side_of_link'	=> 'major',
														'indirect'		=> TRUE,
														'is_exclusive'	=> NULL,
														'is_dependant'	=> 1,
														'value'			=> '',
														'link_type'		=> SQ_LINK_TYPE_2 | SQ_LINK_TYPE_1,
													   ),
				),
			   );

	}//end getEventList()


}//end class
?>
