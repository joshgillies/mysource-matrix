<?php

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Area_Menu_Recursive_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve::packages::__core__
*/
class Design_Area_Menu_Recursive_Management extends Asset_Management
{
	/**
	* Constructor
	*
	*/
	function Design_Area_Menu_Recursive_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
							'min_num_levels' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'int',
								'default'		=> 0,
								'description'	=> 'The minimum number of levels into the hierarchy to show'
							),
							'max_num_levels' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'int',
								'default'		=> 0,
								'description'	=> 'The maximum number of levels into the hierarchy to show, blank or zero means no maximum'
							),
							'alignment' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'text',
								'default'		=> 'left',
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
								'default'		=> '000000',
								'description'	=> 'The Foreground Colour for the stalks'
							),
							'stalk_bg_colour' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'colour',
								'default'		=> '',
								'description'	=> 'The Background Colour for the stalks, if not set a transperant background will be used'
							)
						);

	}//end constructor

}//end class
?>
