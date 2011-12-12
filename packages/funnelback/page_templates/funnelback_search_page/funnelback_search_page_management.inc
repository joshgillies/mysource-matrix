<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: funnelback_search_page_management.inc,v 1.10 2011/12/12 00:07:33 akarelia Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Funnelback_Search_Page_Management
*
* @see Asset_Management
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.10 $
* @package MySource_Matrix_Packages
* @subpackage funnelback
*/
class Funnelback_Search_Page_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Funnelback_Search_Page_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars['search_type'] = Array(
											'added'			=> '0.1',
											'type'			=> 'selection',
											'default'		=> 'integrated',
											'parameters'	=> Array(
																'options'     => Array(
																					'integrated'	=> 'Integrated Search',
																					'web'			=> 'Funnelback Server Search',
																					'squiz'			=> 'Squiz Search',
																					'hosted'		=> 'Funnelback Search',
																				 ),
																'multiple'    => FALSE,
																'allow_empty' => FALSE,
																'allow_other' => FALSE,
															   ),
										  );

		$this->vars['systemid'] = Array(
									'added'			=> '0.1',
									'type'			=> 'text',
									'default'		=> '',
									'parameters'	=> Array(),
								  );

		$this->vars['funnelback_url'] = Array(
											'added'			=> '0.1',
											'type'			=> 'text',
											'default'		=> '',
											'parameters'	=> Array(),
										  );

		$this->vars['collection'] = Array(
										'added'			=> '0.1',
										'type'			=> 'text',
										'default'		=> '',
										'parameters'	=> Array(),
									);

		$this->vars['allow_empty'] = Array(
										'added'			=> '0.1',
										'type'			=> 'boolean',
										'default'		=> TRUE,
										'description'	=> 'Allow empty search queries',
										'parameters'	=> Array('allow_empty' => FALSE),
									 );

		$this->vars['sort_order'] = Array(
										'added'			=> '0.1',
										'type'			=> 'text',
										'default'		=> '',
										'parameters'	=> Array(),
									);

		$this->vars['scope'] = Array(
								'added'			=> '0.1',
								'type'			=> 'serialise',
								'default'		=> Array('ALL'=>'Site'),
							   );

		$this->vars['default_scope'] = Array(
										'added'			=> '0.1',
										'type'			=> 'text',
										'default'		=> '',
										'parameters'	=> Array(),
									   );

		$this->vars['dynamic_parameters'] = Array(
												'added'			=> '0.1',
												'type'			=> 'parameter_map',
												'default'		=> Array(),
												'parameters'	=> Array(
																	'parameters'	=> Array(
																						'query'				=> 'Query',
																						'results_per_page'	=> 'Results per page',
																						'scope'				=> 'Scope',
																						'sort_order'		=> 'Sort Order',
																					   ),
																   ),
										    );

		$this->vars['stored_query_show_results'] = Array(
													'added'			=> '0.1',
													'type'			=> 'boolean',
													'default'		=> FALSE,
													'description'	=> 'Go directly to the results if the query is stored',
													'parameters'	=> Array('allow_empty' => FALSE),
												   );

		$this->vars['show_fluster_more'] = Array(
											'added'			=> '0.1',
											'type'			=> 'boolean',
											'default'		=> FALSE,
											'description'	=> 'Go directly to the results if the query is stored',
											'parameters'	=> Array('allow_empty' => FALSE),
										   );

		$this->vars['more_type_text'] = Array(
											'added'			=> '0.1',
											'type'			=> 'text',
											'default'		=> 'more types',
											'parameters'	=> Array(),
										);

		$this->vars['more_type_num'] = Array(
											'added'			=> '0.1',
											'type'			=> 'int',
											'default'		=> '20',
											'parameters'	=> Array(),
									   );

		$this->vars['less_type_text'] = Array(
											'added'			=> '0.1',
											'type'			=> 'text',
											'default'		=> 'fewer types',
											'parameters'	=> Array(),
										);

		$this->vars['less_type_num'] = Array(
											'added'			=> '0.1',
											'type'			=> 'int',
											'default'		=> '8',
											'parameters'	=> Array(),
									   );

		$this->vars['more_topic_text'] = Array(
											'added'			=> '0.1',
											'type'			=> 'text',
											'default'		=> 'more topics',
											'parameters'	=> Array(),
										);

		$this->vars['more_topic_num'] = Array(
											'added'			=> '0.1',
											'type'			=> 'int',
											'default'		=> '20',
											'parameters'	=> Array(),
										);

		$this->vars['less_topic_text'] = Array(
											'added'			=> '0.1',
											'type'			=> 'text',
											'default'		=> 'fewer topics',
											'parameters'	=> Array(),
										);

		$this->vars['less_topic_num'] = Array(
											'added'			=> '0.1',
											'type'			=> 'int',
											'default'		=> '8',
											'parameters'	=> Array(),
										);

		$this->vars['more_site_text'] = Array(
											'added'			=> '0.1',
											'type'			=> 'text',
											'default'		=> 'more sites',
											'parameters'	=> Array(),
										);

		$this->vars['more_site_num'] = Array(
											'added'			=> '0.1',
											'type'			=> 'int',
											'default'		=> '20',
											'parameters'	=> Array(),
									   );

		$this->vars['less_site_text'] = Array(
											'added'			=> '0.1',
											'type'			=> 'text',
											'default'		=> 'fewer sites',
											'parameters'	=> Array(),
										);

		$this->vars['less_site_num'] = Array(
											'added'			=> '0.1',
											'type'			=> 'int',
											'default'		=> '8',
											'parameters'	=> Array(),
									   );

		// Num per page is set here, to override the default of 0
		$this->vars['num_per_page'] = Array(
										'added'		=> '0.1',
										'type'		=> 'text',
										'default'	=> 10,
									  );

		$this->vars['word_logic'] = Array(
										'added'			=> '0.1',
										'type'			=> 'selection',
										'default'		=> '1',
										'parameters'	=> Array(
															'multiple'		=> FALSE,
															'allow_empty'	=> FALSE,
															'options'		=> Array(
																				0 => 'Exclude',
																				1 => 'Include any',
																				2 => 'Include all',
																			),
											),
									  );

	$this->vars['disable_keyword_replacements'] = Array(
										'added'			=> '0.2',
										'type'			=> 'boolean',
										'default'		=> TRUE,
										'description'	=> 'If true, will not replace any keywords from the user input',
										'parameters'	=> Array('allow_empty' => FALSE),
									   );


	$this->vars['hosted_mode'] = Array(
										'added'			=> '0.2',
										'type'			=> 'selection',
										'default'		=> 1,
										'parameters'	=> Array(
															'multiple'		=> FALSE,
															'allow_empty'	=> FALSE,
															'options'		=> Array(
																				0 => 'Classic',
																				1 => 'Advanced',
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
