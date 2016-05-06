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
* $Id: page_whats_new_management.inc,v 1.21 2012/08/30 00:58:09 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Whats_New_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.21 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Whats_New_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(&$pm)
	{
		parent::__construct($pm);

		$this->vars = Array(
						'date_field'				=> Array(
														'added'			=> '0.1',
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
													   ),
						'range_type'				=> Array(
														'added'			=> '0.1',
														'type'			=> 'text',
														'default'		=> 'default',
													   ),
						'date_range_from'			=> Array(
														'added'			=> '0.1',
														'type'			=> 'datetime',
														'default'		=> '---------- --:--:--',
														'parameters'	=> Array(
																			'allow_null'	=> TRUE,
																		   ),
													   ),
						'date_range_to'				=> Array(
														'added'			=> '0.1',
														'type'			=> 'datetime',
														'default'		=> '---------- --:--:--',
														'parameters'	=> Array(
																			'allow_null'	=> TRUE,
																		   ),
													   ),
						'date_ranges'				=> Array(
														'added'			=> '0.1',
														'type'			=> 'serialise',
														'default'		=> Array(),
													   ),
						'link_value_filter'			=> Array(
														'added'			=> '0.1',
														'type'			=> 'text',
														'default'		=> '',
													   ),
						'link_value_filter_enabled' => Array(
														'added'			=> '0.1',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																	  	   ),
													   ),
						'link_value_filter_logic'	=> Array(
														'added'			=> '0.16',
														'type'			=> 'boolean',
														'default'		=> TRUE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
												  		),
						'parameter_map' 			=> Array(
														'added'			=> '0.15',
														'type'			=> 'parameter_map',
														'default'		=> Array(),
														'parameters'	=> Array(
																			'parameters'	=> Array(
																								'root_node'			=> 'Replacement Root node for the listing (must be a child of the static root node)',
																								'asset_selection'	=> 'Array of specific asset IDs or selection group (specified assets must be children of the static related node)',
																							   ),
																		   ),
														),
						'dynamic_asset_selection_option' => Array(
															'added'			=> '0.16',
															'type'			=> 'selection',
															'default'		=> 'empty_result',
															'parameters'	=> Array(
																				'multiple'		=> FALSE,
																				'allow_empty'	=> FALSE,
																				'options'		=> Array(
																									'static_root'		=> 'Use static root node',
																									'empty_result'		=> 'Return empty result',
																								   ),
																			   ),														   
														   ),
					  );

	}//end constructor


	/**
	* Return an array with information about the events that the what new asset is listening for
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
