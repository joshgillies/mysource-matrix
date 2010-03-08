<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +--------------------------------------------------------------------+
*
* $Id: funnelback_search_page_management.inc,v 1.1.1.1.2.1 2010/03/08 05:50:11 bpearson Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Funnelback_Search_Page_Management
*
* @see Asset_Management
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.1.1.1.2.1 $
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
																				 ),
																'multiple'    => FALSE,
																'allow_empty' => FALSE,
																'allow_other' => FALSE,
															   ),
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
