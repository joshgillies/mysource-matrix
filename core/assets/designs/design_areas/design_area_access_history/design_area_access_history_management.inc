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
* $Id: design_area_access_history_management.inc,v 1.4 2006/06/28 00:20:13 sdanis Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Area_Access_History_Management
*
* @see Asset_Management
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Access_History_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Design_Area_Access_History_Management(&$pm)
	{
		$this->Asset_Management($pm);

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
					  );

	}//end constructor


}//end class
?>
