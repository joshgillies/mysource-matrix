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
* $Id: design_area_colourise_image_management.inc,v 1.15 2003/12/12 16:58:11 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Area_Colourise_Image_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Colourise_Image_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Design_Area_Colourise_Image_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
							'image' => Array(
								'added'     => '0.0.1',
								'type'      => 'text',
								'default'   => '',
								'description' => 'filename of the Associated File image that we are to colourise',
							),
							'colours' => Array(
								'added'       => '0.0.1',
								'type'        => 'serialise',
								'default'     => Array(),
								'description' => 'Associative Array of colours that are going to be used in the converison process',
							),
							'filename_only' => Array(
								'added'     => '0.0.1',
								'type'      => 'boolean',
								'default'   => 0,
								'description' => 'whether to print just the filename or the full img tag',
							),
							'type' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'selection',
								'default'		=> 'greyscale',
								'parameters'	=> Array(
									'options' => Array(
										'greyscale'			=> 'Greyscale', 
										'multiple_colours'	=> 'Multiple Colours',
									),
								),
							),
							'tolerance' => Array(
								'added'     => '0.0.1',
								'type'      => 'float',
								'default'   => 0.2,
							),
							'extra' => Array(
								'added'     => '0.0.1',
								'type'      => 'text',
								'default'   => '',
								'description' => 'Any extra HTML that you want to end up in the img tag',
							),
						);

	}//end constructor

}//end class
?>
