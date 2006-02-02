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
* $Id: page_link_manager_management.inc,v 1.1 2006/02/02 06:49:02 rhoward Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Link_Manager_Management
*
* @author  David Heppell <dheppell@squiz.net>
* @author  Darren McKee <dmckee@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Link_Manager_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Page_Link_Manager_Management(&$pm)
	{
		$this->Asset_Management($pm);


		$this->vars = Array(
						'asset_selections'	=> Array(
												'added'			=> '0.3',
												'type'			=> 'serialise',
												'default'		=> Array(
																	'Linked'	=> Array(
																					'type'		=> 'radio',
																					'options'	=> Array(
																									'allow_multiple'	=> 1,
																									'allow_negative'	=> 0,
																									'restrict_range'	=> Array(
																															'enable'	=> 0,
																															'min'		=> 0,
																															'max'		=> 10,
																														   ),
																								   ),
																					'permanent'	=> 1,
																				   ),
																   ),
												'description'	=> 'Sets of named checkboxes, drop-down lists, etc',
											   ),
						'link_type'			=> Array(
												'added'			=> '0.2',
												'type'			=> 'selection',
												'default'		=> SQ_LINK_TYPE_1,
												'parameters'	=> Array(
																	'multiple'		=> FALSE,
																	'allow_empty'	=> FALSE,
																	'options'		=> Array(
																						SQ_LINK_TYPE_1	=> 'Type 1',
																						SQ_LINK_TYPE_2	=> 'Type 2',
																						SQ_LINK_NOTICE	=> 'Notice',
																					   ),
																   ),
												'description'	=> 'Type of links to create for new selections',
											   ),
						'managed_asset'		=> Array(
												'added'			=> '0.1',
												'type'			=> 'assetid',
												'default'		=> 0,
												'description'	=> 'The asset that the page will manage the links of',
											   ),
						'parameter_map'		=> Array(
												'added'			=> '0.4',
												'type'			=> 'parameter_map',
												'default'		=> Array(),
												'parameters'	=> Array(
																	'parameters'	=> Array(
																						'root_node'		=> 'Replacement Root node for the listing (must be a child of the static root node)',
																						'managed_asset'	=> 'Replacement Managed Asset for the listing',
																					   ),
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
