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
* $Id: listing_engine_management.inc,v 1.84 2011/07/11 02:04:42 cupreti Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Listing_Engine_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.84 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Listing_Engine_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars['types'] = Array(
								'added'		=> '0.1',
								'type'		=> 'serialise',
								'default'	=> Array(),
							   );
		$this->vars['exclude_assets']	= Array (
											'added'			=> '0.1',
											'type'			=> 'serialise',
											'default'		=> Array(),
										);
		$this->vars['exclude_root_nodes']	= Array (
											'added'			=> '0.17',
											'type'			=> 'serialise',
											'default'		=> Array(),
										  );
		$this->vars['exclude_current_asset']	= Array (
													'added'			=> '0.18',
													'type'			=> 'boolean',
													'default'		=> 'false',
													'parameters'    => Array( 'allow_empty' => FALSE,),
													'description'	=> 'Can now exclude the current asset from the results.',
												   );
		$this->vars['statuses'] = Array(
									'added'			=> '0.8',
									'type'			=> 'serialise',
									'default'		=> Array(),
									'description'	=> 'Statuses that are being listed.',
								  );

		$this->vars['list_permission'] = Array(
											'added'			=> '0.15',
											'type'			=> 'int',
											'default'		=> SQ_PERMISSION_READ,
											'description'	=> 'Only assets that the user has this access to will be listed.',
										 );

		$this->vars['list_effective'] = Array(
											'added'			=> '0.15',
											'type'			=> 'boolean',
											'default'		=> TRUE,
											'description'	=> 'If effective permissions should be used when listing restricted by permissions.',
										);

		$this->vars['link_types'] = Array(
										'added'			=> '0.13',
										'type'			=> 'int',
										'default'		=> SQ_SC_LINK_BACKEND_NAV,
										'description'	=> 'Link types that are to be searched. Only applies when Direct Links Only is selected.',
									);

		$this->vars['submit_button_text'] = Array(
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> 'Submit',
												'is_contextable'	=> TRUE,
											);

		$this->vars['reset_button_text'] = Array(
											'added'		=> '0.1',
											'type'		=> 'text',
											'default'	=> 'Reset',
											'is_contextable'	=> TRUE,
										   );

		$this->vars['sort_by'] = Array(
									'added'		=> '0.1',
									'type'		=> 'serialise',
									'default'	=> Array(
													0	=> Array (
															'params'	=> Array (
																			'field'	=> '',
																		   ),
															'type'		=> '',
															'name'		=> 'No Sorting',
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

		$this->vars['default_sort_by'] = Array(
											'added'		=> '0.1',
											'type'		=> 'int',
											'default'	=> 0,
										 );

		$this->vars['metadata_sort_type'] = Array(
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> 'presentation',
											);

		$this->vars['reverse_sort'] = Array(
										'added'			=> '0.1',
										'type'			=> 'boolean',
										'default'		=> FALSE,
										'parameters'	=> Array(
															'allow_empty'	=> FALSE,
														   ),
									  );

		$this->vars['sort_direction_asc_text'] = Array(
													'added'		=> '0.1',
													'type'		=> 'text',
													'default'	=> 'Ascending',
													'is_contextable'	=> TRUE,
												 );

		$this->vars['sort_direction_desc_text'] = Array(
													'added'		=> '0.1',
													'type'		=> 'text',
													'default'	=> 'Descending',
													'is_contextable'	=> TRUE,
												  );

		$this->vars['subs_only'] = Array(
									'added'			=> '0.1',
									'type'			=> 'boolean',
									'default'		=> FALSE,
									'parameters'	=> Array( 'allow_empty' => FALSE,),
								   );

		$this->vars['direction'] = Array(
									'added'			=> '0.1',
									'type'			=> 'selection',
									'default'		=> 'down',
									'parameters'	=> Array(
														'multiple'		=> FALSE,
														'allow_empty'	=> FALSE,
														'options'		=> Array(
																			'up'	=> 'Up the tree',
																			'down'	=> 'Down the tree',
																		   ),
													   ),
								   );

		$this->vars['group_by'] = Array(
									'added'			=> '0.1',
									'type'			=> 'selection',
									'default'		=> 'number',
									'parameters'	=> Array(
														'multiple'		=> FALSE,
														'allow_empty'	=> FALSE,
														'options'		=> Array(
																			'number'		=> 'Standard',
																			'letter'		=> 'A-Z listing',
																			'grouped'		=> 'Custom Grouping',
																			'random'		=> 'Random',
																		   ),
													   ),
								  );
								  
		$this->vars['a_z_style'] = Array(
									'added'			=> '0.1',
									'type'			=> 'selection',
									'default'		=> 'multiple',
									'parameters'	=> Array(
														'multiple'		=> FALSE,
														'allow_empty'	=> FALSE,
														'options'		=> Array(
																			'single'	=> 'Single Page',
																			'multiple'	=> 'Multiple Pages',
																		   ),
													   ),
								  );
								  
		$this->vars['a_z_letter_format'] = Array(
											'added'		=> '0.1',
											'type'		=> 'text',
											'default'	=> ' <h3> %letter_group% </h3> ',
										);						  

		$this->vars['num_per_page'] = Array(
										'added'		=> '0.1',
										'type'		=> 'text',
										'default'	=> 0,
									  );
									  

		$this->vars['column_layout'] = Array(
										'added'		=> '0.1',
										'type'		=> 'serialise',
										'default'	=> Array('0' => '1'),
									   );

        $this->vars['column_layout_type'] = Array(
									'added'			=> '0.16',
									'type'			=> 'selection',
									'default'		=> 'table',
									'parameters'	=> Array(
														'multiple'		=> FALSE,
														'allow_empty'	=> FALSE,
														'options'		=> Array(
																			'table'	=> 'Table',
																			'div'	=> 'Div',
																		   ),
													   ),
								   );
        $this->vars['div_row_class'] = Array(
											'added'		=> '0.16',
											'type'		=> 'text',
											'default'	=> '',
										);
		$this->vars['asset_positions'] = Array(
											'added'		=> '0.1',
											'type'		=> 'serialise',
											'default'	=> Array(),
										 );

		$this->vars['lineage_seperator'] = Array (
											'added'			=> '0.1',
											'type'			=> 'text',
											'default'		=> ' > ',
											'description'	=> 'Seperator to use when printing the lineage to a search result asset.',
										   );

		$this->vars['root_in_lineage'] = Array(
											'added'			=> '1.0',
											'type'			=> 'boolean',
											'default'		=> FALSE,
											'parameters'	=> Array(
																'allow_empty'	=> FALSE,
															   ),
										 );

		$this->vars['self_in_lineage'] = Array(
											'added'			=> '1.0',
											'type'			=> 'boolean',
											'default'		=> FALSE,
											'parameters'	=> Array(
																'allow_empty'	=> FALSE,
															   ),
										 );

		$this->vars['prev_page_text'] = Array(
											'added'		=> '0.1',
											'type'		=> 'text',
											'default'	=> 'Previous',
											'is_contextable'	=> TRUE,
										);

		$this->vars['next_page_text'] = Array(
											'added'		=> '0.1',
											'type'		=> 'text',
											'default'	=> 'Next',
											'is_contextable'	=> TRUE,
										);

		$this->vars['parameter_map'] = Array(
										'added'			=> '0.4',
										'type'			=> 'parameter_map',
										'default'		=> Array(),
										'parameters'	=> Array(
															'parameters'	=> Array(
																				'root_node'	=> 'Replacement root nodes for the listing (must be children of static root nodes)',
																			   ),
														   ),
									   );

		$this->vars['asset_grouping'] = Array(
											'added'		=> '0.5',
											'type'		=> 'serialise',
											'default'	=> Array(),
										);

		$this->vars['asset_selections'] = Array(
											'added'			=> '0.9',
											'type'			=> 'serialise',
											'default'		=> Array(),
											'description'	=> 'Sets of named checkboxes, drop-down lists, etc',
										  );

		$this->vars['asset_targets'] = Array(
										'added'			=> '0.9',
										'type'			=> 'serialise',
										'default'		=> Array(),
										'description'	=> 'Adds buttons to the bodycopy that point to other assets or URLs',
									   );

		$this->vars['structured_dropdown_options'] = Array(
														'added'			=> '0.10',
														'type'			=> 'serialise',
														'default'		=> Array(
																			'name'			=> 'root_node_selection',
																			'all_text'		=> 'All',
																			'root_node'		=> '0',
																			'asset_types'	=> Array(),
																			'max_depth'		=> '0',
																			'height'		=> '0',
																			'width'			=> '0',
																			'seperator'		=> '-',
																		   ),
														'description'	=> 'Options for the Structured Drop Down',
													 );
		$this->vars['check_boxes'] = Array(
										'added'		=> '0.10',
										'type'		=> 'int',
										'default'	=> 0,
									 );

		$this->vars['other_page_text'] = Array(
											'added'		=> '0.11',
											'type'		=> 'text',
											'default'	=> '[Other]',
											'is_contextable'	=> TRUE,
										 );

		$this->vars['form_submit_method'] = Array(
												'added'			=> '0.12',
												'type'			=> 'selection',
												'default'		=> 'get',
												'parameters'	=> Array(
																	'multiple'		=> FALSE,
																	'allow_empty'	=> FALSE,
																	'options'		=> Array(
																						'get'	=> 'GET',
																						'post'	=> 'POST',
																					   ),
																   ),
											);

		$this->vars['min_depth'] = Array(
									'added'			=> '0.14',
									'type'			=> 'int',
									'default'		=> '',
									'parameters'	=> Array(
														'allow_negative'	=> FALSE,
														'allow_empty'		=> TRUE,
													   ),
								   );

		$this->vars['max_depth'] = Array(
									'added'			=> '0.14',
									'type'			=> 'int',
									'default'		=> '',
									'parameters'	=> Array(
														'allow_negative'	=> FALSE,
														'allow_empty'		=> TRUE,
													   ),
								   );


		$this->vars['submit_to_page_url'] = Array(
												'added'			=> '0.1',
												'type'			=> 'boolean',
												'default'		=> TRUE,
												'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
											);

		$this->vars['proxy_types'] = Array(
										'added'		=> '0.16',
										'type'		=> 'serialise',
										'default'	=> Array(),
									 );
									 
		$this->vars['page_link_format'] = Array(
											'added'		=> '0.16',
											'type'		=> 'text',
											'default'	=> ' <a href="%page_link%">%page_number%</a> ',
										);
										
		$this->vars['current_page_format'] = Array(
											'added'		=> '0.16',
											'type'		=> 'text',
											'default'	=> ' %page_number% ',
										);
		$this->vars['start_position'] = Array(
											'added'			=> '0.16',
											'type'			=> 'text',
											'default'		=> 0,
											'description' 	=> 'Defines which position in the list to start at',
										);
		$this->vars['dynamic_root_option'] = Array(
										'added'			=> '0.16',
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
										'added'			=> '0.16',
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
	* Gets a list of all files that we want web accessable
	*
	* @return array
	* @access private
	*/
	function _getFileList()
	{
		return Array(
				'js/select_all.js',
			   );

	}//end _getFileList()


}//end class

?>
