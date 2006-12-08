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
* $Id: data_source_db_management.inc,v 1.9 2006/12/08 00:51:59 emcdonald Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Data_Source_DB_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.9 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Data_Source_DB_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Data_Source_DB_Management(&$pm)
	{
		$this->Asset_Management($pm);
		$this->vars = Array(
						'database'		=> Array(
											'added'		=> '0.1',
											'type'		=> 'text',
											'default'	=> '',
										   ),
						'host'			=> Array(
											'added'		=> '0.1',
											'type'		=> 'text',
											'default'	=> '',
										   ),
						'user'			=> Array(
											'added'		=> '0.1',
											'type'		=> 'text',
											'default'	=> '',
										   ),
						'password'		=> Array(
											'added'		=> '0.1',
											'type'		=> 'password',
											'default'	=> '',
										   ),
						'db_type'		=> Array(
											'added'			=> '0.1',
											'type'			=> 'selection',
											'parameters'	=> Array (
																'options'		=> Array(
																					'postgres'	=> 'Postgres',
																					'oracle'	=> 'Oracle',
																					'mysql'		=> 'My SQL',
																				   ),
																'multiple'		=> FALSE,
																'allow_other'	=> FALSE,
																'allow_empty'	=> FALSE,
															   ),
											'default'		=> 'postgres',
										   ),
						'sql'			=> Array(
											'added'		=> '0.1',
											'type'		=> 'text',
											'default'	=> '',
										   ),
						'where'			=> Array(
											'added'		=> '0.1',
											'type'		=> 'serialise',
											'default'	=> Array(),
										   ),
						'group_by'		=> Array(
											'added'		=> '0.1',
											'type'		=> 'serialise',
											'default'	=> Array(),
										   ),
						'order_by'		=> Array(
											'added'		=> '0.1',
											'type'		=> 'serialise',
											'default'	=> Array(),
										   ),
						'having'		=> Array(
											'added'		=> '0.1',
											'type'		=> 'serialise',
											'default'	=> Array(),
										   ),
						'tables'		=> Array(
											'added'		=> '0.1',
											'type'		=> 'text',
											'default'	=> '',
										   ),
						'fields'		=> Array(
											'added'		=> '0.1',
											'type'		=> 'text',
											'default'	=> '',
										   ),
						'custom_fields'	=> Array(
											'added'		=> '0.1',
											'type'		=> 'serialise',
											'default'	=> Array(),
										   ),
						'aggregates'	=> Array(
											'added'		=> '0.1',
											'type'		=> 'serialise',
											'default'	=> Array(),
										   ),
						'join'			=> Array(
											'added'		=> '0.1',
											'type'		=> 'serialise',
											'default'	=> Array(),
										   ),
						'mode'			=> Array(
											'added'			=> '0.1',
											'type'			=> 'selection',
											'parameters'	=> Array (
																'options'		=> Array(
																					'builder'	=> 'Query Builder',
																					'edit'		=> 'Edit Manually',
																					'execute'	=> 'Execute',
																				   ),
																'multiple'		=> FALSE,
																'allow_other'	=> FALSE,
																'allow_empty'	=> FALSE,
															   ),
											'default'		=> 'builder',
										   ),
						'edit_on'		=> Array(
											'added'		=> '0.1',
											'type'		=> 'boolean',
											'default'	=> FALSE,
										   ),
						'reset_edit'	=> Array(
											'added'		=> '0.1',
											'type'		=> 'boolean',
											'default'	=> FALSE,
										   ),
						'limit'			=> Array(
											'added'			=> '0.1',
											'type'			=> 'int',
											'default'		=> '',
											'parameters'	=> Array(
																'allow_empty'		=> TRUE,
																'allow_negative'	=> FALSE,
															   ),
										   ),
					  );

	}//end constructor


}//end class

?>
