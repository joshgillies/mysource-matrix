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
* $Id: design_area_menu_stalks_management.inc,v 1.6 2003/11/26 00:50:57 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Area_Menu_Stalks_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Menu_Stalks_Management extends Asset_Management
{
	/**
	* Constructor
	*
	*/
	function Design_Area_Menu_Stalks_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
							'alignment' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'text',
								'type'			=> 'selection',
								'default'		=> 'left',
								'parameters'	=> Array(
									'options' => Array(
											'left'	=> 'Left',
											'right'	=> 'Right',
									),
								),
								'description'	=> ''
							),
							'ellipsis_length' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'int',
								'default'		=> 0,
								'description'	=> 'The length of a page name before ellipsis are printed - blank or zero means full name always printed'
							),
							'word_wrap' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'boolean',
								'default'		=> 1,
								'description'	=> 'Whether to allow the wrapping of the text in the entry or not'
							),
							'indent' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'int',
								'default'		=> 9,
								'description'	=> 'How many pixels to indent the menu before printing out a level - blank or zero removes indenting'
							),
							'min_height' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'int',
								'default'		=> 9,
								'description'	=> 'The minimum height for in each entry row, used for centreing '
							),
							'show_stalks' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'boolean',
								'default'		=> 1,
								'description'	=> 'Show or hide the stalks - text still indents even if not shown'
							),
							'stalk_fg_colour' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'colour',
								'default'		=> '#000000',
								'description'	=> 'The Foreground Colour for the stalks'
							),
							'stalk_bg_colour' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'colour',
								'default'		=> '',
								'parameters'	=> Array('allow_blanks' => true),
								'description'	=> 'The Background Colour for the stalks, if not set a transperant background will be used'
							)
						);

	}//end constructor

}//end class
?>
