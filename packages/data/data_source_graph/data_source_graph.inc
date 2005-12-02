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
* $Id: data_source_graph.inc,v 1.5 2005/12/02 03:36:41 skim Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Data_Source_Graph
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Data_Source_Graph extends Asset
{

	/**
	* All allowed extensions for an image
	* @var Array(string)
	*/
	var $allowed_extensions = Array( 'png');


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Data_Source_Graph($assetid=0)
	{
		$this->_ser_attrs = true;
		$this->Asset($assetid);

	}//end constructor


	/**
	* Called by the printBody to return the constant for the gradient type
	*
	* @param string	$grad_type	the type of gradient
	*
	* @return int
	* @access public
	*/
	function getGradType($grad_type)
	{
		switch ($grad_type) {
			case 'IMAGE_GRAPH_GRAD_VERTICAL':
				$grad_type = IMAGE_GRAPH_GRAD_VERTICAL;
			break;
			case 'IMAGE_GRAPH_GRAD_HORIZONTAL':
				$grad_type = IMAGE_GRAPH_GRAD_HORIZONTAL;
			break;
			case 'IMAGE_GRAPH_GRAD_VERTICAL_MIRRORED':
				$grad_type = IMAGE_GRAPH_GRAD_VERTICAL_MIRRORED;
			break;
			case 'IMAGE_GRAPH_GRAD_HORIZONTAL_MIRRORED':
				$grad_type = IMAGE_GRAPH_GRAD_HORIZONTAL_MIRRORED;
			break;
			case 'IMAGE_GRAPH_GRAD_DIAGONALLY_TL_BR':
				$grad_type = IMAGE_GRAPH_GRAD_DIAGONALLY_TL_BR;
			break;
			case 'IMAGE_GRAPH_GRAD_DIAGONALLY_BL_TR':
				$grad_type = IMAGE_GRAPH_GRAD_DIAGONALLY_BL_TR;
			break;
			case 'IMAGE_GRAPH_GRAD_RADIAL':
				$grad_type = IMAGE_GRAPH_GRAD_RADIAL;
			break;
		}
		return $grad_type;

	}//end getGradType()


	/**
	* Called by the design to print the body of this asset
	*
	* @return void
	* @access public
	*/
	function printBody()
	{
		$result = Array();
		$details = $this->attr('graph_details');
		$data_source = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, 'data_source', false, 'data_source_for_graph');
		if (!empty($data_source)) {
			$data_source_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($data_source['minorid']);
			$result = $data_source_asset->getResultSet();
		}
		if (!empty($result)) {
			if ($details['accumulative'] == 'Yes') {
				$sum = 0;
				$new_result = Array();
				foreach ($result as $element) {
					$sum += $element[$details['y_axis']];
					$element[$details['y_axis']] = $sum;
					$new_result[] = $element;
				}
				$result = $new_result;
			}
			$line = false;
			if (!empty($details)) {
				$type = $details['type'];
				require_once 'Image/Graph.php';

				// create the graph
				$width = $this->attr('width');
				$height = $this->attr('height');
				$graph =& Image_Graph::factory('graph', Array((int) $width, $height));

				$font =& $graph->addNew('font');
				// set the font size
				$font->setSize(12);
				$graph->setFont($font);

				$title = $this->attr('title');
				$graph->add(Image_Graph::vertical(
										Image_Graph::factory('title', Array(!empty($title)? $title: '', 15)),
										Image_Graph::vertical(
												$plotarea = Image_Graph::factory('plotarea'),
												$legend = Image_Graph::factory('legend'), 90), 5));
				$legend->setPlotarea($plotarea);

				//background gradient
				$grad_type = $this->attr('back_grad_type');
				if (!empty($grad_type)) {
					$grad1 = $this->attr('back_gradient_min');
					$grad2 = $this->attr('back_gradient_max');
					$gridY =& $plotarea->addNew('bar_grid', IMAGE_GRAPH_AXIS_Y);
					$grad_type = $this->getGradType($grad_type);
					$gridY->setFillStyle(Image_Graph::factory('gradient', Array($grad_type, $grad1, $grad2)));
				}
				//filling gradient
				$grad_type = $this->attr('fill_grad_type');
				if (!empty($grad_type)) {
					$grad1 = $this->attr('fill_gradient_min');
					$grad2 = $this->attr('fill_gradient_max');
					$grad_type = $this->getGradType($grad_type);
					$fill =& Image_Graph::factory('Image_Graph_Fill_Array');
					$fill->addNew('gradient', Array($grad_type, $grad1, $grad2));
				}

				//set background colour
				$background_colour = $this->attr('background_colour');
				$graph->setBackgroundColor($background_colour);

				//setting up x-axis colour and title
				$x_axis =& $plotarea->getAxis(IMAGE_GRAPH_AXIS_X);
				$color = $this->attr('x_axis_colour');
				$x_axis->setLineColor($color);

				//setting up font colour for x-axis
				$x_axis_font =& $graph->addNew('Image_Graph_Font');
				$color = $this->attr('x_font_colour');
				$x_axis_font->setSize(12);
				$x_axis_font->setColor($color);
				$x_axis->setFont($x_axis_font);
				$x_axis->setFontAngle('vertical');

				//setting up y-axis colour and title
				$y_axis =& $plotarea->getAxis(IMAGE_GRAPH_AXIS_Y);
				$color = $this->attr('y_axis_colour');
				$y_axis->setLineColor($color);

				//setting title for the axes
				$title = $this->attr('x_set_title');
				if ($title) {
					$x_axis->setTitle($details['x_axis'] != 'none'? $details['x_axis']: '', Array('size' => 13, 'angle' => 180));
				}
				$title = $this->attr('y_set_title');
				if ($title) {
					$y_axis->setTitle($details['y_axis'] != 'none'? $details['y_axis']: '', Array('size' => 13, 'angle' => 90));
				}

				//setting up font colour for y-axis
				$y_axis_font =& $graph->addNew('Image_Graph_Font');
				$color = $this->attr('y_font_colour');
				$y_axis_font->setSize(12);
				$y_axis_font->setColor($color);
				$y_axis->setFont($y_axis_font);

				//setting label intervals for axes
				$interval = $this->attr('x_set_interval');
				if (!empty($interval)) {
					$x_axis->setLabelInterval($interval);
				}
				$interval = $this->attr('y_set_interval');
				if (!empty($interval)) {
					$y_axis->setLabelInterval($interval);
				}

				$dataset =& Image_Graph::factory('dataset');
				$data = Array();
				foreach ($result as $record) {
					$co_ord = Array();
					$axes = Array ('x_axis', 'y_axis');
					foreach ($record as $key => $value) {
						foreach ($axes as $axis) {
							if ($key == $details[$axis]) $co_ord[$axis] = $value;
						}
					}
					foreach ($axes as $axis) {
						if ($details[$axis] == 'none') {
							unset($co_ord[$axis]);
							$line = true;
							$type = 'line';
						}
					}
					if ($line) {
						$type = 'line';
						$key = key($co_ord);
						$data[] = $co_ord[$key];
					} else {
						if (!empty($co_ord)) {
							$dataset->addPoint($co_ord['x_axis'], $co_ord['y_axis']);
						}
					}
				}
				//if both the dataset and the data are empty it means there is no numerical data to be graphed
				if (empty($dataset->_data) && empty($data)) {
					echo '<br />';
					echo translate('no_valid_dataset');
				} else {
					if ($line) {
						require_once 'Image/Graph/Dataset/Sequential.php';
						// create a sequential dataset for $data so that we could draw a line
						$dataset =& new Image_Graph_Dataset_Sequential($data);
					}

					//adding dataset to the graph
					$plot =& $plotarea->addNew($type, $dataset);
					if ($type == 'bar') $plot->setSpacing(30);

					//set line colour and fill colour
					$color = $this->attr('line_colour');
					$line_style = $this->attr('line_style');
					$style =& Image_Graph::factory('Image_Graph_Line_'.$line_style, Array($color, 'transparent'));
					$plot->setLineStyle($style);
					if (isset($fill)) $plot->setFillStyle($fill);

					//set fill colours
					$colors = $this->attr('colour_array');
					if (!empty($colors)) {
						$fillArray =& Image_Graph::factory('Image_Graph_Fill_Array');
						$plot->setFillStyle($fillArray);
						foreach ($colors as $color) {
							$fillArray->addColor($color);
						}
					}

					//create a marker
					$marker_array = null;
					$markers = $this->attr('general_marker');
					if (!empty($markers)) {
						$markers = explode('|', $markers);
						$marker_array = Image_Graph::factory('Image_Graph_Marker_Array');
						foreach ($markers as $marker) {
							$marker_obj =& Image_Graph::factory($marker);
							$marker_colour = $this->attr('marker_background');
							$marker_obj->setFillColor($marker_colour);
							$marker_array->add($marker_obj);
						}
					}

					// create a data value marker
					$marker = $this->attr('value_marker');
					if (!empty($marker)) {
						$marker_obj =& $plot->addNew('Image_Graph_Marker_Value', $marker);
						$marker_colour = $this->attr('marker_background');
						$marker_obj->setBackgroundColor($marker_colour);

						// create a pin-point marker type
						$marker_type = $this->attr('marker_type');
						$pointing_marker =& $plot->addNew($marker_type, Array(20, &$marker_obj));

						// add the marker to the marker array to finally set to the plot
						$marker_array->add($pointing_marker);

						//setting dataPreProcessor Format
						$marker_format = $this->attr('marker_format');
						$format_value = $this->attr('format_value');
						if (!empty($marker_format)) {
							$marker_obj->setDataPreProcessor(Image_Graph::factory($marker_format, !empty($format_value)? $format_value: null));
							$marker_obj->setFontSize(7);
						}
					}

					//set the marker to the plot
					if ($marker_array) $plot->setMarker($marker_array);

					//set border color for the graph
					$border = $this->attr('border_colour');
					$graph->setBorderColor($border);

					//finally graph is done
					$graph->done();
				}
			}
		} else {
			echo translate('no_results_for_graphing');
		}

	}//end printBody()


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the short name or the full name
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	function _getName($short_name=false)
	{
		return ($this->attr('name'));

	}//end _getName()


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* Pages add a default web path when they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	function _createAdditional(&$link)
	{
		if (!parent::_createAdditional($link)) return false;

		// set an initial web path
		$initial_path = strtolower($this->attr('name'));
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		$valid_paths = make_valid_web_paths(Array($initial_path));
		$good_paths = $GLOBALS['SQ_SYSTEM']->am->webPathsInUse($link['asset'], $valid_paths, $this->id, true);
		return $this->saveWebPaths($good_paths);

	}//end _createAdditional()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	* @see Asset::_getAllowLinks()
	*
	* @return array
	* @access private
	*/
	function _getAllowedLinks()
	{
		return Array(
				SQ_LINK_NOTICE	=> Array('data_source' => Array('card' => 1, 'exclusive' => false)),
			   );

	}//end _getAllowedLinks()


	/**
	* Return a human readable description of the passed link
	*
	* @param int	$linkid	the link ID of the link to describe
	*
	* @return string
	* @access public
	*/
	function describeLink($linkid)
	{
		$link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($linkid);
		switch (strtolower($link['value'])) {
			case 'data_source_for_graph' :
				return (translate('data_source_asset_linked_to_graph').' '.$this->id);
			break;
			default :
				return parent::describeLink($linkid);
			break;
		}

	}//end describeLink()


}//end class

?>
