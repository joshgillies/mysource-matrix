<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: bodycopy_table_edit_fns.inc,v 1.37 2012/08/30 01:09:05 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/bodycopy/bodycopy_container/bodycopy_container_edit_fns.inc';

/**
* Bodycopy_Table_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.37 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Bodycopy_Table_Edit_Fns extends Bodycopy_Container_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

	}//end __construct()


	/**
	* Prints the JavaScript needed by the backend functions for this table
	*
	* @param Bodycopy_Table	$asset	the table object being printed
	* @param string			$prefix	prefix for form fields
	*
	* @return void
	* @access public
	*/
	public function paintBackendJsData(Bodycopy_Table $asset, $prefix)
	{
		$table_rows = $asset->attr('rows');
		$table_attributes = $asset->attr('attributes');
		$table_attributes['identifier'] = $asset->name;
		$max_cols = $this->maxCols($table_rows);
		?>
					case <?php echo $asset->id?> :

						switch (rowid) {
							<?php
							for ($i = 0; $i < count($table_rows); $i++) {
								?>
								case <?php echo $i?> :
									switch (cellid) {
										<?php
										if (isset($table_rows[$i]['cells'])) {
											for ($x = 0; $x < count($table_rows[$i]['cells']); $x++) {
												if (isset($table_rows[$i]['cells'][$x])) {
													$table_rows[$i]['cells'][$x]->printBackendJsData($x);
												}
											}
										}
										?>

										default :
											retVal["attributes"] = '<?php echo var_serialise($table_rows[$i]['attributes'], TRUE)?>';

									}//end switch cellid
								break;
								<?php
							}
							?>

							default :
								retVal["attributes"] = '<?php echo var_serialise($table_attributes, TRUE)?>';
								retVal["num_cols"]   = <?php echo (int) $max_cols?>;
								retVal["num_rows"]   = <?php echo count($table_rows)?>;

						}//end switch rowid

					break;
		<?php

	}//end paintBackendJsData()


	/**
	* Paint JS data that tables need to use to do their processing
	*
	* @param Bodycopy_Table		$bodycopy	the bodycopy being printed
	* @param Backend_Outputter	$o			the Backend Outputter class
	* @param string				$prefix		prefix for the form element
	*
	* @return void
	* @access public
	*/
	public function paintGenericBackend(Asset $bodycopy, Backend_Outputter $o, $prefix)
	{
		// If the parent bodycopy class is itself, fudge the containers array
		// so that the proper stuff gets drawn when putting out the javascript
		// to edit.
		if ($bodycopy instanceof Bodycopy_Table) {
			$containers = Array(Array('minorid'=>$bodycopy->id, 'minor_type_code'=>'bodycopy_table'));
		} else {
			$containers = $GLOBALS['SQ_SYSTEM']->am->getLinks($bodycopy->id, SQ_LINK_TYPE_2, 'bodycopy_container', FALSE);
		}

		?>
		<script language="JavaScript" src="<?php echo sq_web_path('data').'/asset_types/bodycopy/js/bodycopy_edit_tables.js'?>"></script>

		<script language="JavaScript" type="text/javascript">
		function serialise_table(bodycopy_name, bodycopy_data, tableid, rowid, cellid) {
			var form = document.main_form;
			if (!bodycopy_saved[bodycopy_name] && bodycopy_name != null) {
				bodycopy_saved[bodycopy_name] = new Object();
			}

			if (tableid != null) {
				if (!bodycopy_saved[bodycopy_name][tableid]) {
					bodycopy_saved[bodycopy_name][tableid] = new Object();
				}

				if (rowid != null) {
					if (!bodycopy_saved[bodycopy_name][tableid]['rows']) {
						bodycopy_saved[bodycopy_name][tableid]['rows'] = new Object();
					}
					if (!bodycopy_saved[bodycopy_name][tableid]['rows'][rowid] && rowid != null) {
						bodycopy_saved[bodycopy_name][tableid]['rows'][rowid] = new Object();
					}
					if (cellid != null) {
						if (!bodycopy_saved[bodycopy_name][tableid]['rows'][rowid]['cells']) {
							bodycopy_saved[bodycopy_name][tableid]['rows'][rowid]['cells'] = new Object();
						}
						bodycopy_saved[bodycopy_name][tableid]['rows'][rowid]['cells'][cellid] = bodycopy_data;
					} else {
						// no cellid means that we are setting row attributes
						bodycopy_saved[bodycopy_name][tableid]['rows'][rowid]['attributes'] = bodycopy_data['attributes'];
					}//end if cellid

				} else {
					// no rowid means that we are settign the table attributes
					bodycopy_saved[bodycopy_name][tableid]['attributes'] = bodycopy_data['attributes'];
				}//end if rowid

			}//end if tableid

			form.elements['bodycopy_saved[' + bodycopy_name + '][' + tableid + ']'].value = var_serialise(bodycopy_saved[bodycopy_name][tableid]);
		}//end serialise_table()


		// general fn that the generic include fns can use to get data to use
		function get_bodycopy_current_table_data(bodycopy_name, tableid, rowid, cellid) {
			if (tableid == null) tableid = -1;
			if (rowid   == null) rowid   = -1;
			if (cellid  == null) cellid  = -1;
			return bodycopy_current_data[bodycopy_name].get_table_data(tableid, rowid, cellid);
		}


		function get_bodycopy_<?php echo $prefix?>_current_table_data(tableid, rowid, cellid) {
			retVal = new Object();

			switch (tableid) {
		<?php
		// if we have tables get them to print the js data they need
		$i = 0;
		if (count($containers)) {
			foreach ($containers as $container_data) {
				if ($container_data['minor_type_code'] != 'bodycopy_table') {
					continue;
				}
				$table = $GLOBALS['SQ_SYSTEM']->am->getAsset($container_data['minorid'], $container_data['minor_type_code']);
				$table_edit = $table->getEditFns();
				$table_edit->paintBackendJsData($table, $prefix, $i);
				$i++;
			}
		}
		?>
				default :
					retVal["num_containers"] = <?php echo count($containers); ?>;

			}//end switch tableid


			// check if this piece of data has been changed before
			// if it has, return the changed version, not the original
			if (tableid != -1) {
				if (rowid != -1) {
					if (cellid != -1) {
						if (bodycopy_saved &&
								bodycopy_saved["<?php echo $prefix?>"] &&
								bodycopy_saved["<?php echo $prefix?>"][tableid] &&
								bodycopy_saved["<?php echo $prefix?>"][tableid]["rows"] &&
								bodycopy_saved["<?php echo $prefix?>"][tableid]["rows"][rowid] &&
								bodycopy_saved["<?php echo $prefix?>"][tableid]["rows"][rowid]["cells"] &&
								bodycopy_saved["<?php echo $prefix?>"][tableid]["rows"][rowid]["cells"][cellid]) {
							for (var key in bodycopy_saved["<?php echo $prefix?>"][tableid]["rows"][rowid]["cells"][cellid]) {
								retVal[key] = var_serialise(bodycopy_saved["<?php echo $prefix?>"][tableid]["rows"][rowid]["cells"][cellid][key]);
							}
						}
						return retVal;
					} else {
						// editing row properties
						if (bodycopy_data_exists(new Array('<?php echo $prefix?>', tableid, 'rows', rowid, 'attributes'))) {
							retVal["attributes"] = var_serialise(bodycopy_saved["<?php echo $prefix?>"][tableid]["rows"][rowid]["attributes"]);
							return retVal;
						}
					}
				} else {
					// editing table properties
					if (bodycopy_data_exists(new Array('<?php echo $prefix?>', tableid, 'attributes'))) {
						retVal ["attributes"] = var_serialise(bodycopy_saved["<?php echo $prefix?>"][tableid]["attributes"]);
						return retVal;
					}
				}
			}

			return retVal;
		}//end get_bodycopy_<?php echo $prefix?>_current_table_data()


		// set reference so generic fn can be called above
		bodycopy_current_data["<?php echo $prefix?>"].get_table_data = get_bodycopy_<?php echo $prefix?>_current_table_data;
		</script>

		<?php

	}//end paintGenericBackend()


	/**
	* Paints this table as HTML
	*
	* @param object		&$asset		the table whose interface we are painting
	* @param boolean	$editing	are we printing an editing interface
	* @param boolean	$generating	are we generating the content file
	*
	* @return void
	* @access public
	*/
	public function paint(Bodycopy_Table $asset, $editing=FALSE, $generating=FALSE)
	{

		$table_rows = $asset->attr('rows');
		$table_attributes = $asset->attr('attributes');

		$attribute_list = '';
		$bgimage = '';

		for (reset($table_attributes); $name = key($table_attributes); next($table_attributes)) {
			$val = $table_attributes[$name];
			if ($val == '') continue;
			if ($name == 'disable_keywords') continue;
			if ($name == 'background') {
				$file = $GLOBALS['SQ_SYSTEM']->am->getAsset($val);
				if ($file->id) $bgimage = $file->getURL();
				continue;
			}
			$attribute_list .= ' '.$name.'="'.str_replace('"', '&quot;', $val).'"';
		}

		// Now add the ID of the table
		$val = clean_div_attribute($asset->attr('name'));
		if (!empty($val)) {
			$attribute_list .= ' '.'id="'.$val.'"';
		}

		echo '<table ';
		echo "{$attribute_list}";
		echo '>';

		for ($i = 0; $i < count($table_rows); $i++) {
			$attribute_list = '';
			foreach ($table_rows[$i]['attributes'] as $name => $val) {
				if ($val == '') continue;
				$attribute_list .= ' '.$name.'="'.str_replace('"', '&quot;', $val).'"';
			}

			echo "\t<tr{$attribute_list}>\n";
			// now print the cell contents
			for ($x = 0; $x < count($table_rows[$i]['cells']); $x++) {
				$table_rows[$i]['cells'][$x]->paint($asset, '', $editing, $generating);
			}
			echo "\t</tr>\n";
		}

		echo "</table>\n";

		return TRUE;

	}//end paint()


	/**
	* Paint the interface for editing this table
	*
	* @param Bodycopy_Table		$asset	the table whose interface we are painting
	* @param Backend_Outputter	$o		the Backend Outputter class
	* @param string				$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	public function paintContainer(Bodycopy_Table $asset, Backend_Outputter $o, $prefix)
	{
		if (!parent::paintContainer($asset, $o, $prefix)) {
			return FALSE;
		}

		$table_rows = $asset->attr('rows');
		$table_attributes = $asset->attr('attributes');

		$attribute_list = '';
		$popup_attribute_list = '';
		$bgimage = '';
		$dir_attribute = '';

		if ($asset->deprecated === TRUE) {
			$GLOBALS['SQ_SYSTEM']->printDeprecatedWarning($asset);
		}

		if (!empty($table_attributes)) {
			for (reset($table_attributes); $name = key($table_attributes); next($table_attributes)) {
				$val = $table_attributes[$name];
				if ($val == '') continue;
				if ($name == 'background') {
					$file = $GLOBALS['SQ_SYSTEM']->am->getAsset($val);
					if ($file->id) {
						$popup_attribute_list .= ' '.$name.'="'.str_replace('"', '&quot;', $file->attr('name')).'"';
					}
					$bgimage = $file->getURL();
					continue;
				}
				$popup_attribute_list .= ' '.$name.'="'.str_replace('"', '&quot;', $val).'"';
				// if this is the width and the value isn't a percentage,
				// then add 20 to it for the icons in the left column
				if ($name == 'width' && !strstr($val, '%')) {
					$val = (int) $val + 20;
				}
				if ($name == 'dir') $dir_attribute = $val;
				$attribute_list .= ' '.$name.'="'.str_replace('"', '&quot;', $val).'"';
			}
		}

		// set the tmp variable storing dir attribute of whole table
		if (!empty($dir_attribute)) {
			$this->_tmp['dir_attr'] = $dir_attribute;
		}

		// need to find out if there's any colspans used at all. We can't handle
		// column reshuffles if there is.
		$colspans_in_row = FALSE;
		for ($i = 0; $i < count($table_rows); $i++) {
			for ($x = 0; $x < count($table_rows[$i]['cells']); $x++) {
				if ($table_rows[$i]['cells'][$x]->colspan() > 1) {
					$colspans_in_row = TRUE;
					break;
				}
			}
			if ($colspans_in_row) break;
		}

		// require library functions needed for drawing the bodycopy
		$type_info = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo('bodycopy');

		?>
		<table<?php echo $attribute_list?> class="sq-edit-bodycopy-section sq-bodycopy-container-edit-table-wrapper" background="<?php echo $bgimage?>">

		<?php

		// For each defined cell, we're going to draw a special cell table
		// with all the extra stuff around it. This lets each cell have
		// proper row and colspan stuff in the editing interface.
		$num_rows = count($table_rows);
		for ($i = 0; $i < $num_rows; $i++) {
			$attribute_list = '';
			foreach ($table_rows[$i]['attributes'] as $name => $val) {
				if ($val == '') continue;
				$attribute_list .= ' '.$name.'="'.str_replace('"', '&quot;', $val).'"';
			}
			?>

			<tr<?php echo $attribute_list?>>
				<?php
				$num_cols = count($table_rows[$i]['cells']);
				for ($x = 0; $x < $num_cols; $x++) {
					$cell = $table_rows[$i]['cells'][$x];
					?>
					<td style="vertical-align: top" colspan="<?php echo $cell->colspan(); ?>">
						<?php
						// when painting a cell, it needs to know if there's more than 1 row or column too for drawing
						// the delete row/col buttons
						$this->_paintEditingCell($asset, $prefix, $x, $i, $table_rows[$i]['cells'][$x], $num_cols > 1, $num_rows > 1, !$colspans_in_row);
						?>
					</td>
					<?php
				}//end for (each cell)
				?>
			</tr>

			<?php
		}//end for

		?>
		</table>
		<?php

		return TRUE;

	}//end paintContainer()


	/**
	* Output a single cell from the table. Checks the cell's position in the
	* table and draws extra icons as needed (ie the reorder columns/rows,
	* table properties etc).
	*
	* the editing cells are made up of 4 regions;
	*	- Top left that is blank except for the first cell which contains a
	*	  table editing icon
	*	- Top right which contains editing icons for the cell and column
	*	- Bottom left which contains row editing icons
	*	- Bottom right which contains the actual cell contents.
	*
	* @param Bodycopy_Table	$asset		The parent table asset being edited
	* @param string			$prefix		The prefix for this asset in a form
	* @param int			$x			The x coordinate of this cell
	* @param int			$i			The y coordinate of this cell
	* @param int			&$cell		The cell being drawn.
	* @param boolean		$multiCol	Are there more than 1 cols in the table?
	* @param boolean		$multiRow	Are there more than 1 rows in the table?
	* @param boolean		$colReorder	Are we allowed to reorder columns? (colspan issues).
	*
	* @return void
	* @access private
	*/
	protected function _paintEditingCell(Bodycopy_Table $asset, $prefix, $x, $i, &$cell, $multiCol, $multiRow, $colReorder)
	{
		$attribute_list = '';
		$table_rows = $asset->attr('rows');
		$dir_attribute = '';

		foreach ($table_rows[$i]['attributes'] as $name => $val) {
			if ($val == '') continue;
			$attribute_list .= ' '.$name.'="'.str_replace('"', '&quot;', $val).'"<br/>';
			if ($name == 'dir' && !empty($val)) {
				$dir_attribute = $val;
			}
		}

		// set the dir attribute to one inherited from the whole table if there is no specified for row
		if (empty($dir_attribute) && isset($this->_tmp['dir_attr'])) {
			$dir_attribute = $this->_tmp['dir_attr'];
		}

		?>
		<table class="sq-bodycopy-container-edit-table-table">
		<tbody>
		<tr>
		<?php
		// only print out the table prop edit icons if this is the first cell in the
		// row and first row..
		if ($x == 0) {
		?>
			<td id="<?php echo $prefix.'_table_'.$asset->id;?>" >
		<?php
			if (($x+$i) == 0) {
				$popup_attribute_list = '';
				$table_attributes = $asset->attr('attributes');
				if (!empty($table_attributes)) {
					foreach ($table_attributes as $name => $val) {
						if ($val == '') continue;
						if ($name == 'background') continue;
						$popup_attribute_list .= ' '.$name.'="'.str_replace('"', '&quot;', $val).'"';

					}
				}

				$public_userid = $GLOBALS['SQ_SYSTEM']->am->getSystemAssetid('public_user');

				$public_read = FALSE;
				if ($asset->status == SQ_STATUS_LIVE && $asset->readAccess(Array($public_userid))) {
					$public_read = TRUE;
				}
				$status_name = get_status_description($asset->status);
				$status_img = get_asset_status_icon($asset->status);
				$desc = '<table class="sq-toolbox-properties-table">';
				$desc .= '<tr><td>'.translate('Status').'</td><td>'.$status_img.''.get_status_description($asset->status).'</td><tr>';

				$desc .= '<tr><td>'.translate('Public').'</td><td>'.($public_read ? translate('Yes') : translate('No')).'</td><tr>';



				$desc .= '<tr><td>'.(($popup_attribute_list) ? translate('Current Properties').'</td><td>'.$popup_attribute_list : '').'</td><tr>';

				$desc .= '</table>';
				Bodycopy_Edit_Fns::printBodycopyIcon('bodycopy_edit_table_properties(\''.$prefix.'\', '.$asset->id.', '.(($asset->status & SQ_SC_STATUS_SAFE_EDITING) ? 'false' : 'true').');', sprintf(translate('Edit %s\'s Properties'), $asset->attr('name')), $desc, 'table_properties', '', 'class="sq-icon"');


			} else {
				echo '&nbsp;';
			}
		?>
		</td>
		<?php
		}//end if
		?>

		<td style="min-width: 70px;" id="<?php echo $prefix.'_cell_'.$asset->id.'_'.$i.'_'.$x; ?>">
		<!-- These next 3 icons float across to the right side -->
		<span style="float: right;">
		<?php

		// prints backend status icons, e.g. htmltidy status
		$cell->paintCellBackendStatusIcons($asset, $prefix, $asset->id, $i, $x);

		if ($i==0) {
			if (($x+$i) == 0 && $multiCol && $colReorder) {
				Bodycopy_Edit_Fns::printBodycopyIcon("bodycopy_edit_table_col_order('$prefix', $asset->id);", sprintf(translate('Reorder %s\'s Columns?'), $asset->attr('name')), '', 'reorder', '['.translate('Reorder Columns').']');



			}

			// if there's more than one row, and we're on the first row, let the col be deleted.
			if ($multiCol) {
				Bodycopy_Edit_Fns::printBodycopyIcon("bodycopy_delete_table_col('$prefix', $asset->id, $x);", sprintf(translate('Delete Column %s?'), ($x + 1)), '', 'delete', '['.translate('Delete Column').']');


			}

			Bodycopy_Edit_Fns::printBodycopyIcon("bodycopy_insert_table_col('$prefix', $asset->id, $x, false);", translate('Insert New Column on Right'), '', 'add', '['.translate('Add Column').' >>]');


		}

		?> </span> <?php

		if ($i==0) {
			Bodycopy_Edit_Fns::printBodycopyIcon("bodycopy_insert_table_col('$prefix', $asset->id, $x, true);", translate('Insert New Column on Left'), '', 'add', '[<< '.translate('Add Column').']');


		}
		$cell->setDirAttr($dir_attribute);
		$cell->paintCell($asset, $prefix, $asset->id, $i, $x);

		?>
		</td>
		</tr>
		<tr <?php echo $attribute_list?>>
		<?php
		if ($x == 0) {
			?>
			<td id="<?php echo $prefix.'_row_'.$asset->id.'_'.$i?>"  nowrap="nowrap">
			<?php
			if ($i == 0 && $multiRow) {
				Bodycopy_Edit_Fns::printBodycopyIcon("bodycopy_edit_table_row_order('$prefix', $asset->id);", sprintf(translate('Reorder %s\'s Rows?'), $asset->attr('name')), '', 'reorder', '['.translate('Reorder Rows').']', ' class="sq-icon-vertical"');


				echo '<br/>';
			}
			Bodycopy_Edit_Fns::printBodycopyIcon("bodycopy_insert_table_row('$prefix', $asset->id,$i, true);", translate('Insert a New Row Above'), '', 'add', '[^ '.translate('Add Row').']', ' class="sq-icon-vertical"');



			echo '<br/>';
			Bodycopy_Edit_Fns::printBodycopyIcon("bodycopy_edit_table_row_properties('$prefix', $asset->id,$i);", sprintf(translate('Edit row %s Properties'), ($i + 1)), (($attribute_list) ? translate('Current Properties').':<br>'.$attribute_list : ''), 'row_properties', '['.translate('Edit Row Properties').']', ' class="sq-icon-vertical"');



			if ($multiRow) {
				echo '<br/>';
				Bodycopy_Edit_Fns::printBodycopyIcon("bodycopy_delete_table_row('$prefix', $asset->id, $i);", sprintf(translate('Delete row %s?'), ($i + 1)), '', 'delete', '['.translate('Delete Row').']', ' class="sq-icon-vertical"');


			}
			echo '<br/>';
			Bodycopy_Edit_Fns::printBodycopyIcon("bodycopy_insert_table_row('$prefix', $asset->id, $i, false);", translate('Insert a New Row Below'), '', 'add', '[v '.translate('Add Row').']', ' class="sq-icon-vertical"');


			?>
		</td>
		<?php
		}
			// a cell paints its own <td> tag
			$cell->paint($asset, $prefix);
		?>
		</tr>
		</tbody>
		</table>

		<?php

	}//end _paintEditingCell()


	/**
	* Process the interface for editing this table
	*
	* @param Bodycopy_Table		$asset		the table whose interface we are painting
	* @param Backend_Outputter	$o			the Backend Outputter class
	* @param string				$prefix		prefix for the form element
	* @param string				$bc_action	a global bodycopy action to perform (eg insert_table_row)
	* @param string				$bc_name	the name of the bodycopy that submitted the action
	* @param array				$bc_data	an array of data needed to perform the action
	* @param array				$bc_saved	any saved changes that were performed without committing
	*
	* @return boolean
	* @access public
	*/
	public function processContainer(Bodycopy_Table $asset, Backend_Outputter $o, $prefix, $bc_action, $bc_name, Array $bc_data, Array $bc_saved)
	{
		$updated = parent::processContainer($asset, $o, $prefix, $bc_action, $bc_name, $bc_data, $bc_saved);

		// update the rows
		$rows_updated = FALSE;
		$rows = $asset->attr('rows');


		for ($rowid = 0; $rowid < count($rows); $rowid++) {
			if (!empty($bc_saved[$prefix][$asset->id]['rows'][$rowid]['attributes'])) {
				// save any changed row attributes
				foreach ($bc_saved[$prefix][$asset->id]['rows'][$rowid]['attributes'] as $name => $value) {
					$name = strtolower($name);
					if (!isset($rows[$rowid]['attributes'][$name]) || $rows[$rowid]['attributes'][$name] != $value) {
						$rows[$rowid]['attributes'][$name] = $value;
						$rows_updated = TRUE;
					}
				}
			}

			// only update if we are the table row that submitted the form
			if ((!empty($bc_data['rowid']) && $bc_data['rowid'] == $rowid && $prefix == $bc_name) || isset($bc_saved[$prefix][$asset->id]['rows'][$rowid])) {

				$counter = 0;
				$colspan_todo = Array();

				// check to see if any cells from this row need their colspans changed
				if (isset($bc_saved[$prefix][$asset->id]['rows'][$rowid]['cells'])) {
					foreach ($bc_saved[$prefix][$asset->id]['rows'][$rowid]['cells'] as $cellid => $cell_data) {

						if (isset($rows[$rowid]['cells'][$cellid])) {

							$current_span = $rows[$rowid]['cells'][$cellid]->colspan();
							$colspaning   = $cell_data['attributes']['colspan'];

							// if the current span is '1' and the new span is '' then a cell is being created
							// so we dont want to do anything
							if ($current_span == 1 && trim($colspaning) == '') {
								$colspaning = $current_span;
							}

							// if the user left colspan blank, dont change anything
							if (trim($colspaning) == '') {
								$colspaning = $current_span;
								$bc_saved[$prefix][$asset->id]['rows'][$rowid]['cells'][$cellid]['attributes']['colspan'] = $current_span;
							}

							// if the user entered a negative colspan - or '0', we can assume they want a colspan of '1'
							if (intval($colspaning) <= 0) {
								$colspaning = 1;
								$bc_saved[$prefix][$asset->id]['rows'][$rowid]['cells'][$cellid]['attributes']['colspan'] = 1;
							}
						} else {
							$colspaning = 0;
							$current_span = 0;
						}

						if ($colspaning != $current_span) {
							// put this colspan into a todo list that we will do later
							$colspan_todo[$counter]['cell']			= $cellid;
							$colspan_todo[$counter]['colspaning']	= $colspaning;
							$colspan_todo[$counter]['current_span'] = $current_span;
							$counter++;
						}
					}//end foreach

					// doing the colspan thing
					if (count($colspan_todo)) {

						foreach ($colspan_todo as $colspan) {

							$cellid = $colspan['cell'];
							$colspaning = $colspan['colspaning'];
							$current_span = $colspan['current_span'];

							// if the current span is '1' and the new span is '' then a cell is being created
							// so we dont want to do anything
							if ($current_span == 1 && trim($colspaning) == '') {
								$colspaning = $current_span;
							}

							// if the user left colspan blank, dont change anything
							if (trim($colspaning) == '') {
								$colspaning = $current_span;
								$bc_saved[$prefix][$asset->id]['rows'][$rowid]['cells'][$cellid]['attributes']['colspan'] = $current_span;
							}

							// if the user entered a negative colspan - or '0', we can assume they want a colspan of '1'
							if (intval($colspaning) <= 0) {
								$colspaning = 1;
								$bc_saved[$prefix][$asset->id]['rows'][$rowid]['cells'][$cellid]['attributes']['colspan'] = 1;
							}


							if ($colspaning != $current_span) {

								// make sure there are sufficient cells to create the colspan
								$spanable_cells = count($rows[$rowid]['cells']) - ($cellid +1);
								$needed_cells = ($colspaning - $current_span);

								if ($spanable_cells < $needed_cells) {
									// dont end up changing the colspan at all
									$bc_saved[$prefix][$asset->id]['rows'][$rowid]['cells'][$cellid]['attributes']['colspan'] = $current_span;
								} else {
									if ($colspaning > $current_span && $spanable_cells >= 1) {
										// check for more colspans first
										$colspan_error = 0;
										for ($i = ($cellid + 1); $i <= ($cellid + ($colspaning - $current_span)); $i++) {
											if ($rows[$rowid]['cells'][$i]->colspan() > 1) {
												$colspan_error = 1;
											}
										}
										if ($colspan_error == 0) {
											for ($i = ($cellid + 1); $i <= ($cellid + ($colspaning - $current_span)); $i++) {
												if ($this->deleteCell($asset, $rows, $rowid, ($cellid+1), TRUE)) {
													$rows_updated = TRUE;
												} else {
													// deleteCell failed
													// dont end up changing the colspan at al
													$bc_saved[$prefix][$asset->id]['rows'][$rowid]['cells'][$cellid]['attributes']['colspan'] = $current_span;
												}
											}
										} else {
											// we are trying to colspan into another colspan
											// dont end up changing the colspan at all
											$bc_saved[$prefix][$asset->id]['rows'][$rowid]['cells'][$cellid]['attributes']['colspan'] = $current_span;
										}
									} else if ($colspaning < $current_span) {
										// decreasing the colspan

										for ($i = 0; $i < ($current_span - $colspaning); $i++) {
											if ($this->insertCell($asset, $rows, $rowid, $cellid, FALSE, TRUE)) {
												$rows_updated = TRUE;
											} else {
												// insertCell failed
												// dont end up changing the colspan at al
												$bc_saved[$prefix][$asset->id]['rows'][$rowid]['cells'][$cellid]['attributes']['colspan'] = $current_span;
											}
										}
									}
								}//end else

							}//end if

						}//end foreach

					}//end if we have colspans todo

				}//end if changing colspans

			}//end if we are the row that submitted the form

			// now update the cells
			for ($i = 0; $i < count($rows[$rowid]['cells']); $i++) {
				if ($rows[$rowid]['cells'][$i]->processCell($asset, $prefix, $asset->id, $rowid, $i, $bc_saved)) {
					$rows_updated = TRUE;
				}
			}

			// only set the rows attributes if any of the rows (or cells within) have changed
			if ($rows_updated) {
				$asset->setAttrValue('rows',$rows);
				$updated = TRUE;
			}

		}//end for all rows


		// check if there is anything for this table to do
		$skip_table_editing = FALSE;
		if (empty($bc_data) || !isset($bc_data['tableid']) || $bc_data['tableid'] != $asset->id) {
			// no bodycopy data stuff relating to this table, so check the saved data
			if (empty($bc_saved[$asset->id])) {
				// there is nothing for us here
				$skip_table_editing = TRUE;
			}
		}


		// process any special actions if we are the  table that submitted the form
		if (!$skip_table_editing && !empty($bc_data) && $bc_data['tableid'] == $asset->id && $bc_name == $prefix) {
			switch ($bc_action) {
				case 'insert_table_row' :
					if ($this->insertRow($asset, $bc_data['rowid'], $bc_data['before'])) {
						$updated = TRUE;
					}
				break;

				case 'delete_table_row' :
					if ($this->deleteRow($asset, $bc_data['rowid'])) {
						$updated = TRUE;
					}
				break;

				case 'edit_table_row_order' :
					if ($this->reorderRows($asset, $bc_data['row_order'])) {
						$updated = TRUE;
					}
				break;

				case 'insert_table_column' :
					if ($this->insertColumn($asset, $bc_data['colid'], $bc_data['before'])) {
						$updated = TRUE;
					}
				break;

				case 'delete_table_column' :
					if ($this->deleteColumn($asset, $bc_data['colid'])) {
						$updated = TRUE;
					}
				break;

				case 'edit_table_col_order' :
					if ($this->reorderColumns($asset, $bc_data['col_order'])) {
						$updated = TRUE;
					}
				break;
			}//end switch
		}//end if - we are the table that submitted the form

		if ($updated) $this->generateContentFile($asset);
		$this->_contentsUpdated($asset);
		return $updated;

	}//end processContainer()


	/**
	* Insert a new table row before or after the passed rowid
	*
	* @param Bodycopy_Table	$asset	the table object
	* @param int			$rowid	the id of the row that we are inserting before/after
	* @param boolean		$before	insert before the passed rowid?
	*
	* @return boolean
	* @access public
	*/
	public function insertRow(Bodycopy_Table $asset, $rowid, $before=TRUE)
	{
		$rows = $asset->attr('rows');
		$rowid = intval($rowid);

		// if the row is greater than the number of rows we have set it to the last on
		if ($rowid > count($rows) - 1) {
			$rowid = count($rows) - 1;
		}
		if ($rowid < 0) $rowid = 0;
		$compareid = $rowid;

		// for the moment we will just use the number of columns that the one we are inserting
		// before/after has
		$num_cols = $this->maxCols($rows[$compareid], FALSE);

		// if they want to insert after the passed rowid what
		// they really mean is to insert before the next rowid
		if (!$before) $rowid++;

		// move all the rows up one position
		for ($i = count($rows); $i > $rowid; $i--) {
			$rows[$i] = $rows[$i - 1];
			for ($x = 0; $x < count($rows[$i]['cells']); $x++) {
				$rows[$i]['cells'][$x]->updateContentType($asset, $i, $x);
			}
		}

		if ($rowid <= $compareid) $compareid++;

		// now add the row
		$rows[$rowid]				= Array();
		$rows[$rowid]['attributes'] = Array();
		$rows[$rowid]['cells']		= Array();

		// create the cells and set their properties to
		// the properties of the cells above/below
		for ($i = 0; $i < count($rows[$compareid]['cells']); $i++) {
			$rows[$rowid]['cells'][$i] = new Bodycopy_Table_Cell();
			foreach ($rows[$compareid]['cells'][$i]->attributes as $name => $val) {
				$rows[$rowid]['cells'][$i]->setAttribute($name,$val);
			}
			$rows[$rowid]['cells'][$i]->setContentType($asset, 'content_type_wysiwyg', $rowid, $i);
		}

		$asset->setAttrValue('rows',$rows);
		return TRUE;

	}//end insertRow()


	/**
	* Delete the passed table row from the array
	*
	* @param Bodycopy_Table	$asset	the table object
	* @param int			$rowid	the id of the row to delete
	*
	* @return boolean
	* @access public
	*/
	public function deleteRow(Bodycopy_Table $asset, $rowid)
	{
		$rows = $asset->attr('rows');

		// make sure it exists
		if (isset($rows[$rowid]) && count($rows) > 1) {

			// delete all the cells in the row so they can clean up
			for ($i = 0; $i < count($rows[$rowid]['cells']); $i++) {
				$rows[$rowid]['cells'][$i]->delete($asset);
			}

			// move all the rows down one position
			for ($i = $rowid; $i < count($rows) - 1; $i++) {
				// let the cells know that they are moving to a new rowid
				$rows[$i] = $rows[$i + 1];
				for ($x = 0; $x < count($rows[$i]['cells']); $x++) {
					$rows[$i]['cells'][$x]->updateContentType($asset, $i, $x);
				}
			}

			// remove the last element
			array_pop($rows);

			$asset->setAttrValue('rows',$rows);
			return TRUE;

		} else if (count($rows) == 1) {
			return FALSE;
		}

		return FALSE;

	}//end deleteRow()


	/**
	* Reorder the rows of this table
	*
	* @param Bodycopy_Table	$asset		the table object
	* @param array			$row_order	an array of the structure (new_place => old_place)
	*
	* @return boolean
	* @access public
	*/
	public function reorderRows(Bodycopy_Table $asset, Array $row_order)
	{
		$rows = $asset->attr('rows');

		// if there ain't no changes, die
		$changes = FALSE;
		foreach ($row_order as $new_place => $old_place) {
			$changes |= ($new_place != $old_place);
		}
		if (!$changes) return FALSE;

		// if there is a conflict in the number of rows, die
		if (count($rows) != count(array_unique($row_order))) {
			return FALSE;
		}

		$new_array = Array();

		for ($i = 0; $i < count($row_order); $i++) {
			for ($x = 0; $x < count($rows[$i]['cells']); $x++) {
				$rows[$i]['cells'][$x]->updateContentType($asset, $row_order[$i], $x);
			}
			$new_array[$i] = $rows[$row_order[$i]];
		}

		// now assign the new ordered array
		$rows = $new_array;

		$asset->setAttrValue('rows',$rows);
		return TRUE;

	}//end reorderRows()


	/**
	* Reorder the columns of this table
	*
	* @param Bodycopy_Table	$asset		the table object
	* @param array			$col_order	an array of the structure (new_place => old_place)
	*
	* @return boolean
	* @access public
	*/
	public function reorderColumns(Bodycopy_Table $asset, Array $col_order)
	{
		// if there ain't no changes, die
		$changes = FALSE;
		foreach ($col_order as $new_place => $old_place) {
			$changes |= ($new_place != $old_place);
		}
		if (!$changes) return FALSE;

		$rows = $asset->attr('rows');

		for ($r = 0; $r < count($rows); $r++) {
			// if there is a conflict in the number of cells, die
			if (count($rows[$r]['cells']) != count(array_unique($col_order))) {
				return FALSE;
			}

			$new_array = Array();
			for ($i = 0; $i < count($col_order); $i++) {
				$rows[$r]['cells'][$i]->updateContentType($asset, $r, $col_order[$i]);
				$new_array[$i] = $rows[$r]['cells'][$col_order[$i]];
			}
			// now assign the new ordered array
			$rows[$r]['cells'] = $new_array;
		}

		$asset->setAttrValue('rows',$rows);
		return TRUE;

	}//end reorderColumns()


	/**
	* Insert a new column in all rows, before or after the passed columnID
	*
	* @param Bodycopy_Table	$asset	the table object
	* @param int			$cellid	the ID of the column that we are inserting before/after
	* @param boolean		$before	insert before the passed cellID?
	*
	* @return boolean
	* @access public
	*/
	public function insertColumn(Bodycopy_Table $asset, $cellid, $before=TRUE)
	{
		$rows = $asset->attr('rows');

		// we dont know how to do this ourselves, so just get
		// every row to insert a new cell and it can handle checking
		for ($r = 0; $r < count($rows); $r++) {
			if (!$this->insertCell($asset, $rows, $r, $cellid, $before)) {
				return FALSE;
			}
		}

		$asset->setAttrValue('rows',$rows);
		return TRUE;

	}//end insertColumn()


	/**
	* Deletes a column from all rows, before or after the passed columnID
	*
	* @param Bodycopy_Table	$asset	the table object
	* @param int			$cellid	the ID of the column that we are deleting
	*
	* @return boolean
	* @access public
	*/
	public function deleteColumn(Bodycopy_Table $asset, $cellid)
	{
		$rows = $asset->attr('rows');

		// we dont know how to do this ourselves, so just get
		// every row to delete the cell and it can handle checking
		for ($r = 0; $r < count($rows); $r++) {
			if (!$this->deleteCell($asset, $rows, $r, $cellid)) {
				return FALSE;
			}
		}

		$asset->setAttrValue('rows',$rows);
		return TRUE;

	}//end deleteColumn()


	/**
	* Insert a new table cell before or after the passed cellID
	*
	* @param Bodycopy_Table	$asset		the table object
	* @param array			&$rows		a reference to the rows attribute of the table
	* @param int			$rowid		the ID of the row we are inserting into
	* @param int			$cellid		the ID of the cell we are inserting before/after
	* @param boolean		$before		insert the cell before the passed cellID?
	* @param boolean		$colspan	ignore colspans in this row
	*
	* @return boolean
	* @access public
	*/
	public function insertCell(Bodycopy_Table $asset, Array &$rows, $rowid, $cellid, $before=TRUE, $colspan=FALSE)
	{
		if (!$colspan) {

			// check for colspan's in this row - colspan's will muck things up
			$colspan_error = 0;
			$colspaning = 0;
			for ($i = 0; $i < count($rows[$rowid]['cells']); $i++) {
				if ($rows[$rowid]['cells'][$i]->colspan() > 1) {
					$colspan_error = 1;
				}
			}
			if ($colspan_error == 1) {
				// work out the correct cellid for the id passed in
				$real_cellid = 0;
				for ($i = 0; $i < count($rows[$rowid]['cells']); $i++) {
					if ($colspaning >= $cellid) break;
					$colspaning += intval($rows[$rowid]['cells'][$i]->colspan());
					if (intval($rows[$rowid]['cells'][$i]->colspan()) == 0) {
						$colspaning++;
					}
					if ($colspaning <= $cellid) {
						// havn't found the cell yet
						$real_cellid++;
					}
				}

				// lets try and find out how many cells are before the cell we are looking at
				$colspaning = 0;
				for ($i = 0; $i < $real_cellid; $i++) {
					$colspaning += intval($rows[$rowid]['cells'][$i]->colspan());
					if (intval($rows[$rowid]['cells'][$i]->colspan()) == 0) {
						$colspaning++;
					}
				}

				if ($real_cellid < 0) $real_cellid = 0;

				// now we have the real cellid, so we will be increasing the colspan of this cell
				// (if it has one) unless we are supposed to be inserting AFTER this cell or BEFORE
				// this cell
				// so lets check shall we?

				$dont_do_this = FALSE;

				// if the real cellid is found to be the same as the cellid, we have a special
				// situation where we have found the first cell in a colspan that has no
				// colspans before it on the row
				if ($cellid == $real_cellid) {
					if ($before) $dont_do_this = TRUE;
				} else {
					if ((($rows[$rowid]['cells'][$real_cellid]->colspan() + $colspaning) == ($cellid + 1)) && !$before) {
						$dont_do_this = TRUE;
					}
				}

				if ($cellid == 0 && $before) {
					$dont_do_this = TRUE;
				}

				$cellid = $real_cellid;
				if ($cellid < 0) $cellid = 0;

				// if this cell has colspan's already, we can just increase the colspan by 1
				if ($rows[$rowid]['cells'][$cellid]->colspan() > 1 && !$dont_do_this) {
					$colspaning = $rows[$rowid]['cells'][$cellid]->colspan() + 1;
					$rows[$rowid]['cells'][$cellid]->setAttribute('colspan', $colspaning);
					return TRUE;
				}

			}//end if colspan_error
		}//end if not colspan

		// if the cell is greater than the number of cells we have set it to the last on
		if ($cellid > count($rows[$rowid]['cells']) - 1) {
			$cellid = count($rows[$rowid]['cells']) - 1;
		}
		if ($cellid < 0) $cellid = 0;
		$compareid = $cellid;

		// if they want to insert after the passed cell what
		// they really mean is to insert before the next cell
		if (!$before) $cellid++;

		// move all the cells up one position
		for ($i = count($rows[$rowid]['cells']); $i > $cellid; $i--) {
			$rows[$rowid]['cells'][$i] = $rows[$rowid]['cells'][$i - 1];
			$rows[$rowid]['cells'][$i]->updateContentType($asset, $rowid, $i);
		}

		if ($cellid <= $compareid) $compareid++;

		// now add the cell
		$rows[$rowid]['cells'][$cellid] = new BodyCopy_Table_Cell();

		// now that the cell exists, set its properties to
		// the properties of the cell to the left/right
		foreach ($rows[$rowid]['cells'][$compareid]->attributes as $name => $val) {
			if (strtolower($name) == 'colspan') continue;
			$rows[$rowid]['cells'][$cellid]->setAttribute($name,$val);
		}
		$rows[$rowid]['cells'][$cellid]->setContentType($asset, 'content_type_wysiwyg', $rowid, $cellid);

		return TRUE;

	}//end insertCell()


	/**
	* Delete the passed table cell from a row
	*
	* @param Bodycopy_Table	&$table		the table object
	* @param array			&$rows		a reference to the rows attribute of the table
	* @param int			$rowid		the ID of the row we are deleting from
	* @param int			$cellid		the ID of the cell we are deleting
	* @param boolean		$colspan	ignore colspans in this row
	*
	* @return boolean
	* @access public
	*/
	public function deleteCell(Bodycopy_Table $table, Array &$rows, $rowid, $cellid, $colspan=FALSE)
	{
		if (!$colspan) {

			// check for colspan's in this row - colspan's will muck things up
			$colspan_error = 0;
			$colspaning = 0;
			for ($i = 0; $i < count($rows[$rowid]['cells']); $i++) {
				if ($rows[$rowid]['cells'][$i]->colspan() > 1) {
					$colspan_error = 1;
				}
			}

			if ($colspan_error == 1) {
				// work out the correct cellid for the id passed in
				$real_cellid = 0;
				for ($i = 0; $i < count($rows[$rowid]['cells']); $i++) {
					if ($colspaning >= $cellid) break;
					$colspaning += intval($rows[$rowid]['cells'][$i]->colspan());
					if (intval($rows[$rowid]['cells'][$i]->colspan()) == 0) {
						$colspaning++;
					}
					if ($colspaning <= $cellid) {
						// havn't found the cell yet
						$real_cellid++;
					}
				}

				$cellid = $real_cellid;

				// if this cell has colspan's already, we can just reduce the colspan by 1
				if ($rows[$rowid]['cells'][$cellid]->colspan() > 1) {
					$colspaning = $rows[$rowid]['cells'][$cellid]->colspan() - 1;
					$rows[$rowid]['cells'][$cellid]->setAttribute('colspan', $colspaning);
					return TRUE;
				}
			}
		}//end if not colspanning

		// make sure cell exists
		if ($rows[$rowid]['cells'][$cellid] && count($rows[$rowid]['cells']) > 1) {
			// do any clean up
			if (!$rows[$rowid]['cells'][$cellid]->delete($table, $rowid, $cellid)) {
				return FALSE;
			}

			// move all the cells down one position
			for ($i = $cellid; $i < count($rows[$rowid]['cells']) - 1; $i++) {
				$rows[$rowid]['cells'][$i] = $rows[$rowid]['cells'][$i + 1];
				$rows[$rowid]['cells'][$i]->updateContentType($table, $rowid, $i);
			}

			// remove the last element
			array_pop($rows[$rowid]['cells']);
			return TRUE;

		} else if (count($this->cells) == 1) {
			return FALSE;
		}

	}//end deleteCell()


	/**
	* Gets the maximum number of columns out of all the rows in this table
	*
	* @param array		$table_rows		the rows attribute of the table
	* @param boolean	$add_colspans	add colspan value while counting
	*
	* @return int
	* @access public
	*/
	public function maxCols($table_rows, $add_colspans=TRUE)
	{
		$max = 0;
		for ($i = 0; $i < count($table_rows); $i++) {
			$num = 0;
			if (empty($table_rows[$i]['cells'])) continue;
			for ($x = 0; $x < count($table_rows[$i]['cells']); $x++) {
				if ($add_colspans) {
					$num += $table_rows[$i]['cells'][$x]->colspan();
				} else {
					$num++;
				}
			}
			if ($max < $num) $max = $num;
		}
		return (int)$max;

	}//end maxCols()


}//end class

?>
