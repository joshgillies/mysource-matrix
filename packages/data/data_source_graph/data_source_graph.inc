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
* $Id: data_source_graph.inc,v 1.37.6.1 2013/08/06 05:01:43 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/files/image/image.inc';

/**
* Data_Source_Graph
*
* Purpose
*
* @author  Paratibha Gannavarapu <pgannavarapu@squiz.net>
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.37.6.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Data_Source_Graph extends Image
{

	/**
	* All allowed extensions for an image
	* @var array(string)
	*/
	var $allowed_extensions = Array( 'png');

	/**
	* Reference to the plotarea of the current graph
	* @var object
	*/
	var $plotarea = NULL;


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Data_Source_Graph($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		$this->Image($assetid);

	}//end constructor


	/**
	* Create this asset
	*
	* Unlike Image asset, we need to privode a dummy graphic file
	* during the creation
	*
	* @param array	&$link	information used to create the initial link
	* @param array	$info	info about the uploaded file
	*
	* @return mixed int|boolean
	* @access public
	* @see Asset::create()
	*/
	function create(&$link, $info=Array())
	{
		// make a dummy png file first
		$this->makeGlobalSettingsPreview(TRUE);

		// fill $info array
		$name = trim(strtolower(str_replace(' ', '_', $this->attr('title'))));
		$info = Array(
					'name'		=> $name.'.png',
					'type'		=> 'image/png',
					'path'		=> SQ_DATA_PATH.'/temp/'.$name.'.png',
					'filename'	=> $name.'.png',
				);
		$this->_tmp['file_create_data'] = &$info;

		if ($name == '') {
			trigger_localised_error('CORE0083', E_USER_WARNING, $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name'));
			return FALSE;
		}
		if (!$this->validFile($info)) return FALSE;

		// check that we are not going to have web path conflicts
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		$valid_names = make_valid_web_paths(Array($info['name']));
		$name = array_shift($valid_names);

		// make sure the new web path is not already is use
		$bad_paths = $GLOBALS['SQ_SYSTEM']->am->webPathsInUse($link['asset'], Array($name));
		if (!empty($bad_paths)) {
			trigger_localised_error('CORE0086', E_USER_WARNING, $name);
			return FALSE;
		}
		$this->setAttrValue('name', $name);

		if (Asset::create($link)) {
			return unlink($info['path']);
		} else {
			unlink($info['path']);
			return FALSE;
		}

	}//end create()


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* Graph image file is copied to the data directory WITHOUT registering
	* the file versioning
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	function _createAdditional(&$link)
	{
		// assetid will not be set if we are creating
		if (!$this->id) return TRUE;

		// make the info array look like a result from getExistingFile()
		$info = $this->_tmp['file_create_data'];

		if (!$this->validFile($info)) return FALSE;

		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		$name = current(make_valid_web_paths(Array(strtolower($info['name']))));

		// make sure the new web path is not already is use
		if ($this->name != $name) {
			$parents = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_SC_LINK_SIGNIFICANT, '', NULL, 'minor');
			for ($i = 0; $i < count($parents); $i++) {
				$parent = $GLOBALS['SQ_SYSTEM']->am->getAsset($parents[$i]['majorid'], $parents[$i]['major_type_code']);
				if (is_null($parent)) continue;
				$bad_paths = $GLOBALS['SQ_SYSTEM']->am->webPathsInUse($parent, Array($name), $this->id);
				if (!empty($bad_paths)) {
					trigger_localised_error('CORE0055', E_USER_WARNING);
					return FALSE;
				}
			}
		}

		// create the destination directory, if necessary (NOTE that checking is internal for this)
		create_directory($this->data_path);

		// copy over the new uploaded file
		if (!copy($info['path'], $this->data_path.'/'.$name)) {
			trigger_localised_error('CORE0036', E_USER_WARNING, $info['name'], $this->data_path.'/'.$name);
			return FALSE;
		}

		// set the graph file name to pass done() function
		$this->_tmp['graph_file_name'] = Array('filename' => $this->data_path.'/'.$name);

		$existing = $this->getExistingFile();

		// set the name of the file to the newly uploaded one
		$this->_tmp['uploading_file'] = FALSE;
		$name_changed = $this->setAttrValue('name', $name);
		if (!$this->saveAttributes()) {
			trigger_localised_error('CORE0052', E_USER_WARNING);
			return FALSE;
		}
		$asset->_tmp['uploading_file'] = FALSE;

		if (!$this->saveWebPaths(Array($name))) {
			trigger_localised_error('CORE0054', E_USER_WARNING);
			return FALSE;
		}

		$em = $GLOBALS['SQ_SYSTEM']->getEventManager();
		$em->broadcastEvent($this, 'ContentsUpdated');

		// Now we can call getExistingFile() function
		$info = $this->getExistingFile();
		if (!empty($info)) {
			if (!$this->setAttrValue('width', $info['width'])) {
				return FALSE;
			}
			if (!$this->setAttrValue('height', $info['height'])) {
				return FALSE;
			}
			if (!$this->setAttrValue('size', $info['size'])) {
				return FALSE;
			}
		}

		$GLOBALS['SQ_SYSTEM']->am->acquireLock($this->id, 'all');
		if (!$this->saveAttributes()) {
			trigger_localised_error('CORE0052', E_USER_WARNING);
			return FALSE;
		}
		$GLOBALS['SQ_SYSTEM']->am->releaseLock($this->id, 'all');

		return TRUE;

	}//end _createAdditional()


	/**
	* Called by the design to print the body of this asset
	*
	* @return void
	* @access public
	*/
	function printBody()
	{
		$this->printImageTag();

	}//end printBody()


	/**
	* Prints the tag for an image
	*
	* The output includes the width and height of the image, and the image
	* name for the alt tag
	*
	* @param string		$alt	the alt tag content. If empty the alt var is used
	* @param string		$title	the alt tag content. If empty the and $alt is set, then that is used, otherwise the title var is used
	* @param boolean	$return	if TRUE the tag will be returned
	*
	* @return mixed string|void
	* @access public
	*/
	function printImageTag($alt='', $title='', $return=FALSE)
	{
		if ($title == '') {
			$title = ($alt != '') ? $alt : $this->attr('title');
		}
		if ($alt == '') $alt = $this->attr('alt');

		$tag = '<img src="'.$this->getURL().'" width="'.$this->attr('width').'" height="'.$this->attr('height').'" alt="'.$alt.'" title="'.$title.'" />';
		if ($return) return $tag;
		echo $tag;

	}//end printImageTag()


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


	/**
	* Prints out the backend interface for the asset
	*
	* We need to override this function to enable backend
	* graph preview functionality
	*
	* @param object	&$o	A reference to the backend outputter
	*
	* @return boolean
	* @access public
	*/
	function paintBackend(&$o)
	{
		if (!isset($_REQUEST['preview_graph'])) {
			return parent::paintBackend($o);
		}

		switch ($_REQUEST['asset_ei_screen']) {

			case 'global_settings' :
				// if count(datasets) >= 1, update the img file here
				$datasets = $this->attr('datasets');
				if (count($datasets) > 0 && $this->isGraphReady()) {
					$this->getFinalGraph(TRUE);
				}

				// global setting preview
				$this->makeGlobalSettingsPreview(FALSE);
				exit();

			case 'datasets' :
				// update the img file here
				$this->getFinalGraph(TRUE);

				// single plot preview
				if (isset($_REQUEST['final_preview'])) {
					$this->getFinalGraph();
				} else {
					// graph info is passed through $_POST var after serialization
					// we need to pass them to makePlotSettingsPreview() function
					$graph_info = unserialize(urldecode($_REQUEST['graph_info']));
					$this->makePlotSettingsPreview($graph_info);
				}
				exit();

			default :
				return parent::paintBackend($o);

		}//end switch

	}//end paintBackend()


