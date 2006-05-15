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
* $Id: structured_asset_info_management.inc,v 1.1 2006/05/15 06:13:31 emcdonald Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Structured_Asset_Info_Management
*
* @see Asset_Management
*
* @author  Elden McDonald <emcdonald@squiz.net>
* @author  Robert Howard <rhoward@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Structured_Asset_Info_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Structured_Asset_Info_Management(&$pm)
	{
		$this->Asset_Management($pm);

		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

		$this->vars = Array(
						'selected_asset_fields'	=> Array(
													'added'			=> '0.1',
													'type'			=> 'selection',
													'default'		=> '0',
													'description'	=> 'The asset at the head of the tree for which we will report details',
													'parameters'	=> Array(
																		'options'		=> Array(
																							'assetid'					=> 'Asset ID',
																							'status'					=> 'Status',
																							'asset_type'				=> 'Asset Type',
																							'name'						=> 'Name',
																							'created_date'				=> 'Created Date',
																							'created_user'				=> 'Created User',
																							'last_updated_date'			=> 'Last Updated Date',
																							'last_updated_user'			=> 'Last Updated User',
																							'last_published_date'		=> 'Last Published Date',
																							'last_published_user'		=> 'Last Published User',
																							'last_status_change_date'	=> 'Last Status Change Date',
																							'last_status_change_user'	=> 'Last Status Change User',
																							'urls'						=> 'URLs',
																							'metadata_schemas'			=> 'Metadata Schemas',
																							'workflow_schemas'			=> 'Workflow Schemas',
																						   ),
																		'multiple'		=> 'TRUE',
																		'allow_empty'	=> 'FALSE',
																		'allow_other'	=> 'FALSE',
																	   ),
												   ),
						'types'					=> Array(
													'added'			=> '0.1',
													'type'			=> 'serialise',
													'default'		=> Array(),
													'description'	=> 'The asset types to include in the report',
												   ),
					  );

	}//end constructor


}//end class

?>
