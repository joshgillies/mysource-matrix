<?php

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';
require_once(dirname(__FILE__).'/bodycopy_table_cell.inc');
require_once SQ_FUDGE_PATH.'/var_serialise/var_serialise.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';

/**
* Bodycopy_Table_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Resolve::packages::cms
*/
class Bodycopy_Table_Edit_Fns extends Asset_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function Bodycopy_Table_Edit_Fns()
	{
		$this->Asset_Edit_Fns();
	}


	/**
	* Sets an attribute value in the attributes var
	* Internal checking of these attributes has to go here as they
	* will not be checked by the system
	*
	* @param array	&$attributes	a reference to the attributes var of the BodyCopy_Table object
	* @param string	$name			the name of the attribute
	* @param string	$value			the value of the attribute
	*
	* @return boolean
	* @access public
	*/
	function set_attribute(&$attributes, $name, $value)
	{
		$name = strtolower($name);
		if (empty($attributes[$name])) $attributes[$name] = '';
		if ($attributes[$name] == $value) return false;
		$attributes[$name] = $value;
		return true;
	}

	
	/**
	* Prints the JavaScript needed by the backend functions for this table
	*
	* @param object Asset	&$asset	the table object being printed
	* @param string			$prefix	prefix for form fields
	*
	* @return void
	* @access public
	*/
	function paintBackendJsData(&$asset, $prefix)
	{
		$table_rows = $asset->attr('rows');
		$table_attributes = $asset->attr('attributes');
		$max_cols = $this->maxCols($table_rows);

		?> 
					case <?php echo $asset->id?> :

						switch (rowid) {
							<?php
							for($i = 0; $i < count($table_rows); $i++) {
								?>
								case <?php echo $i?> :
									switch (cellid) {
										<?php
										for($x = 0; $x < count($table_rows[$i]); $x++) {
											if (isset($table_rows[$i]['cells'][$x])) $table_rows[$i]['cells'][$x]->print_backend_js_data($x);
										}
										?>

										default :
											retVal["attributes"] = '<?php echo var_serialise($table_rows[$i]['attributes'], true)?>';

									}// end switch cellid
								break;
								<?php
							}
							?>

							default :
								retVal["attributes"] = '<?php echo var_serialise($table_attributes, true)?>';
								retVal["num_cols"]   = <?php echo (int) $max_cols?>;
								retVal["num_rows"]   = <?php echo count($table_rows)?>;

						}// end switch rowid

					break;
		<?php
	
	}//end paintBackendJsData()


	/**
	* Paints this table as HTML
	*
	* @param object Bodycopy_Table	$asset	the table whose interface we are painting
	*
	* @return void
	* @access public
	*/
	function paint(&$asset)
	{
		$table_rows = $asset->attr('rows');
		$table_attributes = $asset->attr('attributes');

		$attribute_list = '';
		$bgimage = '';

		for (reset($table_attributes); $name = key($table_attributes); next($table_attributes)) {
			$val = $table_attributes[$name];
			if ($val == '') continue;
			if ($name == 'background') {
				$file = &$GLOBALS['SQ_SYSTEM']->am->getAsset($val);
				if ($file->id) $bgimage = $file->getURL();
				continue;
			}
			$attribute_list .= ' '.$name.'="'.str_replace('"', '&quot;', $val).'"';
		}

		echo "\n<table{$attribute_list} background=\"$bgimage\">\n";

		for($i = 0; $i < count($table_rows); $i++) {
			$attribute_list = '';
			foreach($table_rows[$i]['attributes'] as $name => $val) {
				if ($val == '') continue;
				$attribute_list .= ' '.$name.'="'.str_replace('"', '&quot;', $val).'"';
			}

			echo "\t<tr{$attribute_list}>\n";
			// now print the cell contents
			for($x = 0; $x < count($table_rows[$i]['cells']); $x++) {
				$table_rows[$i]['cells'][$x]->paint($asset);
			}
			echo "\t</tr>\n";
		}

		echo "</table>\n";

		return true;

	}// end paint()
	
	
	/**
	* Paint the interface for editing this table
	*
	* @param object Asset				$asset	the table whose interface we are painting
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintTable(&$asset, &$o, $prefix)
	{
		$table_rows = $asset->attr('rows');
		$table_attributes = $asset->attr('attributes');

		$attribute_list = '';
		$popup_attribute_list = '';
		$bgimage = '';

		if (!empty($table_attributes)) {
			for (reset($table_attributes); $name = key($table_attributes); next($table_attributes)) {
				$val = $table_attributes[$name];
				if ($val == '') continue;
				if ($name == 'background') {
					$file = &$GLOBALS['SQ_SYSTEM']->am->getAsset($val);
					if ($file->id) $popup_attribute_list .= ' '.$name.'="'.str_replace('"', '&quot;', $file->attr('name')).'"';
					$bgimage = $file->getURL();
					continue;
				}
				$popup_attribute_list .= ' '.$name.'="'.str_replace('"', '&quot;', $val).'"';
				// if this is the width and the value isn't a percentage, 
				// then add 20 to it for the icons in the left column
				if ($name == 'width' && !strstr($val, '%')) $val = (int) $val + 20;
				$attribute_list .= ' '.$name.'="'.str_replace('"', '&quot;', $val).'"';
			}
		}

		$max_cols = $this->maxCols($table_rows);
		$colspans_in_row = false;
		for($i = 0; $i < count($table_rows); $i++) {
			for($x = 0; $x < count($table_rows[$i]['cells']); $x++) {
				if ($table_rows[$i]['cells'][$x]->colspan() > 1) {
					$colspans_in_row = true;
					break;
				}
			}
			if ($colspans_in_row) break;
		}

		?>
		<table<?php echo $attribute_list?> background="<?php echo $bgimage?>" border="0">
			<tr> 
				<td align="center" background="" id="<?php echo $prefix.'_table_'.$asset->id;?>" width="4%">
				<?php
				print_bodycopy_icon("Javascript: bodycopy_edit_table_properties('$prefix', $asset->id);", 'Edit '.$asset->attr('name').'\'s Properties', (($popup_attribute_list) ? 'Current Properties :<br>'.$popup_attribute_list : ''), 'edit_table_properties');
				?>
				</td>
		<?php
			for($i = 0; $i < $max_cols; $i++) {
			?>
				<td align="center" background="" valign="middle">
					<table border="0" cellspacing="0" cellpadding="0" width="100%">
						<tr>
							<td align="left">
							<?php
							print_bodycopy_icon("Javascript: bodycopy_insert_table_col('$prefix', $asset->id, $i, true);", 'Insert New Column on Left', '', 'insert_column_left', '', '7', '20');
							?>
							</td>
							<td align="center">
							<?php
							if ($max_cols > 1) {
								print_bodycopy_icon("Javascript: bodycopy_delete_table_col('$prefix', $asset->id, $i);", 'Delete Column '.($i + 1).' ?', '', 		'delete');
							}
							if ($i == 0) {
								if ($max_cols > 1 && (!$colspans_in_row || count($table_rows) == 1)) {
									print_bodycopy_icon("Javascript: bodycopy_edit_table_col_order('$prefix', $asset->id);", 'Reorder '.$asset->attr('name').'\'s Columns ?', '', 'reorder_cols');
								}
							}
							?>
							</td>
							<td align="right">
							<?php
							print_bodycopy_icon("Javascript: bodycopy_insert_table_col('$prefix', $asset->id, $i, false);", 'Insert New Column on Right', '', 'insert_column_right', '', '7', '20');
							?>
							</td>
						</tr>
					</table>
				</td>
			<?php
			}
		?>
			</tr>
		<?php

		$num_rows = count($table_rows);
		for($i = 0; $i < $num_rows; $i++) {
			?>
			<tr>
			<?php
			if ($i == 0) {
				?>
				<td background="" nowrap align="center" width="1%">
				<?php
					# can't reorder if only one row
					if ($num_rows > 1) {
						print_bodycopy_icon("Javascript: bodycopy_edit_table_row_order('$prefix', $asset->id);", 'Reorder '.$asset->attr('name').'\'s Rows ?', '', 'reorder_rows');
					} else {
						echo '&nbsp;';
					}
				?>
				</td>
				<?php
			} else {
			?>
				<td background="">&nbsp;</td>
			<?php
			}

			// print the buttons for the cells in the row
			for($x = 0; $x < count($table_rows[$i]['cells']); $x++) {
				$table_rows[$i]['cells'][$x]->paintCell($asset, $prefix, $asset->id, $i, $x);
			}
			?>
			</tr>
			<?php
			
			$attribute_list = '';
			foreach($table_rows[$i]['attributes'] as $name => $val) {
				if ($val == '') continue;
				$attribute_list .= ' '.$name.'="'.str_replace('"', '&quot;', $val).'"';
			}

			?> 
			<tr<?php echo $attribute_list?>>
				<td align="center" background="" id="<?php echo $prefix.'_row_'.$asset->id.'_'.$i?>" height="100%">
					<table border="0" cellspacing="0" cellpadding="0" height="100%">
						<tr height="1%">
							<td valign="top">
							<?php
							print_bodycopy_icon("Javascript: bodycopy_insert_table_row('$prefix', $asset->id, $i, true);", 'Insert a New Row Above', '', 'insert_row_above', '', '20', '7');
							?>
							</td>
						</tr>
						<tr height="100%">
							<td valign="middle">
							<?php
							print_bodycopy_icon("Javascript: bodycopy_edit_table_row_properties('$prefix', $asset->id, $i);", 'Edit Row '.($i + 1).'\'s Properties', (($attribute_list) ? 'Current Properties :<br>'.$attribute_list : ''), 'edit_row_properties');
							if ($num_rows > 1) {
								echo '<br>';
								print_bodycopy_icon("Javascript: bodycopy_delete_table_row('$prefix', $asset->id, $i);", 'Delete Row '.($i + 1).' ?', '', 'delete');
							}
							?>
							</td>
						</tr>
						<tr height="1%">
							<td valign="bottom">
							<?php
							print_bodycopy_icon("Javascript: bodycopy_insert_table_row('$prefix', $asset->id, $i, false);", 'Insert a New Row Below', '', 'insert_row_below', '', '20', '7'); 
							?>
							</td>
						</tr>
					</table>
				</td> 
			<?php
				// now print the cell contents
				for($x = 0; $x < count($table_rows[$i]['cells']); $x++) {
					$table_rows[$i]['cells'][$x]->paint($asset, $prefix);
				}
			?>
			</tr>
			<?php
		}//end for

		?>
		</table>
		<?php

		return true;

	}// end paintTable()


	/**
	* Process the interface for editing this table
	*
	* @param object BodyCopy_Table	&$asset	the table whose interface we are painting
	* @param string					$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processTable(&$asset, $prefix)
	{
		global $bodycopy_action, $bodycopy_name, $bodycopy_data, $bodycopy_saved;
		$updated = false;

		// save any changed table attributes
		if (!empty($bodycopy_saved[$prefix][$asset->id]['attributes'])) {
			$attributes_updated = false;
			$attributes = $asset->attr('attributes');
			foreach($bodycopy_saved[$prefix][$asset->id]['attributes'] as $name => $value) {
				if ($this->set_attribute($attributes, $name, $value)) $attributes_updated = true;
			}
			if ($attributes_updated) {
				$asset->setAttrValue('attributes',$attributes);
				$updated = true;
			}
		}


		// update the rows
		$rows_updated = false;
		$rows = $asset->attr('rows');
		for($rowid = 0; $rowid < count($rows); $rowid++) {
			if (!empty($bodycopy_saved[$prefix][$asset->id]['rows'][$rowid]['attributes'])) {
				// save any changed row attributes
				foreach($bodycopy_saved[$prefix][$asset->id]['rows'][$rowid]['attributes'] as $name => $value) {
					$name = strtolower($name);
					if (!isset($rows[$rowid]['attributes'][$name]) || $rows[$rowid]['attributes'][$name] != $value) {
						$rows[$rowid]['attributes'][$name] = $value;
						$rows_updated = true;
					}
				}
			}

			// only update if we are the table row that submitted the form
			if ((!empty($bodycopy_data['rowid']) && $bodycopy_data['rowid'] == $rowid && $prefix == $bodycopy_name) || isset($bodycopy_saved[$prefix][$asset->id]['rows'][$rowid])) {

				$counter = 0;
				$colspan_todo = Array();

				// check to see if any cells from this row need their colspans changed
				if (isset($bodycopy_saved[$prefix][$asset->id]['rows'][$rowid]['cells'])) {
					foreach($bodycopy_saved[$prefix][$asset->id]['rows'][$rowid]['cells'] as $cellid => $cell_data) {

						if (isset($rows[$rowid]['cells'][$cellid])) {

							$current_span = $rows[$rowid]['cells'][$cellid]->colspan();
							$colspaning   = $cell_data['attributes']['colspan'];
								
							// if the current span is '1' and the new span is '' then a cell is being created
							// so we dont want to do anything
							if ($current_span == 1 && trim($colspaning) == '') $colspaning = $current_span;
							
							// if the user left colspan blank, dont change anything
							if (trim($colspaning) == '') {
								$colspaning = $current_span; 
								$bodycopy_saved[$prefix][$asset->id]['rows'][$rowid]['cells'][$cellid]['attributes']['colspan'] = $current_span;
							}
							
							// if the user entered a negative colspan - or '0', we can assume they want a colspan of '1'
							if (intval($colspaning) <= 0) { 
								$colspaning = 1; 
								$bodycopy_saved[$prefix][$asset->id]['rows'][$rowid]['cells'][$cellid]['attributes']['colspan'] = 1;
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
					}

					// doing the colspan thing

					if (count($colspan_todo)) {

						foreach ($colspan_todo as $colspan) {

							$cellid = $colspan['cell'];
							$colspaning = $colspan['colspaning'];
							$current_span = $colspan['current_span'];

							// if the current span is '1' and the new span is '' then a cell is being created
							// so we dont want to do anything
							if ($current_span == 1 && trim($colspaning) == '') $colspaning = $current_span;
							
							// if the user left colspan blank, dont change anything
							if (trim($colspaning) == '') {
								$colspaning = $current_span; 
								$bodycopy_saved[$prefix][$asset->id]['rows'][$rowid]['cells'][$cellid]['attributes']['colspan'] = $current_span;
							}
							
							// if the user entered a negative colspan - or '0', we can assume they want a colspan of '1'
							if (intval($colspaning) <= 0) {
								$colspaning = 1; 
								$bodycopy_saved[$prefix][$asset->id]['rows'][$rowid]['cells'][$cellid]['attributes']['colspan'] = 1;
							}
					
							if ($colspaning != $current_span) {

								// make sure there are sufficient cells to create the colspan
								$spanable_cells = count($rows[$rowid]['cells']) - ($cellid +1);
								$needed_cells = ($colspaning - $current_span);
							
								if ($spanable_cells < $needed_cells) {
									// dont end up changing the colspan at all
									$bodycopy_saved[$prefix][$asset->id]['rows'][$rowid]['cells'][$cellid]['attributes']['colspan'] = $current_span;
								} else {
									if ($colspaning > $current_span && $spanable_cells >= 1) {
										// check for more colspans first
										$colspan_error = 0;
										for($i = ($cellid + 1); $i <= ($cellid + ($colspaning - $current_span)); $i++) {
											if ($rows[$rowid]['cells'][$i]->colspan() > 1) $colspan_error = 1;
										}
										if ($colspan_error == 0) {
											for($i = ($cellid + 1); $i <= ($cellid + ($colspaning - $current_span)); $i++) {
												if ($this->deleteCell($asset, $rows, $rowid, ($cellid+1), true)) {
													$rows_updated = true;
												} else {
													// deleteCell failed
													// dont end up changing the colspan at al
													$bodycopy_saved[$prefix][$asset->id]['rows'][$rowid]['cells'][$cellid]['attributes']['colspan'] = $current_span;
												}
											}
										} else {
											// we are trying to colspan into another colspan
											// dont end up changing the colspan at all
											$bodycopy_saved[$prefix][$asset->id]['rows'][$rowid]['cells'][$cellid]['attributes']['colspan'] = $current_span;
										}
									} else if ($colspaning < $current_span) {
										// decreasing the colspan
										for($i = 0; $i < ($current_span - $colspaning); $i++) {
											if ($this->insertCell($rows, $rowid, $cellid, false, true)) {
												$rows_updated = true;
											} else {
												// insertCell failed
												// dont end up changing the colspan at al
												$bodycopy_saved[$prefix][$asset->id]['rows'][$rowid]['cells'][$cellid]['attributes']['colspan'] = $current_span;
											}
										}
									}
								}//end else

							}//end if

						}//end foreach

					}//end if
				}
			}

			// now update the cells
			for($i = 0; $i < count($rows[$rowid]['cells']); $i++) {
				if ($rows[$rowid]['cells'][$i]->processCell($asset, $prefix, $asset->id, $rowid, $i)) $rows_updated = true;
			}

			// only set the rows attributes if any of the rows (or cells within) have changed
			if ($rows_updated) {
				$asset->setAttrValue('rows',$rows);
				$updated = true;
			}

		}


		// process any special actions if we are the  table that submitted the form
		if (!empty($bodycopy_data) && $bodycopy_data['tableid'] == $asset->id && $bodycopy_name == $prefix) {
			switch($bodycopy_action) {
				
				case 'insert_table_row' :
					if ($this->insertRow($asset, $bodycopy_data['rowid'], $bodycopy_data['before'])) $updated = true;
				break;

				case 'delete_table_row' :
					if ($this->deleteRow($asset, $bodycopy_data['rowid'])) $updated = true;
				break;

				case 'edit_table_row_order' :
					if ($this->reorderRows($asset, $bodycopy_data['row_order'])) $updated = true;
				break;

				case 'insert_table_column' :
					if ($this->insertColumn($asset, $bodycopy_data['colid'], $bodycopy_data['before'])) $updated = true;
				break;

				case 'delete_table_column' :
					if ($this->deleteColumn($asset, $bodycopy_data['colid'])) $updated = true;
				break;

				case 'edit_table_col_order' :
					if ($this->reorderColumns($asset, $bodycopy_data['col_order'])) $updated = true;
				break;

			}
		}

		if ($updated) $this->generateContentFile($asset);
	
		return $updated;

	}//end processTable()


	/**
	* Creates a PHP file to print on the frontend instead of doing processing there
	*
	* @param object Bodycopy_Table	&$asset	the table
	*
	* @return boolean
	* @access public
	*/
	function generateContentFile(&$asset) {
		ob_start();
		$this->paint($asset);
		$output = ob_get_contents();
		ob_end_clean();
		create_directory($asset->data_path);
		if (string_to_file($output, $asset->data_path.'/content_file.inc')) return true;
		return false;
	}


	/**
	* Insert a new table row before or after the passed rowid
	*
	* @param object Asset	&$asset	the table object
	* @param int			$rowid	the id of the row that we are inserting before/after
	* @param boolean		$before	insert before the passed rowid?
	*
	* @return boolean
	* @access public
	*/
	function insertRow(&$asset, $rowid, $before=true)
	{
		$rows = $asset->attr('rows');

		$rowid = intval($rowid);

		// if the row is greater than the number of rows we have set it to the last on
		if ($rowid > count($rows) - 1) $rowid = count($rows) - 1;
		if ($rowid < 0) $rowid = 0;
		$compareid = $rowid;

		// for the moment we will just use the number of columns that the one we are inserting
		// before/after has
		$num_cols = $this->maxCols($rows[$compareid], false);

		// if they want to insert after the passed rowid what 
		// they really mean is to insert before the next rowid
		if (!$before) $rowid++;

		// move all the rows up one position
		for($i = count($rows); $i > $rowid; $i--) {
			$rows[$i] = $rows[$i - 1];
			for($x = 0; $x < count($rows[$i]['cells']); $x++) {
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
		for($i = 0; $i < count($rows[$compareid]['cells']); $i++) {
			$rows[$rowid]['cells'][$i] = new Bodycopy_Table_Cell();
			foreach ($rows[$compareid]['cells'][$i]->attributes as $name => $val) {
				$rows[$rowid]['cells'][$i]->set_attribute($name,$val);
			}
			$rows[$rowid]['cells'][$i]->setContentType($asset, 'wysiwyg', $rowid, $i);
		}

		$asset->setAttrValue('rows',$rows);
		return true;

	}//end insertRow()


	/**
	* Delete the passed table row from the array
	*
	* @param object Asset	&$asset	the table object
	* @param int			$rowid	the id of the row to delete
	*
	* @return boolean
	* @access public
	*/
	function deleteRow(&$asset, $rowid)
	{
		$rows = $asset->attr('rows');

		// make sure it exists
		if (isset($rows[$rowid]) && count($rows) > 1) {

			// delete all the cells in the row so they can clean up
			for($i = 0; $i < count($rows[$rowid]['cells']); $i++) {
				$rows[$rowid]['cells'][$i]->delete($asset);
			}

			// move all the rows down one position
			for($i = $rowid; $i < count($rows) - 1; $i++) {
				// let the cells know that they are moving to a new rowid
				$rows[$i] = $rows[$i + 1];
				for($x = 0; $x < count($rows[$i]['cells']); $x++) {
					$rows[$i]['cells'][$x]->updateContentType($asset, $i, $x);
				}
			}

			// remove the last element
			array_pop($rows);

			$asset->setAttrValue('rows',$rows);
			return true;

		} elseif (count($rows) == 1) {
			return false;
		}
	
		return false;

	}//end deleteRow()


	/**
	* Re-orders the table rows
	*
	* @param object Asset	&$asset	the table object
	* @param array			$row_order	an array of the structure (new_place => old_place)
	*
	* @return boolean
	* @access public
	*/
	function reorderRows(&$asset, $row_order)
	{
		$rows = $asset->attr('rows');

		// if there ain't no changes, die
		$changes = false;
		foreach($row_order as $new_place => $old_place) $changes |= ($new_place != $old_place);
		if (!$changes) return false;

		// if there is a conflict in the number of rows, die
		if (count($rows) != count(array_unique($row_order))) return false;

		$new_array = Array();

		for($i = 0; $i < count($row_order); $i++) {
			for($x = 0; $x < count($rows[$i]['cells']); $x++) {
				$rows[$i]['cells'][$x]->updateContentType($asset, $row_order[$i], $x);
			}
			$new_array[$i] = $rows[$row_order[$i]];
		}
		
		// now assign the new ordered array
		$rows = $new_array;

		$asset->setAttrValue('rows',$rows);
		return true;

	}//end reorderRows()


	/**
	* Re-orders the table columns
	*
	* @param object Asset	&$asset	the table object
	* @param array			$col_order	an array of the structure (new_place => old_place)
	*
	* @return boolean
	* @access public
	*/
	function reorderColumns(&$asset, $col_order)
	{
		// if there ain't no changes, die
		$changes = false;
		foreach($col_order as $new_place => $old_place) $changes |= ($new_place != $old_place);
		if (!$changes) return false;

		$rows = $asset->attr('rows');
		
		for($r = 0; $r < count($rows); $r++) {
			// if there is a conflict in the number of cells, die
			if (count($rows[$r]['cells']) != count(array_unique($col_order))) return false;

			$new_array = Array();
			for($i = 0; $i < count($col_order); $i++) {
				$rows[$r]['cells'][$i]->updateContentType($asset, $r, $col_order[$i]);
				$new_array[$i] = $rows[$r]['cells'][$col_order[$i]];
			}
			// now assign the new ordered array
			$rows[$r]['cells'] = $new_array;
		}
	
		$asset->setAttrValue('rows',$rows);
		return true;

	}//end reorderColumns()


	/**
	* Insert a new column in all rows, before or after the passed columnid
	*
	* @param object BodyCopy_Table	&$asset	the table object
	* @param int					$cellid	the id of the column that we are inserting before/after
	* @param boolean				$before	insert before the passed cellid?
	*
	* @return boolean
	* @access public
	*/
	function insertColumn(&$asset, $cellid, $before=true)
	{
		$rows = $asset->attr('rows');

		// we dont know how to do this ourselves, so just get
		// every row to insert a new cell and it can handle checking
		for($r = 0; $r < count($rows); $r++) {
			if (!$this->insertCell($asset, $rows, $r, $cellid, $before)) return false;
		}

		$asset->setAttrValue('rows',$rows);
		return true;

	}//end insertColumn()


	/**
	* Deletes a column from all rows, before or after the passed columnid
	*
	* @param object BodyCopy_Table	&$asset	the table object
	* @param int					$cellid	the id of the column that we are deleting
	*
	* @return boolean
	* @access public
	*/
	function deleteColumn(&$asset, $cellid)
	{
		$rows = $asset->attr('rows');

		// we dont know how to do this ourselves, so just get
		// every row to delete the cell and it can handle checking
		for($r = 0; $r < count($rows); $r++) {
			if (!$this->deleteCell($asset, $rows, $r, $cellid)) return false;
		}

		$asset->setAttrValue('rows',$rows);
		return true;

	}//end deleteColumn()


	/**
	* Insert a new table cell before or after the passed cellid
	*
	* @param array		&$rows		a reference to the rows attribute of the table
	* @param int		$rowid		the id of the row we are inserting into
	* @param int		$cellid		the id of the cell we are inserting before/after
	* @param boolean	$before		insert the cell before the passed cellid?
	* @param boolean	$colspan	ignore colspans in this row
	*
	* @return boolean
	* @access public
	*/
	function insertCell(&$asset, &$rows, $rowid, $cellid, $before=true, $colspan=false)
	{
		if (!$colspan) {

			// check for colspan's in this row - colspan's will muck things up
			$colspan_error = 0;
			$colspaning = 0;
			for($i = 0; $i < count($rows[$rowid]['cells']); $i++) {
				if ($rows[$rowid]['cells'][$i]->colspan() > 1) { 
					$colspan_error = 1;
				}
			}
			if ($colspan_error == 1) {
				// work out the correct cellid for the id passed in
				$real_cellid = 0;
				for($i = 0; $i < count($rows[$rowid]['cells']); $i++) {
					if ($colspaning >= $cellid) break;
					$colspaning += intval($rows[$rowid]['cells'][$i]->colspan());
					if (intval($rows[$rowid]['cells'][$i]->colspan()) == 0) $colspaning++;
					if ($colspaning <= $cellid) {
						// havn't found the cell yet
						$real_cellid++;
					}
				}
				
				// lets try and find out how many cells are before the cell we are looking at
				$colspaning = 0;
				for($i = 0; $i < $real_cellid; $i++) {
					$colspaning += intval($rows[$rowid]['cells'][$i]->colspan());
					if (intval($rows[$rowid]['cells'][$i]->colspan()) == 0) $colspaning++;
				}

				if ($real_cellid < 0) $real_cellid = 0;

				// now we have the real cellid, so we will be increasing the colspan of this cell
				// (if it has one) unless we are supposed to be inserting AFTER this cell or BEFORE
				// this cell
				// so lets check shall we?

				$dont_do_this = false;

				// if the real cellid is found to be the same as the cellid, we have a special
				// situation where we have found the first cell in a colspan that has no
				// colspans before it on the row
				if ($cellid == $real_cellid) {
					if ($before) $dont_do_this = true;
				} else {
					if ((($rows[$rowid]['cells'][$real_cellid]->colspan() + $colspaning) == ($cellid + 1)) && !$before) $dont_do_this = true;
				}

				if ($cellid == 0 && $before) $dont_do_this = true;
					
				$cellid = $real_cellid;
				if ($cellid < 0) $cellid = 0;

				// if this cell has colspan's already, we can just increase the colspan by 1
				if ($rows[$rowid]['cells'][$cellid]->colspan() > 1 && !$dont_do_this) {
					$colspaning = $rows[$rowid]['cells'][$cellid]->colspan() + 1;
					$rows[$rowid]['cells'][$cellid]->set_attribute('colspan', $colspaning);
					return true;
				}
				
			}
		}
	
		// if the cell is greater than the number of cells we have set it to the last on
		if ($cellid > count($rows[$rowid]['cells']) - 1) $cellid = count($rows[$rowid]['cells']) - 1;
		if ($cellid < 0) $cellid = 0;
		$compareid = $cellid;

		// if they want to insert after the passed cell what 
		// they really mean is to insert before the next cell
		if (!$before) $cellid++;

		// move all the cells up one position
		for($i = count($rows[$rowid]['cells']); $i > $cellid; $i--) {
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
			$rows[$rowid]['cells'][$cellid]->set_attribute($name,$val);
		}
		$rows[$rowid]['cells'][$cellid]->setContentType($asset, 'wysiwyg', $rowid, $cellid);

		return true;

	}//end insertCell()


	/**
	* Delete the passed table cell from the array
	*
	* @param object BodyCopy_Table	&$asset	the table object
	* @param array		&$rows		a reference to the rows attribute of the table
	* @param int		$rowid		the id of the row we are deleting from
	* @param int		$cellid		the id of the cell we are deleting
	* @param boolean	$colspan	ignore colspans in this row
	*
	* @return boolean
	* @access public
	*/
	function deleteCell(&$table, &$rows, $rowid, $cellid, $colspan=false)
	{
		if (!$colspan) {

			// check for colspan's in this row - colspan's will muck things up
			$colspan_error = 0;
			$colspaning = 0;
			for($i = 0; $i < count($rows[$rowid]['cells']); $i++) {
				if ($rows[$rowid]['cells'][$i]->colspan() > 1) {
					$colspan_error = 1;
				}
			}

			if ($colspan_error == 1) {
				// work out the correct cellid for the id passed in
				$real_cellid = 0;
				for($i = 0; $i < count($rows[$rowid]['cells']); $i++) {
					if ($colspaning >= $cellid) break;
					$colspaning += intval($rows[$rowid]['cells'][$i]->colspan());
					if (intval($rows[$rowid]['cells'][$i]->colspan()) == 0) $colspaning++;
					if ($colspaning <= $cellid) {
						// havn't found the cell yet
						$real_cellid++;
					}
				}

				$cellid = $real_cellid;

				// if this cell has colspan's already, we can just reduce the colspan by 1
				if ($rows[$rowid]['cells'][$cellid]->colspan() > 1) {
					$colspaning = $rows[$rowid]['cells'][$cellid]->colspan() - 1;
					$rows[$rowid]['cells'][$cellid]->set_attribute('colspan', $colspaning);
					return true;
				}
			}
		}

		// make sure cell exists
		if ($rows[$rowid]['cells'][$cellid] && count($rows[$rowid]['cells']) > 1) {
			// do any clean up
			$rows[$rowid]['cells'][$cellid]->delete($table, $rowid, $cellid);

			// move all the cells down one position
			for($i = $cellid; $i < count($rows[$rowid]['cells']) - 1; $i++) {
				$rows[$rowid]['cells'][$i] = $rows[$rowid]['cells'][$i + 1];
				$rows[$rowid]['cells'][$i]->updateContentType($table, $rowid, $i);
			}

			// remove the last element
			array_pop($rows[$rowid]['cells']);
			return true;

		} elseif (count($this->cells) == 1) {
			return false;
		}

	}//end deleteCell()


	/**
	* Gets the maximum number of columns out of all the rows in this table
	*
	* @param array		$table_rows		the rows attribute of the table
	* @param boolean	$add_colspans	add colspan value while counting
	*
	* @return integer
	* @access public
	*/
	function maxCols($table_rows, $add_colspans=true)
	{
		$max = 0;
		for($i = 0; $i < count($table_rows); $i++) {
			$num = 0;
			if (empty($table_rows[$i]['cells'])) continue;
			for($x = 0; $x < count($table_rows[$i]['cells']); $x++) {
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
