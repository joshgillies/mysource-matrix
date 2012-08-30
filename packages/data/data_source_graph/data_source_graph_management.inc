<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: data_source_graph_management.inc,v 1.13 2012/08/30 00:58:15 ewang Exp $
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
* @version $Revision: 1.13 $
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
						'mode'					=> Array(
													'added'			=> '0.1',
													'type'			=> 'selection',
													'default'		=> 'add',
													'is_admin'		=> 0,
													'parameters'	=> Array(
																		'options'		=> Array(
																							'add'	=> 'Add Plot',
																							'edit'	=> 'Edit Plot',
																						   ),
																		'multiple'		=> FALSE,
																		'allow_empty'	=> FALSE,
																		'allow_other'	=> FALSE,
																	   ),
												   ),
						'title'					=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> '',
													'description'	=> 'Some text to be associated with the image and can be accessed and printed out separately if required',
													'is_admin'		=> FALSE,
													'is_contextable'	=> TRUE,
												   ),
						'name'					=> Array(
													'added'		=> '0.1',
													'type'		=> 'text',
													'default'	=> '',
													'is_admin'	=> FALSE,
													'is_contextable'	=> TRUE,
												   ),
						'alt'					=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> '',
													'description'	=> 'The text to be used in the Alt attribute of the IMG tag',
													'is_admin'		=> FALSE,
													'is_contextable'	=> TRUE,
												   ),
						'caption'				=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> '',
													'description'	=> 'Some text to be associated with the image and can be accessed and printed out separately if required',
													'is_admin'		=> FALSE,
													'is_contextable'	=> TRUE,
												   ),
						'width'					=> Array(
													'added'			=> '0.1',
													'type'			=> 'int',
													'default'		=> 600,
													'description'	=> 'Width of the image (generated from the image file)',
													'is_admin'		=> FALSE,
												   ),
						'height'				=> Array(
													'added'			=> '0.1',
													'type'			=> 'int',
													'default'		=> 400,
													'description'	=> 'Height of the image (generated from the image file)',
													'is_admin'		=> FALSE,
												   ),
						'size'					=> Array(
													'added'			=> '0.1',
													'type'			=> 'int',
													'default'		=> 0,
													'description'	=> 'Size (in bytes) of the image (generated from the image file)',
													'is_admin'		=> FALSE,
												   ),
						'show_graph_title'		=> Array(
													'added'			=> '0.1',
													'type'			=> 'boolean',
													'default'		=> TRUE,
													'parameters'	=> Array(
																		'allow_empty'	=> FALSE,
																	   ),
												   ),
						'x_set_title'			=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> 'x-Axis',
													'description'	=> 'The text to be used in the x-Axis label',
													'is_admin'		=> FALSE,
													'is_contextable'	=> TRUE,
												   ),
						'y_set_title'			=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> 'y-Axis',
													'description'	=> 'The text to be used in the y-Axis label',
													'is_admin'		=> FALSE,
													'is_contextable'	=> TRUE,
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
						'graph_angle'			=> Array(
													'added'			=> '0.1',
													'type'			=> 'boolean',
													'default'		=> TRUE,
													'parameters'	=> Array(
																		'allow_empty'	=> FALSE,
																	   ),
												   ),
						'x_font_label_angle'	=> Array(
													'added'			=> '0.1',
													'type'			=> 'boolean',
													'default'		=> FALSE,
													'parameters'	=> Array(
																		'allow_empty'	=> FALSE,
																	   ),
												   ),
						'y_font_label_angle'	=> Array(
													'added'			=> '0.1',
													'type'			=> 'boolean',
													'default'		=> TRUE,
													'parameters'	=> Array(
																		'allow_empty'	=> FALSE,
																	   ),
												   ),
						'show_legend'			=> Array(
													'added'			=> '0.1',
													'type'			=> 'boolean',
													'default'		=> TRUE,
													'parameters'	=> Array(
																		'allow_empty'	=> FALSE,
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
						'axis_left_padding'		=> Array(
													'added'			=> '0.1',
													'type'			=> 'int',
													'default'		=> 0,
													'description'	=> 'Size (in px) of the padding on the left side of the axis',
													'is_admin'		=> FALSE,
												   ),
						'axis_right_padding'	=> Array(
													'added'			=> '0.1',
													'type'			=> 'int',
													'default'		=> 0,
													'description'	=> 'Size (in px) of the padding on the right side of the axis',
													'is_admin'		=> FALSE,
												   ),
						'axis_top_padding'		=> Array(
													'added'			=> '0.1',
													'type'			=> 'int',
													'default'		=> 0,
													'description'	=> 'Size (in px) of the padding on the top side of the axis',
													'is_admin'		=> FALSE,
												   ),
						'axis_bottom_padding'	=> Array(
													'added'			=> '0.1',
													'type'			=> 'int',
													'default'		=> 0,
													'description'	=> 'Size (in px) of the padding on the bottom side of the axis',
													'is_admin'		=> FALSE,
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
																	'bar_spacing'			=> '0',
																	'background'			=> 'ffffff',
																	'border_colour'			=> '000000',
																	'x_axis_colour'			=> '000000',
																	'y_axis_colour'			=> '000000',
																	'x_font_axis_colour'	=> '000000',
																	'y_font_axis_colour'	=> '000000',
																   ),
													'is_admin'	=> FALSE,
												   ),
						'datasets'				=> Array(
													'added'		=> '0.1',
													'type'		=> 'serialise',
													'default'	=> Array(),
													'is_admin'	=> FALSE,
												   ),
						'plot_order'			=> Array(
													'added'		=> '0.1',
													'type'		=> 'serialise',
													'default'	=> Array(),
													'is_admin'	=> FALSE,
												   ),
					  );

	}//end constructor


}//end class
?>
