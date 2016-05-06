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
* $Id: page_calendar_events_search_management.inc,v 1.6 2013/09/17 06:39:40 cupreti Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Calendar_Events_Search_Management
*
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: 1.6 $
* @package MySource_Matrix_Packages
* @subpackage calendar
*/
class Page_Calendar_Events_Search_Management extends Asset_Management
{


	/**
	* Constructor
	*
	* @param object Package_Manager	&$pm	Reference to the package manager object for this package
	*
	*/
	function Page_Calendar_Events_Search_Management(&$pm)
	{
		$this->Asset_Management($pm);		
		
		$this->vars['types'] = Array(
										'added'			=> '0.1',
										'type'			=> 'serialise',
										'default'		=> Array(
															'calendar_event_single' => '1',
															'calendar_event_recurring' => '1',
															'calendar_event_multi_date' => '1',
															),
										'description'	=> 'Searchable asset types',
									 );
       $this->vars['statuses'] = Array(
									'added'         => '0.8',
									'type'          => 'serialise',
									'default'       => Array(),
									'description'   => 'Statuses that are being listed.',
                                  );									 
		$this->vars['events_filters'] = Array(
										'added'			=> '0.1',
										'type'			=> 'serialise',
										'default'		=> Array(),
										'description'	=> 'Calendar events date filter fields.',
									);

		$this->vars['events_filter_logic'] = Array(
												'added'			=> '0.1',
												'type'			=> 'selection',
												'default'		=> 'AND',
												'description'	=> 'Whether to include all or any events date filter fields.',
												'parameters'	=> Array(
																	'multiple'		=> FALSE,
																	'allow_empty'	=> FALSE,
																	'options'		=> Array(
																						'AND'	=> 'Match on ALL fields',
																						'OR'	=> 'Match on at least ONE field',
																					   ),
																   ),
											   );
		
		$this->vars['main_logic'] = Array(
												'added'			=> '0.1',
												'type'			=> 'selection',
												'default'		=> 'AND',
												'description'	=> 'Whether to include all or any events date filter fields.',
												'parameters'	=> Array(
																	'multiple'		=> FALSE,
																	'allow_empty'	=> FALSE,
																	'options'		=> Array(
																						'AND'	=> 'Match on both the Date Filter and Search Field Settings',
																						'OR'	=> 'Match on either the Date Filter or Search Field Settings',
																					   ),
																   ),
											   );
		
		$this->vars['events_filter_allow_empty_search']	= Array(
												'added'			=> '0.1',
												'type'			=> 'boolean',
												'default'		=> FALSE,
												'description'	=> 'If true, empty date filter query will search for everything',
												'parameters'	=> Array('allow_empty' => FALSE),
											  );
		
		$this->vars['dynamic_root_option'] = Array(
										'added'			=> '0.27',
										'type'			=> 'selection',
										'default'		=> 'static_root',
										'parameters'	=> Array(
															'multiple'		=> FALSE,
															'allow_empty'	=> FALSE,
															'options'		=> Array(
																				'static_root'		=> 'Use static root node',
																				'empty_result'		=> 'Return empty result',
																			   ),
														   ),														   
									   );
		$this->vars['dynamic_asset_selection_option'] = Array(
										'added'			=> '0.27',
										'type'			=> 'selection',
										'default'		=> 'static_root',
										'parameters'	=> Array(
															'multiple'		=> FALSE,
															'allow_empty'	=> FALSE,
															'options'		=> Array(
																				'static_root'		=> 'Use static root node',
																				'empty_result'		=> 'Return empty result',
																			   ),
														   ),														   
									   );
											   
									 
	}//end constructor
	
	
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
