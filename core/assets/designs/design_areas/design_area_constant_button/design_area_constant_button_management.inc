<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: design_area_constant_button_management.inc,v 1.7 2003/11/12 18:02:27 brobertson Exp $
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
