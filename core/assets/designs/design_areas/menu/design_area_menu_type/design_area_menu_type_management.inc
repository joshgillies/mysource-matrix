<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: design_area_menu_type_management.inc,v 1.10 2003/09/26 05:26:25 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Area_Menu_Type_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
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
							'level' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'int',
								'default'		=> 0
							),
							'settings' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'serialise',
								'default'		=> Array()
							),
							'page_contents' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'serialise',
								'default'		=> Array(),
								'description'   => 'The contents that prints the page info in the output',
							),
							'divider_contents' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'serialise',
								'default'		=> Array(),
								'description'   => 'The contents that divides the pages in the output',
							),
							'prefix_with_divider' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'boolean',
								'default'		=> false,
								'description'	=> 'whether or not to start the list with a divider'
							),
							'suffix_with_divider' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'boolean',
								'default'		=> true,
								'description'	=> 'whether or not to end the list with a divider'
							),
							'show_subs' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'selection',
								'type'			=> 'selection',
								'default'		=> 'on_current',
								'parameters'	=> Array(
									'options' => Array(
											'on_current'	=> 'Only if in current lineage',
											'always'		=> 'Always',
									),
								),
								'description'   => 'When and how to display any sub menus that this menu might have'
							),
						);

	}//end constructor


}//end class
?>
