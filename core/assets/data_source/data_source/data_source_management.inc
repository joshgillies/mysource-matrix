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
* $Id: data_source_management.inc,v 1.9 2012/08/30 01:09:05 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Data_Source_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.9 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Data_Source_Management extends Asset_Management
{


	/**
	* Constructor
	*
	* @param Package_Manager	$pm	The package manager relevant to this particular asset
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);

		$this->vars = Array(
						'name'			=> Array(
											'added'			=> '0.1',
											'type'			=> 'text',
											'default'		=> '',
											'is_admin'		=> FALSE,
											'is_contextable'=> TRUE,
										   ),
						'dsn'			=> Array(
											'added'		=> '0.1',
											'type'		=> 'text',
											'default'	=> '',
										   ),
						'shadow_name'	=> Array(
											'added'		=> '0.1',
											'type'		=> 'text',
											'default'	=> '',
										   ),
						'dynamic_vars'		=> Array(
												'added'			=> '0.1',
												'type'			=> 'serialise',
												'default'		=> Array(),
												),
						'parameter_map'		=> Array(
												'added'			=> '0.1',
												'type'			=> 'parameter_map',
												'default'		=> Array(),
												'parameters'	=> Array(
																'parameters'	=> Array(),
															),
												),
						'filter_enabled' => Array (
											'added'			=> '0.1',
											'type'			=> 'boolean',
											'default'		=> FALSE,
											'parameters'    => Array('allow_empty'  => FALSE),
											'description'	=> 'Whether to run the filters',											
											),
						'record_filters' => Array (
											'added'			=> '0.1',
											'type'			=> 'serialise',
											'default'		=> Array(),
											'description'	=> 'Fields info for filtering the records in the datasource',
										   ),
						'filter_logic' => Array(
											'added'			=> '0.1',
											'type'			=> 'selection',
											'default'		=> 'AND',
											'description'	=> 'Whether to include all or any filter fields.',
											'parameters'	=> Array(
																'multiple'		=> FALSE,
																'allow_empty'	=> FALSE,
																'options'		=> Array(
																					'AND'	=> 'Match on ALL conditions',
																					'OR'	=> 'Match on at least ONE condition',
																				   ),
															   ),
											),
						'filter_mode' => Array(
											'added'			=> '0.1',
											'type'			=> 'selection',
											'default'		=> 'include',
											'description'	=> 'Whether to include all or any filter fields.',
											'parameters'	=> Array(
																'multiple'		=> FALSE,
																'allow_empty'	=> FALSE,
																'options'		=> Array(
																					'exclude'	=> 'Filter out the records matching the given conditions',
																					'include'	=> 'Return the records matching the given conditions',
																				   ),
															   ),
											),
						'cache_key' => Array (
											'added'			=> '0.1',
											'type'			=> 'text',
											'default'		=> '',
											'description'	=> 'Hash to unqiuely indentify the datasource results in cache',
										   ),
											
					  );

	}//end constructor


}//end class

?>
