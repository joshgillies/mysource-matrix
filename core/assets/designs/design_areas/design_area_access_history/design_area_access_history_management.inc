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
* $Id: design_area_access_history_management.inc,v 1.8 2012/08/30 01:09:05 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Area_Access_History_Management
*
* @see Asset_Management
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.8 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Access_History_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);

		$this->vars = Array(
						'asset_types'					=> Array(
															'added'			=> '0.1',
															'type'			=> 'serialise',
															'default'		=> Array(),
															'description'	=> 'Asset types to track',
														   ),
						'asset_count'					=> Array(
															'added'			=> '0.1',
															'type'			=> 'int',
															'default'		=> '4',
															'description'	=> 'How many assets to remember',
															'parameters'	=> Array(
																				'allow_negative'	=> FALSE,
																			   ),
														   ),
						'asset_contents'				=> Array(
															'added'			=> '0.1',
															'type'			=> 'serialise',
															'default'		=> Array(),
															'description'	=> 'The contents that prints the asset info in the output',
														   ),
						'time_format'					=> Array(
															'added'			=> '0.1',
															'type'			=> 'text',
															'default'		=> 'G:i:s',
															'description'	=> 'format string as specified by date() PHP function',
														   ),
						'exclude_assets'				=> Array(
															'added'			=> '0.1',
															'type'			=> 'serialise',
															'default'		=> Array(),
															'description'	=> 'These assets are excluded from the asset history',
														   ),
						'auto_access_history'			=> Array(
															'added'			=> '0.2',
															'type'			=> 'boolean',
															'default'		=> TRUE,
															'description'	=> 'If enabled then assets are added to the access history automaticaly',
															'parameters'	=> Array( 'allow_empty' => FALSE,),
														   ),
						'add_history_button_label'		=> Array(
															'added'			=> '0.2',
															'type'			=> 'text',
															'default'		=> 'Add to Access History',
															'description'	=> 'Text that will be displayed on the Add to Access History button',
														   ),
						'remove_history_button_label'	=> Array(
															'added'			=> '0.2',
															'type'			=> 'text',
															'default'		=> 'Remove from Access History',
															'description'	=> 'Text that will be displayed on the Remove from Access History button',
														   ),
						'add_history_button_image'		=> Array(
															'added'			=> '0.2',
															'type'			=> 'boolean',
															'default'		=> FALSE,
															'description'	=> 'If enabled then image used instead of form button',
															'parameters'	=> Array( 'allow_empty' => FALSE,),
														   ),
						'remove_history_button_image'	=> Array(
															'added'			=> '0.2',
															'type'			=> 'boolean',
															'default'		=> FALSE,
															'description'	=> 'If enabled then image used instead of form button',
															'parameters'	=> Array( 'allow_empty' => FALSE,),
														   ),
						'add_history_asset_types'		=> Array(
															'added'			=> '0.2',
															'type'			=> 'serialise',
															'default'		=> Array(),
															'description'	=> 'Add to Access History button will be printed for these asset types only',
														   ),
						'remove_history_image'			=> Array(
															'added'			=> '0.2',
															'type'			=> 'assetid',
															'default'		=> '',
															'description'	=> 'Selected asset will be used as the Remove from History Button',
														   ),
						'remove_history_image_small'	=> Array(
															'added'			=> '0.2',
															'type'			=> 'assetid',
															'default'		=> '',
															'description'	=> 'Selected asset will be used as the Remove from History Image which is displayed for each asset',
														   ),
						'add_history_image'				=> Array(
															'added'			=> '0.2',
															'type'			=> 'assetid',
															'default'		=> '',
															'description'	=> 'Selected asset will be used as the Add to History Button',
														   ),
														   
						'store_permanent'			=> Array(
															'added'			=> '0.2',
															'type'			=> 'boolean',
															'default'		=> FALSE,
															'description'	=> 'If enabled then access history will be stored in database permanently for logged in users. This will decrease performance slightly.',
															'parameters'	=> Array( 'allow_empty' => FALSE,),
														   ),
					  );

	}//end constructor


}//end class
?>
