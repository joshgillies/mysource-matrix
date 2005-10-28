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
* $Id: data_source_graph_management.inc,v 1.3 2005/10/28 06:59:26 pgannavarapu Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Data_Source_Graph_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.3 $
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
						'name'				=> Array(
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> '',
											   ),
						'alt'				=> Array(
												'added'			=> '0.1',
												'type'			=> 'text',
												'default'		=> '',
												'description'	=> 'The text to be used in the Alt attribute of the IMG tag',
											   ),
						'title'				=> Array(
												'added'			=> '0.1',
												'type'			=> 'text',
												'default'		=> '',
												'description'	=> 'Some text to be associated with the image and can be accessed and printed out separately if required',
											   ),
						'width'				=> Array(
												'added'			=> '0.1',
												'type'			=> 'int',
												'default'		=> 400,
												'description'	=> 'Width of the image (generated from the image file)',
											   ),
						'height'			=> Array(
												'added'			=> '0.1',
												'type'			=> 'int',
												'default'		=> 400,
												'description'	=> 'Height of the image (generated from the image file)',
											   ),
						'graph_details'		=> Array(
												'added'		=> '0.1',
												'type'		=> 'serialise',
												'default'	=> Array(),
											   ),
						'x_axis_colour'		=> Array(
												'added'		=> '0.1',
												'type'		=> 'colour',
												'default'	=> '#000000',
											   ),
						'y_axis_colour'		=> Array(
												'added'		=> '0.1',
												'type'		=> 'colour',
												'default'	=> '#000000',
											   ),
						'x_font_colour'		=> Array(
												'added'		=> '0.1',
												'type'		=> 'colour',
												'default'	=> '#000000',
											   ),
						'y_font_colour'		=> Array(
												'added'		=> '0.1',
												'type'		=> 'colour',
												'default'	=> '#000000',
											   ),
						'line_colour'		=> Array(
												'added'		=> '0.1',
												'type'		=> 'colour',
												'default'	=> ' #000000',
											   ),
						'colour_array'		=> Array(
												'added'		=> '0.1',
												'type'		=> 'serialise',
												'default'	=> Array(),
											   ),
						'fill_colour'		=> Array(
												'added'			=> '0.1',
												'type'			=> 'colour',
												'default'		=> '',
												'parameters'	=> Array(
																	'allow_blanks'	=> true,
																   ),
											   ),
						'background_colour'	=> Array(
												'added'		=> '0.1',
												'type'		=> 'colour',
												'default'	=> '#FFFFFF',
											   ),
						'line_style'		=> Array(
												'added'			=> '0.1',
												'type'			=> 'selection',
												'parameters'	=> Array (
																	'options'		=> Array(
																						'Dotted'	=> 'Dotted',
																						'Dashed'	=> 'Dashed',
																						'Solid'		=> 'Solid',
																					   ),
																	'multiple'		=> false,
																	'allow_other'	=> false,
																	'allow_empty'	=> false,
																   ),
												'default'		=> 'Solid',
											   ),
						'value_marker'		=> Array(
												'added'			=> '0.1',
												'type'			=> 'selection',
												'parameters'	=> Array (
																	'options'		=> Array(
																						'IMAGE_GRAPH_VALUE_X'	=> 'Mark x-Axis Values',
																						'IMAGE_GRAPH_VALUE_Y'	=> 'Mark y-Axis Values',
																					   ),
																	'multiple'		=> false,
																	'allow_other'	=> false,
																	'allow_empty'	=> true,
																   ),
												'default'		=> '',
											   ),
						'general_marker'	=> Array(
												'added'			=> '0.1',
												'type'			=> 'selection',
												'parameters'	=> Array (
																	'options'		=> Array(
																						'Image_Graph_Marker_Star'		=> 'Mark Star For A Dataset',
																						'Image_Graph_Marker_Asterisk'	=> 'Mark Asterisk For A Dataset',
																						'Image_Graph_Marker_Box'		=> 'Mark Box For A Dataset',
																						'Image_Graph_Marker_Circle'		=> 'Mark Circle For A Dataset',
																						'Image_Graph_Marker_Cross'		=> 'Mark Cross For A Dataset',
																						'Image_Graph_Marker_Diamond'	=> 'Mark Diamond For A Dataset',
																						'Image_Graph_Marker_Triangle'	=> 'Mark Triangle For A Dataset',
																					   ),
																	'multiple'		=> true,
																	'allow_other'	=> false,
																	'allow_empty'	=> true,
																   ),
												'default'		=> '',
											   ),
						'marker_type'		=> Array(
												'added'			=> '0.1',
												'type'			=> 'selection',
												'parameters'	=> Array (
																	'options'		=> Array(
																						'Image_Graph_Marker_Pointing_Angular'	=> 'Marker that points away from the graph',
																						'Image_Graph_Marker_Pointing_Radial'	=> 'A pointing marker in a random angle from the data',
																					   ),
																	'multiple'		=> false,
																	'allow_other'	=> false,
																	'allow_empty'	=> false,
																   ),
												'default'		=> 'Image_Graph_Marker_Pointing_Angular',
											   ),
						'marker_background'	=> Array(
												'added'		=> '0.1',
												'type'		=> 'colour',
												'default'	=> '#FFFFFF',
											   ),
						'fill_gradient_min'	=> Array(
												'added'		=> '0.1',
												'type'		=> 'colour',
												'default'	=> '#FFFFFF',
											   ),
						'fill_gradient_max'	=> Array(
												'added'		=> '0.1',
												'type'		=> 'colour',
												'default'	=> '#FFFFFF',
											   ),
						'back_gradient_min'	=> Array(
												'added'		=> '0.1',
												'type'		=> 'colour',
												'default'	=> '#FFFFFF',
											   ),
						'back_gradient_max'	=> Array(
												'added'		=> '0.1',
												'type'		=> 'colour',
												'default'	=> '#FFFFFF',
											   ),
						'border_colour'		=> Array(
												'added'		=> '0.1',
												'type'		=> 'colour',
												'default'	=> '#FFFFFF',
											   ),
						'back_grad_type'	=> Array(
												'added'			=> '0.1',
												'type'			=> 'selection',
												'parameters'	=> Array (
																	'options'		=> Array(
																						'IMAGE_GRAPH_GRAD_VERTICAL'				=> 'Vertically',
																						'IMAGE_GRAPH_GRAD_HORIZONTAL'			=> 'Horizontally',
																						'IMAGE_GRAPH_GRAD_VERTICAL_MIRRORED'	=> 'Mirrored Vertically (the color grades from a-b-a vertically)',
																						'IMAGE_GRAPH_GRAD_HORIZONTAL_MIRRORED'	=> 'Mirrored Horizontally (the color grades from a-b-a horizontally)',
																						'IMAGE_GRAPH_GRAD_DIAGONALLY_TL_BR'		=> 'Diagonally from top-left to right-bottom',
																						'IMAGE_GRAPH_GRAD_DIAGONALLY_BL_TR'		=> 'Diagonally from bottom-left to top-right',
																						'IMAGE_GRAPH_GRAD_RADIAL'				=> 'Radially (concentric circles in the center)',
																					   ),
																	'multiple'		=> false,
																	'allow_other'	=> false,
																	'allow_empty'	=> true,
																   ),
												'default'		=> '',
											   ),
						'fill_grad_type'	=> Array(
												'added'			=> '0.1',
												'type'			=> 'selection',
												'parameters'	=> Array (
																	'options'		=> Array(
																						'IMAGE_GRAPH_GRAD_VERTICAL'				=> 'Vertically',
																						'IMAGE_GRAPH_GRAD_HORIZONTAL'			=> 'Horizontally',
																						'IMAGE_GRAPH_GRAD_VERTICAL_MIRRORED'	=> 'Mirrored Vertically (the color grades from a-b-a vertically)',
																						'IMAGE_GRAPH_GRAD_HORIZONTAL_MIRRORED'	=> 'Mirrored Horizontally (the color grades from a-b-a horizontally)',
																						'IMAGE_GRAPH_GRAD_DIAGONALLY_TL_BR'		=> 'Diagonally from top-left to right-bottom',
																						'IMAGE_GRAPH_GRAD_DIAGONALLY_BL_TR'		=> 'Diagonally from bottom-left to top-right',
																						'IMAGE_GRAPH_GRAD_RADIAL'				=> 'Radially (concentric circles in the center)',
																					   ),
																	'multiple'		=> false,
																	'allow_other'	=> false,
																	'allow_empty'	=> true,
																   ),
												'default'		=> '',
											   ),
						'marker_format'		=> Array(
												'added'			=> '0.1',
												'type'			=> 'selection',
												'parameters'	=> Array (
																	'options'		=> Array(
																						'Image_Graph_DataPreprocessor_Currency'			=> 'Currency',
																						'Image_Graph_DataPreprocessor_Date'				=> 'Date',
																						'Image_Graph_DataPreprocessor_NumberText'		=> 'Textual Representation of Numbers',
																						'Image_Graph_DataPreprocessor_RomanNumerals'	=> 'Roman Representation Of Numbers',
																						'Image_Graph_DataPreprocessor_Formatted'		=> 'Format data using a (s)printf pattern',
																					   ),
																	'multiple'		=> false,
																	'allow_other'	=> false,
																	'allow_empty'	=> true,
																   ),
												'default'		=> '',
											   ),
						'format_value'		=> Array(
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> '',
											   ),
						'x_set_title'		=> Array(
												'added'			=> '0.1',
												'type'			=> 'boolean',
												'default'		=> false,
												'parameters'	=> Array(
																	'allow_empty'	=> false,
																   ),
											   ),
						'y_set_title'		=> Array(
												'added'			=> '0.1',
												'type'			=> 'boolean',
												'default'		=> false,
												'parameters'	=> Array(
																	'allow_empty'	=> false,
																   ),
											   ),
						'x_set_interval'	=> Array(
												'added'			=> '0.1',
												'type'			=> 'int',
												'default'		=> 0,
												'description'	=> 'x-axis set label interval',
											   ),
						'y_set_interval'	=> Array(
												'added'			=> '0.1',
												'type'			=> 'int',
												'default'		=> 0,
												'description'	=> 'y-axis set label interval',
											   ),
					  );


	}//end constructor


}//end class
?>
