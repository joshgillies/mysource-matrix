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
* $Id: data_source_graph_edit_fns.inc,v 1.23 2012/08/30 00:58:15 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/files/image/image_edit_fns.inc';

/**
* Data_Source_Graph_Edit_Fns
*
* Purpose
*
* @author  Paratibha Gannavarapu <pgannavarapu@squiz.net>
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.23 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Data_Source_Graph_Edit_Fns extends Image_Edit_Fns
{

	/**
	* Option list for the type of graph
	* @var array
	*/
	public $graph_types = Array(
						'Image_Graph_Plot_Bar',
						'Image_Graph_Plot_Line',
						'Image_Graph_Plot_Area',
						'Image_Graph_Plot_Smoothed_Line',
						'Image_Graph_Plot_Smoothed_Area',
						'Image_Graph_Plot_Pie',
						'Image_Graph_Plot_Step',
						'Image_Graph_Plot_Impulse',
						'Image_Graph_Plot_Dot',
						'Image_Graph_Plot_Radar',
						'Image_Graph_Plot_CandleStick',
						'Image_Graph_Plot_Band',
					   );

	/**
	* Option list for gradient types
	* @var array
	*/
	public $fill_grad_types = Array(
							'none'									=> '-- Leave Empty --',
							'IMAGE_GRAPH_GRAD_VERTICAL'				=> 'Vertically',
							'IMAGE_GRAPH_GRAD_HORIZONTAL'			=> 'Horizontally',
							'IMAGE_GRAPH_GRAD_VERTICAL_MIRRORED'	=> 'Mirrored Vertically (the color grades from a-b-a vertically)',
							'IMAGE_GRAPH_GRAD_HORIZONTAL_MIRRORED'	=> 'Mirrored Horizontally (the color grades from a-b-a horizontally)',
							'IMAGE_GRAPH_GRAD_DIAGONALLY_TL_BR'		=> 'Diagonally from top-left to right-bottom',
							'IMAGE_GRAPH_GRAD_DIAGONALLY_BL_TR'		=> 'Diagonally from bottom-left to top-right',
							'IMAGE_GRAPH_GRAD_RADIAL'				=> 'Radially (concentric circles in the center)',
						   );


	/**
	* Option list for mark types
	* @var array
	*/
	public $marker_types = Array(
							'none'							=> '-- Leave Empty --',
							'Image_Graph_Marker_Asterisk'	=> 'Mark Asterisk For A Dataset',
							'Image_Graph_Marker_Average'	=> 'Mark Average For A Dataset',
							'Image_Graph_Marker_Box'		=> 'Mark Box For A Dataset',
							'Image_Graph_Marker_Cross'		=> 'Mark Cross For A Dataset',
							'Image_Graph_Marker_Diamond'	=> 'Mark Diamond For A Dataset',
							'Image_Graph_Marker_Plus'		=> 'Mark Plus For A Dataset',
							'Image_Graph_Marker_Star'		=> 'Mark Star For A Dataset',
							'Image_Graph_Marker_Triangle'	=> 'Mark Triangle For A Dataset',
							'Image_Graph_Marker_Value'		=> 'Mark Value For A Dataset',
							'Image_Graph_Marker_Bubble'		=> 'Mark Bubble For A Dataset',
						);

	/**
	* Option list for marker value types
	* @var array
	*/
	public $value_marker_types = Array(
								'IMAGE_GRAPH_VALUE_X'	=> 'Mark x-Axis Values',
								'IMAGE_GRAPH_VALUE_Y'	=> 'Mark y-Axis Values',
							  );

	/**
	* Option list for mark types
	* @var array
	*/
	public $pointing_types = Array(
							'none'									=> '-- Leave Empty --',
							'Image_Graph_Marker_Pointing_Angular'	=> 'Image_Graph_Marker_Pointing_Angular',
							'Image_Graph_Marker_Pointing_Radial'	=> 'Image_Graph_Marker_Pointing_Radial',
							'Image_Graph_Marker_ReversePinpoint'	=> 'Image_Graph_Marker_ReversePinpoint',
						  );


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

	}//end constructor


