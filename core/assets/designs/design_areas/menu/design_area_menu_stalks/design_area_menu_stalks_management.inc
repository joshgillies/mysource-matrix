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
* $Id: design_area_menu_stalks_management.inc,v 1.13 2007/12/06 04:28:50 lwright Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Area_Menu_Stalks_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.13 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Menu_Stalks_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);

		$this->vars = Array(
						'alignment'			=> Array(
												'added'			=> '0.0.1',
												'type'			=> 'text',
												'type'			=> 'selection',
												'default'		=> 'left',
												'parameters'	=> Array(
																	'options'		=> Array(
																						'left'	=> 'Left',
																						'right'	=> 'Right',
																					   ),
																	'multiple'		=> FALSE,
																	'allow_empty'	=> FALSE,
																   ),
												'description'	=> '',
											   ),
						'ellipsis_length'	=> Array(
												'added'			=> '0.0.1',
												'type'			=> 'int',
												'default'		=> 0,
												'description'	=> 'The length of a asset name before ellipsis are printed - blank or zero means full name always printed',
											   ),
						'word_wrap'			=> Array(
												'added'			=> '0.0.1',
												'type'			=> 'boolean',
												'default'		=> 1,
												'description'	=> 'Whether to allow the wrapping of the text in the entry or not',
												'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
											   ),
						'indent'			=> Array(
												'added'			=> '0.0.1',
												'type'			=> 'int',
												'default'		=> 9,
												'description'	=> 'How many pixels to indent the menu before printing out a level - blank or zero removes indenting',
											   ),
						'min_height'		=> Array(
												'added'			=> '0.0.1',
												'type'			=> 'int',
												'default'		=> 9,
												'description'	=> 'The minimum height for in each entry row. Controls the size of the stalk images and is used for centreing the stalks vertically with the contents of the &lt;MySource_ASSET&gt; tag',
											   ),
						'show_stalks'		=> Array(
												'added'			=> '0.0.1',
												'type'			=> 'boolean',
												'default'		=> 1,
												'description'	=> 'Show or hide the stalks - text still indents even if not shown',
												'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
											   ),
						'stalk_fg_colour'	=> Array(
												'added'			=> '0.0.1',
												'type'			=> 'colour',
												'default'		=> '#000000',
												'description'	=> 'The Foreground Colour for the stalks',
											   ),
						'stalk_bg_colour'	=> Array(
												'added'			=> '0.0.1',
												'type'			=> 'colour',
												'default'		=> '',
												'parameters'	=> Array('allow_blanks' => TRUE),
												'description'	=> 'The Background Colour for the stalks, if not set a transperant background will be used',
											   ),
					  );

	}//end constructor


}//end class
?>
