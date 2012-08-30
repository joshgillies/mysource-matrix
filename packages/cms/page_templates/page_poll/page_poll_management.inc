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
* $Id: page_poll_management.inc,v 1.10 2012/08/30 00:58:09 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Poll_Management
*
* @see Asset_Management
*
* @author  Nathan de Vries <ndvries@squiz.net>
* @author  Robert Howard <rhoward@squiz.net>
* @author  David Heppell <dheppell@squiz.net>
* @version $Revision: 1.10 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Poll_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(&$pm)
	{
		$this->Asset_Management($pm);
		$this->vars = Array(
						// currently active question
						'active_question'			=> Array(
														'added'		=> '0.1',
														'type'		=> 'assetid',
														'default'	=> '',
													   ),
						'vote_button_text'			=> Array(
														'added'		=> '0.2',
														'type'		=> 'text',
														'default'	=> 'Vote',
														'is_contextable'	=> TRUE,
													   ),
						'allow_multiple_votes'		=> Array(
														'added'			=> '0.2',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
													   ),
						'previous_questions_text'	=> Array(
														'added'		=> '0.2',
														'type'		=> 'text',
														'default'	=> 'View Previous Questions',
														'is_contextable'	=> TRUE,
													   ),
						'view_results_text'			=> Array(
														'added'		=> '0.2',
														'type'		=> 'text',
														'default'	=> 'View Results',
														'is_contextable'	=> TRUE,
													   ),
						'new_window_optionsss'		=> Array(
														'added'		=> '0.2',
														'type'		=> 'serialise',
														'default'	=> Array(
																		'width'			=> 250,
																		'height'		=> 400,
																		'scrollbars'	=> 'no',
																		'status'		=> 'no',
																		'resizable'		=> 'no',
																	   ),
													   ),
						'new_window'				=> Array(
														'added'			=> '0.2',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
													   ),
						'new_window_options'		=> Array(
														'added'			=> '0.2',
														'type'			=> 'selection',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'multiple'		=> TRUE,
																			'allow_empty'	=> TRUE,
																			'options'		=> Array(
																								'hide_location'		=> 'Hide Location Bar',
																								'hide_menubar'		=> 'Hide Menubar',
																								'hide_scrollbars'	=> 'Hide Scrollbars',
																								'hide_status'		=> 'Hide Status Bar',
																								'hide_toolbar'		=> 'Hide Tool Bar',
																								'not_resizable'		=> 'Not Resizable',
																							   ),
																		   ),
													   ),
						'new_window_width'			=> Array(
														'added'		=> '0.2',
														'type'		=> 'int',
														'default'	=> 0,
													   ),
						'new_window_height'			=> Array(
														'added'		=> '0.2',
														'type'		=> 'int',
														'default'	=> 0,
													   ),
						'format_bodycopies'			=> Array(
														'added'		=> '0.2',
														'type'		=> 'serialise',
														'default'	=> Array(
																		'question_format'				=> 'vote_formats',
																		'option_list_format'			=> 'vote_formats',
																		'results_format'				=> 'results_formats',
																		'results_list_format'			=> 'results_formats',
																		'past_questions_format'			=> 'past_questions_formats',
																		'past_questions_list_format'	=> 'past_questions_formats',
																	   ),
													   ),
						// graph box
						'graph_box_width'			=> Array(
														'added'		=> '0.1',
														'type'		=> 'text',
														'default'	=> '400',
													   ),
						'graph_box_height'			=> Array(
														'added'		=> '0.1',
														'type'		=> 'text',
														'default'	=> '400',
													   ),
						'graph_box_colour_bar'		=> Array(
														'added'		=> '0.1',
														'type'		=> 'colour',
														'default'	=> '#0000cc',
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
