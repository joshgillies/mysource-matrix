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
* $Id: page_template_builder_management.inc,v 1.4 2004/12/06 14:41:40 brobertson Exp $
*
*/


require_once  SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Template_Builder_Management
*
* @see Asset_Management
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Template_Builder_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Page_Template_Builder_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
							/// Create Status ///
							'create_status' => Array(
												 'added'       => '0.1',
												 'type'        => 'selection',
												 'default'     => (string)SQ_STATUS_UNDER_CONSTRUCTION,
												 'description' => 'Status of created bodycopies',
												 'parameters'  => Array(
																	'options' => Array(
																				   (string)SQ_STATUS_UNDER_CONSTRUCTION => 'Under Construction',
																				   (string)SQ_STATUS_LIVE               => 'Live',
																				   (string)SQ_STATUS_ARCHIVED           => 'Archived',
																				 ),
																	'multiple' => false,
																	'allow_empty' => false,
																  ),
									           ),
							/// Create Locations ///
							'create_locations' => Array(
													'added'		=> '0.0.1',
													'type'		=> 'serialise',
													'default'	=> Array(),
									),


							/// Map for connecting runtime data sources with parameters ///
							'parameter_map' => Array(
													'added'		=> '0.0.2',
													'type'		=> 'parameter_map',
													'parameters' => Array(
																		'parameters' => Array(
																			'additional_locations' => 'Additional Create Locations',
																		)
																	),
													'default' => Array(),
												),
							/// Additional (query-fed) create locations must be children of one of these ///
							'additional_location_parents' => Array(
													'added'		=> '0.1',
													'type'		=> 'serialise',
													'default'	=> Array(),
												),
						);

	}//end constructor


	/**
	* returns an array with information about the events that the form is listening for
	*
	* @access public
	* @return boolean
	*/
	function getEventList()
	{
		return Array(
				Array(
					'event_name'				 => 'requestKeywords',
					'broadcast_type_code'		 => 'content_type_wysiwyg',
					'broadcast_strict_type_code' => true,
					'options'	=> Array(
						'side_of_link'		=> 'major',
						'indirect'			=> true,
						'is_exclusive'		=> null,
						'is_dependant'		=> 1,
						'value'				=> '',
						'link_type'			=> SQ_LINK_TYPE_2 | SQ_LINK_TYPE_1,
					),
				),
		);
	}//end getEventList()


}//end class
?>