//--        GRAPHS        --//


	/**
	* Returns a reference to a new graph object
	*
	* User must call this function to create a new graph. All the
	* global settings are applied at here during the creation
	*
	* @return object
	* @access public
	*/
	function &getNewGraph()
	{
		require_once 'Image/Graph.php';

		$width	= $this->attr('width');
		$height	= $this->attr('height');
		$graph	= Image_Graph::factory('graph', Array((int) $width, (int) $height));
		$title = $this->attr('show_graph_title') ? $this->attr('title') : '';
		if ($this->attr('show_legend')) {
			$graph_ratio = 90;
		} else {
			$graph_ratio = 100;
		}
		if ($this->attr('graph_angle')) {
			$graph->add(
				Image_Graph::vertical(
					Image_Graph::factory('title', Array($title, 12)),
					Image_Graph::vertical(
						$plotarea = Image_Graph::factory('plotarea'),
						$legend = Image_Graph::factory('legend'),
						$graph_ratio
					),
					5
				)
			);
		} else if (!$this->attr('graph_angle')) {
			$param = Array(
						'Image_Graph_Axis_Category',
						'Image_Graph_Axis',
						'horizontal',
					 );
			$graph->add(
				Image_Graph::vertical(
					Image_Graph::factory('title', Array($title, 12)),
					Image_Graph::vertical(
						$plotarea = Image_Graph::factory('plotarea', $param),
						$legend = Image_Graph::factory('legend'),
						$graph_ratio - 1
						),
					5
				)
			);
		}
		$plotarea->setAxisPadding(
			Array(
				'left'		=> (int) $this->attr('axis_left_padding'),
				'right'		=> (int) $this->attr('axis_right_padding'),
				'top'		=> (int) $this->attr('axis_top_padding'),
				'bottom'	=> (int) $this->attr('axis_bottom_padding'),
			)
		);
		$legend->setPlotarea($plotarea);

		// Background Color
		$graph->setBackgroundColor($this->attr('background'));

		// Border Color
		$graph->setBorderColor($this->attr('border_colour'));

		$x_axis_title_angle = ($this->attr('x_font_label_angle') ? 90 : 0);
		$y_axis_title_angle = ($this->attr('y_font_label_angle') ? 90 : 0);

		$x_axis =& $plotarea->getAxis(IMAGE_GRAPH_AXIS_X);
		$x_axis->setTitle($this->attr('x_set_title'), Array('size' => 13, 'angle' => $x_axis_title_angle));
		$x_axis->setLabelInterval($this->attr('x_label_interval'));

		$y_axis =& $plotarea->getAxis(IMAGE_GRAPH_AXIS_Y);
		$y_axis->setTitle($this->attr('y_set_title'), Array('size' => 13, 'angle' => $y_axis_title_angle));
		$y_axis->setLabelInterval($this->attr('y_label_interval'));

		// setting up x-axis colour and title
		$x_axis->setLineColor($this->attr('x_axis_colour'));

		// setting up font colour for x-axis
		$x_axis_font =& $graph->addNew('Image_Graph_Font');
		$x_axis_font->setSize(12);
		$x_axis_font->setColor($this->attr('x_font_axis_colour'));
		$x_axis->setFont($x_axis_font);
		$x_axis->setFontAngle($this->attr('x_font_label_angle') ? 'vertical' : 'horizontal');

		// setting up y-axis colour and title
		$y_axis->setLineColor($this->attr('y_axis_colour'));

		// setting up font colour for y-axis
		$y_axis_font =& $graph->addNew('Image_Graph_Font');
		$y_axis_font->setSize(12);
		$y_axis_font->setColor($this->attr('y_font_axis_colour'));
		$y_axis->setFont($y_axis_font);
		$y_axis->setFontAngle($this->attr('y_font_label_angle') ? 'vertical' : 'horizontal');

		// set the reference to plotarea
		$this->plotarea =& $plotarea;

		return $graph;

	}//end getNewGraph()


	/**
	* Finish a graph
	*
	* User must call this function to create a new graph. All the
	* global settings are applied at here during the creation
	*
	* @param boolean	$to_file	If it's TRUE, it saves a image as a file in the data directory
	*								instead of outputting a binary file with HTTP header information
	*
	* @return object
	* @access public
	*/
	function getFinalGraph($to_file=FALSE)
	{
		// Turn off error level. PEAR Image package generates
		// PHP 4 warning, which prevents printFroend() sending
		// proper header information
		$old_err_level = error_reporting(0);

		$graph		= $this->getNewGraph();
		$datasets	= $this->attr('datasets');

		$plot_order	= $this->attr('plot_order');
		foreach ($plot_order as $key => $plot_name) {
			$dataset = $datasets[$plot_name];

			$x_axis_name = $dataset['settings']['x_axis'];
			$y_axis_name = $dataset['settings']['y_axis'];
			$is_accumulative = intval($dataset['settings']['accumulative']);

			// dataset processing
			$data_source_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($dataset['assetid']);
			$result = $data_source_asset->getResultSet('default');

			$tmp = 0;
			$image_dataset = Image_Graph::factory('dataset');
			foreach ($result as $record) {
				if ($is_accumulative) {
					$tmp += intval($record[$y_axis_name]);
					$image_dataset->addPoint($record[$x_axis_name], $tmp);
				} else {
					$image_dataset->addPoint($record[$x_axis_name], intval($record[$y_axis_name]));
				}
			}

			  ////////////////////////////
			 // Plot specific settings //
			////////////////////////////
			switch ($dataset['settings']['graph_type']) {
				case 'Image_Graph_Plot_Bar' :
				case 'Image_Graph_Plot_Area' :
					// use stacked bar
					if ($dataset['settings']['use_stacked']) {
						$multiple_dataset = Array();
						$multiple_dataset[] = $image_dataset;

						foreach ($dataset['settings']['stacked_data'] as $stack) {
							$image_dataset = Image_Graph::factory('dataset');
							$image_dataset->setName($stack['title']);

							// add each stack dataset
							$data_source_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($stack['datasource']);
							$result = $data_source_asset->getResultSet('default');
							foreach ($result as $record) {
								$image_dataset->addPoint($record[$stack['x_axis']], intval($record[$stack['y_axis']]));
							}
							$multiple_dataset[] =$image_dataset;

						}
						$plot =& $this->plotarea->addNew($dataset['settings']['graph_type'], Array($multiple_dataset, 'stacked'));
					} else {
						$plot =& $this->plotarea->addNew($dataset['settings']['graph_type'], $image_dataset);
					}
				break;
				case 'Image_Graph_Plot_Smoothed_Area' :
				case 'Image_Graph_Plot_Line' :
				case 'Image_Graph_Plot_Smoothed_Line' :
				case 'Image_Graph_Plot_Pie' :
				case 'Image_Graph_Plot_Step' :
				case 'Image_Graph_Plot_Impulse' :
				case 'Image_Graph_Plot_Dot' :
				case 'Image_Graph_Plot_Radar' :
				case 'Image_Graph_Plot_CandleStick' :
				case 'Image_Graph_Plot_Band' :
				default :
					$plot =& $this->plotarea->addNew($dataset['settings']['graph_type'], $image_dataset);
				break;

			}//end switch

			if ($dataset['settings']['plot_name'] != $dataset['settings']['display_name']) {
				$plot->setTitle($dataset['settings']['display_name']);
			} else {
				$plot->setTitle($dataset['settings']['plot_name']);
			}

			$fill = Image_Graph::factory('Image_Graph_Fill_Array');
			if ($dataset['settings']['gradient_type'] != 'none') {
				// gradient colour
				$grad_type = $this->getGradType($dataset['settings']['gradient_type']);
				$fill->addNew('gradient', Array($grad_type, '#'.$dataset['settings']['gradient_1'], '#'.$dataset['settings']['gradient_2']));
			} else {
				// fill colour
				$fill->addColor('#'.$dataset['settings']['fill_colour']);
			}

			switch ($dataset['settings']['graph_type']) {
				case 'Image_Graph_Plot_Bar' :
					$plot->setSpacing(intval($dataset['settings']['bar_spacing']));
				case 'Image_Graph_Plot_Area' :
					// use stacked bar
					if ($dataset['settings']['use_stacked']) {
						foreach ($dataset['settings']['stacked_data'] as $stack) {
							if ($stack['gradient_type'] != 'none') {
								$fill->addNew('gradient', Array($this->getGradType($stack['gradient_type']), '#'.$stack['colour_1'], '#'.$stack['colour_2']));
							} else {
								$fill->addColor('#'.$stack['colour_1']);
							}
						}
					}
					$plot->setFillStyle($fill);
					$plot->setLineColor('#'.$dataset['settings']['line_colour']);
				break;
				case 'Image_Graph_Plot_Line' :
				case 'Image_Graph_Plot_Smoothed_Area' :
				case 'Image_Graph_Plot_Smoothed_Line' :
				case 'Image_Graph_Plot_Pie' :
				case 'Image_Graph_Plot_Step' :
				case 'Image_Graph_Plot_Impulse' :
				case 'Image_Graph_Plot_Dot' :
				case 'Image_Graph_Plot_Radar' :
				case 'Image_Graph_Plot_CandleStick' :
				case 'Image_Graph_Plot_Band' :
				default :
					$plot->setFillStyle($fill);
					$plot->setLineColor('#'.$dataset['settings']['line_colour']);
				break;

			}//end switch

			// marker
			if (isset($dataset['settings']['marker_type']) && $dataset['settings']['marker_type'] != 'none') {
				$this->_addMarkerToPlot($dataset, $plot);
			}

		}//end foreach

		if ($to_file) {
			if (empty($this->_tmp['graph_file_name'])) {
				// mainly for the dummy file during the asset creation
				$filename = $this->data_path.'/'.$this->name;
				$graph->done(Array('filename' => $filename));
			} else {
				$graph->done(Array('filename' => $this->_tmp['graph_file_name']));
			}

			if ($this->attr('variety_count') > 0) {
				// Update varieties
				$GLOBALS['SQ_SYSTEM']->setRunLevel($GLOBALS['SQ_SYSTEM']->getRunLevel() & SQ_RUN_LEVEL_FORCED);
				$am = $GLOBALS['SQ_SYSTEM']->am;
				$links = $am->getLinks($this->id, SQ_LINK_TYPE_2, 'image_variety');
				foreach ($links as $link) {
					$variety = $am->getAsset($link['minorid'], 'image_variety');
					if (!$this->saveVariety($variety, TRUE, TRUE, FALSE, FALSE)) {
						trigger_error('Failed to update the image varieties', E_USER_WARNING);
					}
				}
				$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
			}
		} else {
			$graph->done();
		}

		// Restore error level
		error_reporting($old_err_level);

	}//end getFinalGraph()


	/**
	* Returns TRUE if the graph is ready to draw
	*
	* @return boolean
	* @access public
	*/
	function isGraphReady()
	{
		$datasets		= $this->attr('datasets');
		$graph_ready	= TRUE;
		foreach ($datasets as $name => $dataset) {
			if ($dataset['settings']['x_axis'] == 'Not saved' || $dataset['settings']['y_axis'] == 'Not saved') {
				$graph_ready = FALSE;
				break;
			}
		}
		return $graph_ready;

	}//end isGraphReady()


	/**
	* Returns the preview graph to show the current plot setting
	*
	* @param array	$graph_info	The array contains the setting information
	*							for this plot.
	*
	* @return boolean
	* @access public
	*/
	function makePlotSettingsPreview($graph_info)
	{
		// Turn off error level. PEAR Image package generates
		// PHP 4 warning, which prevents printFroend() sending
		// proper header information
		$old_err_level = error_reporting(0);

		require_once 'Image/Graph.php';
		$global_settings = $this->attr('global_settings');

		$graph =& $this->getNewGraph();
		$sample_dataset = Image_Graph::factory('random', Array(10, 2, 15, TRUE));

		// plot settings
		$plot =& $this->plotarea->addNew($graph_info['settings']['graph_type'], Array(&$sample_dataset));
		if ($graph_info['settings']['gradient_type'] != 'none') {
			// gradient colour
			$grad_type = $this->getGradType($graph_info['settings']['gradient_type']);
			$fill = Image_Graph::factory('Image_Graph_Fill_Array');
			$fill->addNew('gradient', Array($grad_type, '#'.$graph_info['settings']['gradient_1'], '#'.$graph_info['settings']['gradient_2']));
		} else {
			// fill colour
			$fill = Image_Graph::factory('Image_Graph_Fill_Array');
			$fill->addColor('#'.$graph_info['settings']['fill_colour']);
		}
		$plot->setFillStyle($fill);
		$plot->setLineColor('#'.$graph_info['settings']['line_colour']);

		  ////////////////////////////
		 // Plot specific settings //
		////////////////////////////
		switch ($graph_info['settings']['graph_type']) {
			case 'Image_Graph_Plot_Bar' :
				$plot->setSpacing(intval($graph_info['settings']['bar_spacing']));
			break;
			case 'Image_Graph_Plot_Line' :
			case 'Image_Graph_Plot_Area' :
			case 'Image_Graph_Plot_Smoothed_Line' :
			case 'Image_Graph_Plot_Smoothed_Area' :
			case 'Image_Graph_Plot_Pie' :
			case 'Image_Graph_Plot_Step' :
			case 'Image_Graph_Plot_Impulse' :
			case 'Image_Graph_Plot_Dot' :
			case 'Image_Graph_Plot_Radar' :
			case 'Image_Graph_Plot_CandleStick' :
			case 'Image_Graph_Plot_Band' :
			break;
			default :
			break;
		}

		// marker settings
		if (isset($graph_info['settings']['value_marker']) && $graph_info['settings']['value_marker'] != 'none') {
			$this->_addMarkerToPlot($graph_info, $plot);
		}

		$graph->done();

		// Restore error level
		error_reporting($old_err_level);

	}//end makePlotSettingsPreview()


	/**
	* Returns the preview graph to show the current plot setting
	*
	* @param boolean	$to_file	If it's TRUE, it saves a image as a file in the data directory
	*								instead of outputting a binary file with HTTP header information
	*								In this function, it's mainly used to generate a dummy graph image
	*								during the creation of this asset
	*
	* @return boolean
	* @access public
	*/
	function makeGlobalSettingsPreview($to_file=FALSE)
	{
		// Turn off error level. PEAR Image package generates
		// PHP 4 warning, which prevents printFroend() sending
		// proper header information
		$old_err_level = error_reporting(0);

		$graph = $this->getNewGraph();
		$sample_dataset = Image_Graph::factory('random', Array(10, 2, 15, TRUE));

		$plot = $this->plotarea->addNew('Image_Graph_Plot_Bar', Array(&$sample_dataset));

		if ($to_file) {
			if (empty($this->_tmp['graph_file_name'])) {
				// mainly for the dummy file during the asset creation
				$filename = SQ_DATA_PATH.'/temp/'.strtolower(str_replace(' ', '_', $this->attr('title'))).'.png';
				$graph->done(Array('filename' => $filename));
			} else {
				$graph->done($this->_tmp['graph_file_name']);
			}
		} else {
			$graph->done();
		}

		// Restore error level
		error_reporting($old_err_level);

	}//end makeGlobalSettingsPreview()


	/**
	* Add a marker to the plot
	*
	* @param array	&$marker_info	The array contains the setting information
	* @param array	&$plot			The reference to the plot object which we are
	*								drawing
	*
	* @return boolean
	* @access public
	*/
	function _addMarkerToPlot(&$marker_info, &$plot)
	{
		$marker_array = NULL;
		if ($marker_info['settings']['marker_type'] != 'none') {
			$marker_type = $marker_info['settings']['marker_type'];
			$marker_value_type = $marker_info['settings']['value_marker_type'];

			if ($marker_value_type == 'IMAGE_GRAPH_VALUE_X') {
				$new_marker =& $plot->addNew($marker_type, IMAGE_GRAPH_VALUE_X);
			} else if ($marker_value_type == 'IMAGE_GRAPH_VALUE_Y') {
				$new_marker =& $plot->addNew($marker_type, IMAGE_GRAPH_VALUE_Y);
			}

			$new_marker->setFontColor('#'.$marker_info['settings']['marker_font_colour']);
			$new_marker->setLineColor('#'.$marker_info['settings']['marker_line_colour']);
			$new_marker->setFillColor('#'.$marker_info['settings']['marker_fill_colour']);

			$pointing_marker = NULL;
			if ($marker_info['settings']['pointer_type'] != 'none') {
				$pointer_type = $marker_info['settings']['pointer_type'];
				$pointer_length = $marker_info['settings']['pointer_length'];
				$pointing_marker =& $plot->addNew($pointer_type, Array((int) $pointer_length, &$new_marker));
			}

			if (is_null($pointing_marker)) {
				// set the marker without the pointer
				$plot->setMarker($new_marker);
			} else {
				// set the marker with the pointer
				$plot->setMarker($pointing_marker);
			}
		}

	}//end _addMarkerToPlot()


