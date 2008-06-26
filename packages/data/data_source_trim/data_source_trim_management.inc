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
* $Id: data_source_trim_management.inc,v 1.1.2.2 2008/06/26 04:34:05 hnguyen Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Data_Source_DB_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.1.2.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Data_Source_TRIM_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Data_Source_TRIM_Management(&$pm)
	{
		$this->Asset_Management($pm);
		$this->vars = Array(
						'wsdl'					=> Array (
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> '',
												   ),
						'use_downloader'		=> Array(
													'added'         => '0.1',
													'type'          => 'selection',
													'default'       => 'No',
													'description'	=> 'Whether to create the helper downloader asset',
													'is_admin'      => FALSE,
													'parameters'    => Array(
																		'allow_empty'   => FALSE,
																		'multiple'      => FALSE,
																		'options'		=> Array (
																							'Yes'	=> 'Yes',
																							'No'	=> 'No',
																						   ),
																	   ),
												   ),
                        'username'				=> Array(
		                                            'added'     => '0.1',
		                                            'type'      => 'text',
		                                            'default'   => '',
		                                           ),
                        'password'				=> Array(
		                                        	'added'     => '0.1',
		                                            'type'      => 'password',
		                                            'default'   => '',
		                                           ),
						'download_limit'			=> Array(
													'added'			=> '0.1',
													'type'			=> 'int',
													'default'		=> 2097152,
													'parameters'	=> Array (
																		'allow_empty'		=> TRUE,
																		'allow_negative'	=> FALSE,
																	   ),
												   ),
						'record_search_type'	=> Array (
											   		'added'			=> '0.1',
											   		'type'			=> 'serialise',
											   		'default'		=> Array(
												   						'StringSearch'	=> 'StringSearch',
												   					   ),
											   		'parameters'	=> Array (
												   						'allow_empty'		=> FALSE,
												   					   ),
											  	   ),

						'dynamic_vars'			=> Array(
													'added'		=> '0.2',
													'type'		=> 'serialise',
													'default'	=> Array (
																	'search_keyword'			=> 'doc',
																	'string_search_type'		=> Array (
																									'TitleWord'	=> 'TitleWord',
																								   ),
																	'search_start_time'			=> '',
																	'search_end_time'			=> '',
																	'search_date_range_type'	=> Array (
																									'DateUpdated'	=> 'DateUpdated',
																								   ),
																	'search_start_rec_num'		=> '',
																	'search_end_rec_num'		=> '',
																	'use_expanded_number'		=> FALSE,
																	'search_location_uri'		=> '',
																	'search_location_recursive'	=> FALSE,
																	'search_location_type'		=> Array (
																									'NotAtHome'	=> 'NotAtHome',
																								   ),
																	'fields'					=> Array (
																									'recTitle'	=> 'recTitle',
																									'recNumber'	=> 'recNumber',
																									'recRecordType'	=> 'recRecordType',
																									'recDocumentType' => 'recDocumentType',
																									'recCreatorLoc'	=> 'recCreatorLoc',
																								   ),
																	'fileTypes'					=> '',
																	'limit'						=> 30,
																	'search_sort_1'				=> 'None',
																	'search_sort_2'				=> 'None',
																	'search_sort_3'				=> 'None',
																	'sort_1_descending'			=> FALSE,
																	'sort_2_descending'			=> FALSE,
																	'sort_3_descending'			=> FALSE,
																   ),
												   ),
						'parameter_map'			=> Array (
													'added'			=> '0.1',
													'type'			=> 'parameter_map',
													'default'		=> Array(),
													'parameters'	=> Array (
																		'parameters'	=> Array(),
																	   ),
												   ),

						// This array allows TRIM data source to map the dynamic parameters with trim options
						// Eg: if you want to set a dynamic parameter to pass the keyword option in
						// with parameter name 'search_term', then trim_options['keyword'] need to be set to 'search_term'

					  );

	}//end constructor


}//end class

?>
