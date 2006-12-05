<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: design_area_menu_type_management.inc,v 1.22 2006/12/05 22:42:15 emcdonald Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Area_Menu_Type_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.22 $
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