//--        FRONTEND CACHING        --//


	/**
	* Prints out the frontend for this file with no design
	*
	* @return void
	* @access public
	* @see Data_Source_Graph::usePublicPath()
	* @see Data_Source_Graph::updateCache()
	*/
	function printFrontend()
	{
		// if we are in limbo, we want to show our editing interface and design
		// instead of just sending our file over
		if (SQ_IN_LIMBO) return parent::printFrontend();

		if (!$this->readAccess()) {
			$GLOBALS['SQ_SYSTEM']->paintLogin(translate('login'), translate('cannot_access_asset', $this->name));
			return;
		}

		// check the cache. If there is no cache or it's expired,
		// re-generate the image file
		$cm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');
		$result = $cm->loadFromCache($this->id, $this->type(), 'default');

		if ($result === FALSE) $this->updateCache();

		$existing = $this->getExistingFile();
		if (!empty($existing)) {
			require_once SQ_FUDGE_PATH.'/standards_lists/mime_types.inc';
			require_once SQ_FUDGE_PATH.'/general/file_system.inc';

			$ext = get_file_type($existing['filename']);
			$type = (empty($standards_lists_mime_types[$ext])) ? 'text/plain' : $standards_lists_mime_types[$ext];
			header('Cache-Control:');
			header('Pragma: cache');
			header('Expires: '.gmdate('D, d M Y H:i:s', (time() + 86400)).' GMT');
			header('Content-Type: '.$type);
			header('Content-Disposition: inline; filename="'.$existing['filename'].'";');
			header('Last-Modified: '.gmdate('D, d M Y H:i:s',$existing['modified']).' GMT');
			header('Content-Length: '.$existing['size']);
			readfile($existing['path']);
			@ob_flush();
		}

	}//end printFrontend()


	/**
	* Update system cache for this asset
	*
	* It checks the current cache status and updates it with
	* the up-to-date graph image
	*
	* @return boolean
	* @access public
	* @see Data_Source_Graph::printFrontend()
	* @see Data_Source_Graph::usePublicPath()
	*/
	function updateCache()
	{
		$cm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');

		// regenerate the image
		$this->getFinalGraph(TRUE);

		// read the file
		$content = file_get_contents($this->data_path.'/'.$this->name);
		if ($content === FALSE) {
			trigger_localised_error('DATA0003', E_USER_WARNING, $this->name);
			return FALSE;
		}
		// cache dummy string, will be used later to check for expiry time of this image file
		$result = $cm->saveToCache($this->id, $this->type(), 'default', 'Dummy String');

		return $result;

	}//end updateCache()


	/**
	* Returns whether to use the public path or not for this file
	*
	* NOTE: This function is overriden here since we want to cache
	* the image file for the frontend display. If the public path
	* name is returned to serve the image then, there is no way to
	* inform the Cache_Manager when to update its cache. Hence it should
	* always return matrix-friendly web path, and the logic for caching can be
	* implemented in printFrontend
	*
	* @return boolean
	* @access public
	* @see Data_Source_Graph::printFrontend()
	* @see Data_Source_Graph::updateCache()
	* @see File::updateLookups()
	*/
	function usePublicPath()
	{
		return FALSE;

	}//end usePublicPath()