//--        DATASETS SCREEN        --//


	/**
	* Paints the interface for adding a new plot
	*
	* @param object	&$asset	the graph asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function paintAddPlots(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		if ($wa) {
			$o->openField('Name');
				text_box($prefix.'_new_name', '', '20', '20');
			$o->closeField();
			$o->openField('Data Source');
				asset_finder($prefix.'_new_data_source', 0, Array('data_source_db'	=> 'I', 'data_source_rss'	=> 'I'));
			$o->closeField();
		}

		$datasets = $asset->attr('datasets');
		$plot_order = $asset->attr('plot_order');
		if (!empty($datasets)) {
			$o->openField('Current Plots');
			echo '<table class="sq-backend-table">';
			echo '<tr>
					<td>Plot Name</td>
					<td>Data Source</td>';
			if ($wa) echo '<td>Delete?</td>';
			echo '</tr>';
			foreach ($plot_order as $key => $plot_name) {
				?>
				<tr>
					<td><?php echo $plot_name; ?></td>
					<td><?php echo get_asset_tag_line($datasets[$plot_name]['assetid']); ?></td>
					<?php
					if ($wa) {
						echo '<td>';
						check_box($prefix.'_delete_plot_'.$plot_name, $plot_name);
						echo '</td>';
					}
					?>
				</tr>
				<?php
			}
			echo '</table>';
			$o->closeField();
		} else {
			$o->openField('Current Plots');
				echo 'There is no plot.';
			$o->closeField();
		}
		return TRUE;

	}//end paintAddPlots()


	/**
	* Processes the interface for adding a new plot
	*
	* @param object	&$asset	the graph asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function processAddPlots(&$asset, &$o, $prefix)
	{
		$datasets = $asset->attr('datasets');
		$plot_order = $asset->attr('plot_order');

		if (!empty($_POST[$prefix.'_new_name']) && $_POST[$prefix.'_new_data_source']['assetid'] != '0') {
			$datasets        = $asset->attr('datasets');
			$dataset_assetid = $_POST[$prefix.'_new_data_source']['assetid'];

			// get the first result, extract the field names from that
			// otherwise, fall back on 'Not saved'
			$axis_default_val  =  'Not saved';
			$data_source_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($dataset_assetid);
			if (!is_null($data_source_asset)) {
				$first_result = reset($data_source_asset->getResultSet('default'));
				if (is_array($first_result)) {
					$data = array_keys($first_result);
					// default to the first field available to us
					$axis_default_val = reset($data);
				}
			}

			$datasets[$_POST[$prefix.'_new_name']] = Array(
														'assetid'	=> $dataset_assetid,
														'settings'	=> Array(
																		'plot_name'				=> $_POST[$prefix.'_new_name'],
																		'display_name'			=> $_POST[$prefix.'_new_name'],
																		'graph_type'			=> 'Image_Graph_Plot_Bar',
																		'x_axis'				=> $axis_default_val,
																		'y_axis'				=> $axis_default_val,
																		'accumulative'			=> 0,
																		'fill_colour'			=> 'ffffff',
																		'gradient_type'			=> 'none',
																		'gradient_1'			=> '000000',
																		'gradient_2'			=> '000000',
																		'line_colour'			=> '000000',
																		'marker_font_colour'	=> 'ffffff',
																		'marker_line_colour'	=> '000000',
																		'marker_fill_colour'	=> '000000',
																		'bar_spacing'			=> 0,
																		'use_stacked'			=> FALSE,
																		'marker_type'			=> 'none',
																		'value_marker_type'		=> 'IMAGE_GRAPH_VALUE_X',
																		'pointer_type'			=> 'none',
																		'pointer_length'		=> 20,
																	   ),
													 );
			// add plot_order entry
			$index = count($plot_order);
			$plot_order[$index] = $_POST[$prefix.'_new_name'];

			// plot has been added. Modify $_POST value here to bring the
			// moved plot's settings in the next screen
			$_POST[$prefix.'_plot_name'] = $index;
			$_POST[$prefix.'_prev_plot_name'] = $index;
		}//end if

		if (!empty($datasets)) {
			foreach ($datasets as $key => $dataset) {
				if (isset($_POST[$prefix.'_delete_plot_'.str_replace(' ', '_', $key)]) && !empty($_POST[$prefix.'_delete_plot_'.str_replace(' ', '_', $key)])) {
					unset($datasets[$key]);
					unset($plot_order[array_search($key, $plot_order)]);

					// re-order plot_order array
					if (!empty($datasets)) {
						$keys = array_keys($plot_order);
						ksort($keys, SORT_NUMERIC);

						$index = 0;
						$new_plot_order = Array();
						foreach ($keys as $key) {
							$new_plot_order[$index] = $plot_order[$key];
							$index++;
						}
						$plot_order = $new_plot_order;
					}

				}
			}
		}
		$asset->setAttrValue('datasets', $datasets);
		$asset->setAttrValue('plot_order', $plot_order);
		return TRUE;

	}//end processAddPlots()


	/**
	* Paints the interface for choosing a plot to edit
	*
	* @param object	&$asset	the graph asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function paintChoosePlots(&$asset, &$o, $prefix)
	{
		$plot_order = $asset->attr('plot_order');
		$o->openField('Choose a plot');

			$plot_name = isset($_POST[$prefix.'_plot_name']) ? $_POST[$prefix.'_plot_name'] : '0';

			combo_box($prefix.'_plot_name', $plot_order, FALSE, $plot_name);
			?>
				<br /><b>Note</b>&nbsp;:&nbsp;If you choose a plot other than one displayed below and submit, then the current changes in the plot settings <b>WILL NOT</b> be applied,
				i.e. If you want to submit the changes in the plot settings, <b>DO NOT</b> choose a new plot here
			<?php
			hidden_field($prefix.'_prev_plot_name', $plot_name);
		$o->closeField();
		return TRUE;

	}//end paintChoosePlots()


	/**
	* Processes the interface for choosing a plot to edit
	*
	* @param object	&$asset	the graph asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function processChoosePlots(&$asset, &$o, $prefix)
	{
		return TRUE;

	}//end processChoosePlots()


	/**
	* Paints the interface for editing the plot settings
	*
	* @param object	&$asset	the graph asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function paintGeneralPlotSettings(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		$datasets = $asset->attr('datasets');
		$plot_order = $asset->attr('plot_order');

		if (isset($_POST[$prefix.'_plot_name'])) {
			$plot_not_deleted = !empty($datasets[$plot_order[intval($_POST[$prefix.'_plot_name'])]]);
			if ($plot_not_deleted) {
				$plot_name = $plot_order[intval($_POST[$prefix.'_plot_name'])];
			} else {
				$plot_name = $plot_order[0];
			}
		} else {
			$plot_name = $plot_order[0];
		}

		$data_source_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($datasets[$plot_name]['assetid']);
		$result = $data_source_asset->getResultSet('default');
		$data = array_keys($result[0]);
		foreach ($data as $field) {
			$fields[$field] = $field;
		}

		$o->openField('Plot Name');
			echo '<b>'.$plot_name.'</b>';
		$o->closeField();

		if ($wa) {
			$o->openField('Display Name');
				if (empty($datasets[$plot_name]['settings']['display_name']) || $datasets[$plot_name]['settings']['display_name'] == $plot_name) {
					$display_name = $plot_name;
				} else {
					$display_name = $datasets[$plot_name]['settings']['display_name'];
				}
				text_box($prefix.'_display_name', $display_name);
			$o->closeField();
			if ($this->moreThanOnePlot($asset, $prefix)) {
				$o->openField('Move the plot');
					combo_box($prefix.'_move_this_plot', Array(0 => '-- Leave Empty --', 1 => 'Backward', 2 => 'Forward'), FALSE, 0);
					?>
					<br /><b>Note</b>&nbsp;:&nbsp;You can move this plot <b>backward</b> or <b>forward</b> as shown in the final graph
					<?php
				$o->closeField();
			}
			$o->openField('Data Source');
				asset_finder($prefix.'_data_source', !empty($datasets[$plot_name]['assetid']) ? $datasets[$plot_name]['assetid'] : '0', Array('data_source_db'	=> 'I', 'data_source_rss'	=> 'I'));
			$o->closeField();
			$o->openField(translate('choose').' x-Axis');
				combo_box($prefix.'_x_axis', $fields, FALSE, empty($datasets[$plot_name]['settings']['x_axis']) ? current($fields) : $datasets[$plot_name]['settings']['x_axis']);
			$o->closeField();
			$o->openField(translate('choose').' y-Axis');
				combo_box($prefix.'_y_axis', $fields, FALSE, empty($datasets[$plot_name]['settings']['y_axis']) ? current($fields) : $datasets[$plot_name]['settings']['y_axis']);
			$o->closeField();
			$o->openField(translate('choose').' '.translate('graph_type'));
				combo_box($prefix.'_graph_type', $this->graph_types, FALSE, array_search($datasets[$plot_name]['settings']['graph_type'], $this->graph_types));
			$o->closeField();
			$o->openField('Accumulative?');
				combo_box($prefix.'_accumulative', Array(1 => 'Yes', 0 => 'No'), FALSE, $datasets[$plot_name]['settings']['accumulative']);
				?>
				<br /><b>Note</b>&nbsp;:&nbsp;If it's yes, y-Axis values are accumulated
				<?php
				hidden_field($prefix.'_plot_settings_submit', '1');
			$o->closeField();

			$o->openField('Fill Colour');
				colour_box($prefix.'_fill_colour', empty($datasets[$plot_name]['settings']['fill_colour']) ? 'ffffff' : $datasets[$plot_name]['settings']['fill_colour']);
			$o->closeField();
			$o->openField('Line Colour');
				colour_box($prefix.'_line_colour', empty($datasets[$plot_name]['settings']['line_colour']) ? '000000' : $datasets[$plot_name]['settings']['line_colour']);
			$o->closeField();
			$o->openField('Fill Gradient Type');
				combo_box($prefix.'_gradient_type', $this->fill_grad_types, FALSE, $datasets[$plot_name]['settings']['gradient_type']);
				?>
				<br /><b>Note</b>&nbsp;:&nbsp;If the type is set other than 'Leave Empty', a fill colour is ignored
				<?php
			$o->closeField();
			$o->openField('Gradient Colour 1');
				colour_box($prefix.'_gradient_1', empty($datasets[$plot_name]['settings']['gradient_1']) ? '000000' : $datasets[$plot_name]['settings']['gradient_1']);
			$o->closeField();
			$o->openField('Gradient Colour 2');
				colour_box($prefix.'_gradient_2', empty($datasets[$plot_name]['settings']['gradient_2']) ? '000000' : $datasets[$plot_name]['settings']['gradient_2']);
			$o->closeField();

		} else {

			$o->openField('Display Name');
				echo $datasets[$plot_name]['settings']['display_name'];
			$o->closeField();
			$o->openField('Data Source');
				echo get_asset_tag_line($datasets[$plot_name]['assetid']);
			$o->closeField();
			$o->openField('x-Axis');
				echo $datasets[$plot_name]['settings']['x_axis'];
			$o->closeField();
			$o->openField('y-Axis');
				echo $datasets[$plot_name]['settings']['y_axis'];
			$o->closeField();
			$o->openField('Type of Graph');
				echo $datasets[$plot_name]['settings']['graph_type'];
			$o->closeField();
			$o->openField('Accumulative');
				echo (intval($datasets[$plot_name]['settings']['accumulative'])) ? 'Yes' : 'No';
				?>
				<br /><b>Note</b>&nbsp;:&nbsp;If it's yes, y-Axis values are accumulated
				<?php
			$o->closeField();
			$o->openField('Fill Colour');
				if (empty($datasets[$plot_name]['settings']['fill_colour'])) {
					echo 'none';
				} else {
					echo '<table><tr>
					<td width="20" height="13" style="background-color: #'.$datasets[$plot_name]['settings']['fill_colour'].'"></td>
					<td>&nbsp;#'.$datasets[$plot_name]['settings']['fill_colour'].'</td>
					</tr></table>';
				}
			$o->closeField();
			$o->openField('Line Colour');
				if (empty($datasets[$plot_name]['settings']['line_colour'])) {
					echo 'none';
				} else {
					echo '<table><tr>
					<td width="20" height="13" style="background-color: #'.$datasets[$plot_name]['settings']['line_colour'].'"></td>
					<td>&nbsp;#'.$datasets[$plot_name]['settings']['line_colour'].'</td>
					</tr></table>';
				}
			$o->closeField();
			$o->openField('Fill Gradient Type');
				echo $datasets[$plot_name]['settings']['gradient_type'];
				?>
				<br /><b>Note</b>&nbsp;:&nbsp;If the type is set other than 'Leave Empty', a fill colour is ignored
				<?php
			$o->closeField();
			$o->openField('Gradient Colour 1');
				if (empty($datasets[$plot_name]['settings']['gradient_1'])) {
					echo 'none';
				} else {
					echo '<table><tr>
					<td width="20" height="13" style="background-color: #'.$datasets[$plot_name]['settings']['gradient_1'].'"></td>
					<td>&nbsp;#'.$datasets[$plot_name]['settings']['gradient_1'].'</td>
					</tr></table>';
				}
			$o->closeField();
			$o->openField('Gradient Colour 2');
				if (empty($datasets[$plot_name]['settings']['gradient_2'])) {
					echo 'none';
				} else {
					echo '<table><tr>
					<td width="20" height="13" style="background-color: #'.$datasets[$plot_name]['settings']['gradient_2'].'"></td>
					<td>&nbsp;#'.$datasets[$plot_name]['settings']['gradient_2'].'</td>
					</tr></table>';
				}
			$o->closeField();
		}//end else

		return TRUE;

	}//end paintGeneralPlotSettings()


	/**
	* Processes the interface for editing the plot settings
	*
	* @param object	&$asset	the graph asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function processGeneralPlotSettings(&$asset, &$o, $prefix)
	{
		$plot_settings_submit = isset($_POST[$prefix.'_plot_settings_submit']);
		$wa = $asset->writeAccess('attributes');
		if ($wa && $plot_settings_submit) {
			$datasets = $asset->attr('datasets');
			$plot_order = $asset->attr('plot_order');

			$plot_not_deleted = isset($datasets[$plot_order[intval($_POST[$prefix.'_plot_name'])]]);
			$plot_changed = ($_POST[$prefix.'_plot_name'] !== $_POST[$prefix.'_prev_plot_name']);

			if (!$plot_changed && $plot_not_deleted) {
				$plot_name = $plot_order[intval($_POST[$prefix.'_plot_name'])];
				$datasets[$plot_name]['assetid'] = $_POST[$prefix.'_data_source']['assetid'];
				$datasets[$plot_name]['settings']['display_name']	= $_POST[$prefix.'_display_name'];
				$datasets[$plot_name]['settings']['x_axis']			= $_POST[$prefix.'_x_axis'];
				$datasets[$plot_name]['settings']['y_axis']			= $_POST[$prefix.'_y_axis'];
				$datasets[$plot_name]['settings']['graph_type']		= $this->graph_types[intval($_POST[$prefix.'_graph_type'])];
				$datasets[$plot_name]['settings']['accumulative']	= $_POST[$prefix.'_accumulative'];
				$datasets[$plot_name]['settings']['fill_colour']	= $_POST[$prefix.'_fill_colour'];
				$datasets[$plot_name]['settings']['line_colour']	= $_POST[$prefix.'_line_colour'];
				$datasets[$plot_name]['settings']['gradient_type']	= $_POST[$prefix.'_gradient_type'];
				$datasets[$plot_name]['settings']['gradient_1']		= $_POST[$prefix.'_gradient_1'];
				$datasets[$plot_name]['settings']['gradient_2']		= $_POST[$prefix.'_gradient_2'];
				if (isset($_POST[$prefix.'_bar_spacing'])) {
					$datasets[$plot_name]['settings']['bar_spacing']	= $_POST[$prefix.'_bar_spacing'];
				}
				$asset->setAttrValue('datasets', $datasets);

				// Re-order plots
				if (isset($_POST[$prefix.'_move_this_plot']) && $_POST[$prefix.'_move_this_plot'] != '0') {

					$current_index = array_search($plot_name, $plot_order);

					$swap = FALSE;
					switch ($_POST[$prefix.'_move_this_plot']) {
						case '1':
							// move backward
							if ($current_index > 0) {
								$dest_index = $current_index - 1;
								$swap = TRUE;
							}
						break;
						case '2':
							// move forward
							if ($current_index < (count($plot_order) - 1)) {
								$dest_index = $current_index + 1;
								$swap = TRUE;
							}
						break;
					}
					if ($swap) {
						$dest_value = $plot_order[$dest_index];
						$current_value = $plot_order[$current_index];
						$plot_order[$dest_index] = $current_value;
						$plot_order[$current_index] = $dest_value;
						$asset->setAttrValue('plot_order', $plot_order);

						// plot has been moved. Modify $_POST value here to bring the
						// moved plot's settings in the next screen
						$_POST[$prefix.'_plot_name'] = $dest_index;
						$_POST[$prefix.'_prev_plot_name'] = $dest_index;
					}
				}//end if
			} else {
				return FALSE;
			}
		}//end if

		return TRUE;

	}//end processGeneralPlotSettings()


	/**
	* Paints the interface for editing the plot settings
	*
	* @param object	&$asset	the graph asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function paintPlotSpecificSettings(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		$datasets = $asset->attr('datasets');
		$plot_order = $asset->attr('plot_order');
		if (isset($_POST[$prefix.'_plot_name'])) {
			$plot_not_deleted = !empty($datasets[$plot_order[intval($_POST[$prefix.'_plot_name'])]]);
			if ($plot_not_deleted) {
				$plot_name = $plot_order[intval($_POST[$prefix.'_plot_name'])];
			} else {
				$plot_name = $plot_order[0];
			}
		} else {
			$plot_name = $plot_order[0];
		}

		$data_source_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($datasets[$plot_name]['assetid']);
		$result = $data_source_asset->getResultSet('default');
		$data = array_keys($result[0]);
		foreach ($data as $field) {
			$fields[$field] = $field;
		}

		$plot_type = $datasets[$plot_name]['settings']['graph_type'];
		if ($wa) {
			switch ($plot_type) {
				case 'Image_Graph_Plot_Bar' :
				case 'Image_Graph_Plot_Area' :
					$o->openField('Spacing');
						text_box($prefix.'_bar_spacing', empty($datasets[$plot_name]['settings']['bar_spacing']) ? '0' : $datasets[$plot_name]['settings']['bar_spacing'], '10');
					$o->closeField();
					$o->openField('Stacked');
						check_box($prefix.'_use_stacked', '1', isset($datasets[$plot_name]['settings']['use_stacked']) ? $datasets[$plot_name]['settings']['use_stacked'] : FALSE);
						$o->note('Do you want to use stacked bar graph?');
						?>
						<table class="sq-backend-table">
						<tr>
							<td>Title and Data Source</td>
							<td>Gradient Details</td>
						<?php if ($wa) echo '<td>Delete?</td>'; ?>
						</tr>

						<?php
						$key = 0;
						if (!empty($datasets[$plot_name]['settings']['stacked_data'])) {
							foreach ($datasets[$plot_name]['settings']['stacked_data'] as $key => $stack_data) {
								?>
								<tr>
									<td>
										Title : <?php text_box($prefix.'_stacked_'.strval($key).'_title', $stack_data['title'], 50); ?><br />
										<?php
											asset_finder($prefix.'_stacked_'.strval($key).'_datasource', $stack_data['datasource'], Array('data_source_db'	=> 'I', 'data_source_rss'	=> 'I'));
											echo '<br />';
											if (!empty($stack_data['datasource'])) {
												$data_source_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($stack_data['datasource']);
												$result = $data_source_asset->getResultSet('default');
												$data = array_keys($result[0]);
												foreach ($data as $field) {
													$fields[$field] = $field;
												}
											}
											echo 'x-axis&nbsp;:&nbsp;';
											combo_box($prefix.'_stacked_'.strval($key).'_x_axis', $fields, FALSE, $stack_data['x_axis']);
											echo '<br />y-axis&nbsp;:&nbsp;';
											combo_box($prefix.'_stacked_'.strval($key).'_y_axis', $fields, FALSE, $stack_data['y_axis']);
										?>
									</td>
									<td>
										<?php combo_box($prefix.'_stacked_'.strval($key).'_gradient_type', $this->fill_grad_types, FALSE, $stack_data['gradient_type']); ?><br />
										<?php colour_box($prefix.'_stacked_'.strval($key).'_gradient_colour_2', $stack_data['colour_1']); ?>Gradient Colour 1<br />
										<?php colour_box($prefix.'_stacked_'.strval($key).'_gradient_colour_1', $stack_data['colour_2']); ?>Gradient Colour 2<br />
									</td>
									<td>
									<?php
										if ($wa) {
											check_box($prefix.'_stacked_'.strval($key).'_delete', '1', FALSE);
										}
									?>
									</td>
								</tr>
								<?php
							}//end foreach

							$key++;
						}//end if
						?>
						<tr>
							<td>
								Title : <?php text_box($prefix.'_stacked_'.strval($key).'_title', '', 50); ?><br />
								<?php asset_finder($prefix.'_stacked_'.strval($key).'_datasource', 0, Array('data_source_db'	=> 'I', 'data_source_rss'	=> 'I')); ?>
							</td>
							<td>
								<?php combo_box($prefix.'_stacked_'.strval($key).'_gradient_type', $this->fill_grad_types, FALSE, 'none'); ?><br />
								<?php colour_box($prefix.'_stacked_'.strval($key).'_gradient_colour_1', '000000'); ?>Gradient Colour 1<br />
								<?php colour_box($prefix.'_stacked_'.strval($key).'_gradient_colour_2', '000000'); ?>Gradient Colour 2<br />
							</td>
							<td></td>
						</tr>
						</table>
						<?php
						$o->note('Note: If the gradient option is not chosen, the first colour is used to fill the bar.');
					$o->closeField();
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
				break;
				default :
				break;

			}//end switch

		} else {
			switch ($plot_type) {
				case 'Image_Graph_Plot_Bar' :
					$o->openField('Spacing');
						echo $datasets[$plot_name]['settings']['bar_spacing'];
					$o->closeField();
					$o->openField('Stacked');
					$o->closeField();
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
					$o->openField('None');
					$o->closeField();
				break;
				default :
				break;
			}
		}
		return TRUE;

	}//end paintPlotSpecificSettings()


	/**
	* Processes the interface for editing the plot settings
	*
	* @param object	&$asset	the graph asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function processPlotSpecificSettings(&$asset, &$o, $prefix)
	{
		$plot_settings_submit = isset($_POST[$prefix.'_plot_settings_submit']);
		$wa = $asset->writeAccess('attributes');
		if ($wa && $plot_settings_submit) {
			$datasets = $asset->attr('datasets');
			$plot_order = $asset->attr('plot_order');

			$plot_not_deleted = isset($datasets[$plot_order[intval($_POST[$prefix.'_plot_name'])]]);
			$plot_changed = ($_POST[$prefix.'_plot_name'] != $_POST[$prefix.'_prev_plot_name']);

			if (!$plot_changed && $plot_not_deleted) {
				$plot_name = $plot_order[intval($_POST[$prefix.'_plot_name'])];
				$plot_type = $datasets[$plot_name]['settings']['graph_type'];
				switch ($plot_type) {
					case 'Image_Graph_Plot_Bar' :
					case 'Image_Graph_Plot_Area' :
						if (isset($_POST[$prefix.'_bar_spacing'])) {
							$datasets[$plot_name]['settings']['bar_spacing']	= $_POST[$prefix.'_bar_spacing'];
						}
						$datasets[$plot_name]['settings']['use_stacked']	= isset($_POST[$prefix.'_use_stacked']);
						$key = 0;
						if (!empty($datasets[$plot_name]['settings']['stacked_data'])) {
							// Delete?
							foreach ($datasets[$plot_name]['settings']['stacked_data'] as $key => $stack_data) {
								if (isset($_POST[$prefix.'_stacked_'.strval($key).'_delete'])) {
									unset($datasets[$plot_name]['settings']['stacked_data'][$key]);
									unset($_POST[$prefix.'_stacked_'.strval($key).'_datasource']);
								}
							}
							if (!empty($datasets[$plot_name]['settings']['stacked_data'])) {
								foreach ($datasets[$plot_name]['settings']['stacked_data'] as $key => $stack_data) {
									if (isset($_POST[$prefix.'_stacked_'.strval($key).'_title'])) {
										$datasets[$plot_name]['settings']['stacked_data'][$key]['title']			= $_POST[$prefix.'_stacked_'.strval($key).'_title'];
										$datasets[$plot_name]['settings']['stacked_data'][$key]['datasource']		= $_POST[$prefix.'_stacked_'.strval($key).'_datasource']['assetid'];
										$datasets[$plot_name]['settings']['stacked_data'][$key]['colour_1']		= $_POST[$prefix.'_stacked_'.strval($key).'_gradient_colour_1'];
										$datasets[$plot_name]['settings']['stacked_data'][$key]['colour_2']		= $_POST[$prefix.'_stacked_'.strval($key).'_gradient_colour_2'];
										$datasets[$plot_name]['settings']['stacked_data'][$key]['gradient_type']	= $_POST[$prefix.'_stacked_'.strval($key).'_gradient_type'];
										$datasets[$plot_name]['settings']['stacked_data'][$key]['x_axis']			= $_POST[$prefix.'_stacked_'.strval($key).'_x_axis'];
										$datasets[$plot_name]['settings']['stacked_data'][$key]['y_axis']			= $_POST[$prefix.'_stacked_'.strval($key).'_y_axis'];
									}

								}
								$key++;
							}
						}
						// check a new one
						if (!empty($_POST[$prefix.'_stacked_'.strval($key).'_datasource']) && $_POST[$prefix.'_stacked_'.strval($key).'_datasource']['assetid'] != '0') {
							$datasets[$plot_name]['settings']['stacked_data'][$key]['title']			= $_POST[$prefix.'_stacked_'.strval($key).'_title'];
							$datasets[$plot_name]['settings']['stacked_data'][$key]['datasource']		= $_POST[$prefix.'_stacked_'.strval($key).'_datasource']['assetid'];
							$datasets[$plot_name]['settings']['stacked_data'][$key]['colour_1']		= $_POST[$prefix.'_stacked_'.strval($key).'_gradient_colour_1'];
							$datasets[$plot_name]['settings']['stacked_data'][$key]['colour_2']		= $_POST[$prefix.'_stacked_'.strval($key).'_gradient_colour_2'];
							$datasets[$plot_name]['settings']['stacked_data'][$key]['gradient_type']	= $_POST[$prefix.'_stacked_'.strval($key).'_gradient_type'];

							$data_source_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($_POST[$prefix.'_stacked_'.strval($key).'_datasource']['assetid']);
							$result = $data_source_asset->getResultSet('default');
							$datasets[$plot_name]['settings']['stacked_data'][$key]['x_axis'] = array_pop(array_keys($result[0]));
							$datasets[$plot_name]['settings']['stacked_data'][$key]['y_axis'] = $datasets[$plot_name]['settings']['stacked_data'][$key]['x_axis'];
						}
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
					break;
					default :
					break;
				}//end switch

				$asset->setAttrValue('datasets', $datasets);

			}//end if

		}//end if

		return TRUE;

	}//end processPlotSpecificSettings()


	/**
	* Paints the interface for editing the marker settings
	*
	* @param object	&$asset	the graph asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function paintMarkerSettings(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		$datasets = $asset->attr('datasets');
		$plot_order = $asset->attr('plot_order');

		if (isset($_POST[$prefix.'_plot_name'])) {
			$plot_not_deleted = !empty($datasets[$plot_order[intval($_POST[$prefix.'_plot_name'])]]);
			if ($plot_not_deleted) {
				$plot_name = $plot_order[intval($_POST[$prefix.'_plot_name'])];
			} else {
				asort($datasets);
				$plot_name = array_pop(array_keys($datasets));
			}
		} else {
			asort($datasets);
			$plot_name = array_pop(array_keys($datasets));
		}


		$data_source_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($datasets[$plot_name]['assetid']);
		$result = $data_source_asset->getResultSet('default');
		$data = array_keys($result[0]);
		foreach ($data as $field) {
			$fields[$field] = $field;
		}

		$colours = Array(
					'Set Font Colour'	=> 'marker_font_colour',
					'Set Line Colour'	=> 'marker_line_colour',
					'Set Fill Colour'	=> 'marker_fill_colour',
				   );
		if ($wa) {
			$o->openField('Choose The Type Of Marker');
				combo_box($prefix.'_marker_type', $this->marker_types, FALSE, empty($datasets[$plot_name]['settings']['marker_type']) ? 'none' : $datasets[$plot_name]['settings']['marker_type']);
				?>
					<br /><b>Note</b>&nbsp;:&nbsp;If you don't want to use marker, leave this empty
				<?php
			$o->closeField();
			$o->openField('Choose Value To be Marked On The Graph');
				combo_box($prefix.'_value_marker_type', $this->value_marker_types, FALSE, empty($datasets[$plot_name]['settings']['value_marker_type']) ? 'none' : $datasets[$plot_name]['settings']['value_marker_type']);
			$o->closeField();
			foreach ($colours as $field_title => $key) {
				$o->openField($field_title);
					colour_box($prefix.'_'.$key, empty($datasets[$plot_name]['settings'][$key]) ? '000000' : $datasets[$plot_name]['settings'][$key]);
				$o->closeField();
			}
			$o->openField('Choose The Marker Pointer');
				combo_box($prefix.'_pointer_type', $this->pointing_types, FALSE, empty($datasets[$plot_name]['settings']['pointer_type']) ? 'none' : $datasets[$plot_name]['settings']['pointer_type']);
				?>
					<br /><b>Note</b>&nbsp;:&nbsp;If you don't want to use pointer, leave this empty
				<?php
			$o->closeField();
			$o->openField('Set The Length Of The Pointer');
				text_box($prefix.'_pointer_length', empty($datasets[$plot_name]['settings']['pointer_length']) ? '20' : $datasets[$plot_name]['settings']['pointer_length'], 5);
			$o->closeField();
		} else {
			$o->openField('Choose The Type Of Marker');
				echo $datasets[$plot_name]['settings']['marker_type'];
			$o->closeField();
			$o->openField('Choose Value To be Marked On The Graph');
				echo $datasets[$plot_name]['settings']['value_marker_type'];
			$o->closeField();
			foreach ($colours as $field_title => $key) {
				$o->openField($field_title);
				if (empty($datasets[$plot_name]['settings'][$key])) {
					echo 'none';
				} else {
					echo '<table><tr>
					<td width="20" height="13" style="background-color: #'.$datasets[$plot_name]['settings'][$key].'"></td>
					<td>&nbsp;#'.$datasets[$plot_name]['settings'][$key].'</td>
					</tr></table>';
				}
				$o->closeField();
			}
			$o->openField('Choose The Marker Pointer');
				echo $datasets[$plot_name]['settings']['pointer_type'];
			$o->closeField();
			$o->openField('Set The Length Of The Pointer');
				echo $datasets[$plot_name]['settings']['pointer_length'];
			$o->closeField();

		}

	}//end paintMarkerSettings()


	/**
	* Processes the interface for editing the plot settings
	*
	* @param object	&$asset	the graph asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function processMarkerSettings(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		$plot_settings_submit = isset($_POST[$prefix.'_plot_settings_submit']);
		if ($wa && $plot_settings_submit) {
			$datasets = $asset->attr('datasets');
			$plot_order = $asset->attr('plot_order');
			$plot_not_deleted = @isset($datasets[$plot_order[intval($_POST[$prefix.'_plot_name'])]]);
			$plot_changed = ($_POST[$prefix.'_plot_name'] != $_POST[$prefix.'_prev_plot_name']);
			if (!$plot_changed && $plot_not_deleted) {
				$plot_name = $plot_order[intval($_POST[$prefix.'_plot_name'])];
				$datasets[$plot_name]['settings']['marker_type']				= $_POST[$prefix.'_marker_type'];
				$datasets[$plot_name]['settings']['value_marker_type']			= $_POST[$prefix.'_value_marker_type'];
				$datasets[$plot_name]['settings']['marker_font_colour']			= $_POST[$prefix.'_marker_font_colour'];
				$datasets[$plot_name]['settings']['marker_line_colour']			= $_POST[$prefix.'_marker_line_colour'];
				$datasets[$plot_name]['settings']['marker_fill_colour']			= $_POST[$prefix.'_marker_fill_colour'];
				$datasets[$plot_name]['settings']['pointer_type']				= $_POST[$prefix.'_pointer_type'];
				$datasets[$plot_name]['settings']['pointer_length']				= $_POST[$prefix.'_pointer_length'];
				$asset->setAttrValue('datasets', $datasets);
			}
		}

		return $wa;

	}//end processMarkerSettings()


//--        PREVIEWS        --//


	/**
	* Paints the interface for display the global settings preview
	*
	* @param object	&$asset	the graph asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function paintGlobalSettingPreview(&$asset, &$o, $prefix)
	{
		$href	= $asset->getBackendHref('global_settings');
		$width	= $asset->attr('width');
		$height	= $asset->attr('height');
		$alt	= $asset->attr('alt');
		$title	= $asset->attr('title');
		?>
			<b>Note</b><br />
			<ul>
				<li>
					This graph is generated with <b>random dataset</b> to present the look and feel of the graph
				</li>
				<li>
					The front end image <b>WILL NOT</b> be updated until a dataset is added.<br />i.e. the changes on this screen is not shown in the front end unless a dataset is set.
				</li>
			</ul>
			<img alt="<?php echo $alt; ?>" title="<?php echo $title; ?>" src="<?php echo $href.'&preview_graph=1' ?>" width="<?php echo $width; ?>" height="<?php echo $height; ?>" alt="" />
		<?php

		return TRUE;

	}//end paintGlobalSettingPreview()


	/**
	* Processes the interface for display the global settings preview
	*
	* @param object	&$asset	the graph asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function processGlobalSettingPreview(&$asset, &$o, $prefix)
	{
		return TRUE;

	}//end processGlobalSettingPreview()


	/**
	* Paints the interface for display the single settings preview
	*
	* @param object	&$asset	the graph asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function paintSinglePreview(&$asset, &$o, $prefix)
	{
		$datasets = $asset->attr('datasets');
		$plot_order = $asset->attr('plot_order');
		$plot_not_deleted = @isset($datasets[$plot_order[intval($_POST[$prefix.'_plot_name'])]]);
		if (!empty($_POST[$prefix.'_plot_name']) && $plot_not_deleted) {
			$plot_name = $plot_order[intval($_POST[$prefix.'_plot_name'])];
		} else {
			$plot_name = $plot_order[0];
		}
		if ($datasets[$plot_name]['settings']['x_axis'] == 'Not saved' || $datasets[$plot_name]['settings']['y_axis'] == 'Not saved') {
			?>
			<b>Not available</b>&nbsp;:&nbsp;Please set both of x-Axis and y-Axis correctly<br />
			<?php
		} else {
			$graph_info	= serialize($datasets[$plot_name]);
			$href		= $asset->getBackendHref('datasets');
			$alt		= $asset->attr('alt');
			$title		= $asset->attr('title');
			?>
			<b>Note</b>&nbsp;:&nbsp;This graph is generated with <b>random dataset</b> to present the look and feel of the graph<br /><br />
			<img alt="<?php echo $alt; ?>" title="<?php echo $title; ?>" src="<?php echo $href.'&preview_graph=1&graph_info='.urlencode($graph_info) ?>" width="<?php echo $asset->attr('width'); ?>" height="<?php echo $asset->attr('height'); ?>" />
			<?php
		}
		return FALSE;

	}//end paintSinglePreview()


	/**
	* Processes the interface for display the single settings preview
	*
	* @param object	&$asset	the graph asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function processSinglePreview(&$asset, &$o, $prefix)
	{
		return FALSE;

	}//end processSinglePreview()


	/**
	* Paints the interface for display the final graph preview
	*
	* @param object	&$asset	the graph asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function paintFinalPreview(&$asset, &$o, $prefix)
	{
		$datasets	= $asset->attr('datasets');
		$preview	= TRUE;
		$not_set	= Array();
		foreach ($datasets as $name => $dataset) {
			if ($dataset['settings']['x_axis'] == 'Not saved' || $dataset['settings']['y_axis'] == 'Not saved') {
				$preview = FALSE;
				$not_set[] = $name;
			}
		}

		if (!$preview) {
			?>
			<b>Not available</b>&nbsp;:&nbsp;Please set both of x-Axis and y-Axis correctly for <?php echo implode(',', $not_set); ?><br />
			<?php
		} else {
			$href	= $asset->getBackendHref('datasets');
			$alt	= $asset->attr('alt');
			$title	= $asset->attr('title');
			?>
			<b>Note</b>&nbsp;:&nbsp;This graph is the same as one that is printed on the frontend with <b>real datasets</b>.<br /><br />
			<img alt="<?php echo $alt; ?>" title="<?php echo $title; ?>" src="<?php echo $href.'&preview_graph=1&final_preview=1' ?>" width="<?php echo $asset->attr('width'); ?>" height="<?php echo $asset->attr('height'); ?>" alt="" />
			<?php
		}

		return FALSE;

	}//end paintFinalPreview()


	/**
	* Processes the interface for display the final graph preview
	*
	* @param object	&$asset	the graph asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function processFinalPreview(&$asset, &$o, $prefix)
	{
		return FALSE;

	}//end processFinalPreview()


//--        SHOW IFS        --//


	/**
	* Returns TRUE if the screen is in Add Mode
	*
	* @param object	&$asset	the graph asset
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function isAddMode(&$asset, $prefix)
	{
		return $this->_isMode($asset, $prefix, 'add');

	}//end isAddMode()


	/**
	* Returns TRUE if the screen is in Edit Mode
	*
	* @param object	&$asset	the graph asset
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function isEditMode(&$asset, $prefix)
	{
		return $this->_isMode($asset, $prefix, 'edit') &&
				$this->datasetNotEmpty($asset, $prefix);

	}//end isEditMode()


	/**
	* Returns the current mode of plot screen
	*
	* @param object	&$asset	the graph asset
	* @param string	$prefix	prefix for form elements
	* @param string	$type	type of mode checking
	*
	* @return void
	* @access public
	*/
	function _isMode(&$asset, $prefix, $type)
	{
		$mode = $asset->attr('mode');
		return ($mode == $type);

	}//end _isMode()


	/**
	* It returns TRUE if there are not dataset
	*
	* @param object	&$asset	the graph asset
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function datasetNotEmpty(&$asset, $prefix)
	{
		$datasets = $asset->attr('datasets');
		return !empty($datasets);

	}//end datasetNotEmpty()


	/**
	* It reuturns TRUE if there exist more than one plot
	*
	* @param object	&$asset	the graph asset
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function moreThanOnePlot(&$asset, $prefix)
	{
		return count($asset->attr('datasets')) > 1;

	}//end moreThanOnePlot()


}//end class

?>
