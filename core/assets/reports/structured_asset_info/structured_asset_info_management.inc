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
* $Id: structured_asset_info_management.inc,v 1.5 2012/08/30 01:09:09 ewang Exp $
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
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Structured_Asset_Info_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);

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
																							'designs'					=> 'Designs',
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
