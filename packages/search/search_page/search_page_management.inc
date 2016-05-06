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
* $Id: search_page_management.inc,v 1.71 2012/08/30 00:59:15 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Search_Page_Management
*
* @see Asset_Management
*
* @author  Dominic Wong <dwong@squiz.net>
* @version $Revision: 1.71 $
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
											'default'		=> TRUE,
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

		$this->vars['parameter_map'] = Array(
										'added'			=> '0.15',
										'type'			=> 'parameter_map',
										'default'		=> Array(),
										'parameters'	=> Array(
															'parameters'	=> Array(
																				'root_node'			=> 'Replacement Root node for the listing (must be a child of the static root node)',
																				'asset_selection'	=> 'Array of specific asset IDs or selection group (specified assets must be children of the static root node)',
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


	/**
	* Upgrade
	*
	* @param string	$current_version	the version of the asset that is currenly installed
	*
	* @return boolean
	* @access public
	*/
	function _upgrade($current_version)
	{
		parent::_upgrade($current_version);

		if (version_compare($current_version, '0.27', '<')) {
			echo('UPGRADING SEARCH PAGE FROM '.$current_version."\n");
			$count = 0;
			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
			pre_echo('Updating Search Pages...');
			// Get all Listing Engine Page assets
			$search_pages = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('search_page', FALSE, TRUE);
			$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_OPEN);
			foreach ($search_pages as $id => $type) {
				$search_page = $GLOBALS['SQ_SYSTEM']->am->getAsset($id);
				$get_attr_details = $search_page->getAttribute('no_roles_check');
				$attr_id = $get_attr_details->id;
				$sql = 'SELECT custom_val FROM sq_ast_attr_val';
				$where = ' WHERE assetid = '.MatrixDAL::quote($id);
				$where .= ' AND attrid = '.MatrixDAL::quote($attr_id);
				$query = MatrixDAL::preparePdoQuery($sql.$where);
				$result = DAL::executePdoAll($query);
				if(empty($result)) {
					$search_page->setAttrValue('no_roles_check', FALSE);
					if (!$search_page->saveAttributes()) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
						pre_echo('UPGRADE FAILED - Could not save attribute of asset #'.$id);
						$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
						return FALSE;
					}//end if
					$count++;
				}
				
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($search_page, TRUE);
		        unset($search_page);
			}

			pre_echo('Updated - '.$count.' search page(s)...');

			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

			pre_echo('SEARCH PAGES SUCCESSFULLY UPGRADED TO VERSION 0.27');
		}

		if (version_compare($current_version, '0.26', '<')) {
			echo('UPGRADING SEARCH PAGE FROM '.$current_version."\n");
			// Version 0.26 ownwards the "exclude" search field is stored
			// as regular search field having its own set of datasoruce settings.
			// A flag in search field "exclude_field" tells if the search
			// field is "exclude" or regular one.

			$count = 0;

			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

			pre_echo('Updating Search Pages...');
			
			// Get all Search Page assets
			$search_pages = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('search_page', FALSE, TRUE);

			// Update assets
			foreach ($search_pages as $id => $type) {
				$search_page = $GLOBALS['SQ_SYSTEM']->am->getAsset($id);				
				
				$old_fields = $search_page->attr('fields');
				$new_fields = Array();
				foreach($old_fields as $field_name => $field_data) {
					// Update "exclude" fields
					if (isset($field_data['data_sources'][0]['type']) && $field_data['data_sources'][0]['type'] == 'exclude') {
						$new_fields[$field_name]['name'] = $field_name;
						$new_fields[$field_name]['word_logic'] = 'OR';
						$new_fields[$field_name]['exclude_field'] = TRUE;
						$new_fields[$field_name]['data_sources'] = Array(
																		0 => Array(
																			'type'		=> 'include_all',
																			'params'		=> Array(),
																			),
																	);
					} else {
						$new_fields[$field_name] = $field_data;
						$new_fields[$field_name]['exclude_field'] = FALSE;
					}	
				}//end foreach old_fields
				
				$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
					$search_page->setAttrValue('fields', $new_fields);
					
					if (!$search_page->saveAttributes()) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
						
						pre_echo('UPGRADE FAILED - Could not save attributes');
						return FALSE;
					}//end if					
				$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
				
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($search_pages, TRUE);
				unset($search_pages);
				
				$count++;
			}//end foreach search pages

			pre_echo('Updated - '.$count.' search page(s)...');

			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

			pre_echo('SEARCH PAGES SUCCESSFULLY UPGRADED TO VERSION 0.26');
		}//end if
		return TRUE;

	}//end _upgrade()
	
	
}//end class

?>
