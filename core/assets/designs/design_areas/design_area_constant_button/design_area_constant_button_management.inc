<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: design_area_constant_button_management.inc,v 1.4 2003/09/26 05:26:24 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Area_Constant_Button_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Constant_Button_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Design_Area_Constant_Button_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
							'type_codes' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'serialise',
								'default'		=> Array(),
								'description'   => 'Comma or space separated list of type codes to restrict the menu to display, these types are not strict however so any sub-types of these will also be valid'
							),
							'image' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'text',
								'default'		=> '',
								'description'	=> 'Image to display for the button',
							),
							'image_rollover' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'text',
								'default'		=> '',
								'description'	=> 'Image to display when the button is rolled over',
							),
							'image_current' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'text',
								'default'		=> '',
								'description'	=> 'Image to display when the asset/url that this button is pointing to is the current page',
							),
							'image_blank' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'text',
								'default'		=> '',
								'description'	=> 'Image to display no asset or url have been selected',
							),
							'text' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'text',
								'default'		=> '',
								'description'	=> 'The text that forms the link if there is no image',
							),
							'class' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'text',
								'default'		=> '',
								'description'	=> 'CSS class that will be applied to the link and image tags',
							),
							'assetid' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'int',
								'default'		=> 0,
								'description'	=> 'The asset that the button is to link to'
							),
							'url' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'text',
								'default'		=> '',
								'description'	=> 'The URL that the button is to link to'
							),
							'target' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'text',
								'default'		=> '',
								'description'	=> 'The name of the target frame for the link',
							),
							'new_window' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'boolean',
								'default'		=> false,
								'description'	=> 'Whether to open in a new window or not',
							),
							'new_window_width' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'int',
								'default'		=> 0,
								'description'	=> 'Width of the new window to be created',
							),
							'new_window_height' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'int',
								'default'		=> 0,
								'description'	=> 'Width of the new window to be created',
							),
						);

	}//end constructor

}//end class
?>
