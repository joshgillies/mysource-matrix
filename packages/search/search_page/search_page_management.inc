<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: search_page_management.inc,v 1.67.8.2 2011/02/21 04:15:42 cupreti Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Search_Page_Management
*
* @see Asset_Management
*
* @author  Dominic Wong <dwong@squiz.net>
* @version $Revision: 1.67.8.2 $
* @package MySource_Matrix_Packages
* @subpackage search
*/
class Search_Page_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Search_Page_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars['group_by'] = Array(
									'added'			=> '0.1',
									'type'			=> 'selection',
									'default'		=> 'number',
									'parameters'	=> Array(
														'multiple'		=> FALSE,
														'allow_empty'	=> FALSE,
														'options'		=> Array(
																			'number'	=> 'Standard',
																			'grouped'	=> 'Custom Grouping',
																		   ),
													   ),
								  );

		$this->vars['sis_button_text'] = Array(
											'added'			=> '0.5',
											'type'			=> 'text',
											'default'		=> 'Search',
											'description'	=> 'Text that would appear on the search in search submit button',
											'is_contextable'	=> TRUE,
										 );

		$this->vars['fields'] = Array(
									'added'			=> '0.1',
									'type'			=> 'serialise',
									'default'		=> Array(),
									'description'	=> 'Search fields.',
								);

		$this->vars['categories'] = Array(
										'added'			=> '0.4',
										'type'			=> 'serialise',
										'default'		=> Array(),
										'description'	=> 'Search categories.',
									);

		$this->vars['no_category_text'] = Array(
											'added'			=> '0.4',
											'type'			=> 'text',
											'default'		=> 'No Category Selected',
											'description'	=> 'The text that appears for a "no category selected" options.',
											'is_contextable'	=> TRUE,
										  );

		$this->vars['auto_synonym_search_field'] = Array(
													'added'			=> '0.21',
													'type'			=> 'text',
													'default'		=> '',
													'description'	=> 'Field to automatically search for synonyms on if the original search term(s) return no results. Empty string = auto synonym searching disabled',
												   );

		$this->vars['search_types'] = Array(
										'added'			=> '0.1',
										'type'			=> 'serialise',
										'default'		=> Array(),
										'description'	=> 'Searchable asset types',
									  );

		$this->vars['search_styles'] = Array(
										'added'			=> '0.1',
										'type'			=> 'serialise',
										'default'		=> Array(),
										'description'	=> 'Styles of the search fields',
									   );

		$this->vars['use_picker'] = Array(
										'added'			=> '0.6',
										'type'			=> 'boolean',
										'default'		=> FALSE,
										'description'	=> 'Use the JS date picker or not',
										'parameters'	=> Array('allow_empty' => FALSE),
									);

		$this->vars['logic'] = Array(
								'added'			=> '0.1',
								'type'			=> 'selection',
								'default'		=> 'AND',
								'description'	=> 'Whether to include all or any fields.',
								'parameters'	=> Array(
													'multiple'		=> FALSE,
													'allow_empty'	=> FALSE,
													'options'		=> Array(
																		'AND'	=> 'Match on ALL fields',
																		'OR'	=> 'Match on at least ONE field',
																	   ),
												   ),
							   );

		$this->vars['allow_empty_search']	= Array(
												'added'			=> '0.12',
												'type'			=> 'boolean',
												'default'		=> FALSE,
												'description'	=> 'If true, empty search query will search for everything',
												'parameters'	=> Array('allow_empty' => FALSE),
											  );
		$this->vars['search_as_public_user']	= Array(
													'added'			=> '0.12',
													'type'			=> 'boolean',
													'default'		=> FALSE,
													'description'	=> 'If true, only look for assets which were granted public user read',
													'parameters'	=> Array('allow_empty' => FALSE),
												  );
		$this->vars['no_roles_check']	= Array(
											'added'			=> '0.12',
											'type'			=> 'boolean',
											'default'		=> FALSE,
											'description'	=> 'If true, search for assets without checking roles',
											'parameters'	=> Array('allow_empty' => FALSE),
										  );
		$this->vars['no_group_access_check'] = Array(
												'added'			=> '0.12',
												'type'			=> 'boolean',
												'default'		=> FALSE,
												'description'	=> 'If true, search for assets without checking groups access priviledge for current user',
												'parameters'	=> Array('allow_empty' => FALSE),
											   );


		$this->vars['spell_check_string']	= Array(
												'added'			=> '0.19',
												'type'			=> 'text',
												'default'		=> 'Did you mean: %suggestions%',
												'description'	=> 'String to use for the "spell check suggestions" keyword',
												'is_contextable'	=> TRUE,
											  );

		$this->vars['synonym_string']	= Array(
											'added'			=> '0.19',
											'type'			=> 'text',
											'default'		=> 'Did you mean: %suggestions%',
											'description'	=> 'String to use for the "synonyms" keyword',
											'is_contextable'	=> TRUE,
										  );

		$this->vars['auto_synonym_string']	= Array(
												'added'			=> '0.21',
												'type'			=> 'text',
												'default'		=> '(and related terms)',
												'description'	=> 'String to display for the "auto synonym search triggered" keyword',
												'is_contextable'	=> TRUE,
											  );

		$this->vars['stored_query_session']	= Array(
												'added'			=> '0.21',
												'type'			=> 'parameter_map',
												'default'		=> Array(),
												'description'	=> 'Use saved variables to conduct the search query',
												'parameters'	=> Array(
																	'parameters'	=> Array(
																						'stored_query'	=> 'Search all fields',
																					   ),
																   ),
											  );

		$this->vars['stored_query_show_results']	= Array(
														'added'			=> '0.21',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'description'	=> 'Go directly to the results if the query is stored',
														'parameters'	=> Array('allow_empty' => FALSE),
													  );

		// Default sort-by options are different for search page
		$this->vars['sort_by'] = Array(
									'added'		=> '0.1',
									'type'		=> 'serialise',
									'default'	=> Array(
													0	=> Array (
															'params'	=> Array (
																			'field'	=> 'score',
																		   ),
															'type'		=> '',
															'name'		=> 'Relevance',
														   ),
													1	=> Array (
															'params'	=> Array (
																			'field'	=> 'name',
																		   ),
															'type'		=> 'field',
															'name'		=> 'Name',
														   ),
													2	=> Array (
															'params'	=> Array (
																			'field'	=> 'short_name',
																		   ),
															'type'		=> 'field',
															'name'		=> 'Short Name',
														   ),
													3	=> Array (
															'params'	=> Array (
																			'field'	=> 'created',
																		   ),
															'type'		=> 'field',
															'name'		=> 'Created Date',
														   ),
													4	=> Array (
															'params'	=> Array (
																			'field'	=> 'updated',
																		   ),
															'type'		=> 'field',
															'name'		=> 'Last Updated Date',
														   ),
													5	=> Array (
															'params'	=> Array (
																			'field'	=> 'published',
																		   ),
															'type'		=> 'field',
															'name'		=> 'Last Published Date',
														   ),
													6	=> Array (
															'params'	=> Array (
																			'field'	=> 'type_code',
																		   ),
															'name'		=> 'Asset Type',
															'type'		=> 'field',
														   ),
													7	=> Array (
															'params'	=> Array (
																			'field'	=> '',
																		   ),
															'name'		=> 'Random',
															'type'		=> '',
														   ),
												   ),
								 );

		$this->vars['enable_multi_word_synonym_lookups']	= Array(
																'added'			=> '0.22',
																'type'			=> 'boolean',
																'default'		=> FALSE,
																'description'	=> 'If true, a Thesaurus lookup will be performed for matching synonyms based on combinations of words from the search string',
																'parameters'	=> Array('allow_empty' => FALSE),
															  );

		$this->vars['exclude_self']	= Array(
													'added'			=> '0.23',
													'type'			=> 'boolean',
													'default'		=> FALSE,
													'description'	=> 'If true, only look for assets underneath root nodes',
													'parameters'	=> Array('allow_empty' => FALSE),
												  );
		$this->vars['restrict_date_field'] = Array(
												'added'			=> '0.24',
												'type'			=> 'selection',
												'default'		=> 'updated',
												'parameters'	=> Array(
																	'options'	=> Array(
																					'created'	=> 'Created Date',
																					'updated'	=> 'Last Updated Date',
																					'published'	=> 'Last Published Date',
																					),
																   ),
												'multiple'		=> FALSE,
												'allow_empty'	=> FALSE,
											   );
		$this->vars['restrict_date_range_from'] = Array(
												'added'			=> '0.24',
												'type'			=> 'datetime',
												'default'		=> '---------- --:--:--',
												'parameters'	=> Array(
																	'allow_null'	=> TRUE,
																   ),
											   );
		$this->vars['restrict_date_range_to'] = Array(
												'added'			=> '0.24',
												'type'			=> 'datetime',
												'default'		=> '---------- --:--:--',
												'parameters'	=> Array(
																	'allow_null'	=> TRUE,
																   ),
											   );
		$this->vars['page_link_format'] = Array(
											'added'		=> '0.22',
											'type'		=> 'text',
											'default'	=> ' <a href="%page_link%">%page_number%</a>',
										);
										
		$this->vars['current_page_format'] = Array(
											'added'		=> '0.22',
											'type'		=> 'text',
											'default'	=> ' <b>%page_number%</b>',
										);
		$this->vars['disable_keyword_replacements']		= Array(
															'added'			=> '0.25',
															'type'			=> 'boolean',
															'default'		=> TRUE,
															'description'	=> 'If true, will not replace any keywords from the user input',
															'parameters'	=> Array('allow_empty' => FALSE),
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