//--        HELPER        --//


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


//--        NO FILE VERSIONING        --//


	///////////////////////////////////////////////////////
	// The below functions are overriden in order to
	// bypass file versioning functionality in File asset
	///////////////////////////////////////////////////////


	/**
	* Saves a copy of this asset at this point in time
	* This copy gets used when the asset is live and the user does not have write access
	*
	* @return boolean
	* @access public
	*/
	function saveSystemVersion()
	{
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';

		// make sure our data directory exists
		if (!create_directory($this->data_path)) {
			trigger_localised_error('CORE0049', E_USER_WARNING, $this->name);
			return FALSE;
		}

		// make sure our system directory exists
		if (!create_directory($this->data_path.'/.sq_system')) {
			trigger_localised_error('CORE0050', E_USER_WARNING, $this->name);
			return FALSE;
		}

		// make sure there is nothing in the system directory
		if (!clear_directory($this->data_path.'/.sq_system')) {
			trigger_localised_error('CORE0046', E_USER_WARNING, $this->name);
			return FALSE;
		}

		// save the object for later (in the restricted directory)
		if (!string_to_file(serialize($this), $this->data_path.'/.sq_system/.object_data')) {
			trigger_localised_error('CORE0051', E_USER_WARNING, $this->name);
			return FALSE;
		}

		// update our lookup tables to not use __data
		if (!parent::updateLookups()) {
			trigger_localised_error('CORE0053', E_USER_WARNING, $this->name);
			return FALSE;
		}

		return TRUE;

	}//end saveSystemVersion()


	/**
	* Replaces the editing version with the system (live) version
	*
	* @return boolean
	* @access public
	*/
	function revertToSystemVersion()
	{
		// get the safe edit file name before it's lost
		$se_name = $this->attr('name');

		// pretend we are uploading a file because we may be changing the name of the file
		// and we want to fool setAttributeValue
		$this->_tmp['uploading_file'] = TRUE;
		if (!Asset::revertToSystemVersion()) {
			$this->_tmp['uploading_file'] = FALSE;
			return FALSE;
		}
		$this->_tmp['uploading_file'] = FALSE;

		return TRUE;

	}//end revertToSystemVersion()


	/**
	* Morph's this asset to either one of it's parent type or one of it's children types
	* Only succeeds if $type_code asset's is allowed to link to all children that this asset has
	*
	* @param string	$new_type_code	the new type_code to attempt to cast this asset to
	*
	* @return boolean
	* @access public
	*/
	function morph($new_type_code)
	{
		$am = $GLOBALS['SQ_SYSTEM']->am;

		$file_types = array_merge($am->getTypeAncestors($this->type(), FALSE), $am->getTypeDescendants($this->type()));

		if (in_array($new_type_code, $file_types)) {
			// we are morphing into another file type, so check that the actual
			// file currently uploaded is not going to conflict with the new type
			$am->includeAsset($new_type_code);
			$tmp = new $new_type_code();
			$existing = $this->getExistingFile();
			if (!empty($existing)) {
				if (!$tmp->validFile($existing)) {
					trigger_localised_error('CORE0008', E_USER_WARNING, $this->name, $new_type_code, $existing['filename'], $new_type_code);
					return FALSE;
				}

			}
			unset($tmp);
		}

		return Asset::morph($new_type_code);

	}//end morph()


	/**
	* Look's after the placing and removing of out files in the public directory
	*
	* @return boolean
	* @access private
	*/
	function _checkFileState()
	{
		if ($this->status & SQ_SC_STATUS_SAFE_EDITING) return TRUE; //don't touch public file in safe edit
		
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';

		$existing = $this->getExistingFile();
		if (empty($existing)) return TRUE;

		$unrestricted = $this->data_path_public.'/'.$existing['filename'];

		$changed = FALSE;

		// if we should be storing our file in the public
		// data directory for all the world to see
		if ($this->effectiveUnrestricted() && $this->attr('allow_unrestricted')) {
			// make sure our data directory exists
			if (!create_directory($this->data_path_public)) {
				trigger_localised_error('CORE0049', E_USER_WARNING, $this->name);
				return FALSE;
			}

			if (!file_exists($unrestricted)) {
				// copy the file to the public data directory
				if (!copy($this->data_path.'/'.$this->name, $unrestricted)) {
					trigger_localised_error('DATA0004', E_USER_WARNING, $this->name);
				} else {
					$changed = TRUE;
				}
			}
		} else {
			// need to make sure our file is in the NOT in the public data directory
			if (is_file($unrestricted)) {
				// unlink the file
				if (!unlink($unrestricted)) {
					trigger_localised_error('DATA0004', E_USER_WARNING, $this->name);
				} else {
					$changed = TRUE;
				}
			}
		}//end if effectiveUnrestricted

		if ($changed) {
			return $this->updateLookups();
		} else {
			return TRUE;
		}

	}//end _checkFileState()


	/**
	* Set the value of an attribute
	*
	* @param boolean	$dont_run_updated	if TRUE this->_updated() won't be run
	*										(THIS IS ONLY EVER CALLED FROM Asset::create())
	*
	* @return boolean
	* @access public
	*/
	function saveAttributes($dont_run_updated=FALSE)
	{
		if (!$this->id) return TRUE;

		// if there are no values for the title attribute use the filename
		if (trim($this->attr('title')) == '') {
			$this->setAttrValue('title', $this->attr('name'));
		}//end if

		if (isset($this->_tmp['vars_set']['name'])) {

			$uploading_file = !empty($this->_tmp['uploading_file']);

			if (!$uploading_file) {

				$old_filename = $this->_tmp['vars_set']['name']['old_value'];
				$new_filename = $this->attr('name');

				// try and set the new filename - return FALSE if it fails
				if (!parent::saveAttributes($dont_run_updated)) {
					return FALSE;
				}

				if (is_file($this->data_path.'/'.$old_filename)) {
					if (!$this->_renameFile($this->data_path, $old_filename, $new_filename)) {
						parent::setAttrValue('name', $old_filename);
						parent::saveAttributes($dont_run_updated);
						return FALSE;
					}
				}

				if ($this->effectiveUnrestricted()) {
					if (is_file($this->data_path_public.'/'.$old_filename)) {
						copy($this->data_path_public.'/'.$old_filename, $this->data_path_public.'/'.$new_filename);
						unlink($this->data_path_public.'/'.$old_filename);
					}
				}

				// update the lookups so that the __data path gets rewritten if applicable
				$this->updateLookups();

				return TRUE;

			}//end if not uploading a file

		}//end if we are changing the filename

		if (isset($this->_tmp['vars_set']['allow_unrestricted'])) {
			// the unrestricted setting is being changed, so we may need to update lookups
			if ($this->effectiveUnrestricted() && !($this->status & SQ_SC_STATUS_SAFE_EDITING)) {
				$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
				$vars = Array('assetids' => Array($this->id));
				$hh->queueHipo('hipo_job_update_lookups', $vars);
			}
		}

		return parent::saveAttributes($dont_run_updated);

	}//end saveAttributes()


	/**
	* Rename a file in file versioning (copy, create new FV entry and then delete old FV entry)
	*
	* @param string	$data_path		data path of the asset
	* @param string	$old_filename	old filename
	* @param string	$new_filename	file to rename to
	*
	* @return boolean
	* @access private
	*/
	function _renameFile($data_path, $old_filename, $new_filename)
	{
		$fv = $GLOBALS['SQ_SYSTEM']->getFileVersioning();

		// copy the file instead of a direct rename because removing the
		// file from the repository will also remove it from the file system
		if (!copy($data_path.'/'.$old_filename, $data_path.'/'.$new_filename)) {
			trigger_localised_error('CORE0026', E_USER_WARNING);
			return FALSE;
		}

		if (!unlink($data_path.'/'.$old_filename)) {
			trigger_localised_error('DATA0004', E_USER_WARNING, $this->name);
			return FALSE;
		}

		return TRUE;

	}//end _renameFile()


	/**
	* Get an array of some useful data about the existing uploaded file
	*
	* @return array
	* @access public
	*/
	function getExistingFile()
	{
		$info = Array();
		$path = $this->data_path.'/'.$this->attr('name');

		if (is_file($path)) {
			$info['path'] = $path;
			$info['filename'] = basename($info['path']);
			$info['modified'] = filemtime($info['path']);
			$info['size'] = filesize($info['path']);
		}

		if (!empty($info['path'])) {
			$size = getimagesize($info['path']);
			$info['width']  = $size[0];
			$info['height'] = $size[1];
		}

		return $info;

	}//end getExistingFile()


}//end class

?>
