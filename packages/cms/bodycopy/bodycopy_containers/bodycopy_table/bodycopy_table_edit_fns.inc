<?php
/**
* +--------------------------------------------------------------------+
* | MySource 3 - MySource Matrix                                       |
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
* $Id: bodycopy_table_edit_fns.inc,v 1.3 2003/11/18 15:43:34 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_PACKAGES_PATH.'/cms/bodycopy/bodycopy_container/bodycopy_container_edit_fns.inc';

/**
* Bodycopy_Table_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Bodycopy_Table_Edit_Fns extends Bodycopy_Container_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function Bodycopy_Table_Edit_Fns()
	{
		$this->Bodycopy_Container_Edit_Fns();

	}//end constructor


	/**
	* Prints the JavaScript needed by the backend functions for this table
	*
	* @param object Bodycopy_Table	&$asset	the table object being printed
	* @param string					$prefix	prefix for form fields
	*
	* @return void
	* @access public
	*/
	function paintBackendJsData(&$asset, $prefix)
	{
		$table_rows = $asset->attr('rows');
		$table_attributes = $asset->attr('attributes');
		$table_attributes['identifier'] = $asset->name;
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
											if (isset($table_rows[$i]['cells'][$x])) $table_rows[$i]['cells'][$x]->printBackendJsData($x);
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
	* Paint JS data that tables need to use to do their processing
	*
	* @param object Bodycopy			&$bodycopy	the bodycopy being printed
	* @param object	Backend_Outputter	&$o			the outputter class
	* @param string						$prefix		prefix for the form element
	*
	* @return void
	* @access public
	*/
	function paintGenericBackend(&$bodycopy, &$o, $prefix)
	{
		// get all the containers linked to the bodycopy
		$containers = $GLOBALS['SQ_SYSTEM']->am->getLinks($bodycopy->id, SQ_LINK_TYPE_2, 'bodycopy_container', false);

		?>
		<script language="JavaScript" src="<?php echo sq_web_path('data').'/asset_types/bodycopy/js/bodycopy_edit_tables.js'?>"></script>

		<script language="JavaScript" type="text/javascript">
		function serialise_table(bodycopy_name, bodycopy_data, tableid, rowid, cellid) {
			var form = document.main_form;
			if (!bodycopy_saved[bodycopy_name] && bodycopy_name != null) bodycopy_saved[bodycopy_name] = new Object();

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
					}// end if cellid

				} else {
					// no rowid means that we are settign the table attributes
					bodycopy_saved[bodycopy_name][tableid]['attributes'] = bodycopy_data['attributes'];
				}//end if rowid

			}// end if tableid

			form.elements['bodycopy_saved[' + bodycopy_name + '][' + tableid + ']'].value = var_serialise(bodycopy_saved[bodycopy_name][tableid]);
		}// end serialise_table()


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
				if ($container_data['minor_type_code'] != 'bodycopy_table') continue;
				$table = &$GLOBALS['SQ_SYSTEM']->am->getAsset($container_data['minorid'], $container_data['minor_type_code']);
				$table_edit = $table->getEditFns();
				$table_edit->paintBackendJsData($table, $prefix, $i);
				$i++;
			}
		}
		?>
				default :
					retVal["num_containers"] = <?php echo count($containers); ?>;

			}// end switch tableid


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
		}// end get_bodycopy_<?php echo $prefix?>_current_table_data()


		// set reference so generic fn can be called above
		bodycopy_current_data["<?php echo $prefix?>"].get_table_data = get_bodycopy_<?php echo $prefix?>_current_table_data;
		</script>

		<?php

	}//end paintGenericBackend()


	/**
	* Paints this table as HTML
	*
	* @param object Bodycopy_Table	&$asset		the table whose interface we are painting
	* @param booelan				$generating	are we generating the content file
	*
	* @return void
	* @access public
	*/
	function paint(&$asset, $generating=false)
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

		echo "\n<table{$attribute_list}>\n";

		for($i = 0; $i < count($table_rows); $i++) {
			$attribute_list = '';
			foreach($table_rows[$i]['attributes'] as $name => $val) {
				if ($val == '') continue;
				$attribute_list .= ' '.$name.'="'.str_replace('"', '&quot;', $val).'"';
			}

			echo "\t<tr{$attribute_list}>\n";
			// now print the cell contents
			for($x = 0; $x < count($table_rows[$i]['cells']); $x++) {
				$table_rows[$i]['cells'][$x]->paint($asset, '', $generating);
			}
			echo "\t</tr>\n";
		}

		echo "</table>\n";

		return true;

	}//end paint()


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
	function paintContainer(&$asset, &$o, $prefix)
	{
		if (!parent::paintContainer($asset, $o, $prefix)) return false;
		
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
		<table<?php echo $attribute_list?> background="<?php echo $bgimage?>" border="0" cellpadding="0" cellspacing="0" style="width:100%;">
			<tr> 
				<td align="center" background="" id="<?php echo $prefix.'_table_'.$asset->id;?>" width="4%" style="border-top: 1px dashed #C0C0C0; border-right: 1px dashed #C0C0C0; border-left: 1px dashed #C0C0C0;">
				<?php
				print_bodycopy_icon("Javascript: bodycopy_edit_table_properties('$prefix', $asset->id);", 'Edit '.$asset->attr('name').'\'s Properties', (($popup_attribute_list) ? 'Current Properties :<br>'.$popup_attribute_list : ''), 'table_properties');
				?>
				</td>
		<?php
			for($i = 0; $i < $max_cols; $i++) {
			?>
				<td align="center" background="" valign="middle" style="border-top: 1px dashed #C0C0C0; border-right: 1px dashed #C0C0C0; border-bottom: 1px dashed #C0C0C0;" id="<?php echo $prefix.'_cell_'.$asset->id.'_0_'.$i; ?>">
					<table border="0" cellspacing="0" cellpadding="0" width="100%">
						<tr>
							<td align="left" nowrap>
							<?php
							print_bodycopy_icon("Javascript: bodycopy_insert_table_col('$prefix', $asset->id, $i, true);", 'Insert New Column on Left', '', 'add');
							?>
							</td>
							<td nowrap>
							<?php
							$table_rows[0]['cells'][$i]->paintCell($asset, $prefix, $asset->id, 0, $i);
							?>
							</td>
							<td width="100%" align="right" nowrap>
							<?php
							if ($i == 0) {
								if ($max_cols > 1 && (!$colspans_in_row || count($table_rows) == 1)) {
									print_bodycopy_icon("Javascript: bodycopy_edit_table_col_order('$prefix', $asset->id);", 'Reorder '.$asset->attr('name').'\'s Columns ?', '', 'reorder');
								}
							}
							if ($max_cols > 1) {
								print_bodycopy_icon("Javascript: bodycopy_delete_table_col('$prefix', $asset->id, $i);", 'Delete Column '.($i + 1).' ?', '', 'delete');
							}
							?>
							</td>
							<td align="right" nowrap>
							<?php
							print_bodycopy_icon("Javascript: bodycopy_insert_table_col('$prefix', $asset->id, $i, false);", 'Insert New Column on Right', '', 'add');
							?>
							</td>
						</tr>
					</table>
				</td>
			<?php
				if ($i < ($max_cols-1)) {
					?><td style="border-right: 1px dashed #C0C0C0" width="1%"><img src="<?php echo sq_web_path('lib'); ?>/web/images/blank.gif" width="10" height="1"></td><?php
				}
			}
			?>
			</tr>
		<?php

		$num_rows = count($table_rows);
		for($i = 0; $i < $num_rows; $i++) {
			if ($i > 0) {
				?>
				<tr>
					<td style="border-left: 1px dashed #C0C0C0; border-right: 1px dashed #C0C0C0; border-bottom: 1px dashed #C0C0C0;">&nbsp;</td>
					<?php
					for($x = 0; $x < count($table_rows[$i]['cells']); $x++) {
						?>
						<td style="border-right: 1px dashed #C0C0C0; border-bottom: 1px dashed #C0C0C0;" id="<?php echo $prefix.'_cell_'.$asset->id.'_'.$i.'_'.$x; ?>">
							<table border="0" cellspacing="0" cellpadding="0" width="100%">
								<tr>
									<td align="left" nowrap>
									<?php
									print_bodycopy_icon("Javascript: bodycopy_insert_table_col('$prefix', $asset->id, $x, true);", 'Insert New Column on Left', '', 'add');
									?>
									</td>
									<td nowrap>
									<?php
									$table_rows[$i]['cells'][$x]->paintCell($asset, $prefix, $asset->id, $i, $x);
									?>
									</td>
									<td align="right" width="100%" nowrap>
									<?php
									print_bodycopy_icon("Javascript: bodycopy_insert_table_col('$prefix', $asset->id, $x, false);", 'Insert New Column on Right', '', 'add');
									?>
									</td>
								</tr>
							</table>
						</td>
						<?php
						if ($x < count($table_rows[$i]['cells'])-1) {
							?><td style="border-right: 1px dashed #C0C0C0" width="1%"><img src="<?php echo sq_web_path('lib'); ?>/web/images/blank.gif" width="10" height="1"></td><?php
						}
					}//end for (each cell)
					?>
				</tr>
				<?php
			}


			$attribute_list = '';
			foreach($table_rows[$i]['attributes'] as $name => $val) {
				if ($val == '') continue;
				$attribute_list .= ' '.$name.'="'.str_replace('"', '&quot;', $val).'"';
			}
			?>

			<tr<?php echo $attribute_list?>>
				<td align="center" background="" id="<?php echo $prefix.'_row_'.$asset->id.'_'.$i?>" height="100%" style="border-left: 1px dashed #C0C0C0; border-right: 1px dashed #C0C0C0; border-bottom: 1px dashed #C0C0C0;" >
					<table border="0" cellspacing="0" cellpadding="0" height="100%">
						<?php
						if ($i == 0) {
							# can't reorder if only one row
							if ($num_rows > 1) {
								?>
								<tr height="1%">
									<td valign="top" nowrap>
										<?php
										print_bodycopy_icon("Javascript: bodycopy_edit_table_row_order('$prefix', $asset->id);", 'Reorder '.$asset->attr('name').'\'s Rows ?', '', 'reorder');
										?>
									</td>
								</tr>
								<?php
							}
						}
						?>
						<tr height="1%">
							<td valign="top">
							<?php
							print_bodycopy_icon("Javascript: bodycopy_insert_table_row('$prefix', $asset->id, $i, true);", 'Insert a New Row Above', '', 'add');
							?>
							</td>
						</tr>
						<tr height="100%">
							<td valign="middle" nowrap>
							<?php
							print_bodycopy_icon("Javascript: bodycopy_edit_table_row_properties('$prefix', $asset->id, $i);", 'Edit Row '.($i + 1).'\'s Properties', (($attribute_list) ? 'Current Properties :<br>'.$attribute_list : ''), 'row_properties');
							if ($num_rows > 1) {
								echo '<br>';
								print_bodycopy_icon("Javascript: bodycopy_delete_table_row('$prefix', $asset->id, $i);", 'Delete Row '.($i + 1).' ?', '', 'delete');
							}
							?>
							</td>
						</tr>
						<tr height="1%">
							<td valign="bottom" nowrap>
							<?php
							print_bodycopy_icon("Javascript: bodycopy_insert_table_row('$prefix', $asset->id, $i, false);", 'Insert a New Row Below', '', 'add'); 
							?>
							</td>
						</tr>
					</table>
				</td>
			<?php
				// now print the cell contents
				for($x = 0; $x < count($table_rows[$i]['cells']); $x++) {
					$table_rows[$i]['cells'][$x]->paint($asset, $prefix);
					if ($x < count($table_rows[$i]['cells'])-1) {
						?><td style="border-right: 1px dashed #C0C0C0" width="1%"><img src="<?php echo sq_web_path('lib'); ?>/web/images/blank.gif" width="10" height="1"></td><?php
					}
				}
			?>
			</tr>
			<?php
		}//end for

		?>
		</table>
		<?php

		return true;

	}//end paintContainer()


	/**
	* Process the interface for editing this table
	*
	* @param object BodyCopy_Table		&$asset		the table whose interface we are painting
	* @param object	Backend_Outputter	$o			the outputter class
	* @param string						$prefix		prefix for the form element
	* @param string						$bc_action	a global bodycopy action to perform (eg insert_table_row)
	* @param string						$bc_name	the name of the bodycopy that submitted the action
	* @param array						$bc_data	an array of data needed to perform the action
	* @param array						$bc_saved	any saved changes that were performed without committing
	*
	* @return boolean
	* @access public
	*/
	function processContainer(&$asset, &$o, $prefix, $bc_action, $bc_name, $bc_data, $bc_saved)
	{
		$updated = parent::processContainer($asset, $o, $prefix, $bc_action, $bc_name, $bc_data, $bc_saved);

		// update the rows
		$rows_updated = false;
		$rows = $asset->attr('rows');
		for($rowid = 0; $rowid < count($rows); $rowid++) {
			if (!empty($bc_saved[$prefix][$asset->id]['rows'][$rowid]['attributes'])) {
				// save any changed row attributes
				foreach($bc_saved[$prefix][$asset->id]['rows'][$rowid]['attributes'] as $name => $value) {
					$name = strtolower($name);
					if (!isset($rows[$rowid]['attributes'][$name]) || $rows[$rowid]['attributes'][$name] != $value) {
						$rows[$rowid]['attributes'][$name] = $value;
						$rows_updated = true;
					}
				}
			}

			// only update if we are the table row that submitted the form
			if ((!empty($bc_data['rowid']) && $bc_data['rowid'] == $rowid && $prefix == $bc_name) || isset($bc_saved[$prefix][$asset->id]['rows'][$rowid])) {

				$counter = 0;
				$colspan_todo = Array();

				// check to see if any cells from this row need their colspans changed
				if (isset($bc_saved[$prefix][$asset->id]['rows'][$rowid]['cells'])) {
					foreach($bc_saved[$prefix][$asset->id]['rows'][$rowid]['cells'] as $cellid => $cell_data) {

						if (isset($rows[$rowid]['cells'][$cellid])) {

							$current_span = $rows[$rowid]['cells'][$cellid]->colspan();
							$colspaning   = $cell_data['attributes']['colspan'];
								
							// if the current span is '1' and the new span is '' then a cell is being created
							// so we dont want to do anything
							if ($current_span == 1 && trim($colspaning) == '') $colspaning = $current_span;
							
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
										for($i = 0; $i < ($current_span - $colspaning); $i++) {
											if ($this->insertCell($rows, $rowid, $cellid, false, true)) {
												$rows_updated = true;
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
			for($i = 0; $i < count($rows[$rowid]['cells']); $i++) {
				if ($rows[$rowid]['cells'][$i]->processCell($asset, $prefix, $asset->id, $rowid, $i, $bc_saved)) $rows_updated = true;
			}

			// only set the rows attributes if any of the rows (or cells within) have changed
			if ($rows_updated) {
				$asset->setAttrValue('rows',$rows);
				$updated = true;
			}

		}//end for all rows


		// check if there is anything for this table to do
		$skip_table_editing = false;
		if (empty($bc_data) || !isset($bc_data['tableid']) || $bc_data['tableid'] != $asset->id) {
			// no bodycopy data stuff relating to this table, so check the saved data
			if (empty($bc_saved[$asset->id])) {
				// there is nothing for us here
				$skip_table_editing = true;
			}
		}


		// process any special actions if we are the  table that submitted the form
		if (!$skip_table_editing && !empty($bc_data) && $bc_data['tableid'] == $asset->id && $bc_name == $prefix) {
			switch($bc_action) {
				case 'insert_table_row' :
					if ($this->insertRow($asset, $bc_data['rowid'], $bc_data['before'])) $updated = true;
				break;

				case 'delete_table_row' :
					if ($this->deleteRow($asset, $bc_data['rowid'])) $updated = true;
				break;

				case 'edit_table_row_order' :
					if ($this->reorderRows($asset, $bc_data['row_order'])) $updated = true;
				break;

				case 'insert_table_column' :
					if ($this->insertColumn($asset, $bc_data['colid'], $bc_data['before'])) $updated = true;
				break;

				case 'delete_table_column' :
					if ($this->deleteColumn($asset, $bc_data['colid'])) $updated = true;
				break;

				case 'edit_table_col_order' :
					if ($this->reorderColumns($asset, $bc_data['col_order'])) $updated = true;
				break;
			}
		}

		if ($updated) $this->generateContentFile($asset);
		$this->_contentsUpdated($asset);
		return $updated;

	}//end processContainer()


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
				$rows[$rowid]['cells'][$i]->setAttribute($name,$val);
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
	* Reorder the rows of this table
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
	* Reorder the columns of this table
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
	* Insert a new column in all rows, before or after the passed columnID
	*
	* @param object BodyCopy_Table	&$asset	the table object
	* @param int					$cellid	the ID of the column that we are inserting before/after
	* @param boolean				$before	insert before the passed cellID?
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
	* Deletes a column from all rows, before or after the passed columnID
	*
	* @param object BodyCopy_Table	&$asset	the table object
	* @param int					$cellid	the ID of the column that we are deleting
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
	* Insert a new table cell before or after the passed cellID
	*
	* @param array		&$rows		a reference to the rows attribute of the table
	* @param int		$rowid		the ID of the row we are inserting into
	* @param int		$cellid		the ID of the cell we are inserting before/after
	* @param boolean	$before		insert the cell before the passed cellID?
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
					$rows[$rowid]['cells'][$cellid]->setAttribute('colspan', $colspaning);
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
			$rows[$rowid]['cells'][$cellid]->setAttribute($name,$val);
		}
		$rows[$rowid]['cells'][$cellid]->setContentType($asset, 'wysiwyg', $rowid, $cellid);

		return true;

	}//end insertCell()


	/**
	* Delete the passed table cell from a row
	*
	* @param object BodyCopy_Table	&$asset	the table object
	* @param array		&$rows		a reference to the rows attribute of the table
	* @param int		$rowid		the ID of the row we are deleting from
	* @param int		$cellid		the ID of the cell we are deleting
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
					$rows[$rowid]['cells'][$cellid]->setAttribute('colspan', $colspaning);
					return true;
				}
			}
		}

		// make sure cell exists
		if ($rows[$rowid]['cells'][$cellid] && count($rows[$rowid]['cells']) > 1) {
			// do any clean up
			if (!$rows[$rowid]['cells'][$cellid]->delete($table, $rowid, $cellid)) return false;

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