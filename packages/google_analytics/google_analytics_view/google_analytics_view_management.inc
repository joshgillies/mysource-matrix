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
* $Id: data_record_management.inc,v 1.5 2012/08/30 01:09:05 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Data_Record_Management
*
* @see Asset_Management
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Google_Analytics_View_Management extends Asset_Management
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
						'viewid' => Array(
									'added'			=> '0.0.1',
									'type'			=> 'text',
									'default'		=> '',
									'description'	=> 'The internal ID of the View',
									'update_fn'		=> '',
									'is_admin'		=> FALSE,
									'is_contextable'=> TRUE,
								   ),
						'name'	=> Array(
									'added'			=> '0.0.1',
									'type'			=> 'text',
									'default'		=> '',
									'description'	=> 'The name of the View',
									'update_fn'		=> '',
									'is_admin'		=> FALSE,
									'is_contextable'=> TRUE,
								   ),
						'refresh_period' => Array(
									'added'			=> '0.2',
									'type'			=> 'selection',
									'default'		=> '0',
									'description'	=> 'When the canned queries in this view are re-generated.',
									'update_fn'		=> '',
									'parameters'	=> Array(
										'multiple'		=> FALSE,
										'allow_empty'	=> FALSE,
										'options'		=> Array(
															'0'		=> translate('Do not update'),
															'14400'	=> translate('Every 4 hours'),
															'28800'	=> translate('Every 8 hours'),
															'86400' => translate('Every 24 hours'),
														   ),
									   ),
									'is_admin'		=> FALSE,
									'is_contextable'=> FALSE,
								   ),
						'last_generated' => Array(
									'added'			=> '0.2',
									'type'			=> 'int',
									'default'		=> NULL,
									'description'	=> 'When the canned queries in this view were last generated.',
									'update_fn'		=> '',
									'is_admin'		=> FALSE,
									'is_contextable'=> FALSE,
								   ),
					  );

	}//end constructor


}//end class
?>
