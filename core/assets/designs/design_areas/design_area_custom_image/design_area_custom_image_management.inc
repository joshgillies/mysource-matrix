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
* $Id: design_area_custom_image_management.inc,v 1.7 2003/12/12 18:45:43 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Area_Custom_Image_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Custom_Image_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Design_Area_Custom_Image_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
							'image' => Array(
								'added'     => '0.0.1',
								'type'      => 'text',
								'default'   => '',
							),
							'max_width' => Array(
								'added'     => '0.0.1',
								'type'      => 'int',
								'default'   => 0,
								'description' => 'The maximum width that the uploaded image can be',
							),
							'max_height' => Array(
								'added'     => '0.0.1',
								'type'      => 'int',
								'default'   => 0,
								'description' => 'The maximum height that the uploaded image can be',
							),
							'alt' => Array(
								'added'       => '0.0.1',
								'type'        => 'text',
								'default'     => '',
								'description' => 'The text that you want to appear in the alt attribute for this image'
							),
							'extra' => Array(
								'added'     => '0.0.1',
								'type'      => 'text',
								'default'   => '',
								'description' => 'Any extra HTML that you want to end up in the img tag',
							),
							'filename_only' => Array(
								'added'     => '0.0.1',
								'type'      => 'boolean',
								'default'   => 0,
								'description' => 'whether to print just the filename or the full img tag',
							),
						);

	}//end constructor

}//end class
?>
