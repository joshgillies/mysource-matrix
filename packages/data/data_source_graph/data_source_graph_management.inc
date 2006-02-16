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
* $Id: data_source_graph_management.inc,v 1.4.2.1 2006/02/16 03:52:44 skim Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Data_Source_Graph_Management
*
* @see Asset_Management
*
* @author  Paratibha Gannavarapu <pgannavarapu@squiz.net>
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.4.2.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Data_Source_Graph_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Data_Source_Graph_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'title'					=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> '',
													'description'	=> 'Some text to be associated with the image and can be accessed and printed out separately if required',
													'is_admin'		=> false,
												   ),
						'name'					=> Array(
													'added'		=> '0.1',
													'type'		=> 'text',
													'default'	=> '',
													'is_admin'	=> false,
												   ),
						'alt'					=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> '',
													'description'	=> 'The text to be used in the Alt attribute of the IMG tag',
													'is_admin'		=> false,
												   ),
						'caption'				=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> '',
													'description'	=> 'Some text to be associated with the image and can be accessed and printed out separately if required',
													'is_admin'		=> false,
												   ),
						'width'					=> Array(
													'added'			=> '0.1',
													'type'			=> 'int',
													'default'		=> 600,
													'description'	=> 'Width of the image (generated from the image file)',
													'is_admin'		=> false,
												   ),
						'height'				=> Array(
													'added'			=> '0.1',
													'type'			=> 'int',
													'default'		=> 400,
													'description'	=> 'Height of the image (generated from the image file)',
													'is_admin'		=> false,
												   ),
						'size'					=> Array(
													'added'			=> '0.1',
													'type'			=> 'int',
													'default'		=> 0,
													'description'	=> 'Size (in bytes) of the image (generated from the image file)',
													'is_admin'		=> false,
												   ),
						'x_set_title'			=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> 'x-Axis',
													'description'	=> 'The text to be used in the x-Axis label',
													'is_admin'		=> false,
												   ),
						'y_set_title'			=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> 'y-Axis',
													'description'	=> 'The text to be used in the y-Axis label',
													'is_admin'		=> false,
												   ),
						'x_label_interval'		=> Array(
													'added'			=> '0.1',
													'type'			=> 'int',
													'default'		=> 0,
													'description'	=> 'x-axis set label interval',
												   ),
						'y_label_interval'		=> Array(
													'added'			=> '0.1',
													'type'			=> 'int',
													'default'		=> 0,
													'description'	=> 'y-axis set label interval',
												   ),
						'x_font_label_angle'	=> Array(
													'added'			=> '0.1',
													'type'			=> 'boolean',
													'default'		=> false,
													'parameters'	=> Array(
																		'allow_empty'	=> false,
																	   ),
												   ),
						'y_font_label_angle'	=> Array(
													'added'			=> '0.1',
													'type'			=> 'boolean',
													'default'		=> true,
													'parameters'	=> Array(
																		'allow_empty'	=> false,
																	   ),
												   ),
						'show_legend'			=> Array(
													'added'			=> '0.1',
													'type'			=> 'boolean',
													'default'		=> true,
													'parameters'	=> Array(
																		'allow_empty'	=> false,
																	   ),
												   ),
						'background'			=> Array(
													'added'		=> '0.1',
													'type'		=> 'colour',
													'default'	=> '#ffffff',
												   ),
						'border_colour'			=> Array(
													'added'		=> '0.1',
													'type'		=> 'colour',
													'default'	=> '#000000',
												   ),
						'x_axis_colour'			=> Array(
													'added'		=> '0.1',
													'type'		=> 'colour',
													'default'	=> '#000000',
												   ),
						'y_axis_colour'			=> Array(
													'added'		=> '0.1',
													'type'		=> 'colour',
													'default'	=> '#000000',
												   ),
						'x_font_axis_colour'	=> Array(
													'added'		=> '0.1',
													'type'		=> 'colour',
													'default'	=> '#000000',
												   ),
						'y_font_axis_colour'	=> Array(
													'added'		=> '0.1',
													'type'		=> 'colour',
													'default'	=> '#000000',
												   ),
						'global_settings'		=> Array(
													'added'		=> '0.1',
													'type'		=> 'serialise',
													'default'	=> Array(
																	'title'					=> 'Enter the title',
																	'alt'					=> 'Enter the alt tag',
																	'width'					=> '600',
																	'height'				=> '400',
																	'x_set_titles'			=> 'x-Axis',
																	'y_set_titles'			=> 'y-Axis',
																	'x_label_interval'		=> '1',
																	'y_label_interval'		=> '1',
																	'x_font_label_angle'	=> '90',
																	'y_font_label_angle'	=> '180',
																	'background'			=> 'ffffff',
																	'border_colour'			=> '000000',
																	'x_axis_colour'			=> '000000',
																	'y_axis_colour'			=> '000000',
																	'x_font_axis_colour'	=> '000000',
																	'y_font_axis_colour'	=> '000000',
																   ),
													'is_admin'	=> false,
												   ),
						'datasets'				=> Array(
													'added'		=> '0.1',
													'type'		=> 'serialise',
													'default'	=> Array(),
													'is_admin'	=> false,
												   ),
						'plot_order'			=> Array(
													'added'		=> '0.1',
													'type'		=> 'serialise',
													'default'	=> Array(),
													'is_admin'	=> false,
												   ),
					  );

	}//end constructor


}//end class
?>
