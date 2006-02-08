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
* $Id: design_area_menu_type_management.inc,v 1.21 2006/02/08 02:52:24 sdanis Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Area_Menu_Type_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.21 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Menu_Type_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Design_Area_Menu_Type_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'level'					=> Array(
													'added'			=> '0.0.1',
													'type'			=> 'int',
													'default'		=> 0,
													'description'	=> 'The level in the heirarchy that we want to print',
												   ),
						'settings'				=> Array(
													'added'		=> '0.0.1',
													'type'		=> 'serialise',
													'default'	=> Array(),
												   ),
						'asset_contents'		=> Array(
													'added'			=> '0.0.1',
													'type'			=> 'serialise',
													'default'		=> Array(),
													'description'	=> 'The contents that prints the asset info in the output',
												   ),
						'divider_contents'		=> Array(
													'added'			=> '0.0.1',
													'type'			=> 'serialise',
													'default'		=> Array(),
													'description'	=> 'The contents that divides the assets in the output',
												   ),
						'prefix_with_divider'	=> Array(
													'added'			=> '0.0.1',
													'type'			=> 'boolean',
													'default'		=> FALSE,
													'description'	=> 'whether or not to start the list with a divider',
													'parameters'	=> Array(
																		'allow_empty'	=> FALSE,
																	   ),
												   ),
						'suffix_with_divider'	=> Array(
													'added'			=> '0.0.1',
													'type'			=> 'boolean',
													'default'		=> TRUE,
													'description'	=> 'whether or not to end the list with a divider',
													'parameters'	=> Array(
																		'allow_empty'	=> FALSE,
																	   ),
												   ),
						'show_subs'				=> Array(
													'added'			=> '0.0.1',
													'type'			=> 'selection',
													'default'		=> 'on_current',
													'parameters'	=> Array(
																		'options'		=> Array(
																							'on_current'	=> 'Only if in current lineage',
																							'always'		=> 'Always',
																						   ),
																		'multiple'		=> FALSE,
																		'allow_empty'	=> FALSE,
																	   ),
													'description'	=> 'When and how to display any sub menus that this menu might have',
												   ),
						'self_link'				=> Array(
													'added'			=> '0.0.1',
													'type'			=> 'boolean',
													'default'		=> TRUE,
													'description'	=> 'whether or not to add self-referential links',
													'parameters'	=> Array(
																		'allow_empty'	=> FALSE,
																	   ),
												   ),
					  );

	}//end constructor


}//end class
?>
