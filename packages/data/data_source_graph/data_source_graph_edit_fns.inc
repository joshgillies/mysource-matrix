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
* $Id: data_source_graph_edit_fns.inc,v 1.3 2005/10/28 06:59:26 pgannavarapu Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Data_Source_Graph_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Data_Source_Graph_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Data_Source_Graph_Edit_Fns()
	{
		$this->Asset_Edit_Fns();

	}//end constructor


	/**
	* Paints the interface for entering the Graph Details
	*
	* @param object	&$asset	the graph asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function paintGraphDetails(&$asset, &$o, $prefix)
	{
		$data_source = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, 'data_source', false, 'data_source_for_graph');
		$graph_details = $asset->attr('graph_details');
		$types = Array('bar', 'line', 'area', 'smooth_line', 'smooth_area', 'pie', 'step', 'impulse', 'dot', 'radar', 'Image_Graph_Plot_CandleStick', 'Image_Graph_Plot_Band');
		$wa = $asset->writeAccess('attributes');
		if (!empty($data_source)) {
			$data_source_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($data_source['minorid']);
			$result = $data_source_asset->getResultSet();
			$keys = Array();
			if (!empty($result)) {
				$record = $result[0];
				foreach ($record as $key => $value) {
					$keys[$key] = $key;
					//if (is_numeric($value) && $key != 'dn') $keys[$key] = $key;
				}
			}
			$keys['none'] = translate('none');
			if ($wa) {
				?>
				<table>
					<tr>
						<td><b><?php echo translate('choose').' '.translate('x_axis'); ?></b></td>
						<td><?php combo_box('x_axis', $keys, false, !empty($graph_details) && in_array($graph_details['x_axis'], $keys)?$graph_details['x_axis']:''); ?></td>
					</tr>
					<tr>
						<td><b><?php echo translate('choose').' '.translate('y_axis');?></b></td>
						<td><?php combo_box('y_axis', $keys, false, !empty($graph_details) && in_array($graph_details['y_axis'], $keys)?$graph_details['y_axis']:'');?></td>
					</tr>
					<tr>
						<td><b><?php echo  translate('choose').' '.translate('graph_type'); ?></b></td>
						<td><?php combo_box('graph_type', $types, false, !empty($graph_details) ? array_search($graph_details['type'], $types) :'');?></td>
					</tr>
				</table>
				<?php
				return true;
			} else {
				if (!empty($graph_details)) {
					?>
					<table>
						<?php
						foreach ($graph_details as $key => $value) {
							if ($key != 'colors') {
								?>
								<tr>
									<td><b><?php echo (ucwords(strpos($key, '_') !== false? str_replace('_','-', $key) : $key));?></b></td>
									<td><?php echo(ucwords($value));?></td>
								</tr>
							<?php
							}
						}
					?>
					</table>
				<?php
				} else {
					echo translate('no_co_ordinates_entered');
				}
			}
		} else {
			echo translate('no_data_sources_chosen');
		}
		return false;

	}//end paintGraphDetails()


	/**
	* Processes the interface for entering graph details
	*
	* @param object	&$asset	the graph asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processGraphDetails(&$asset, &$o, $prefix)
	{
		$types = Array('bar', 'line', 'area', 'smooth_line', 'smooth_area', 'pie', 'step', 'impulse', 'dot', 'radar', 'Image_Graph_Plot_CandleStick', 'Image_Graph_Plot_Band');
		$details = Array();
		if (isset($_POST['x_axis']) && isset($_POST['y_axis'])) {
			$x_axis =$_POST['x_axis'];
			$y_axis = $_POST['y_axis'];
			if ($x_axis != $y_axis) {
				$details['x_axis'] = $x_axis;
				$details['y_axis'] = $y_axis;
				if (isset($_POST['graph_type'])) {
					$details['type'] = $types[$_POST['graph_type']];
				} else {
					$details['type'] = 'bar';
				}
			}
			$asset->setAttrValue('graph_details', $details);
			$asset->saveAttributes();
			return true;
		}

	}//end processGraphDetails()


	/**
	* Paints the interface for entering the DataSource Details
	*
	* @param object	&$asset	the graph asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function paintDataSourceChooser(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		$already_linked = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, 'data_source', false, 'data_source_for_graph');
		if ($wa) {
			?>
			<table>
			<tr>
				<td><?php asset_finder($prefix.'_data_source', !empty($already_linked)? $already_linked['minorid'] : 0, Array('data_source_db'	=> 'I', 'data_source_rss'	=> 'I'));?></td>
			</tr>
			</table>
		<?php
		} else {
			if (!empty($already_linked)) {
				?>
				<table>
				<tr>
					<td><b><?php echo translate('current_data_source_id');?></b></td>
					<td><b><?php echo $already_linked['minorid'];?></b></td>
				</tr>
				</table>
			<?php
			}
		}
		return true;

	}//end paintDataSourceChooser()


	/**
	* Processes the interface for choosing the data source
	*
	* @param object	&$asset	the search page asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processDataSourceChooser(&$asset, &$o, $prefix)
	{
		if (isset($_POST[$prefix.'_data_source'])) {
			$data_source_id = $_POST[$prefix.'_data_source']['assetid'];
			$is_a_link = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($asset->id, $data_source_id, SQ_LINK_NOTICE,'data_source_for_graph');
			if (empty($is_a_link)) {
				$already_linked = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, 'data_source', false, 'data_source_for_graph');
				if (!empty($already_linked)) {
					$asset->deleteLink($already_linked['linkid']);
				}
				$data_source_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($data_source_id);
				$res = $asset->createLink($data_source_asset, SQ_LINK_NOTICE, 'data_source_for_graph');
			}
		}
		return true;

	}//end processDataSourceChooser()


	/**
	* Paints the interface for displaying the chosen fill colours and option to delete them
	*
	* @param object	&$asset	the graph asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function paintFillColour(&$asset, &$o, $prefix)
	{
		$fill_colours = $asset->attr('colour_array');
		$wa = $asset->writeAccess('attributes');
		?>
		<table class="sq-backend-table">
			<tr>
				<?php
				if (!empty($fill_colours)) {
					?>
					<th><?php echo translate('colour');?></th>
					<?php
					if ($wa) {
						?>
						<th><?php echo translate('delete_question');?></th>
						<?php
					}
				}
				?>
			</tr>
			<?php
			foreach ($fill_colours as $i => $colour) {
				?>
				<tr>
					<?php
					if ($wa) {
						?>
						<td><?php colour_box('fill_colour['.$i.'][colour]', $colour, true);?></td>
						<td><?php check_box('fill_colour['.$i.'][delete]', 'delete', false);?></td>
					<?php
					} else {
						?>
						<td><?php echo $colour;?></td>
					<?php
					}
					?>
				</tr>
			<?php
			}
			?>
			<tr>
		</table>
		<br />

		<?php
		if ($wa) {
			return true;
		} else {
			return false;
		}

	}//end paintFillColour()


	/**
	* Processes the interface for fill colours
	*
	* @param object	&$asset	the graph asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processFillColour(&$asset, &$o, $prefix)
	{
		$colours = Array();
		if (isset($_POST['fill_colour'])) {
			$fill_colours = $_POST['fill_colour'];
			$colours = Array();
			foreach ($fill_colours as $i => $colour) {
				if (!empty($colour['colour']) && !isset($colour['delete'])) {
					$colours[] = '#'.$colour['colour'];
				}
			}
		}
		$colour = $asset->attr('fill_colour');
		if (!empty($colour)) $colours[] = $colour;
		$asset->setAttrValue('fill_colour', null);
		$asset->setAttrValue('colour_array', $colours);
		return true;

	}//end processFillColour()


	/**
	* Check if the user has chosen to have a value marker
	*
	* @param object	&$asset	The asset that might use the files
	* @param string	$prefix	The prefix for this asset
	*
	* @return boolean
	* @access protected
	*/
	function valueMarkerNotEmpty(&$asset, $prefix)
	{
		$marker = $asset->attr('value_marker');
		if (!empty($marker)) return true;
		return false;

	}//end valueMarkerNotEmpty()


	/**
	* Check if we have write access to the asset
	*
	* @param object	&$asset	The asset that might use the files
	* @param string	$prefix	the prefix for this asset
	*
	* @return boolean
	* @access protected
	*/
	function assetWriteAccess(&$asset, $prefix)
	{
		return $asset->writeAccess('attributes');

	}//end assetWriteAccess()


}//end class

?>
