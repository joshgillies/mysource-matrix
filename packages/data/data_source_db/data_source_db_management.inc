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
* $Id: data_source_db_management.inc,v 1.5.2.1 2006/02/24 01:06:03 arailean Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Data_Source_DB_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.5.2.1 $
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
																'multiple'		=> false,
																'allow_other'	=> false,
																'allow_empty'	=> false,
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
																					'execute'	=> 'Execute Mode',
																					'edit'		=> 'Edit Manually',
																				   ),
																'multiple'		=> false,
																'allow_other'	=> false,
																'allow_empty'	=> false,
															   ),
											'default'		=> 'builder',
										   ),
						'edit_on'		=> Array(
											'added'		=> '0.1',
											'type'		=> 'boolean',
											'default'	=> false,
										   ),
						'reset_edit'		=> Array(
											'added'		=> '0.1',
											'type'		=> 'boolean',
											'default'	=> false,
										   ),
						'limit'			=> Array(
											'added'			=> '0.1',
											'type'			=> 'int',
											'default'		=> '',
											'parameters'	=> Array(
																'allow_empty'		=> true,
																'allow_negative'	=> false,
															   ),
										   ),
					  );

	}//end constructor


}//end class

?>
